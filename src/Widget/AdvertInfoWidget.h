#pragma once

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>
#include <QtWidgets/QMenu>
#include "ui_AdvertInfoWidget.h"

#include "Utils.h"

class AdvertInfoWidget : public QWidget
{
	Q_OBJECT
public:
	AdvertInfoWidget(QWidget *parent = Q_NULLPTR);
	~AdvertInfoWidget();
	//
	void setRemainTime(qint64 time);
Q_SIGNALS:
	void tryCloseAdvert();
	void tryMuteVolume(bool isMute);
protected:
	void mousePressEvent(QMouseEvent *event);
protected Q_SLOTS:
	void on_closeButton_clicked();
	void on_soundButton_clicked(bool isChecked);
	void on_detailButton_clicked();
private:
	Ui::AdvertInfoWidget ui;
};