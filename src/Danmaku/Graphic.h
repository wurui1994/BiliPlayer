#pragma once

#include <QtGui>
#include <QtCore>
#include "Utils.h"
#include "Common.h"
#include "Attribute.h"
#include "Setting.h"
#include <exception>

class Graphic
{
public:
	 bool isAlive(double time) ;
	 void draw(QPainter *painter);
	 void setRect(QRectF const& r);
	 //
	 QList<QRectF> locate();
	 //
	 uint intersects(Graphic const& other);
	 uint intersects(QList<Graphic> const& others);
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
	QImage m_image;
	double evaluate(QString expression);
};
