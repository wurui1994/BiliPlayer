#pragma once

#include <QtCore/QtCore>
#include "Utils.h"

namespace Parse
{
	QVector<Comment> parseComment(const QByteArray &data, Utils::Site site);
}
