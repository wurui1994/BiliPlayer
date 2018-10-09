#pragma once

#include <QtGui>
#include <QtCore>
#include "Utils.h"
#include "Common.h"
#include "Attribute.h"
#include "Setting.h"
#include "Sprite.h"
#include <exception>

class Graphic
{
public:
	 QList<QRectF> locate();
	 bool isAlive(double time) ;
	 void draw(QPainter *painter);
	 uint intersects(Graphic const&other);
	 uint intersects(QList<Graphic> const& others);
	 bool stay(){ return false; }
	 QRectF &currentRect(){ return m_rect; }
	 void setRect(QRectF const& r);
public:
	Graphic(const Comment &comment);
public:
	double speed; // Model 1 6
	double life;  // Model 4 5

	int getMode() const
	{
		return m_source.mode;
	}

	 bool isEnabled()
	{
		return m_enabled;
	}

	void setEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	Comment source()
	{
		return m_source;
	}

	void setSource(Comment const& source)
	{
		m_source = source;
	}

protected:
	bool m_enabled = true;
	QRectF m_rect;
	QRectF m_origin_rect;
	Comment m_source;

protected:
	Sprite *sprite;
	double evaluate(QString expression);
};
