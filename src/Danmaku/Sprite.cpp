#include "Sprite.h"

Sprite::Sprite(const Comment &comment) :
texture(0)
{
	QSize size = ARender::instance()->size();
	QFont font = Attribute::getFont(comment.font*Attribute::getScale(comment.mode, comment.date, size));
	QSize need = Attribute::getSize(comment.string, font);
	source = QImage(Attribute::getCache(comment.string, comment.color, font, need, comment.isLocal()));
}

Sprite::~Sprite()
{

}

void Sprite::draw(QPainter *painter, QRectF dest)
{
#if USE_OPENGL
	if (!texture)
	{
		render->glGenTextures(1, &texture);
		render->loadTexture(texture, 4, source->width(), source->height(), source->bits(), 4);
		delete source;
		source = nullptr;
	}

	painter->beginNativePainting();
	QRect rect(QPoint(0, 0), ARender::instance()->getActualSize());
	render->glEnable(GL_BLEND);
	render->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	render->drawTexture(&texture, 4, dest, rect);
	painter->endNativePainting();
#else
	if (source.isNull())
	{
		qDebug() << "source is null";
	}
	else
	{
		painter->drawImage(dest, source);
	}
#endif
}

QSize Sprite::getSize()
{
	return source.size();
}
