#pragma once

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include "Setting.h"

namespace Attribute
{
	QFont  getFont(int pixelSize,
		QString family = Setting::getValue("/Danmaku/Font", QFont().family()));

	QSize  getSize(QString string,
		QFont font);

	QSizeF getPlayer(qint64 date);

	double getScale(int mode,
		qint64 date,
		QSize size);

	QImage getCache(QString string,
		int color,
		QFont font,
		QSize size,
		bool frame,
		int effect = Setting::getValue("/Danmaku/Effect", 5) / 2,
		int opacity = Setting::getValue("/Danmaku/Alpha", 100));
}
