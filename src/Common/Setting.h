#pragma once

#include <QtCore>
#include <exception>

namespace
{
	template<class TypeName>
	TypeName fromJsonValue(QJsonValue v)
	{
		QVariant t = v.toVariant();
		if (!t.canConvert<TypeName>())
		{
			throw std::runtime_error("type missmatch");
		}
		return t.value<TypeName>();
	}

	template<>
	QVariant fromJsonValue(QJsonValue v)
	{
		return v.toVariant();
	}

	template<>
	QJsonArray fromJsonValue(QJsonValue v)
	{
		if (QJsonValue::Array != v.type())
		{
			throw std::runtime_error("type missmatch");
		}
		return v.toArray();
	}

	template<>
	QJsonObject fromJsonValue(QJsonValue v)
	{
		if (QJsonValue::Object != v.type())
		{
			throw std::runtime_error("type missmatch");
		}
		return v.toObject();
	}

	template<class TypeName>
	QJsonValue toJsonValue(TypeName v)
	{
		return QJsonValue(v);
	}

	template<>
	QJsonValue toJsonValue(QVariant v)
	{
		return QJsonValue::fromVariant(v);
	}
}

class Setting :public QObject
{
	Q_OBJECT
public:
	explicit Setting(QObject *parent = 0);

	template<class T>
	static T getValue(QString key, T def = T())
	{
		QStringList tree = key.split('/', QString::SkipEmptyParts);
		QString last = tree.takeLast();
		lock.lockForRead();
		QJsonObject cur = settings;
		lock.unlock();
		QList<QJsonObject> path;
		for (const QString &k : tree)
		{
			path.append(cur);
			cur = cur.value(k).toObject();
		}
		//
		if (cur.contains(last))
		{
			try
			{
				return fromJsonValue<T>(cur.value(last));
			}
			catch (...){}
		}
		QJsonValue val = toJsonValue(def);
		if (!val.isNull())
		{
			cur[last] = val;
			while (!path.isEmpty())
			{
				QJsonObject pre = path.takeLast();
				pre[tree.takeLast()] = cur;
				cur = pre;
			}
			lock.lockForWrite();
			settings = cur;
			lock.unlock();
		}
		return def;
	}

	template<class T>
	static void setValue(QString key, T set)
	{
		QStringList tree = key.split('/', QString::SkipEmptyParts);
		QString last = tree.takeLast();
		QJsonObject cur = settings;
		QList<QJsonObject> path;
		for (const QString &k : tree)
		{
			path.append(cur);
			cur = cur.value(k).toObject();
		}
		cur[last] = toJsonValue(set);
		while (!path.isEmpty())
		{
			QJsonObject pre = path.takeLast();
			pre[tree.takeLast()] = cur;
			cur = pre;
		}
		lock.lockForWrite();
		settings = cur;
		lock.unlock();
	}

	static Setting *instance();

private:
	static Setting *m_instance;
	static QJsonObject settings;
	static QReadWriteLock lock;

signals:
	void aboutToSave();

public slots:
	static void load();
	static void save();
	void setVariant(QString key, QVariant val);
	QVariant getVariant(QString key, QVariant val = QVariant());
};
