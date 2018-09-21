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
	static QHash<QString, QObject *> objects;
private:
	void setDefaultFont()
	{
		QString def = Utils::defaultFont();
		QFontInfo i(qApp->font());
		if (!QFontDatabase().families().contains(def))
		{
			def = i.family();
		}
		double p = i.pointSizeF();
		QFont f;
		f.setFamily(Setting::getValue("/Interface/Font/Family", def));
		f.setPointSizeF(Setting::getValue("/Interface/Font/Size", p));
		qApp->setFont(f);
	}

	void loadTranslator(QString transFile)
	{
		QTranslator* translator = new QTranslator(qApp);
		translator->load(transFile);
		qApp->installTranslator(translator);
	}

	void loadTranslator()
	{
		QString locale = Setting::getValue("/Interface/Locale", QLocale::system().name());
		QFileInfoList list;
		list += QDir("./locale/" + locale).entryInfoList();
		list += QFileInfo("./locale/" + locale + ".qm");
		locale.resize(2);
		list += QDir("./locale/" + locale).entryInfoList();
		list += QFileInfo("./locale/" + locale + ".qm");
		for (QFileInfo info : list)
		{
			if (!info.isFile())
			{
				continue;
			}
			QTranslator *trans = new QTranslator(qApp);
			if (trans->load(info.absoluteFilePath()))
			{
				qApp->installTranslator(trans);
			}
			else
			{
				delete trans;
			}
		}
	}

	void setToolTipBase()
	{
		QPalette tip = qApp->palette();
		tip.setColor(QPalette::Inactive, QPalette::ToolTipBase, Qt::white);
		qApp->setPalette(tip);
		QToolTip::setPalette(tip);
	}

public slots:
	void quit();
};
