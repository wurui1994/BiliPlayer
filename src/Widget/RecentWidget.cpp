#include "RecentWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

RecentWidget::RecentWidget(QWidget *parent)
	: QWidget(parent)
{
	//
	ui.setupUi(this);
	//
	setAcceptDrops(true);
	//
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground);
	//
	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	//
	loadRecents();
}

RecentWidget::~RecentWidget()
{
	saveRecents();
}

RecentWidgetItem* RecentWidget::addRecent(Recent const & recent)
{
	if (m_lastSelect)m_lastSelect->setIsSelect(false);
	//
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		if (select->recent().path == recent.path)
		{
			select->setIsSelect(true);
			m_lastSelect = select;
			return m_lastSelect;
		}
	}
	//
	RecentWidgetItem* itemWidget = new RecentWidgetItem(ui.listWidget);
	itemWidget->setRecent(recent);
	itemWidget->setIsSelect(true);
	//
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(262, 56));
	ui.listWidget->addItem(item);
	ui.listWidget->setItemWidget(item, itemWidget);
	//
	ui.listWidget->setCurrentItem(item);
	//
	connect(itemWidget, &RecentWidgetItem::watchFinished, [=]()
	{
		if (ui.filterCheckBox->isChecked())
		{
			item->setHidden(true);
		}
		nextVideo();
	});
	//
	m_lastSelect = itemWidget;
	return m_lastSelect;
}

void RecentWidget::removeRecent(Recent const & recent)
{
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		if (select->recent().path == recent.path)
		{
			ui.listWidget->takeItem(i);
			break;
		}
	}
}

QList<Recent> RecentWidget::recents()
{
	QList<Recent> recents;
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		//
		recents << select->recent();
	}
	return recents;
}

void RecentWidget::saveRecents()
{
	QJsonArray recentArray;
	for (auto const& recent : recents())
	{
		QJsonObject recentObject;
		recentObject["path"] = recent.path;
		recentObject["title"] = recent.title;
		recentObject["time"] = recent.time;
		recentObject["totalTime"] = recent.totalTime;
		recentArray << recentObject;
	}
	//
	QJsonObject recentsObject;
	recentsObject["recents"] = recentArray;
	//
	QJsonDocument doc(recentsObject);
	//
	QString filePath = Utils::filePath("./Recents.json");
	//
	Utils::writeFile(filePath, doc.toJson());
}

void RecentWidget::loadRecents()
{
	//
	QString filePath = Utils::filePath("./Recents.json");
	if (!QFileInfo(filePath).exists())
	{
		return;
	}
	QString json = Utils::readFile(filePath);
	QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
	//
	QJsonObject recentsObject = doc.object();
	QJsonArray recentArray = recentsObject["recents"].toArray();
	//
	for (auto const& ele : recentArray)
	{
		QJsonObject  recentObject = ele.toObject();
		QString path = recentObject["path"].toString();
		QString title = recentObject["path"].toString();
		int time = recentObject["time"].toInt();
		int totalTime = recentObject["totalTime"].toInt();
		//
		Recent recent = {path,title,time,totalTime};
		addRecent(recent);
	}
	//
	resetSelect();
}

void RecentWidget::resetSelect()
{
	//
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		select->setIsSelect(false);
	}
	m_lastSelect = nullptr;
}

void RecentWidget::clearList()
{
	ui.listWidget->clear();
	m_lastSelect = nullptr;
}

bool RecentWidget::nextVideo()
{
	//
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		int index = i + 1;
		if (select == m_lastSelect && index < ui.listWidget->count())
		{
			on_listWidget_itemClicked(ui.listWidget->item(index));
			return true;
		}
	}
	return false;
}

void RecentWidget::onNextVideo()
{
	if (nextVideo())
	{
		return;
	}
	//
	if (ui.listWidget->count() > 0)
	{
		on_listWidget_itemClicked(ui.listWidget->item(0));
	}
}

void RecentWidget::dragEnterEvent(QDragEnterEvent *e)
{
	if (e->mimeData()->hasFormat("text/uri-list"))
	{
		e->acceptProposedAction();
	}
	QWidget::dragEnterEvent(e);
}

void RecentWidget::dropEvent(QDropEvent * e)
{
	if (e->mimeData()->hasFormat("text/uri-list"))
	{
		QString uriList = e->mimeData()->data("text/uri-list");
		QStringList items = uriList.split('\n', QString::SkipEmptyParts);
		for (const QString &item : items)
		{
			QString file = QUrl(item).toLocalFile().trimmed();
			emit tryOpenFile(file);
			break;
		}
	}
}

void RecentWidget::on_clearCheckBox_clicked(bool isChecked)
{
	Q_UNUSED(isChecked);
	emit tryClearList();
}

void RecentWidget::on_filterCheckBox_clicked(bool isChecked)
{
	//
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
		Recent recent = select->recent();
		double percent = 100.0*recent.time / recent.totalTime;
		if (qAbs(percent - 100) < 1e-3)
		{
			item->setHidden(isChecked);
		}
	}
}

void RecentWidget::on_listWidget_itemClicked(QListWidgetItem *item)
{
	if (m_lastSelect)m_lastSelect->setIsSelect(false);
	RecentWidgetItem* select = qobject_cast<RecentWidgetItem*>(ui.listWidget->itemWidget(item));
	select->setIsSelect(true);
#if 0
	if (select == m_lastSelect)
	{
		return;
	}
#endif
	//
	m_lastSelect = select;
	//
	emit tryOpenRecent(select->recent());
}