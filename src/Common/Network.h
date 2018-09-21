#pragma once
#ifndef Network_H
#define Network_H

#include <QtCore/QtCore>

class Network:public QObject
{
	Q_OBJECT
private:
    Network();
    ~Network();
public:
    static Network& instance();
	//
	void initAll();
	//
	void playerInit();
	void parseInit(QString json);
	void playerAds();
	void parseAds(QString json);
	//
	void setVidHash(QString hash);
	//
	void sendDanmaku(QString content,int time, int font = 1,QString color = "FFFFFF");
	void getDanmaku(int time);
	//
	QString barrageIP();
	//
	QString videoPath();
	QString imagePath();
signals:
	void danmakuData(QString json, int time);
private:
	QString m_barrageIp;
	QString m_videoPath;
	QString m_imagePath;
	QString m_videoUrl;
	QString m_vidHash;
};
//
#define NetworkInstance Network::instance()
//
#endif // Network_H