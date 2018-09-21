#pragma once

#include <QWidget>
#include <QString>
#include <QTextStream>
#include <QMessageBox>

#include <QtWidgets/QtWidgets>

#define MsgBox() QMessageBox::information(NULL,__FILE__,__FUNCTION__)
#define Debug() qDebug() << __FILE__ << __FUNCTION__ << __LINE__

struct Recent
{
	Recent(QString s = QString(), QString t = QString(), int p = 0,int tt = 0) :
		path(s), title(t), time(p), totalTime(tt) {}

	operator QString() const
	{
		return path;
	}

	bool operator==(const Recent &recent) const
	{
		return (path == recent.path);
	}

	QString path;
	QString title;
	int  time;
	int totalTime;
};

class Comment
{
public:
	int mode = 0;
	int font;
	int color;
	qint64 time;
	qint64 date;
	QString sender;
	QString string;
	bool blocked;

	Comment()
	{
		mode = font = color = time = date = 0;
		blocked = false;
	}

	inline bool operator <(const Comment &o) const
	{
		return time < o.time;
	}

	inline bool operator==(const Comment &o) const
	{
		return mode == o.mode 
			&& font == o.font 
			&& color == o.color 
			&& qFuzzyCompare((float)time, (float)o.time) 
			&& date == o.date 
			&& sender == o.sender 
			&& string == o.string;
	}

	inline bool isLocal() const
	{
		return date == 0 && sender.isEmpty();
	}

	inline bool isEmpty() const
	{
		return mode == 0 
			&& font == 0 
			&& color == 0 
			&& time == 0 
			&& date == 0 
			&& sender.isEmpty() 
			&& string.isEmpty() 
			&& !blocked;
	}
};

inline uint qHash(const Comment &c, uint seed = 0)
{
	uint h = qHash(c.mode, seed);
	h = (h << 1) ^ qHash(c.font, seed);
	h = (h << 1) ^ qHash(c.color, seed);
	h = (h << 1) ^ qHash(c.date, seed);
	h = (h << 1) ^ qHash(c.sender, seed);
	h = (h << 1) ^ qHash(c.string, seed);
	return h;
}

class Record
{
public:
	bool full;
	qint64 delay;
	qint64 limit;
	QString source;
	QString string;
	QString access;
	QVector<Comment> danmaku;

	Record()
	{
		full = false;
		delay = limit = 0;
	}
};

class MessageWidget :public QWidget {
public:
	MessageWidget(QWidget* parent = nullptr, const QString& text = "",
		const QSize& size = QSize(200, 80), int timeout = 3000);
	//
	void setText(const QString& text);
	//
	void setSize(const QSize& size);
	//
	void setRect(const QRect& rect);
	//
	void setTimeOut(int timeout);
	//
	void show();
private:
	QLabel * m_label;
	QRect m_rect;
	QSize m_size;
	int m_timeout;
};


// No need to be a class, like global function and constant value
// but enclosed in a namespace
namespace Utils
{
	enum Site
	{
		Unknown,
		Bilibili,
		AcFun,
		Tudou,
		Letv,
		AcfunLocalizer,
		Niconico,
		TuCao,
		ASS
	};

	enum Type
	{
		Video = 1,
		Audio = 2,
		Subtitle = 4,
		Danmaku = 8
	};

	Site parseSite(QString url);
	void setCenter(QWidget *widget);
	void setGround(QWidget *widget, QColor color);
	QString defaultFont(bool monospace = false);
	QString customUrl(Site site);
	QString decodeTxt(const QByteArray &data);
	QString decodeXml(QString string, bool fast = false);
	QString decodeXml(QStringRef ref, bool fast = false);
	QStringList getSuffix(int type, QString format = QString());
	double evaluate(QString expression);
	//
	void MessageShow(QWidget* parent = nullptr, const QString& text = "",
		const QSize& size = QSize(200, 80), int timeout = 1500);
	//
		// platform specific
	QString VersionFileUrl();

	// bool DimLightsSupported();
	// void SetAlwaysOnTop(WId wid, bool);
	QString SettingsLocation();

	bool IsValidFile(QString path);
	bool IsValidLocation(QString loc); // combined file and url

	void ShowInFolder(QString path, QString file);

	QString MonospaceFont();

	// common
	bool IsValidUrl(QString url);

	QString FormatTime(int time);
	QString FormatRelativeTime(int time);
	QString FormatNumber(int val, int length);
	QString FormatNumberWithAmpersand(int val, int length);
	QString HumanSize(qint64);
	QString ShortenPathToParent(const Recent &recent);
	QStringList ToNativeSeparators(QStringList list);
	QStringList FromNativeSeparators(QStringList list);
	int GCD(int v, int u);
	QString Ratio(int w, int h);
	QString readFile(QString filePath);
	bool writeFile(QString filePath,QString content);
	//
	void asyncWebRequest(QString url, QString method = "get",
		QByteArray postData = "", std::function<void(const QString&)> callback = nullptr);
	//
	void asyncDownload(QString url,QString filePath, std::function<void(const QString&)> callback = nullptr);
	//
	QString filePath(QString fileName);
	//
	QString fileHash(QString fileName);
}
