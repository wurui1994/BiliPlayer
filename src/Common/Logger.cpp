#include "Logger.h"
#include "Utils.h"
#include <QMetaObject>
// 
static void recvMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	LoggerInstance.onMessage(type,context,msg);
}

Logger::Logger()
{
	m_mapMessageType =
	{
		{QtDebugMsg,    "Debug"},
		{QtInfoMsg,     "Info "},
		{QtWarningMsg,  "Warn "},
		{QtCriticalMsg, "Error"},
		{QtFatalMsg,    "Fatal"},
	};
}

Logger::~Logger()
{
	m_logFile.flush();
	m_logFile.close();

	QMutexLocker locker(&m_logMutex);
	qInstallMessageHandler(0);
}
//
void Logger::onMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QMutexLocker locker(&m_logMutex);
	QString level = m_mapMessageType[type];
	//
	QByteArray localMsg = msg.toLocal8Bit();
	// 
	QString fileName = QFileInfo(context.file).fileName();
	//
	QString dateTime = QDateTime::currentDateTime().toString("hh:mm:ss,zzz");
	QString output = QString("[%1] [%2] (%3:%4): %6\n")
						 .arg(level)
						 .arg(dateTime)
						 .arg(fileName)
						 .arg(context.line)
						 .arg(msg);
	//
	m_textStream << output;
}

Logger& Logger::instance()
{
	static Logger logger;
	return logger;
}

void Logger::setLogPath(QString path)
{
	QMutexLocker locker(&m_logMutex);
	//
	m_logDir.setPath(path);
	// 
	if (!m_logDir.exists())
	{
		m_logDir.mkpath("."); // 
	}
	QString fileName = QDate::currentDate().toString("yyyy-MM-dd_log.txt");
	QString logPath = m_logDir.absoluteFilePath(fileName); //

	m_logFile.setFileName(logPath);
	if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
	{
		Utils::MessageShow(0, "Can't write log file",QSize(200,80),5000);
		return;
	}
	//
	m_textStream.setDevice(&m_logFile);
	m_textStream.setCodec("UTF-8");
	//
	m_textStream << QString().fill('-', 120);
	m_textStream << "\n";
	//
	qInstallMessageHandler(recvMessage); 
	//
	m_flushTimer.setInterval(100);
	m_flushTimer.start();
	connect(&m_flushTimer, &QTimer::timeout, [=]
	{
		//
		QMutexLocker locker(&m_logMutex);
		m_textStream.flush();
	});
}
