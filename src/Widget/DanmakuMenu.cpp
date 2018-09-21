#include "DanmakuMenu.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>

#include "Utils.h"

DanmakuMenu::DanmakuMenu(QWidget *parent)
	: QMenu(parent)
{
	//
	ui.setupUi(this);
	//
	setWindowFlag(Qt::FramelessWindowHint, true);
	setAttribute(Qt::WA_TranslucentBackground);
	setStyleSheet("background-color: rgba(0, 0, 0, 128);border-radius:4px;");
	//
}

DanmakuMenu::~DanmakuMenu()
{
}

bool DanmakuMenu::isSmallFont()
{
	return ui.smallRadioButton->isChecked();
}

QColor DanmakuMenu::selectColor()
{
	QRadioButton* button = qobject_cast<QRadioButton*>(ui.colorButtonGroup->checkedButton());
	return button->grab().toImage().pixelColor(button->rect().center());
}

void DanmakuMenu::resetSetting()
{
	ui.bigFontRadioButton->setChecked(true);
	ui.redRadioButton->setChecked(true);
}

void DanmakuMenu::showEvent(QShowEvent * event)
{
	Q_UNUSED(event);
	int x = pos().x() - geometry().width() / 2 + 17;
	int y = pos().y() - geometry().height() - 40;
	move(x,y);
}

bool DanmakuMenu::eventFilter(QObject * obj, QEvent * event)
{
	return QWidget::eventFilter(obj, event);
}

void DanmakuMenu::on_resetButton_clicked()
{
	resetSetting();
}