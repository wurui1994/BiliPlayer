#include "RecentWidgetItem.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "Utils.h"

RecentWidgetItem::RecentWidgetItem(QWidget *parent)
	: QWidget(parent)
{
	//
	ui.setupUi(this);
	//
	setAcceptDrops(true);
	//
	ui.fileNameLabel->setWordWrapMode(QTextOption::WrapAnywhere);
	//ui.fileNameLabel->installEventFilter(this);
	ui.fileNameLabel->viewport()->setCursor(Qt::ArrowCursor);
}

RecentWidgetItem::~RecentWidgetItem()
{
}

void RecentWidgetItem::setRecent(Recent const & recent)
{
	m_recent = recent;
	//
	setToolTip("<nobr>" + m_recent.path);
	//
	QString fileName = QFileInfo(m_recent.path).fileName();
	QString elidedText = QFontMetrics(ui.fileNameLabel->font())
		.elidedText(fileName, Qt::ElideMiddle,480);
	//
	ui.fileNameLabel->setText(elidedText);
	ui.lastTimeLabel->setText(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm"));
	if (m_recent.totalTime > 0)
	{
		//
		QString status = QString("%1%").arg(100.0*m_recent.time / m_recent.totalTime, 0, 'f', 2);
		ui.statusLabel->setText(status);
	}
}

Recent & RecentWidgetItem::recent()
{
	return m_recent;
}

void RecentWidgetItem::setIsSelect(bool isSelect)
{
	m_isSelect = isSelect;
	if (isSelect)
	{
		setStyleSheet("QWidget{color:red;}");
	}
	else
	{
		setStyleSheet("");
	}
}

bool RecentWidgetItem::isSelect()
{
	return m_isSelect;
}

void RecentWidgetItem::setTime(int time, int totalTime)
{
	m_recent.time = time;
	m_recent.totalTime = totalTime;
	if (totalTime > 0)
	{
		//
		QString status = QString("%1%").arg(100.0*time / totalTime, 0, 'f', 2);
		ui.statusLabel->setText(status);
	}
	//
	double percent = 100.0*m_recent.time / m_recent.totalTime;
	if (qAbs(percent - 100) < 1e-3)
	{
		emit watchFinished();
	}
}

void RecentWidgetItem::enterEvent(QEvent * event)
{
	Q_UNUSED(event);
	if (m_isSelect)
	{
		setStyleSheet("QWidget{color:red;}");
	}
	else
	{
		setStyleSheet("QWidget{color:cyan;}");
	}
}

void RecentWidgetItem::leaveEvent(QEvent * event)
{
	Q_UNUSED(event);	
	if (m_isSelect)
	{
		setStyleSheet("QWidget{color:red;}");
	}
	else
	{
		setStyleSheet("");
	}
}

bool RecentWidgetItem::eventFilter(QObject * obj, QEvent * event)
{
	return QWidget::eventFilter(obj,event);
}
