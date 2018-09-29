#pragma once

#include <QtWidgets/QtWidgets>
#include "Parse.h"

#include "SettingDialog.h"

class Danmaku;
class ARender;

class Window :public QWidget
{
	Q_OBJECT
public:
	explicit Window(QWidget *parent = 0);
	~Window();
    static Window *instance();
	bool isDanmakuEmpty();
	void sendDanmaku(QString source, QString text,
		QColor color = Qt::red,int fontSize = 25);
	void setDanmakuTime(qint64 time);
	void setDanmakuVisiable(bool isVisible);
	void loadDanmaku(QString filePath);
	//
	void setupConnect();
	//
	void showSettingDialog();

protected:
	virtual void closeEvent(QCloseEvent *e) override;
	virtual void dragEnterEvent(QDragEnterEvent *e) override;
	virtual void dropEvent(QDropEvent *e) override;

signals:
	void windowFlagsChanged(QFlags<Qt::WindowType>);
	void openMediaFile(QString file);
	void modelReset();
	void settingChanged();
public slots:
	void tryLocal(QString path);
	void tryLocal(QStringList paths);
private:
	static Window *m_instance;
	//
	qint64 m_lastTime = 0;
	qint64 m_lastDanmakuTime = 0;

	//
	QPointer<QDialog> msg;

	Danmaku *danmaku;
	ARender *arender;
	//
	SettingDialog* m_settingDialog;
};
