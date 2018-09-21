#include "Common.h"
#include "Setting.h"

Setting *Setting::m_instance = nullptr;

QReadWriteLock Setting::lock;

Setting *Setting::instance()
{
	return m_instance ? m_instance : new Setting(qApp);
}

Setting::Setting(QObject *parent) :
QObject(parent)
{
	m_instance = this;
}

QJsonObject Setting::settings;

void Setting::load()
{
	QFile settingFile(Utils::filePath("./Settings.json"));
	if (settingFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		settings = QJsonDocument::fromJson(settingFile.readAll()).object();
		settingFile.close();
	}
}

void Setting::save()
{
	emit instance()->aboutToSave();
	QFile settingFile(Utils::filePath("./Settings.json"));
	settingFile.open(QIODevice::WriteOnly | QIODevice::Text);
	settingFile.write(QJsonDocument(settings).toJson());
	settingFile.close();
}

void Setting::setVariant(QString key, QVariant val)
{
	setValue(key, val);
}

QVariant Setting::getVariant(QString key, QVariant val)
{
	return getValue(key, val);
}