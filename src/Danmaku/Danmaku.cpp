#include "Common.h"
#include "Danmaku.h"
#include "Setting.h"
#include "Application.h"
#include "ARender.h"
#include "Graphic.h"
#include "Network.h"
#include <algorithm>

namespace
{
	class CommentComparer
	{
	public:
		inline bool operator ()(const Comment &f, const Comment &s)
		{
			return f.time < s.time;
		}

		//overloads for comparing with time
		inline bool operator ()(const Comment &c, qint64 time)
		{
			return c.time < time;
		}

		inline bool operator ()(qint64 time, const Comment &c)
		{
			return time < c.time;
		}
	};
}

Danmaku *Danmaku::m_instance = nullptr;

Danmaku *Danmaku::instance()
{
	return m_instance ? m_instance : new Danmaku(qApp);
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
			cc.color = cList.at(2).toInt(0,16);
			cc.date = cList.at(3).toInt();
			cc.sender = cList.at(4);
		}
		record.danmaku << cc;
	}
	//
	appendToPool(record);
}

QList<Comment> Danmaku::getDanmakuRange(qint64 start, qint64 end)
{
	QList<Comment> buffer;
	if (m_data.danm.isEmpty())
	{
		return buffer;
	}
	buffer << m_data.danm[m_data.curr];
	for (qint32 curr = m_data.curr + 1;  curr < m_data.danm.size(); ++curr)
	{
		int left = curr - m_data.curr;
		int right = curr + m_data.curr;
		//
		Comment leftComment = m_data.danm[left];
		if (leftComment.time < start)
		{
			break;
		}
		else
		{
			buffer.push_front(leftComment);
		}
		//
		Comment rightComment = m_data.danm[left];
		if (rightComment.time >= end)
		{
			break;
		}
		else
		{
			buffer.push_back(rightComment);
		}
	}
	return buffer;
}

QList<Comment> Danmaku::selectDanmaku(qint64 start, qint64 end)
{
	QList<Comment> buffer;
	if (m_data.danm.isEmpty())
	{
		return buffer;
	}
	//
	for (auto const& comment : m_data.danm)
	{
		if (comment.time >= start && comment.time < end)
		{
			buffer << comment;
		}
	}
	return buffer;
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
	auto curr = qLowerBound(m_data.danm.begin(), m_data.danm.end(), time, CommentComparer());
	return  curr - m_data.danm.begin();
}

void Danmaku::prepareDanmaku(QList<Comment> buffer)
{
	//
	for (auto const&comment : buffer)
	{
		Graphic graphic = process(m_data.draw, comment);
		m_data.draw.append(graphic);
	}
}

Danmaku::Danmaku(QObject *parent)
	:QObject(parent)
{
	m_instance = this;
	setObjectName("Danmaku");
	m_data.curr = m_data.time = 0;
	m_data.dura = -1;

	//
	connect(&Network::instance(), &Network::danmakuData, [=](QString json, int time)
	{
		qDebug() << time;
		parseDanmaku(json);
	});
}


void Danmaku::drawGraphic(QPainter *painter)
{
	QList<Graphic> dirty;
	for (auto graphic : m_data.draw)
	{
		if (graphic.move(m_data.time))
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

void Danmaku::appendToPool(const Record &record)
{
	m_data.record = record;
	stepOne();
	stepTwo();
}

void Danmaku::appendToPool(QString source, const Comment &comment)
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
	Comment &c = m_data.record.danmaku.last();
	c.time += m_data.record.delay;
	auto upperBound = qUpperBound(m_data.danm.begin(), m_data.danm.end(), c, CommentComparer());
	m_data.danm.insert(upperBound, c);
	//m_data.record.limit = m_data.record.limit == 0 ? 0 : qMax(m_data.record.limit, c.date);
	stepTwo();
}

void Danmaku::appendToPool(const Comment & comment)
{
	QString source = m_data.record.source;
	appendToPool(source, comment);
}

void Danmaku::clearCurrent()
{
	m_data.draw.clear();
	ARender::instance()->update();
}

void Danmaku::parse(int flag)
{
	if ((flag & 0x1) > 0)
	{
		stepOne();
	}
	if ((flag & 0x2) > 0)
	{
		stepTwo();
	}
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
	QList<Comment> buffer;
	QList<Graphic> drawList;
	for (; curr < m_data.danm.size(); ++curr)
	{
		Comment comment = m_data.danm[curr];
		if (comment.time >= time)
		{
			break;
		}
		buffer << comment;
		drawList << Graphic(comment);
	}
	//
	//m_data.draw.clear();
	//
	QList<Graphic> draw;

	for (auto g : drawList)
	{
		Comment comment = g.source();
		Graphic graphic = process(draw, comment);
		draw.append(graphic);
	}

	QList<Graphic> dirty;
	for (auto graphic : draw)
	{
		if (graphic.move(m_data.time))
		{
			dirty.append(graphic);
		}
	}
	m_data.draw = dirty;
	//m_data.draw = draw;

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

Graphic Danmaku::process(QList<Graphic> draw, const Comment& comment)
{
	//
	Graphic graphic(comment);
	//
	if (comment.isEmpty())
	{
		return graphic;
	}
	//
	QList<QRectF> locate = graphic.locate();
	if (locate.size() == 1)
	{
		//
		QRectF rect = locate.first();
		//
		graphic.setRect(rect);
	}
	else
	{
		//
		QList<int> result = calculate(draw, graphic);
		//
		int thin = result.first();
		QRectF rect = locate.first();
		for (int i = 1;  i < result.size(); ++i)
		{
			if (thin > result[i] && thin != 0)
			{
				thin = result[i];
				rect = locate[i];
			}
		}
		//
		graphic.setRect(rect);
	}
	//
	return graphic;
}

QList<int> Danmaku::calculate(QList<Graphic> data, Graphic const& g)
{
	Graphic graphic = g;
	QList<int> result;
	//
	for (auto const& rect: graphic.locate())
	{
		int sum = 0;
		for (Graphic const& curr : data)
		{
			Graphic c = curr;
			bool isTooFar = qAbs(c.source().time - graphic.source().time) > 5000;
			if (c.getMode() != graphic.getMode()
				||  isTooFar)
			{
				continue;
			}
			graphic.setRect(rect);
			sum += graphic.intersects(curr);
		}
		result << sum;
	}
	
	return result;
}

void Danmaku::stepOne()
{
	//beginResetModel();
	m_data.danm.clear();
	Record record = m_data.record;
	//m_data.danm.reserve(m_data.danm.size() + record.danmaku.size());
	for (Comment &comment : record.danmaku)
	{
		m_data.danm.append(comment);
	}
	qStableSort(m_data.danm.begin(), m_data.danm.end(), CommentComparer());
	m_data.dura = -1;
	for (Comment c : m_data.danm)
	{
		if (c.time < 10000000 || c.time < m_data.dura * 2)
		{
			m_data.dura = c.time;
		}
		else
		{
			break;
		}
	}
	jumpToTime(m_data.time);
	//endResetModel();
	emit modelReset();
}

void Danmaku::stepTwo()
{
	// Date Limit
	Record r = m_data.record;
	for (Comment &c : r.danmaku)
	{
		c.blocked = r.limit != 0 && c.date > r.limit;
	}
	// Repeat Limit
	int limit = Setting::getValue("/Shield/Limit/Count", 5);
	int range = Setting::getValue("/Shield/Limit/Range", 10000);
	if (limit != 0)
	{
		QVector<QString> clean;
		int size = m_data.danm.size();
		clean.reserve(size);
		for (const Comment iter : m_data.danm)
		{
			QString raw = iter.string;

			int length = raw.length();
			const QChar *data = raw.data();

			QString clr;

			int passed = 0;
			const QChar *head = data;

			for (int i = 0; i < length; ++i)
			{
				const QChar &c = data[i];
				if (c.isLetterOrNumber() || c.isMark() || c == '_')
				{
					++passed;
				}
				else if (passed > 0)
				{
					clr.reserve(length);
					clr.append(head, passed);
					passed = 0;
					head = data + i + 1;
				}
			}
			if (passed == length)
			{
				clean.append(raw);
			}
			else
			{
				if (passed > 0)
				{
					clr.append(head, passed);
				}
				clean.append(clr);
			}
		}
		QHash<QString, int> count;
		int sta = 0, end = sta;
		for (; end < size; ++end)
		{
			Comment e = m_data.danm[end];
			while (m_data.danm[sta].time + range < e.time)
			{
				auto i = count.find(clean[sta]);
				if (i.value() == 1)
				{
					count.erase(i);
				}
				else if (i.value() > 1)
				{
					--(i.value());
				}
				++sta;
			}
			int &num = count[clean[end]];
			if (num >= 0 && ++num > limit && e.mode <= 6)
			{
				num = -1;
			}
		}
		for (; sta < size; ++sta)
		{
			auto i = count.find(clean[sta]);
			if (i.value() > 0)
			{
				count.erase(i);
			}
		}
		for (int i = 0; i < size; ++i)
		{
			Comment c = m_data.danm[i];
			c.blocked = c.blocked || count.contains(clean[i]);
		}
	}

	QList<Graphic> draw;
	for (auto graphic : m_data.draw)
	{
		Comment cur = graphic.source();
		if (cur.blocked)
		{
		}
		else
		{
			draw.append(graphic);
		}
	}
	m_data.draw = draw;

}
