#pragma once

#include <QWidget>
#include <QPointer>
#include <QMenu>
#include "ui_RecentWidgetItem.h"

#include "Utils.h"

class RecentWidgetItem : public QWidget
{

	Q_OBJECT
public:
	RecentWidgetItem(QWidget *parent = Q_NULLPTR);
	~RecentWidgetItem();

	void setRecent(Recent const& recent);
	Recent& recent();
	void setIsSelect(bool isSelect);
	bool isSelect();
	void setTime(int time, int totalTime);
Q_SIGNALS:
	void watchFinished(RecentWidgetItem*);
protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
protected Q_SLOTS:

private:
	Ui::RecentWidgetItem ui;
	//
	Recent m_recent;
	bool m_isSelect = false;
};