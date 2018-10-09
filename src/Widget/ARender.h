#pragma once

#include "Sprite.h"
#include "Common.h"

class ARender:public QWidget
{
	Q_OBJECT
public:
	static ARender *instance();
	ARender(QWidget *parent = 0);
public:
	void drawDanm(QPainter *painter, QRect rect);
protected:
	void paintEvent(QPaintEvent *event);
private:
	static ARender *m_instance;
};
