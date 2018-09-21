#include "SpeedMenu.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "Utils.h"

SpeedMenu::SpeedMenu(QWidget *parent)
	: QMenu(parent)
{
	//
	ui.setupUi(this);
	//
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setStyleSheet("background-color: rgba(0, 0, 0, 128);border-radius:4px;");
	//
	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listWidget->setCurrentRow(3);
}

SpeedMenu::~SpeedMenu()
{
}

void SpeedMenu::showEvent(QShowEvent * event)
{
	Q_UNUSED(event);
	int x = pos().x() - geometry().width() / 2 + 17;
	int y = pos().y() - geometry().height() - 40;
	move(x,y);
}

bool SpeedMenu::eventFilter(QObject * obj, QEvent * event)
{
	return QWidget::eventFilter(obj, event);
}


void SpeedMenu::on_listWidget_itemClicked(QListWidgetItem *item)
{
	emit selectText(item->text());
}