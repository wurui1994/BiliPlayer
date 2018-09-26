#ifndef PLAYERBAKA_H
#define PLAYERBAKA_H

#include <QMainWindow>
#include <QStringList>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QTranslator>
#include <QHash>
#include <QAction>
#include <QRect>

#include "MpvTypes.h"
#include "MpvWidget.h"
#include "VolumeMenu.h"
#include "SpeedMenu.h"
#include "DanmakuMenu.h"
#include "Window.h"
#include "RecentWidget.h"
#include "AdvertInfoWidget.h"
#include "AdvertWidget.h"
#include "FramelessHelper.h"

#include "ui_Player.h"

#define USE_DANMAKU 1

class MpvWidget;

class Player : public QDialog
{
    Q_OBJECT
public:
    explicit Player(QWidget *parent = 0);
    ~Player();

	void setupWindow();
	void setupConnect();
	void setSubtitleFile(QString f);
	//
	void setupShortcut();
	void genAction(QString const& key, QString const& actionName,
		QKeySequence shortCut, QMenu* menu);
	//
	void setVideoTitle(QString title);
	//
	void loadAdvert(QString path);
	//
	bool isPlaying();
	bool isPause();
	void setPauseOnStart(bool isPauseOnStart);
	void restartOnStop(bool isRestartOnStop);

    QString getLang()          { return lang; }
    QString getOnTop()         { return onTop; }
    int getAutoFit()           { return autoFit; }
    bool getHidePopup()        { return hidePopup; }
    bool getRemaining()        { return remaining; }
    bool getScreenshotDialog() { return screenshotDialog; }
    bool getDebug()            { return debug; }
    bool getGestures()         { return gestures; }
    bool getResume()           { return resume; }
    bool getHideAllControls()  { return hideAllControls; }
    bool isFullScreenMode()    { return hideAllControls || isFullScreen(); }

	void HideOrShowControls(bool isHide);

	void seek(int t);

	void seekRelativeTime(double time);

    Ui::Player ui;
    QImage albumArt;
public slots:
    void Load(QString f = QString());
	void Pause();
	void onFileInfoChange(const Mpv::FileInfo &fileInfo);
	void onAdFileInfoChange(const Mpv::FileInfo &fileInfo);
protected:
	//
	void contextMenuEvent(QContextMenuEvent *event);
	//
    void dragEnterEvent(QDragEnterEvent *event);    // drag file into
    void dropEvent(QDropEvent *event);              // drop file into

    void mousePressEvent(QMouseEvent *event);       // pressed mouse down
    void mouseReleaseEvent(QMouseEvent *event);     // released mouse up
    void mouseMoveEvent(QMouseEvent *event);        // moved mouse on the form
    //void leaveEvent(QEvent *event);                 // mouse left the form
    void mouseDoubleClickEvent(QMouseEvent *event); // double clicked the form
    //bool eventFilter(QObject *obj, QEvent *event);  // event filter (get mouse move events from mpvFrame)
    void wheelEvent(QWheelEvent *event);            // the mouse wheel is used
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
    void SetPlaybackControls(bool enable);          // macro to enable/disable playback controls
	//
private slots:
    void HideAllControls(bool h, bool s = true);    // hideAllControls--s option is used by fullscreen
    void FullScreen(bool fs);                       // makes window fullscreen
    void SetPlayButtonIcon(bool play);
    void SetRemainingLabels(int time);
public slots:
	void on_settingsButton_clicked();
	void on_sendButton_clicked();
	void on_fullscreenButton_clicked(bool isChecked);
	void on_danmakuCheckBox_clicked(bool isChecked);
	void on_localCheckBox_clicked(bool isChecked);
	//
	void on_minButton_clicked();
	void on_maxButton_clicked(bool isChecked);
	void on_closeButton_clicked();
	//
public:
	//
	QMenu* m_menu;
	//
	QMap<QString, std::function<void()>> m_keyFuncMap;
#if USE_DANMAKU
	Window* m_window;
#endif
	RecentWidget* m_recent;
	AdvertInfoWidget* m_advertInfo;
	AdvertWidget* m_advert;

	QTimer* click_timer;
    QPoint m_last_pos;
	QPoint m_lastGlobalPos;

    bool            pathChanged,
                    menuVisible,
                    firstItem       = false,
                    init            = false,
                    playlistState   = false;

	//
	Mpv::PlayState playState = Mpv::Idle;
    // variables
    QList<Recent> m_recentList;
    Recent *current = nullptr;
    QString lang,
            onTop;
    int autoFit;
    bool hidePopup,
         remaining,
         screenshotDialog,
         debug,
         gestures,
         resume,
         hideAllControls = false;
    //QHash<QString, QAction*> commandActionMap;

public slots:
    void setLang(QString s)          { emit langChanged(lang = s); }
    void setOnTop(QString s)         { emit onTopChanged(onTop = s); }
    void setAutoFit(int i)           { emit autoFitChanged(autoFit = i); }
    void setHidePopup(bool b)        { emit hidePopupChanged(hidePopup = b); }
    void setRemaining(bool b)        { emit remainingChanged(remaining = b); }
    void setScreenshotDialog(bool b) { emit screenshotDialogChanged(screenshotDialog = b); }
    //void setDebug(bool b)            { emit debugChanged(debug = b); }
    void setGestures(bool b)         { emit gesturesChanged(gestures = b); }
    void setResume(bool b)           { emit resumeChanged(resume = b); }
    void setHideAllControls(bool b)  { emit hideAllControlsChanged(hideAllControls = b); }
	void setPlayState(Mpv::PlayState s) { emit playStateChanged(playState = s); }
signals:
    void langChanged(QString);
    void onTopChanged(QString);
    void autoFitChanged(int);
    void hidePopupChanged(bool);
    void remainingChanged(bool);
    void screenshotDialogChanged(bool);
    //void debugChanged(bool);
    void gesturesChanged(bool);
    void resumeChanged(bool);
    void hideAllControlsChanged(bool);
	//
    void clicked(QPoint pos);
	void doubleClicked();
	void escape();
	void msecsTimeChanged(QString);
	void totalTime(qint64 t);
	//
	void playStateChanged(Mpv::PlayState);
	//
	void volumeChanged(int);
	//
	void advertPlayEnd();
public:
	bool m_isPauseOnStart = false;
	bool m_isRestartOnStop = false;
	//
	VolumeMenu* volumeMenu;
	SpeedMenu* speedMenu;
	DanmakuMenu* danmakuMenu;
	//
	RecentWidgetItem* m_lastSelect = nullptr;
	//
	qint64 m_totalTime = 0;
	Recent m_lastRecent;
	QString m_file;
	//
	QString originTitle;
	//
	QString m_fileHash;
	//
	FramelessHelper *m_FramelessHelper;
	//
	bool m_bPressed;
	QPoint m_point;
};

#endif // PLAYERBAKA_H
