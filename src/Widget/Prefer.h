#pragma once

#include <QtWidgets/QtWidgets>
#include <QtNetwork/QtNetwork>

namespace UI
{
	class Prefer :public QDialog
	{
		Q_OBJECT
	public:
		Prefer(QWidget *parent = 0);
		void setupPlay();
		void setupInterface();
		void setupShortcut();
		//
		void saveSettings();
	private:
		QTabWidget *tab;

		QWidget* widgetPlaying;
		QWidget* widgetInterface;
		QWidget* widgetShortcut;

		//Playing
		QLineEdit *factor;
		QCheckBox *bold;
		QComboBox *dmfont;
		QComboBox *effect;
		//QLineEdit *play[2];

		//Interface
		QComboBox *font;
		QComboBox *reop;
		QCheckBox *vers;
		QCheckBox *sens;
		QCheckBox *less;
		QCheckBox *upda;
		QComboBox *loca;
		QComboBox *stay;
		QLineEdit *jump;
		QLineEdit *size;
		QLineEdit *back;

		//Shortcut
		QTreeWidget *hotkey;

		//Shared
		QHash<QString, QVariant> restart;
		QHash<QString, QVariant> reparse;

		QHash<QString, QVariant> getRestart();
		QHash<QString, QVariant> getReparse();
		QString getLogo(QString name);
	};
}
