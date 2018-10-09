#include "Common.h"
#include "Window.h"
#include "Setting.h"
#include "Application.h"

#include "Danmaku.h"
#include "ARender.h"
#include "Network.h"

Window::Window(QWidget *parent) :
	QWidget(parent)
{
	setAcceptDrops(true);
	setWindowIcon(QIcon(":/Picture/icon.png"));
	//
	setStyleSheet("QWidget{background:transparent}");
	//
	danmaku = Danmaku::instance();
	arender = ARender::instance();
	//
	m_settingDialog = new SettingDialog;

	QVBoxLayout* layout = new QVBoxLayout;
	//
	layout->addWidget(arender);
	setLayout(layout);
	//
	setupConnect();
}

Window::~Window()
{
	delete m_settingDialog;
}

bool Window::isDanmakuEmpty()
{
	return Danmaku::instance()->isDanmakuEmpty();
}

void Window::sendDanmaku(QString source,QString text, QColor color, int fontSize)
{
	Comment c;
	c.mode = 1;
	c.font = fontSize;
	c.time = m_lastTime;
	c.color = color.rgb() & 0xFFFFFF;
	c.string = text;
	Danmaku::instance()->append(source,c);
	//
	QString acolor = color.name().replace("#", "");
	//
	Network::instance().sendDanmaku(text, m_lastTime, 1, acolor);
}

void Window::setDanmakuTime(qint64 time)
{
	if (qAbs(m_lastDanmakuTime - time) > 60000)
	{
		Network::instance().getDanmaku(time);
		//
		m_lastDanmakuTime = time;
	}
	//
	if (qAbs(m_lastTime - time) > 500)
	{
		Danmaku::instance()->jumpToTime(time);
	}
	else
	{
		Danmaku::instance()->setTime(time);
	}
	//
	arender->update();
	m_lastTime = time;
}

void Window::setDanmakuVisiable(bool isVisible)
{
	setVisible(isVisible);
}

void Window::closeEvent(QCloseEvent *e)
{
	if (!isFullScreen() && !isMaximized())
	{
		QString conf = Setting::getValue("/Interface/Size", QString("720,405"));
		QString size = QString("%1,%2").arg(width() * 72 / logicalDpiX()).arg(height() * 72 / logicalDpiY());
		Setting::setValue("/Interface/Size", conf.endsWith(' ') ? conf.trimmed() : size);
	}
	QWidget::closeEvent(e);
}

void Window::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasFormat("text/uri-list")) 
	{
		e->acceptProposedAction();
	}
	QWidget::dragEnterEvent(e);
}

void Window::dropEvent(QDropEvent *e)
{
	if (e->mimeData()->hasFormat("text/uri-list")) 
	{
		QString uriList = e->mimeData()->data("text/uri-list");
		QStringList items = uriList.split('\n', QString::SkipEmptyParts);
		for (const QString &item : items) 
		{
			QString file = QUrl(item).toLocalFile().trimmed();
			tryLocal(file);
		}
	}
}

void Window::loadDanmaku(QString filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return;
	}

	Record load;
	QUrl url = filePath;
	QByteArray data(file.readAll());
	load.source = url.url();
	load.access = url.isLocalFile() ? url.toLocalFile() : load.source;
	load.string = QFileInfo(filePath).fileName();
	load.delay = 0;
	QString head = Utils::decodeTxt(data.left(512));
	if (head.startsWith("[Script Info]")) {
		load.danmaku = Parse::parseComment(data, Utils::ASS);
	}
	else if (!head.startsWith("<?xml")) {
		load.danmaku = Parse::parseComment(data, Utils::AcFun);
	}
	else if (head.indexOf("<packet>") != -1) {
		load.danmaku = Parse::parseComment(data, Utils::Niconico);
	}
	else if (head.indexOf("<i>") != -1) {
		load.danmaku = Parse::parseComment(data, Utils::Bilibili);
		QString i = QRegularExpression("(?<=<chatid>)\\d+(?=</chatid>)").match(head).captured();
		if (!i.isEmpty()) {
			load.source = "http://comment.%1/%2.xml";
			load.source = load.source.arg(Utils::customUrl(Utils::Bilibili)).arg(i);
		}
	}
	else if (head.indexOf("<c>") != -1) {
		load.danmaku = Parse::parseComment(data, Utils::AcfunLocalizer);
	}
	if (load.delay != 0) {
		for (Comment &c : load.danmaku) {
			c.time += load.delay;
		}
	}
	Danmaku::instance()->setRecord(load);
}

void Window::setupConnect()
{
	connect(Danmaku::instance(), &Danmaku::modelReset, [=]()
	{
		emit modelReset();
	});
	//
	connect(m_settingDialog, &SettingDialog::settingChanged, [=]()
	{
		emit settingChanged();
	});
}

void Window::showSettingDialog()
{
	m_settingDialog->show();
}

void Window::tryLocal(QString file)
{
	QFileInfo info(file);
	QString suffix = info.suffix().toLower();
	if (Utils::getSuffix(Utils::Danmaku).contains(suffix))
	{
		loadDanmaku(file);
	}
	else if (Utils::getSuffix(Utils::Subtitle).contains(suffix)) 
	{
		//
	}
	else 
	{
		emit openMediaFile(file);
	}
}

void Window::tryLocal(QStringList paths)
{
	for (const QString &path : paths) 
	{
		tryLocal(path);
	}
}
