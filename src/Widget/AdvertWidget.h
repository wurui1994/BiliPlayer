#pragma once

#include <QWidget>
#include <QPointer>
#include <QMenu>
#include "ui_AdvertWidget.h"

#include "Utils.h"

class AdvertWidget : public QWidget
{
	Q_OBJECT
public:
	AdvertWidget(QWidget *parent = Q_NULLPTR);
	~AdvertWidget();
	//
	void setPixmap(QPixmap const& pixmap);
	//
Q_SIGNALS:
	void tryCloseAdvert();
protected:
	void mousePressEvent(QMouseEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
protected Q_SLOTS:
	void on_closeButton_clicked();
private:
	Ui::AdvertWidget ui;
};