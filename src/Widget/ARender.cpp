#include "Common.h"
#include "ARender.h"
#include "Setting.h"
#include "Application.h"
#include "Utils.h"
#include "Danmaku.h"

ARender *ARender::m_instance = nullptr;

ARender *ARender::instance()
{
	if (!m_instance)
	{
		m_instance = new ARender;
	}
	return m_instance;
}

ARender::ARender(QWidget *parent) :
	QWidget(parent)
{
}

void ARender::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHints(QPainter::SmoothPixmapTransform);
	QRect rect(QPoint(0, 0), ARender::instance()->size());
	drawDanm(&painter, rect);
}

void ARender::drawDanm(QPainter *painter, QRect)
{
	Danmaku::instance()->drawGraphic(painter);
}