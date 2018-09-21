#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <QtCore/QtCore>

class Logger:public QObject
{
	Q_OBJECT
private:
    Logger();
    ~Logger();
public:
    static Logger& instance();
    void setLogPath(QString path = "");
	//
	void onMessage(QtMsgType type,  
		QMessageLogContext const& context,  
		QString const& msg);
private:
	QDir m_logDir;
	QFile m_logFile;
	QMutex m_logMutex;
	QTimer m_flushTimer;
	QTextStream m_textStream;
	//
	QMap<int, QString> m_mapMessageType;
};
//
#define LoggerInstance Logger::instance()
//
#endif // LOGGER_H