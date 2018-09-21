#pragma once

#include <QWidget>
#include <QPointer>
#include <QMenu>
#include "ui_RecentWidget.h"

#include "Utils.h"
#include "RecentWidgetItem.h"

class RecentWidget : public QWidget
{
	Q_OBJECT
public:
	RecentWidget(QWidget *parent = Q_NULLPTR);
	~RecentWidget();

	RecentWidgetItem* addRecent(Recent const& recent);
	void removeRecent(Recent const& recent);
	QList<Recent> recents();
	void saveRecents();
	void loadRecents();
	void resetSelect();
	void clearList();
	bool nextVideo();
	void onNextVideo();
Q_SIGNALS:
	void tryOpenFile(QString filePath);
	void tryOpenRecent(Recent recent);
	void tryClearList();
protected:
	void dragEnterEvent(QDragEnterEvent *e) override;
	void dropEvent(QDropEvent *e) override;
protected Q_SLOTS:
	void on_listWidget_itemClicked(QListWidgetItem *item);
	void on_clearCheckBox_clicked(bool isChecked);
	void on_filterCheckBox_clicked(bool isChecked);
private:
	Ui::RecentWidget ui;
	RecentWidgetItem* m_lastSelect = nullptr;
};