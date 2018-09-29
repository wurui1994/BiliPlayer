#pragma once

#include <QtWidgets/QtWidgets>

#include "Utils.h"
#include "Setting.h"

#define App static_cast<Application *>(qApp)

class Application :public QApplication
{
	Q_OBJECT
public:
	Application(int &argc, char **argv);
private:

	void loadTranslator(QString transFile)
	{
		QTranslator* translator = new QTranslator(qApp);
		translator->load(transFile);
		qApp->installTranslator(translator);
	}
public slots:
	void quit();
};
