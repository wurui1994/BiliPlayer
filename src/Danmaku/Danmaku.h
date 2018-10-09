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
	qint64 dura;
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
	QList<Comment> getDanmakuRange(qint64 start, qint64 end);
	QList<Comment> selectDanmaku(qint64 start, qint64 end);
	QList<Comment> danmakuByIndex(qint64& index, qint64 time);
	//
	Graphic process(QList<Graphic> draw, const Comment &w);
	QList<int> calculate(QList<Graphic> data, Graphic const& graphic);
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
	void parse(int flag = 0);
	void jumpToTime(qint64 time);

	void stepOne();
	void stepTwo();
	//
	void appendToPool(const Record &record);
	void appendToPool(const Comment &comment);
	void appendToPool(QString source, const Comment &comment);
};
