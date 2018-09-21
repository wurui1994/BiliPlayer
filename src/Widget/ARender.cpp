#include "Common.h"
#include "ARender.h"
#include "Setting.h"
#include "Application.h"
#include "Utils.h"
#include "Danmaku.h"

ARender *ARender::m_instance = nullptr;

ARender *ARender::instance()
{
	if (m_instance)
	{
		return m_instance;
	}

	return new ARender;
}

ARender::ARender(QWidget *parent) :
	QWidget(parent)
{
	//
	setObjectName("ORender");
	//
	m_instance = this;
}

void ARender::setDisplayTime(double t)
{
	this->time = t;
	QSize s = size();
	update(QRect(0, s.height() - 2, s.width()*this->time, 2));
}

void ARender::draw()
{
	QRect rect;
	rect.setSize(size());
	update(rect);
}

void ARender::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHints(QPainter::SmoothPixmapTransform);
	QRect rect(QPoint(0, 0), ARender::instance()->size());
	//drawStop(&painter, rect);
	drawDanm(&painter, rect);
	drawTime(&painter, rect);
}

void ARender::drawTime(QPainter *painter, QRect rect)
{
	if (time <= 0)
	{
		return;
	}
	rect = QRect(0, rect.height() - 2, rect.width()*time, 2);
	QLinearGradient gradient;
	gradient.setStart(rect.center().x(), rect.top());
	gradient.setFinalStop(rect.center().x(), rect.bottom());
	QColor outline = qApp->palette().background().color().darker(140);
	QColor highlight = qApp->palette().color(QPalette::Highlight);
	QColor highlightedoutline = highlight.darker(140);
	if (qGray(outline.rgb()) > qGray(highlightedoutline.rgb()))
	{
		outline = highlightedoutline;
	}
	painter->setPen(QPen(outline));
	gradient.setColorAt(0, highlight);
	gradient.setColorAt(1, highlight.lighter(130));
	painter->setBrush(gradient);
	painter->drawRect(rect);
}

void ARender::drawDanm(QPainter *painter, QRect)
{
	Danmaku::instance()->drawGraphic(painter);
}