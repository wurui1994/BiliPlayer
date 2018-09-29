#pragma once

#include <QDialog>
#include <QPointer>
#include <QMenu>
#include "ui_SettingDialog.h"

#include "Utils.h"

class SettingDialog : public QDialog
{
	Q_OBJECT
public:
	SettingDialog(QWidget *parent = Q_NULLPTR);
	~SettingDialog();
	//
	void saveSettings();
	void loadSettings();
	//
Q_SIGNALS:
	void settingChanged();
protected:
	void mousePressEvent(QMouseEvent *event);
	bool eventFilter(QObject *obj, QEvent *event);
protected Q_SLOTS:
	void on_okButton_clicked();
	void on_cancelButton_clicked();
	void on_applyButton_clicked();
private:
	Ui::SettingDialog ui;
};