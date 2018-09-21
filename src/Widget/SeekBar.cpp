#include "seekbar.h"

#include <QTime>
#include <QToolTip>
#include <QPainter>
#include <QRect>
#include <QStyle>

#include "Utils.h"

SeekBar::SeekBar(QWidget *parent):
    QSlider(parent),
    tickReady(false),
    totalTime(0)
{
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
    if(totalTime != 0)
    {
		int time = positionValue(event->pos().x());
		QString timeString = Utils::FormatTime(time);

		QToolTip::showText(QPoint(event->globalPos().x() - 25, mapToGlobal(rect().topLeft()).y() - 40),
			timeString, this, rect());
    }
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
void SeekBar::setValueNoSignal(int value)
{
	this->blockSignals(true);
	this->setValue(value);
	this->blockSignals(false);
}

void SeekBar::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		int value = positionValue(event->pos().x());
		emit tryToSeek(value);
		event->accept();
	}
	QSlider::mousePressEvent(event);
}

void SeekBar::mouseReleaseEvent(QMouseEvent * event)
{
	QSlider::mouseReleaseEvent(event);
}
