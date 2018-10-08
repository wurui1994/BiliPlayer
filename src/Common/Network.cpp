#include "Network.h"
#include "Utils.h"
#include <QMetaObject>

Network::Network()
{

}

Network::~Network()
{

}
//

Network& Network::instance()
{
	static Network Network;
	return Network;
}

void Network::initAll()
{
	playerInit();
	playerAds();
}

void Network::playerInit()
{
	Utils::asyncWebRequest("https://ctrl.zmzapi.net/player/init?ver=10&platform=20", "get", "", [=](QString json)
	{
		qDebug() << json;
		parseInit(json);
	});
}

void Network::parseInit(QString json)
{
	QString initFile = Utils::filePath("init.json");
	//
	bool isInitExist = QFileInfo(initFile).exists();
	//
	QJsonDocument docOld;
	QJsonDocument docNew;
	//
	if (isInitExist)
	{
		QString jsonOld = Utils::readFile(initFile);
		docOld = QJsonDocument::fromJson(jsonOld.toUtf8());
	}
	//
	docNew = QJsonDocument::fromJson(json.toUtf8());
	//
	if (docNew.isNull() && docOld.isNull())
	{
		qDebug() << "init json parse error";
		return;
	}
	//
	QJsonObject rootOld = docOld.object();
	//int adUpdateTimeOld = rootOld["adUpdateTime"].toInt();
	QString barrageIpOld = rootOld["barrageIp"].toString();
	//
	QJsonObject rootNew = docNew.object();
	//int adUpdateTimeNew = rootNew["adUpdateTime"].toInt();
	QString barrageIpNew = rootOld["barrageIp"].toString();
	//
	m_barrageIp = barrageIpNew;
	//
	Utils::writeFile(initFile, docNew.toJson());
}

void Network::playerAds()
{
	Utils::asyncWebRequest("https://ctrl.zmzapi.net/player/ads?platform=20", "get", "", [=](QString json)
	{
		qDebug() << json;
		parseAds(json);
	});
	//
	QString url = "http://www.yyets.com/lib/style/images/logo-full.png";
	QString fileName = Utils::filePath(QUrl(url).fileName());
	if (QFileInfo(fileName).exists())
	{
		m_imagePath = fileName;
	}
	else
	{
		//
		Utils::asyncDownload(url, fileName, [=](QString info)
		{
			qDebug() << info;
			m_imagePath = fileName;
		});
		//
	}
	//
	QString initFile = Utils::filePath("ads.json");
	//
	bool isInitExist = QFileInfo(initFile).exists();
	//
	QJsonDocument docOld;
	if (isInitExist)
	{
		QString jsonOld = Utils::readFile(initFile);
		docOld = QJsonDocument::fromJson(jsonOld.toUtf8());
	}
	if (docOld.isNull())
	{
		qDebug() << "ads json parse error";
		return;
	}
	//
	QJsonObject rootOld = docOld.object();
	//int adUpdateTimeOld = rootOld["adUpdateTime"].toInt();
	//
	QJsonArray adsArray = rootOld["ads"].toArray();
	for (auto const& ele : adsArray)
	{
		QJsonObject adObject = ele.toObject();
		int adType = adObject["adType"].toInt();
		if (adType == 1)
		{
			QString videoUrl = adObject["video"].toString();
			QString fileName = Utils::filePath(QUrl(videoUrl).fileName());
			if (QFileInfo(fileName).exists())
			{
				m_videoPath = fileName;
			}
			else
			{
				Utils::asyncDownload(videoUrl, fileName, [=](QString info)
				{
					qDebug() << info;
					m_videoPath = fileName;
				});
			}
			//
			m_videoUrl = videoUrl;
		}
	}
}

void Network::parseAds(QString json)
{
	QString initFile = Utils::filePath("ads.json");
	//
	QJsonDocument docNew;
	//
	docNew = QJsonDocument::fromJson(json.toUtf8());
	//
	if (docNew.isNull())
	{
		qDebug() << "init json parse error";
		return;
	}
	//
	QJsonObject rootNew = docNew.object();
	//int adUpdateTimeNew = rootNew["adUpdateTime"].toInt();
	//
	QJsonArray adsArray = rootNew["ads"].toArray();
	for (auto const& ele : adsArray)
	{
		QJsonObject adObject = ele.toObject();
		int adType = adObject["adType"].toInt();
		if (adType == 1)
		{
			QString videoUrl = adObject["video"].toString();
			QString fileName = Utils::filePath(QUrl(videoUrl).fileName());
			if (QFileInfo(fileName).exists())
			{
				m_videoPath = fileName;
			}
			else
			{
				Utils::asyncDownload(videoUrl, fileName, [=](QString info)
				{
					qDebug() << info;
					m_videoPath = fileName;
				});
			}
			//
			m_videoUrl = videoUrl;
		}
	}
	//
	Utils::writeFile(initFile, docNew.toJson());
	//
}

void Network::setVidHash(QString hash)
{
	m_vidHash = hash;
}

void Network::sendDanmaku(QString content,int time,int font, QString color)
{
	QUrl url("http://" + m_barrageIp + "/v1/send");

	QUrlQuery query;
	query.addQueryItem("vid", "1");
	query.addQueryItem("t", QString::number(time));
	query.addQueryItem("uid", QString::number(m_vidHash.mid(0, 4).toInt(0, 16)));
	query.addQueryItem("font", QString::number(font));
	query.addQueryItem("color", color);
	query.addQueryItem("msg", content);
	query.addQueryItem("r", "1528293836");
	query.addQueryItem("s", "3242449a67516ecaabe189484209719feb30240e");

	url.setQuery(query);
	//
	Utils::asyncWebRequest(url.toString(), "get", "", [=](QString result)
	{
		qDebug() << result;
	});
}

void Network::getDanmaku(int time)
{
#if 0
	QUrl url("http://" + m_barrageIp + "/v1/get");

	QUrlQuery query;
	query.addQueryItem("vid", "1");
	query.addQueryItem("t", QString::number(time));
	query.addQueryItem("r", "1528293836");
	query.addQueryItem("s", "3242449a67516ecaabe189484209719feb30240e");

	url.setQuery(query);
	//
	Utils::asyncWebRequest(url.toString(), "get", "", [=](QString result)
	{
		//qDebug() << result;
		emit danmakuData(result, time);
	});
#endif
}

QString Network::barrageIP()
{
	return m_barrageIp;
}

QString Network::videoPath()
{
	if (!m_videoPath.isEmpty())
	{
		return m_videoPath;
	}
	else
	{
		return m_videoUrl;
	}
}

QString Network::imagePath()
{
	return m_imagePath;
}
