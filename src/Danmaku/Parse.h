#pragma once

#include <QtCore>
#include "Utils.h"

namespace Parse
{
	QVector<Comment> parseComment(const QByteArray &data, Utils::Site site);
}
