#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <QtWidgets/QMenu>
#include "ui_DanmakuMenu.h"

class DanmakuMenu : public QMenu
{
	Q_OBJECT
public:
	DanmakuMenu(QWidget *parent = Q_NULLPTR);
	~DanmakuMenu();
	bool isSmallFont();
	QColor selectColor();
	void resetSetting();
Q_SIGNALS:
	void valueChanged(int value);
protected:
	void showEvent(QShowEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);
protected Q_SLOTS:
	void on_resetButton_clicked();
private:
	Ui::DanmakuMenu ui;
};