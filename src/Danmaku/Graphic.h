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
	 bool move(double time) ;
	 void draw(QPainter *painter);
	 uint intersects(Graphic const&other);
	 bool stay(){ return false; }
	 QRectF &currentRect(){ return m_rect; }
	 void setRect(QRectF const& r);
public:
	Graphic(const Comment &comment);
public:
	double speed; // Model 1 6
	double life;  // Model 4 5

	inline int getMode() const
	{
		return m_source.mode;
	}

	inline bool isEnabled()
	{
		return enabled;
	}

	inline void setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}

	inline quint64 getIndex()
	{
		return index;
	}

	void setIndex();

	inline Comment& source()
	{
		return m_source;
	}

	inline void setSource(Comment const& source)
	{
		this->m_source = source;
	}

protected:
	bool enabled = true;
	QRectF m_rect;
	QRectF origin_rect;
	quint64 index;
	Comment m_source;

protected:
	Sprite *sprite;
	double evaluate(QString expression);
};
