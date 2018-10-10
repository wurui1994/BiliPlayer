#include "Common.h"
#include "Danmaku.h"
#include "Setting.h"
#include "Application.h"
#include "ARender.h"
#include "Graphic.h"
#include "Network.h"
#include <algorithm>

Danmaku *Danmaku::m_instance = nullptr;

Danmaku *Danmaku::instance()
{
	if (!m_instance)
	{
		m_instance = new Danmaku(qApp);
	}

	return m_instance;
}

Danmaku::Danmaku(QObject *parent)
	:QObject(parent)
{
	m_data.curr = m_data.time = 0;
	//
	connect(&Network::instance(), &Network::danmakuData, [=](QString json, int time)
	{
		qDebug() << time;
		parseDanmaku(json);
	});
}

bool Danmaku::isDanmakuEmpty()
{
	return m_data.danm.isEmpty();
}

void Danmaku::parseDanmaku(QString json)
{
	QJsonDocument docNew = QJsonDocument::fromJson(json.toUtf8());
	QJsonObject root = docNew.object();
	QJsonArray dataArray = root["data"].toArray();
	//
	Record record;
	record.source = "1";
	//
	for (auto const& ele : dataArray)
	{
		QJsonObject commentObject = ele.toObject();
		QString c = commentObject["c"].toString();
		QString m = commentObject["m"].toString();
		QStringList cList = c.split(",");
		Comment cc;
		cc.string = m;
		if (cList.size() == 5)
		{
			cc.mode = 1;
			cc.time = cList.at(0).toInt();
			cc.font = cList.at(1).toInt() ? 25 : 12;
			cc.color = cList.at(2).toInt(0, 16);
			cc.date = cList.at(3).toInt();
			cc.sender = cList.at(4);
		}
		record.danmaku << cc;
	}
	//
	setRecord(record);
}

QList<Comment> Danmaku::danmakuByIndex(qint64 & index, qint64 time)
{
	QList<Comment> buffer;
	for (; index < m_data.danm.size(); ++index)
	{
		Comment comment = m_data.danm[index];
		if (comment.time >= time)
		{
			break;
		}
		buffer << comment;
	}
	return buffer;
}

qint32 Danmaku::indexByTime(qint64 time)
{
	auto curr = qLowerBound(m_data.danm.begin(), m_data.danm.end(), time,
		[](const Comment &c, qint64 time)
	{
		return c.time < time;
	});
	return  curr - m_data.danm.begin();
}

void Danmaku::prepareDanmaku(QList<Comment> buffer)
{
	//
	for (auto const&comment : buffer)
	{
		Graphic graphic = relocate(m_data.draw, comment);
		m_data.draw.append(graphic);
	}
}

void Danmaku::drawGraphic(QPainter *painter)
{
	QList<Graphic> dirty;
	for (auto graphic : m_data.draw)
	{
		if (graphic.isAlive(m_data.time))
		{
			dirty.append(graphic);
		}
	}
	//
	for (Graphic graphic : dirty)
	{
		graphic.draw(painter);
	}
	//
	m_data.draw = dirty;
}


void Danmaku::resetTime()
{
	m_data.curr = m_data.time = 0;
}

void Danmaku::setRecord(const Record &record)
{
	m_data.record = record;
	sortDanmaku();
}

void Danmaku::append(QString source, const Comment &comment)
{
	if (m_data.record.source == source)
	{
	}
	else
	{
		m_data.record.danmaku.clear();
		m_data.record.source = source;
	}
	m_data.record.danmaku.append(comment);
	//
	Comment &c = m_data.record.danmaku.last();
	c.time += m_data.record.delay;
	//
	auto upperBound = qUpperBound(m_data.danm.begin(), m_data.danm.end(), c,
		[](const Comment &f, const Comment &s)
	{
		return f.time < s.time;
	});
	m_data.danm.insert(upperBound, c);
}

void Danmaku::clearCurrent()
{
	m_data.draw.clear();
	ARender::instance()->update();
}

void Danmaku::setTime(qint64 time)
{
	m_data.time = time;
	//int limit = Config::getValue("/Shield/Density", 0);
	qint64 curr = m_data.curr;
	QList<Comment> buffer = danmakuByIndex(curr, time);
	//
	m_data.curr = curr;
	//
	prepareDanmaku(buffer);
}

void Danmaku::jumpToTime(qint64 time)
{
#if 1
	qint64 offsetTime = time - 10000;
	if (offsetTime > 5000)
	{
		offsetTime = offsetTime / 5000 * 5000;
	}
	//
	qint64 curr = indexByTime(offsetTime);
	QList<Comment> buffer = danmakuByIndex(curr, time);;
	
	m_data.draw.clear();

	for (auto comment : buffer)
	{
		Graphic graphic = relocate(m_data.draw, comment);
		m_data.draw.append(graphic);
	}

	QList<Graphic> alive;
	for (auto graphic : m_data.draw)
	{
		if (graphic.isAlive(m_data.time))
		{
			alive.append(graphic);
		}
	}
	m_data.draw = alive;
	//clearCurrent();
	m_data.time = time;
	m_data.curr = indexByTime(time);

	ARender::instance()->update();
#else
	clearCurrent();
	m_data.time = time;
	m_data.curr = indexByTime(time);
#endif
}

Graphic Danmaku::relocate(QList<Graphic> draw, const Comment& comment)
{
	Graphic graphic(comment);
	//
	if (comment.isEmpty())
	{
		return graphic;
	}
	//
	graphic.setRect(minRect(draw, graphic));
	//
	return graphic;
}

QRectF Danmaku::minRect(QList<Graphic> data, Graphic const& g)
{
	Graphic graphic = g;
	QList<int> result;
	//
	for (auto const& rect : graphic.locate())
	{
		result << intersectSum(data, graphic, rect);
	}
	//
	QList<int> newResult = result;
	qStableSort(newResult);
	int index = result.indexOf(newResult.first());
	QRectF rect = graphic.locate().at(index);
	//
	return rect;
}

int Danmaku::intersectSum(QList<Graphic> data, Graphic graphic, QRectF currRect)
{
	graphic.setRect(currRect);

	QList<Graphic> remainGraphics;
	for (Graphic curr : data)
	{
		bool isDiffMode = curr.getMode() != graphic.getMode();
		bool isTooFar = qAbs(curr.source().time - graphic.source().time) > 5000;
		if (isDiffMode || isTooFar)
		{
			continue;
		}
		remainGraphics << curr;
	}
	return graphic.intersects(remainGraphics);
}

void Danmaku::sortDanmaku()
{
	m_data.danm.clear();
	//
	for (Comment &comment : m_data.record.danmaku)
	{
		m_data.danm.append(comment);
	}
	qStableSort(m_data.danm.begin(), m_data.danm.end(), 
		[](const Comment &f, const Comment &s)
	{
		return f.time < s.time;
	});

	emit modelReset();
}