#include "seekbar.h"

#include <QTime>
#include <QToolTip>
#include <QPainter>
#include <QRect>
#include <QStyle>

#include "Utils.h"

// ref: https://stackoverflow.com/questions/11132597/qslider-mouse-direct-jump
class MyStyle : public QProxyStyle
{
public:
	using QProxyStyle::QProxyStyle;

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
			return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

SeekBar::SeekBar(QWidget *parent) :
	QSlider(parent),
	tickReady(false),
	totalTime(0),
	m_isPressed(false)
{
	setStyle(new MyStyle(this->style()));
	//
	m_label = new QLabel(this);
	m_label->setWindowFlag(Qt::Tool, true);
	m_label->setWindowFlag(Qt::FramelessWindowHint, true);
}

void SeekBar::setTracking(int _totalTime)
{
	setRange(0, _totalTime);
    if(_totalTime != 0)
    {
        totalTime = _totalTime;
        // now that we've got totalTime, calculate the tick locations
        // we need to do this because totalTime is obtained after the LOADED event is fired--we need totalTime for calculations
		for (auto &tick : ticks)
		{
			tick = ((double)tick / totalTime)*maximum();
		}
        //
        if(ticks.length() > 0)
        {
            tickReady = true; // ticks are ready to be displayed
            repaint(rect());
        }
        setMouseTracking(true);
    }
    else
        setMouseTracking(false);
}

void SeekBar::setTicks(QList<int> values)
{
    ticks = values; // just set the values
    tickReady = false; // ticks need to be converted when totalTime is obtained
}

int SeekBar::positionValue(int pos)
{
	return QStyle::sliderValueFromPosition(minimum(), maximum(), pos, width());
}

void SeekBar::mouseMoveEvent(QMouseEvent* event)
{
#if 1
	if (totalTime != 0)
	{
		m_label->move(QPoint(event->globalPos().x() - 18, 
			mapToGlobal(rect().topLeft()).y() - 15));
		int time = positionValue(event->pos().x());
		QString timeString = Utils::FormatTime(time);
		m_label->setText(timeString);
		if (!m_label->isVisible())
		{
			m_label->show();
		}
	}
#endif
    QSlider::mouseMoveEvent(event);
}

void SeekBar::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);
    if(isEnabled() && tickReady)
    {
        QRect region = event->rect();
        QPainter painter(this);
        painter.setPen(QColor(190,190,190));
        for(auto &tick : ticks)
        {
			int x = positionValue(tick);
            painter.drawLine(x, region.top(), x, region.bottom());
        }
    }
}

void SeekBar::leaveEvent(QEvent * event)
{
	Q_UNUSED(event);
	m_label->hide();
}

void SeekBar::enterEvent(QEvent * event)
{
	Q_UNUSED(event);
	//m_label->show();
}

void SeekBar::setValueNoSignal(int value)
{
	this->blockSignals(true);
	this->setValue(value);
	this->blockSignals(false);
}