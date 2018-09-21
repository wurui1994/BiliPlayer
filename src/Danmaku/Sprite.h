#pragma once

#include <QOpenGLFunctions>

#include "Common.h"

#include "ARender.h"
#include "Attribute.h"
#include "Utils.h"

class Sprite
{
public:
	Sprite(const Comment &comment);
	~Sprite();
	void draw(QPainter *painter, QRectF dest);
	QSize getSize();
private:
	GLuint texture;
	QImage source;
};
