#include "SettingDialog.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtGui/QMouseEvent>

#include "Setting.h"

SettingDialog::SettingDialog(QWidget *parent)
	: QDialog(parent)
{
	//
	ui.setupUi(this);
	//
	setWindowFlag(Qt::WindowStaysOnTopHint, true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	//
	loadSettings();
}

SettingDialog::~SettingDialog()
{
	
}

void SettingDialog::saveSettings()
{
	setSettingValue("/Video/Hwdec",ui.hwdecCheckBox->isChecked());
	setSettingValue("/Video/AutoPlay", ui.autoPlayCheckBox->isChecked());
	emit settingChanged();
}

void SettingDialog::loadSettings()
{
	ui.hwdecCheckBox->setChecked(getSettingValue("/Video/Hwdec", true));
	ui.autoPlayCheckBox->setChecked(getSettingValue("/Video/AutoPlay", true));
}

void SettingDialog::mousePressEvent(QMouseEvent * event)
{
	Q_UNUSED(event);
}

bool SettingDialog::eventFilter(QObject * obj, QEvent * event)
{
	return QWidget::eventFilter(obj, event);
}

void SettingDialog::on_okButton_clicked()
{
	saveSettings();
	close();
}

void SettingDialog::on_cancelButton_clicked()
{
	close();
}

void SettingDialog::on_applyButton_clicked()
{
	saveSettings();
}
