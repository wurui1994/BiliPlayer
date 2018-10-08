#include "Common.h"
#include "Graphic.h"
#include "Setting.h"
#include "ARender.h"
#include "Graphic.h"

using namespace Attribute;

void Graphic::draw(QPainter *painter)
{
	if (enabled)
	{
		sprite->draw(painter, m_rect);
	}
}

double Graphic::evaluate(QString expression)
{
	expression.replace("%{width}", QString::number(m_rect.width()), Qt::CaseInsensitive);
	return Utils::evaluate(expression);
}

QList<QRectF> Graphic::locate()
{
	QList<QRectF> results;
	if (m_source.mode == 1)
	{	
		QSize size = ARender::instance()->size();
		origin_rect.moveLeft(size.width());
		QRectF init = origin_rect;
		int end = size.height()*(Setting::getValue("/Danmaku/Protect", false) ? 0.85 : 1) - origin_rect.height();
		int stp = Setting::getValue("/Danmaku/Grating", 10);
		for (int height = 0; height <= end; height += stp)
		{
			init.moveTop(height);
			results.append(init);
		}
		return results;
	}
	else if (m_source.mode == 4)
	{
		QSize size = ARender::instance()->size();
		origin_rect.moveCenter(QPointF(size.width() / 2.0, 0));
		origin_rect.moveBottom(size.height()*0.90);
		QRectF init = origin_rect;
		int stp = Setting::getValue("/Danmaku/Grating", 10);
		for (int height = init.top(); height >= 0; height -= stp)
		{
			init.moveTop(height);
			results.append(init);
		}
		return results;
	}
	else if (m_source.mode == 5)
	{
		QList<QRectF> results;

		QSize size = ARender::instance()->size();	
		origin_rect.moveCenter(QPointF(size.width() / 2.0, 0));
		QRectF init = origin_rect;
		int end = size.height()*(Setting::getValue("/Danmaku/Protect", false) ? 0.85 : 1) - origin_rect.height();
		int stp = Setting::getValue("/Danmaku/Grating", 10);
		for (int height = 0; height <= end; height += stp)
		{
			init.moveTop(height);
			results.append(init);
		}
		return results;
	}
	else if (m_source.mode == 6)
	{
		QList<QRectF> results;

		QSize size = ARender::instance()->size();
		origin_rect.moveRight(0);
		QRectF init = origin_rect;	
		int end = size.height()*(Setting::getValue("/Danmaku/Protect", false) ? 0.85 : 1) - origin_rect.height();
		int stp = Setting::getValue("/Danmaku/Grating", 10);
		for (int height = 0; height <= end; height += stp)
		{
			init.moveTop(height);
			results.append(init);
		}
		return results;
	}
	else
	{
		return results;
	}
}

bool Graphic::move(double time)
{
	time = (time - m_source.time) / 1000;
	QRectF rect = origin_rect;
	if (m_source.mode == 1)
	{
		if (enabled)
		{
			rect.moveLeft(rect.left() - speed*time);
		}
		m_rect = rect;
		return rect.right() >= 0;
	}
	else if (m_source.mode == 4)
	{
		int real_life = life;
		if (enabled)
		{
			real_life -= time;
		}
		return real_life > 0;
	}
	else if (m_source.mode == 5)
	{
		int real_life = life;
		if (enabled)
		{
			real_life -= time;
		}
		return real_life > 0;
	}
	else if (m_source.mode == 6)
	{
		QSize size = ARender::instance()->size();
		if (enabled)
		{
			rect.moveLeft(rect.left() + speed * time);
		}
		m_rect = rect;
		return rect.left() <= size.width();
	}
	else
	{
		return false;
	}
}

uint Graphic::intersects(Graphic const& other)
{
	QRectF overlap = m_rect.intersected(other.m_rect);
	return overlap.size().width() * overlap.size().height();
}

void Graphic::setRect(QRectF const & r)
{
	m_rect = r;
	origin_rect = m_rect;
}

Graphic::Graphic(const Comment & comment)
{
	speed = evaluate(Setting::getValue<QString>("/Danmaku/Speed", "125+%{width}/5"));
	life = evaluate(Setting::getValue<QString>("/Danmaku/Life", "5"));

	m_source = comment;
	sprite = new Sprite(comment);
	m_rect.setSize(sprite->getSize());
	//
	origin_rect = m_rect;
}
