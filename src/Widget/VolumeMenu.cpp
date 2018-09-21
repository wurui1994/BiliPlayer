#include "VolumeMenu.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "Utils.h"

VolumeMenu::VolumeMenu(QWidget *parent)
	: QMenu(parent)
{
	//
	ui.setupUi(this);
	//
	setWindowFlag(Qt::FramelessWindowHint,true);
	setAttribute(Qt::WA_TranslucentBackground);
	setStyleSheet("background-color: rgba(0, 0, 0, 128);border-radius:4px;");
	//
	ui.verticalSlider->setStyleSheet(Utils::readFile(":/qss/volumeslider.qss"));
	//
	ui.verticalSlider->installEventFilter(this);
	//
	connect(ui.verticalSlider, &QSlider::valueChanged, [=](int value)
	{
		emit valueChanged(value);
	});
}

VolumeMenu::~VolumeMenu()
{
}

void VolumeMenu::setValue(int value)
{
	ui.verticalSlider->blockSignals(true);
	ui.verticalSlider->setValue(value);
	ui.verticalSlider->blockSignals(false);
}

void VolumeMenu::showEvent(QShowEvent * event)
{
	Q_UNUSED(event);
	int x = pos().x() - geometry().width() / 2 + 10;
	int y = pos().y() - geometry().height() - 40;
	move(x,y);
}

bool VolumeMenu::eventFilter(QObject * obj, QEvent * event)
{
	auto slider = ui.verticalSlider;
	if (obj == slider && slider->isEnabled())
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mevent = static_cast<QMouseEvent *>(event);
			qreal value = slider->minimum() + (slider->maximum() - slider->minimum()) 
				* (slider->height()-mevent->localPos().y()) / slider->height();
			if (mevent->button() == Qt::LeftButton)
			{
				int  v = qRound(value);
				slider->setValue(v);
				//
			}
			event->accept();
			return true;
		}
		if (event->type() == QEvent::MouseMove)
		{
			auto mevent = static_cast<QMouseEvent *>(event);
			qreal value = slider->minimum() + (slider->maximum() - slider->minimum()) 
				* (slider->height() - mevent->localPos().y()) / slider->height();
			if (mevent->buttons() & Qt::LeftButton)
			{
				int  v = qRound(value);
				slider->setValue(v);
				//
			}
			event->accept();
			return true;
		}
		if (event->type() == QEvent::MouseButtonDblClick)
		{
			event->accept();
			return true;
		}
	}
	return QWidget::eventFilter(obj, event);
}
