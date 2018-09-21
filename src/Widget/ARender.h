#pragma once

#include "Sprite.h"
#include "Common.h"

class ARender:public QWidget
{
	Q_OBJECT
public:
	static ARender *instance();
	static ARender *m_instance;
	ARender(QWidget *parent = 0);
public:
	double time = 0;

	void drawDanm(QPainter *painter, QRect rect);
	void drawTime(QPainter *painter, QRect rect);

public slots:
	void setDisplayTime(double t);
	void draw();
private:
	void paintEvent(QPaintEvent *event);
};
