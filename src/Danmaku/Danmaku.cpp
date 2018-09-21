#include "Common.h"
#include "Danmaku.h"
#include "Setting.h"
#include "Application.h"
#include "ARender.h"
#include "Graphic.h"
#include "Network.h"
#include <algorithm>


Danmaku *Danmaku::ins = nullptr;

Danmaku *Danmaku::instance()
{
	return ins ? ins : new Danmaku(qApp);
}

DanmakuData const & Danmaku::data()
{
	return m_data;
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

Danmaku::Danmaku(QObject *parent)
	:QObject(parent)
{
	ins = this;
	setObjectName("Danmaku");
	m_data.curr = m_data.time = 0;
	m_data.dura = -1;

	QMetaObject::invokeMethod(this, "alphaChanged", Qt::QueuedConnection, Q_ARG(int, Setting::getValue("/Danmaku/Alpha", 100)));
	//
	connect(&Network::instance(), &Network::danmakuData, [=](QString json, int time)
	{
		qDebug() << time;
		parseDanmaku(json);
	});
}

Danmaku::~Danmaku()
{
	//qDeleteAll(m_data.draw);
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
	m_data.draw.clear();
	for (auto const& graphic : dirty)
	{
		m_data.draw.append(graphic);
	}
	//
	for (Graphic graphic : dirty)
	{
		graphic.draw(painter);
	}
}

Record &Danmaku::record()
{
	return m_data.record;
}

Comment Danmaku::commentAt(QPointF point) const
{
	Comment comment;
	for (Graphic graphic : m_data.draw)
	{
		if (graphic.currentRect().contains(point))
		{
			comment = graphic.source();
			break;
		}
	}
	return comment;
}

void Danmaku::setAlpha(int alpha)
{
	Setting::setValue("/Danmaku/Alpha", alpha);
	emit alphaChanged(alpha);
}

void Danmaku::resetTime()
{
	m_data.curr = m_data.time = 0;
}

void Danmaku::clearPool()
{
	clearCurrent();
	m_data.danm.clear();
	stepOne();
	stepTwo();
}

void Danmaku::appendToPool(const Record &record)
{
	m_data.record = record;
	stepOne();
	stepTwo();
}

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
	auto pool = record();
	QString source = pool.source;
	appendToPool(source, comment);
}

void Danmaku::clearCurrent(bool soft)
{
	QList<Graphic> draw;
	for (auto graphic : m_data.draw)
	{
		if (soft && graphic.stay())
		{
			draw.append(graphic);
		}
	}
	m_data.draw = draw;
	ARender::instance()->draw();
}

void Danmaku::insertToCurrent(Graphic &graphic, int index)
{
	graphic.setIndex();
	int size = m_data.draw.size(), next;
	if (size == 0 || index == 0)
	{
		next = 0;
	}
	else
	{
		int ring = size + 1;
		next = index > 0 ? (index%ring) : (ring + index % ring);
		if (next == 0)
		{
			next = size;
		}
	}
	m_data.draw.insert(next, graphic);
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
	QList<Comment> buffer;
	for (; m_data.curr < m_data.danm.size(); ++m_data.curr)
	{
		Comment comment = m_data.danm[m_data.curr];
		if (comment.time >= time)
		{
			break;
		}
		buffer << comment;
	}
	//
	for (auto const&comment : buffer)
	{
		process(m_data, comment);
	}
	//
}

void Danmaku::delayAll(qint64 time)
{
	Record r = m_data.record;
	r.delay += time;
	for (Comment &c : r.danmaku)
	{
		c.time += time;
	}
	jumpToTime(m_data.time);
}

void Danmaku::jumpToTime(qint64 time)
{
	clearCurrent(true);
	m_data.time = time;
	m_data.curr = qLowerBound(m_data.danm.begin(), m_data.danm.end(), time, CommentComparer()) - m_data.danm.begin();
}

void Danmaku::saveToFile(QString file) const
{
	QFile f(file);
	f.open(QIODevice::WriteOnly | QIODevice::Text);
	bool skip = Setting::getValue("/Interface/Save/Skip", false);
	if (file.endsWith("xml", Qt::CaseInsensitive))
	{
		QXmlStreamWriter w(&f);
		w.setAutoFormatting(true);
		w.writeStartDocument();
		w.writeStartElement("i");
		w.writeStartElement("chatserver");
		w.writeCharacters("chat." + Utils::customUrl(Utils::Bilibili));
		w.writeEndElement();
		w.writeStartElement("mission");
		w.writeCharacters("0");
		w.writeEndElement();
		w.writeStartElement("source");
		w.writeCharacters("k-v");
		w.writeEndElement();
		for (const Comment c : m_data.danm)
		{
			if (c.blocked&&skip)
			{
				continue;
			}
			w.writeStartElement("d");
			QStringList l;
			l << QString::number(c.time / 1000.0) <<
				QString::number(c.mode) <<
				QString::number(c.font) <<
				QString::number(c.color) <<
				QString::number(c.date) <<
				"0" <<
				c.sender <<
				"0";
			w.writeAttribute("p", l.join(','));
			w.writeCharacters(c.string);
			w.writeEndElement();
		}
		w.writeEndElement();
		w.writeEndDocument();
	}
	else
	{
		QJsonArray a;
		for (const Comment c : m_data.danm)
		{
			if (c.blocked && skip)
			{
				continue;
			}
			QJsonObject o;
			QStringList l;
			l << QString::number(c.time / 1000.0) <<
				QString::number(c.color) <<
				QString::number(c.mode) <<
				QString::number(c.font) <<
				c.sender <<
				QString::number(c.date);
			o["c"] = l.join(',');
			o["m"] = c.string;
			a.append(o);
		}
		f.write(QJsonDocument(a).toJson(QJsonDocument::Compact));
	}
	f.close();
}

qint64 Danmaku::getDuration() const
{
	return m_data.dura;
}

void Danmaku::process(DanmakuData &danm, const Comment& comment)
{
	qint64 createTime = QDateTime::currentMSecsSinceEpoch();
	//
	if (comment.isEmpty() || createTime < QDateTime::currentMSecsSinceEpoch() - 500)
	{
		return;
	}
	//
	Graphic graphic(comment);
	const QList<QRectF> &locate = graphic.locate();
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
		QVector<int> result = calculate(locate.size(), danm.draw, graphic);
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
	graphic.setIndex();
	danm.draw.append(graphic);

	danm.wait--;
}

QVector<int> Danmaku::calculate(int size,
	QList<Graphic> &data, Graphic& graphic)
{
	QVector<int> result(size, 0);

	const QList<QRectF> &locate = graphic.locate();
	//
	for (Graphic const& iter : data)
	{
		if (iter.getMode() != graphic.source().mode)
		{
			continue;
		}
		//
		for (int sta = 0; sta < result.size(); ++sta)
		{
			graphic.setRect(locate[sta]);
			result[sta] += graphic.intersects(iter);
		}
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
