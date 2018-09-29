#include "Common.h"
#include "Application.h"
#include "Setting.h"
#include "Utils.h"
#include "Danmaku.h"

#include "ARender.h"
#include "Window.h"
#include "Logger.h"
#include "Network.h"

Application::Application(int &argc, char **argv) :
QApplication(argc, argv)
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	//
	loadTranslator(":/lang/biliplayer_zh");
	//
#ifdef Q_OS_WIN
	setStyleSheet("QWidget{font-family:Microsoft YaHei;}");
#else
	setStyleSheet("QWidget{font-family:PingFang SC;}");
#endif
	//
	LoggerInstance.setLogPath(path);
	//
	qDebug() << "Program" 
		<< QFileInfo(applicationFilePath()).fileName() 
		<< "start at"
		<< QDateTime::currentDateTime().toString();
	//
	Setting::load();
	//
	Network::instance().initAll();
	//
	connect(this,SIGNAL(aboutToQuit()), this,SLOT(quit()));
}

void Application::quit()
{
	qDebug() << "Program"
		<< QFileInfo(applicationFilePath()).fileName()
		<< " quit at"
		<< QDateTime::currentDateTime().toString();
	//
	Setting::save();
	delete Danmaku::instance();
}

