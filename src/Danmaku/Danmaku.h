#pragma once

#include <QtGui>
#include <QtCore>

#include "Utils.h"
#include "Graphic.h"

//
class DanmakuData
{
public:
	qint32 curr;
	qint64 time;
	Record record;
	QList<Comment> danm;
	QList<Graphic> draw;
	QAtomicInt wait;
};

class Danmaku :public QObject
{
	Q_OBJECT
public:
	static Danmaku *instance();

	void drawGraphic(QPainter *painter);

	bool isDanmakuEmpty();
	void parseDanmaku(QString json);

	qint32 indexByTime(qint64 time);
	void prepareDanmaku(QList<Comment> buffer);
	//
	QList<Comment> danmakuByIndex(qint64& index, qint64 time);
	//
	Graphic relocate(QList<Graphic> draw, const Comment &w);
	QRectF minRect(QList<Graphic> data, Graphic const& graphic);
	int intersectSum(QList<Graphic> data, Graphic g,QRectF currRect);
private:
	static Danmaku *m_instance;
	DanmakuData m_data;

	Danmaku(QObject *parent = 0);
signals:
	void modelReset();
public slots:
	void resetTime();
	void setTime(qint64 time);

	void clearCurrent();
	void jumpToTime(qint64 time);

	void sortDanmaku();
	//
	void setRecord(const Record &record);
	void append(QString source, const Comment &comment);
};
