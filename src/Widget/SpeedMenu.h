#pragma once

#include <QWidget>
#include <QPointer>
#include <QMenu>
#include "ui_SpeedMenu.h"

class SpeedMenu : public QMenu
{

	Q_OBJECT
public:
	SpeedMenu(QWidget *parent = Q_NULLPTR);
	~SpeedMenu();
Q_SIGNALS:
	void valueChanged(int value);
	void selectText(QString text);
protected:
	void showEvent(QShowEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);
protected Q_SLOTS:
	void on_listWidget_itemClicked(QListWidgetItem *item);
private:
	Ui::SpeedMenu ui;
};