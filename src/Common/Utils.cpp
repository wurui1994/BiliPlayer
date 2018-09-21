#include "Common.h"

#include <algorithm>
#include <exception>

Utils::Site Utils::parseSite(QString url)
{
	url = url.toLower();
	if (-1 != url.indexOf("letv"))
	{
		return Letv;
	}
	if (-1 != url.indexOf("tudou"))
	{
		return Tudou;
	}
	if (-1 != url.indexOf("bilibili"))
	{
		return Bilibili;
	}
	if (-1 != url.indexOf("acfun"))
	{
		return AcFun;
	}
	if (-1 != url.indexOf("tucao"))
	{
		return TuCao;
	}
	return Unknown;
}

void Utils::setCenter(QWidget *widget)
{
	QRect rect = widget->geometry();
	QWidget *parent = widget->parentWidget();
	if (!parent)
	{
		rect.moveCenter(QApplication::desktop()->screenGeometry(widget).center());
	}
	else
	{
		if (widget->isWindow())
		{
			QPoint center = parent->geometry().center();
			if ((parent->windowFlags()&Qt::CustomizeWindowHint))
			{
				center.ry() += widget->style()->pixelMetric(QStyle::PM_TitleBarHeight) / 2;
			}
			rect.moveCenter(center);
		}
		else
		{
			rect.moveCenter(parent->rect().center());
		}
	}
	widget->setGeometry(rect);
}

void Utils::setGround(QWidget *widget, QColor color)
{
	widget->setAutoFillBackground(true);
	QPalette palette = widget->palette();
	palette.setColor(QPalette::Window, color);
	widget->setPalette(palette);
}

namespace
{
	template<class T>
	class SStack
	{
	public:
		inline T &top()
		{
			if (isEmpty())
			{
				throw std::runtime_error("token mismatch");
			}
			return stk.top();
		}

		inline T pop()
		{
			if (isEmpty())
			{
				throw std::runtime_error("token mismatch");
			}
			return stk.pop();
		}

		inline void push(const T &i)
		{
			stk.push(i);
		}

		inline bool isEmpty()
		{
			return stk.isEmpty();
		}

	private:
		QStack<T> stk;
	};
}

double Utils::evaluate(QString exp)
{
	auto priority = [](QChar o)
	{
		switch (o.unicode())
		{
		case '(':
			return 1;
		case '+':
		case '-':
			return 2;
		case '*':
		case '/':
			return 3;
		case '+' + 128:
		case '-' + 128:
			return 4;
		case ':':
		case ':' + 128:
			return 5;
		default:
			return 0;
		}
	};
	exp.remove(' ');
	QString pst;
	SStack<QChar> opt;
	int i = 0;
	opt.push('#');
	while (i < exp.length())
	{
		if (exp[i].isDigit() || exp[i] == '.')
		{
			pst.append(exp[i]);
		}
		else
		{
			auto tra = [&]()
			{
				pst.append(' ');
				while (priority(exp[i]) <= priority(opt.top()))
				{
					pst.append(opt.pop());
				}
				opt.push(exp[i]);
			};
			int colon = 0;
			switch (exp[i].unicode())
			{
			case '(':
				opt.push(exp[i]);
				break;
			case ')':
				while (opt.top() != '(')
				{
					pst.append(opt.pop());
				}
				opt.pop();
				break;
			case '+':
			case '-':
			{
				if ((i == 0 || (!exp[i - 1].isDigit() && exp[i - 1] != ')')) 
					&& (i + 1) < exp.length() 
					&& (exp[i + 1].isDigit() || exp[i + 1] == '('))
				{
					exp[i].unicode() += 128;
				}
				tra();
				break;
			}
			case ':':
				switch (colon++)
				{
				case 2:
					exp[i].unicode() += 128;
				case 1:
				case 0:
					break;
				default:
					return 0;
				}
				tra();
				break;
			case '*':
			case '/':
				tra();
				break;
			default:
				return 0;
			}
		}
		++i;
	}
	while (!opt.isEmpty())
	{
		pst.append(opt.pop());
	}
	SStack<double> num;
	i = 0;
	while (pst[i] != '#')
	{
		if (pst[i].isDigit() || pst[i] == '.')
		{
			double n = 0;
			while (pst[i].isDigit())
			{
				n = n * 10 + pst[i++].toLatin1() - '0';
			}
			if (pst[i] == '.')
			{
				++i;
				double d = 1;
				while (pst[i].isDigit())
				{
					n += (d /= 10)*(pst[i++].toLatin1() - '0');
				}
			}
			num.push(n);
		}
		else
		{
			switch (pst[i].unicode())
			{
			case '+' + 128:
				num.push(+num.pop());
				break;
			case '-' + 128:
				num.push(-num.pop());
				break;
			case '+':
			{
				double r = num.pop(), l = num.pop();
				num.push(l + r);
				break;
			}
			case '-':
			{
				double r = num.pop(), l = num.pop();
				num.push(l - r);
				break;
			}
			case '*':
			{
				double r = num.pop(), l = num.pop();
				num.push(l*r);
				break;
			}
			case '/':
			{
				double r = num.pop(), l = num.pop();
				num.push(l / r);
				break;
			}
			case ':':
			{
				double r = num.pop(), l = num.pop();
				num.push(l * 60 + r);
				break;
			}
			case ':' + 128:
			{
				double r = num.pop(), l = num.pop();
				num.push(l * 24 + r);
				break;
			}
			}
			i++;
		}
	}
	return num.top();
}

void Utils::MessageShow(QWidget * parent, const QString & text, const QSize & size, int timeout)
{
	static MessageWidget* lastWidget = nullptr;
	QPointer<MessageWidget> box = new MessageWidget(parent, text, size, timeout);
	if (lastWidget) lastWidget->hide();
	lastWidget = box;
	box->show();
}

QString Utils::defaultFont(bool monospace)
{
	if (monospace)
	{
#ifdef Q_OS_LINUX
		return QStringLiteral("文泉驿等宽正黑");
#endif
#ifdef Q_OS_WIN32
		return QStringLiteral("黑体");
#endif
#ifdef Q_OS_MAC
		return QStringLiteral("华文黑体");
#endif
	}
	else
	{
#ifdef Q_OS_LINUX
		return QStringLiteral("文泉驿正黑");
#endif
#ifdef Q_OS_WIN32
		return QStringLiteral("微软雅黑");
#endif
#ifdef Q_OS_MAC
		return QStringLiteral("华文黑体");
#endif
	}
}

QString Utils::customUrl(Site site)
{
	QString name;
	switch (site)
	{
	case AcFun:
		name = "acfun";
		break;
	case Bilibili:
		name = "bili";
		break;
	case Tudou:
		name = "tudou";
		break;
	case Niconico:
		name = "nico";
		break;
	case TuCao:
		name = "tucao";
		break;
	default:
		return QString();
	}
	QStringList urls, defs;
	defs << "acfun.tv" << "bilibili.com" << "tucao.tv";
	urls = Setting::getValue("/Network/Url", defs.join(';')).split(';', QString::SkipEmptyParts);
	for (QString iter : urls + defs)
	{
		if (iter.toLower().indexOf(name) != -1)
		{
			return iter;
		}
	}
	return QString();
}

QString Utils::decodeTxt(const QByteArray &data)
{
	QTextCodec *codec = QTextCodec::codecForUtfText(data, nullptr);
	if (!codec) 
	{
		QByteArray name;
		QByteArray head = data.left(512).toLower();
		if (head.startsWith("<?xml")) 
		{
			int pos = head.indexOf("encoding=");
			if (pos >= 0) 
			{
				pos += 9;
				if (pos < head.size()) 
				{
					auto c = head.at(pos);
					if ('\"' == c || '\'' == c) 
					{
						++pos;
						name = head.mid(pos, head.indexOf(c, pos) - pos);
					}
				}
			}
		}
		else 
		{
			int pos = head.indexOf("charset=", head.indexOf("meta "));
			if (pos >= 0) 
			{
				pos += 8;
				int end = pos;
				while (++end < head.size()) 
				{
					auto c = head.at(end);
					if (c == '\"' || c == '\'' || c == '>') 
					{
						name = head.mid(pos, end - pos);
						break;
					}
				}
			}
		}
		codec = QTextCodec::codecForName(name);
	}
	if (!codec) 
	{
		codec = QTextCodec::codecForLocale();
	}
	return codec->toUnicode(data);
}

QString Utils::decodeXml(QString string, bool fast)
{
	if (fast) 
	{
		return decodeXml(QStringRef(&string), true);
	}

	QTextDocument text;
	text.setHtml(string);
	return text.toPlainText();
}

namespace
{
	template<char16_t... list>
	struct String;

	template<char16_t tail>
	struct String<tail>
	{
		inline static bool equalTo(const char16_t *string, int offset)
		{
			return string[offset] == tail;
		}
	};

	template<char16_t head, char16_t... rest>
	struct String<head, rest...>
	{
		inline static bool equalTo(const char16_t *string, int offset)
		{
			return string[offset] == head && String<rest...>::equalTo(string, offset + 1);
		}
	};

	template<char16_t... list>
	inline bool equal(const char16_t *string, int length, int offset)
	{
		return offset + (int)sizeof...(list) < length && String<list...>::equalTo(string, offset);
	}

	int decodeHtmlEscape(const char16_t *data, int length, int i, char16_t &c)
	{
		if (i + 1 >= length) 
		{
			return 0;
		}

		switch (data[i]) 
		{
		case 'n':
			// &nbsp;
			if (equal<'b', 's', 'p', ';'>(data, length, i + 1)) 
			{
				c = ' ';
				return 5;
			}
			break;
		case 'l':
			// &lt;
			if (equal<'t', ';'>(data, length, i + 1)) 
			{
				c = '<';
				return 3;
			}
			break;
		case 'g':
			// &gt;
			if (equal<'t', ';'>(data, length, i + 1))
			{
				c = '>';
				return 3;
			}
			break;
		case 'a':
			// &amp;
			if (equal<'m', 'p', ';'>(data, length, i + 1)) 
			{
				c = '&';
				return 4;
			}
			break;
		case 'q':
			// &quot;
			if (equal<'u', 'o', 't', ';'>(data, length, i + 1)) 
			{
				c = '\"';
				return 5;
			}
			break;
		case 'c':
			// &copy;
			if (equal<'o', 'p', 'y', ';'>(data, length, i + 1)) 
			{
				c = u'©';
				return 5;
			}
			break;
		case 'r':
			// &reg;
			if (equal<'e', 'g', ';'>(data, length, i + 1)) 
			{
				c = u'®';
				return 4;
			}
			break;
		case 't':
			// &times;
			if (equal<'i', 'm', 'e', 's', ';'>(data, length, i + 1)) 
			{
				c = u'×';
				return 6;
			}
			break;
		case 'd':
			// &divide;
			if (equal<'i', 'v', 'i', 'd', 'e', ';'>(data, length, i + 1)) 
			{
				c = u'÷';
				return 7;
			}
			break;
		}

		return 0;
	}

	int decodeCharEscape(const char16_t *data, int length, int i, char16_t &c)
	{
		if (i + 1 >= length) 
		{
			return 0;
		}

		switch (data[i]) 
		{
		case 'n':
			c = '\n';
			return 1;
		case 't':
			c = '\t';
			return 1;
		case '\"':
			c = '\"';
			return 1;
		}

		return 0;
	}
}

QString Utils::decodeXml(QStringRef ref, bool fast)
{
	if (!fast) 
	{
		return decodeXml(ref.toString(), false);
	}

	int length = ref.length();
	const char16_t *data = (const char16_t *)ref.data();

	QString fixed;
	fixed.reserve(length);

	int passed = 0;
	const char16_t *head = data;

	for (int i = 0; i < length; ++i) 
	{
		char16_t c = data[i];
		if (c < ' ' && c != '\n') 
		{
			continue;
		}

		bool plain = true;
		switch (c) {
		case '&':
		{
			int p = decodeHtmlEscape(data, length, i + 1, c);
			plain = p == 0;
			i += p;
			break;
		}
		case '/':
		case '\\':
		{
			int p = decodeCharEscape(data, length, i + 1, c);
			plain = p == 0;
			i += p;
			break;
		}
		}
		if (plain) {
			++passed;
		}
		else 
		{
			if (passed > 0) 
			{
				fixed.append((QChar *)head, passed);
				passed = 0;
				head = data + i + 1;
			}
			fixed.append(QChar(c));
		}
	}
	if (passed > 0) {
		fixed.append((QChar *)head, passed);
	}
	return fixed;
}

QStringList Utils::getSuffix(int type, QString format)
{
	QStringList set;
	if (type&Video)
	{
		set << "3g2" << "3gp" << "3gp2" << "3gpp" << "amv" << "asf" << "avi" << "divx" << "drc" << "dv" <<
			"f4v" << "flv" << "gvi" << "gxf" << "hlv" << "iso" << "letv" <<
			"m1v" << "m2t" << "m2ts" << "m2v" << "m4v" << "mkv" << "mov" <<
			"mp2" << "mp2v" << "mp4" << "mp4v" << "mpe" << "mpeg" << "mpeg1" <<
			"mpeg2" << "mpeg4" << "mpg" << "mpv2" << "mts" << "mtv" << "mxf" << "mxg" << "nsv" << "nuv" <<
			"ogg" << "ogm" << "ogv" << "ogx" << "ps" <<
			"rec" << "rm" << "rmvb" << "tod" << "ts" << "tts" << "vob" << "vro" <<
			"webm" << "wm" << "wmv" << "wtv" << "xesc";
	}
	if (type&Audio)
	{
		int size = set.size();
		set << "3ga" << "669" << "a52" << "aac" << "ac3" << "adt" << "adts" << "aif" << "aifc" << "aiff" <<
			"amr" << "aob" << "ape" << "awb" << "caf" << "dts" << "flac" << "it" << "kar" <<
			"m4a" << "m4p" << "m5p" << "mka" << "mlp" << "mod" << "mp1" << "mp2" << "mp3" << "mpa" << "mpc" << "mpga" <<
			"oga" << "ogg" << "oma" << "opus" << "qcp" << "ra" << "rmi" << "s3m" << "spx" << "thd" << "tta" <<
			"voc" << "vqf" << "w64" << "wav" << "wma" << "wv" << "xa" << "xm";
		std::inplace_merge(set.begin(), set.begin() + size, set.end());
	}
	if (type&Subtitle)
	{
		int size = set.size();
		set << "aqt" << "ass" << "cdg" << "dks" << "idx" << "jss" << "mks" << "mpl2" << "pjs" << "psb" << "rt" <<
			"smi" << "smil" << "srt" << "ssa" << "stl" << "sub" << "txt" << "usf" << "utf";
		std::inplace_merge(set.begin(), set.begin() + size, set.end());
	}
	if (type&Danmaku)
	{
		int size = set.size();
		set << "json" << "xml";
		std::inplace_merge(set.begin(), set.begin() + size, set.end());
	}
	if (!format.isEmpty())
	{
		for (QString &iter : set)
		{
			iter = format.arg(iter);
		}
	}
	return set;
}

MessageWidget::MessageWidget(QWidget * parent, const QString & text, const QSize & size, int timeout)
	:QWidget(parent), m_label(nullptr)
{
	m_timeout = timeout;
	if (parent)
		m_rect = parent->geometry();
	else
		m_rect = qApp->desktop()->availableGeometry();
	m_size = size;

	//setWindowOpacity(0.8);
	this->setWindowFlag(Qt::FramelessWindowHint, true);
	this->setWindowFlag(Qt::WindowStaysOnTopHint, true);
	this->setAttribute(Qt::WA_TranslucentBackground, true);

	m_label = new QLabel(this);
	m_label->setText(text);
	m_label->setAlignment(Qt::AlignCenter);
	m_label->setStyleSheet("QLabel {color:black;"
		"background-color:rgba(255,255,255,160);border: 0.5px solid gray; border-radius: 10px;}");
	//
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(m_label);
	layout->setMargin(0);
	//
	this->setLayout(layout);
}

void MessageWidget::setText(const QString & text)
{
	if (m_label)
	{
		m_label->setText(text);
	}
}

void MessageWidget::setSize(const QSize & size)
{
	m_size = size;
}

void MessageWidget::setRect(const QRect & rect)
{
	m_rect = rect;
}

void MessageWidget::setTimeOut(int timeout)
{
	m_timeout = timeout;
}

void MessageWidget::show()
{
	if (this->windowFlags().testFlag(Qt::Dialog))
	{

	}
	else
	{
		this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,
			Qt::AlignCenter, m_size, m_rect));
	}
	this->setMinimumSize(m_size);
	QTimer::singleShot(m_timeout, this, [this]()
	{
		QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
		this->setGraphicsEffect(effect);
		QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
		animation->setDuration(350);
		animation->setStartValue(0.5);
		animation->setEndValue(0);
		animation->setEasingCurve(QEasingCurve::OutBack);
		animation->start(QPropertyAnimation::DeleteWhenStopped);
		QObject::connect(animation, SIGNAL(finished()), this, SLOT(hide()));
	});
	QWidget::show();
}

#include <QTime>
#include <QStringListIterator>
#include <QDir>
#include <QApplication>
#include <QRegExp>
#include <QProcess>
#include <QDir>

namespace Utils
{
	bool IsValidUrl(QString url)
	{
		QRegExp rx("^[a-z]{2,}://", Qt::CaseInsensitive); // url
		return (rx.indexIn(url) != -1);
	}

	QString FormatTime(int _time)
	{
		QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
		return time.toString("hh:mm:ss");   // seconds
	}

	QString FormatRelativeTime(int _time)
	{
		QString prefix;
		if (_time < 0)
		{
			prefix = "-";
			_time = -_time;
		}
		else
			prefix = "+";
		QTime time = QTime::fromMSecsSinceStartOfDay(_time * 1000);
		if (_time >= 3600) // hours
			return prefix + time.toString("h:mm:ss");
		if (_time >= 60)   // minutes
			return prefix + time.toString("mm:ss");
		return prefix + time.toString("0:ss");   // seconds
	}

	QString FormatNumber(int val, int length)
	{
		if (length < 10)
			return QString::number(val);
		else if (length < 100)
			return QString("%1").arg(val, 2, 10, QChar('0'));
		else
			return QString("%1").arg(val, 3, 10, QChar('0'));
	}

	QString FormatNumberWithAmpersand(int val, int length)
	{
		if (length < 10)
			return "&" + QString::number(val);
		else if (length < 100)
		{
			if (val < 10)
				return "0&" + QString::number(val);
			return QString("%1").arg(val, 2, 10, QChar('0'));
		}
		else
		{
			if (val < 10)
				return "00&" + QString::number(val);
			return QString("%1").arg(val, 3, 10, QChar('0'));
		}
	}

	QString HumanSize(qint64 size)
	{
		// taken from http://comments.gmane.org/gmane.comp.lib.qt.general/34914
		float num = size;
		QStringList list;
		list << "KB" << "MB" << "GB" << "TB";

		QStringListIterator i(list);
		QString unit("bytes");

		while (num >= 1024.0 && i.hasNext())
		{
			unit = i.next();
			num /= 1024.0;
		}
		return QString().setNum(num, 'f', 2) + " " + unit;
	}

	QString ShortenPathToParent(const Recent &recent)
	{
		const int long_name = 100;
		if (recent.title != QString())
			return QString("%0 (%1)").arg(recent.title, recent.path);
		QString p = QDir::fromNativeSeparators(recent.path);
		int i = p.lastIndexOf('/');
		if (i != -1)
		{
			int j = p.lastIndexOf('/', i - 1);
			if (j != -1)
			{
				QString parent = p.mid(j + 1, i - j - 1),
					file = p.mid(i + 1);
				// todo: smarter trimming
				if (parent.length() > long_name)
				{
					parent.truncate(long_name);
					parent += "..";
				}
				if (file.length() > long_name)
				{
					file.truncate(long_name);
					i = p.lastIndexOf('.');
					file += "..";
					if (i != -1)
					{
						QString ext = p.mid(i);
						file.truncate(file.length() - ext.length());
						file += ext; // add the extension back
					}
				}
				return QDir::toNativeSeparators(parent + "/" + file);
			}
		}
		return QDir::toNativeSeparators(recent.path);
	}

	QStringList ToNativeSeparators(QStringList list)
	{
		QStringList ret;
		for (auto element : list)
		{
			if (Utils::IsValidLocation(element))
				ret.push_back(element);
			else
				ret.push_back(QDir::toNativeSeparators(element));
		}
		return ret;
	}

	QStringList FromNativeSeparators(QStringList list)
	{
		QStringList ret;
		for (auto element : list)
			ret.push_back(QDir::fromNativeSeparators(element));
		return ret;
	}

	int GCD(int u, int v)
	{
		int shift;
		if (u == 0) return v;
		if (v == 0) return u;
		for (shift = 0; ((u | v) & 1) == 0; ++shift)
		{
			u >>= 1;
			v >>= 1;
		}
		while ((u & 1) == 0)
			u >>= 1;
		do
		{
			while ((v & 1) == 0)
				v >>= 1;
			if (u > v)
			{
				unsigned int t = v;
				v = u;
				u = t;
			}
			v = v - u;
		} while (v != 0);
		return u << shift;
	}

	QString Ratio(int w, int h)
	{
		int gcd = GCD(w, h);
		if (gcd == 0)
			return "0:0";
		return QString("%0:%1").arg(QString::number(w / gcd), QString::number(h / gcd));
	}

	QString SettingsLocation()
	{
		// saves to $(application directory)\${SETTINGS_FILE}.ini
		return QString("%0\\%1.ini").arg(QApplication::applicationDirPath(), "setting");
	}

	bool IsValidFile(QString path)
	{
		QRegExp rx("^(\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // relative path, network location, drive
		return (rx.indexIn(path) != -1);
	}

	bool IsValidLocation(QString loc)
	{
		QRegExp rx("^([a-z]{2,}://|\\.{1,2}|[a-z]:|\\\\\\\\)", Qt::CaseInsensitive); // url, relative path, network location, drive
		return (rx.indexIn(loc) != -1);
	}

	void ShowInFolder(QString path, QString file)
	{
		QProcess::startDetached("explorer.exe", QStringList{ "/select,", path + file });
	}

	QString MonospaceFont()
	{
		return "Consolas";
	}

	QString readFile(QString filePath)
	{
		QString content;
		QFile file(filePath);
		file.open(QIODevice::ReadOnly);
		if (file.isOpen())
		{
			content = file.readAll();
			file.close();
		}
		return content;
	}
	//
	bool writeFile(QString filePath, QString content)
	{
		QFile file(filePath);
		file.open(QIODevice::WriteOnly);
		if (file.isOpen())
		{
			file.write(content.toUtf8());
			file.close();
		}
		else
		{
			return false;
		}
		return true;
	}
	void asyncWebRequest(QString url, QString method, QByteArray postData, std::function<void(const QString&)> callback)
	{
		static QNetworkAccessManager networkManager;
		QNetworkRequest request;

		request.setUrl(QUrl(url));

		QNetworkReply* reply;
		if (method == "get")
		{
			reply = networkManager.get(request);
		}
		else if (method == "post")
		{
			request.setHeader(QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");
			reply = networkManager.post(request, postData);
		}
		else
		{
			qDebug() << "method not support.";
			if (callback)
			{
				callback(QString("method not support."));
			}
			return;
		}
		//
		QObject::connect(reply, &QNetworkReply::finished, &networkManager, [=]()
		{
			//
			if (reply->error() > 0)
			{
				qDebug() << "error";// handle error
				if (callback)
				{
					callback(QString("error"));
				}
				return;
			}
			else
			{
				int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

				//
				if (statusCode >= 200 && statusCode < 300)
				{
					if (callback)
					{
						callback(QString(reply->readAll()));
					}
					return;
				}
				if (callback)
				{
					callback(QString::number(statusCode));
				}
				return;
			}
		});
	}
	void asyncDownload(QString url,QString filePath, std::function<void(const QString&)> callback)
	{
		static QNetworkAccessManager networkManager;
		//
		QFile::remove(filePath);
		//
		QPointer<QFile> file =  new QFile(filePath);
		file->open(QIODevice::WriteOnly | QIODevice::Append);
		if (!file->isOpen())
		{
			file->close();
			if (callback)
			{
				callback(QString("file open error"));
			}
			return;
		}
		//
		QNetworkReply* reply = networkManager.get(QNetworkRequest(QUrl(url)));
		//
		QObject::connect(reply, &QNetworkReply::readyRead, &networkManager, [=]()
		{
			file->write(reply->readAll());
			file->flush();
		});
		//
		QObject::connect(reply, &QNetworkReply::finished, &networkManager, [=]()
		{
			file->close();
			//
			int contentLength = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
			QFileInfo info(filePath);
			if (info.size() != contentLength || info.size() < 1024*16)
			{
				QFile::remove(filePath);
				if (callback)
				{
					callback(QString("ContentLength error"));
				}
				return;
			}
			//
			if (reply->error() > 0)
			{
				qDebug() << reply->error();// handle error
				if (callback)
				{
					callback(QString("error"));
				}
				return;
			}
			else
			{
				int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
				//
				if (statusCode >= 200 && statusCode < 300)
				{
					if (callback)
					{
						callback(QString::number(statusCode) + " " + QString("OK"));
					}
					return;
				}
				if (callback)
				{
					callback(QString::number(statusCode) + " " + QString("Unknown"));
				}
				return;
			}
		});
	}
	//
	QString filePath(QString fileName)
	{
		QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		return QDir(path).absoluteFilePath(fileName);
	}
	//
	QString fileHash(QString fileName)
	{
		const qint64 blockSize = 1024;
		QString content;
		QFile file(fileName);
		file.open(QIODevice::ReadOnly);
		qint64 size = file.size();
		if (size < 4096)
		{
			return QString("-1");
		}
		//
		QByteArray bytes;
		//
		if (file.isOpen())
		{
			// start
			file.seek(0);
			bytes += file.read(blockSize);
			// middle
			file.seek(size/2 - blockSize);
			bytes += file.read(2*blockSize);
			// end
			file.seek(size - blockSize);
			bytes += file.read(blockSize);
			//
			file.close();
		}
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(bytes);
		return hash.result().toHex();
	}
}