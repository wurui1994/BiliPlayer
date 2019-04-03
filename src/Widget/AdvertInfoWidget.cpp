#include "AdvertInfoWidget.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtGui/QMouseEvent>

AdvertInfoWidget::AdvertInfoWidget(QWidget *parent)
	: QWidget(parent)
{
	//
	ui.setupUi(this);
	//
	setAcceptDrops(true);
	//
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground);
	//
}

AdvertInfoWidget::~AdvertInfoWidget()
{
	
}

void AdvertInfoWidget::setRemainTime(qint64 time)
{
	ui.remainLabel->setText(QString::number(time));
}

void AdvertInfoWidget::mousePressEvent(QMouseEvent * event)
{
	Q_UNUSED(event);
	QDesktopServices::openUrl(QUrl("http://www.yyets.com/"));
}

void AdvertInfoWidget::on_soundButton_clicked(bool isChecked)
{
	emit tryMuteVolume(isChecked);
}

void AdvertInfoWidget::on_detailButton_clicked()
{
	QDesktopServices::openUrl(QUrl("http://www.yyets.com/"));
}

void AdvertInfoWidget::on_closeButton_clicked()
{
	emit tryCloseAdvert();
}