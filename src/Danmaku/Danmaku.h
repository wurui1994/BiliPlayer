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
	virtual ~Danmaku();
	Record& record();
	void drawGraphic(QPainter *painter);
	static Danmaku *instance();
	DanmakuData const& data();
	bool isDanmakuEmpty();
	void parseDanmaku(QString json);
	QList<Comment> getDanmakuRange(qint64 start, qint64 end);
	QList<Comment> selectDanmaku(qint64 start, qint64 end);
	QList<Comment> danmakuByIndex(qint64& index,qint64 time);
	qint32 indexByTime(qint64 time);
	void prepareDanmaku(QList<Comment> buffer);
private:
	static Danmaku *ins;
	DanmakuData m_data;

	explicit Danmaku(QObject *parent = 0);
signals:
	void alphaChanged(int);
	void modelReset();
public slots:
    Comment commentAt(QPointF point) const;
	void setAlpha(int alpha);
	void clearPool();
	void resetTime();
	void setTime(qint64 time);
	void appendToPool(const Record &record);
	void appendToPool(QString source, const Comment &comment);
	void appendToPool(const Comment &comment);
	void clearCurrent();
	void insertToCurrent(Graphic &graphic, int index = -1);
	void parse(int flag = 0);
	void delayAll(qint64 time);
	void jumpToTime(qint64 time);
	void saveToFile(QString file) const;
	qint64 getDuration() const;
	Graphic process(DanmakuData & danm, const Comment &w);
	QVector<int> calculate(int size, QList<Graphic> &data,Graphic& graphic);
	void stepOne();
	void stepTwo();
};
