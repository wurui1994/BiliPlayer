#include "AdvertWidget.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtGui/QMouseEvent>

AdvertWidget::AdvertWidget(QWidget *parent)
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
	ui.advertLabel->installEventFilter(this);
}

AdvertWidget::~AdvertWidget()
{
	
}

void AdvertWidget::setPixmap(QPixmap const & pixmap)
{
	ui.advertLabel->setFixedSize(pixmap.size());
	//
	ui.advertLabel->setPixmap(pixmap);
}

void AdvertWidget::mousePressEvent(QMouseEvent * event)
{
	Q_UNUSED(event);
	QDesktopServices::openUrl(QUrl("http://www.yyets.com/"));
}

bool AdvertWidget::eventFilter(QObject * obj, QEvent * event)
{
	return QWidget::eventFilter(obj, event);
}

void AdvertWidget::on_closeButton_clicked()
{
	hide();
}