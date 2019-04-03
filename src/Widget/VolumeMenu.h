#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <QtWidgets/QMenu>
#include "ui_VolumeMenu.h"

class VolumeMenu : public QMenu
{

	Q_OBJECT
public:
	VolumeMenu(QWidget *parent = Q_NULLPTR);
	~VolumeMenu();
	void setValue(int value);
Q_SIGNALS:
	void valueChanged(int value);
protected:
	void showEvent(QShowEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);
protected Q_SLOTS:
	
private:
	Ui::VolumeMenu ui;
};