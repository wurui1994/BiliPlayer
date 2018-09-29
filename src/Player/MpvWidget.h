#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <mpv/client.h>

#include "MpvTypes.h"

#define MPV_REPLY_COMMAND 1
#define MPV_REPLY_PROPERTY 2

#ifdef Q_OS_MAC
#define USE_MPV_OPENGL 1
#else
#define USE_MPV_OPENGL 1
#endif

#if USE_MPV_OPENGL
#include <QtWidgets/QOpenGLWidget>
#include <mpv/opengl_cb.h>
#endif

#if USE_MPV_OPENGL
class MpvWidget : public QOpenGLWidget
#else
class MpvWidget : public QWidget
#endif
{
	Q_OBJECT
public:
	void Initialize();
public:
#if USE_MPV_OPENGL
    MpvWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
#else
	MpvWidget(QWidget *parent = 0);
#endif
    ~MpvWidget();
	//
	QString getRealTime();

	void seekRelativeTime(double time);
	void setSubtitleFile(QString f);
	void setKeepAspect(bool isKeepAspect);
	void startABLoop(double start, double end);
	void stopABLoop();
	bool isOpenGLInited();
Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
    void initializeFinished();
	void tryToGetTime();
protected:
#if USE_MPV_OPENGL
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
#endif
private Q_SLOTS:
#if USE_MPV_OPENGL
    void swapped();
    void maybeUpdate();
#endif
	void on_mpv_events();
private:
    void handle_mpv_event(mpv_event *event);
#if USE_MPV_OPENGL
    static void on_update(void *ctx);
#endif

    mpv_handle* mpv = nullptr;
#if USE_MPV_OPENGL
    mpv_opengl_cb_context *mpv_gl = nullptr;
	qreal m_dpr = 1;
#endif

public:
	Mpv::FileInfo getFileInfo();
	Mpv::PlayState getPlayState();
	//
	QString getFile();
	QString getPath();
	//
	QString getVo();
	QString getMsgLevel();
	//
	double getSpeed();
	int getTime();
	int getRemainTime();
	int getVolume();
	int getVid();
	int getAid();
	int getSid();
	//
	bool getMute();
	bool getSubtitleVisible();

	// Speed
	void incSpeed10();
	void decSpeed10();
	void doubleSpeed();
	void halveSpeed();
	void normalSpeed();

	//
	void setRotate(int angle = 0);

protected:
	bool FileExists(QString);

public slots:
	void LoadFile(QString);
	bool PlayFile(QString);
	//
	void Play();
	void Pause();
	void Stop();
	void StopUnload();
	void PlayPause();
	void Restart();
	void Rewind();
	void Mute(bool);
	//
	void Seek(double pos, bool relative = false, bool osd = false);
	int Relative(int pos);
	void FrameStep();
	void FrameBackStep();

	void Chapter(int);
	void NextChapter();
	void PreviousChapter();

	void Volume(int, bool osd = false);
	void Speed(double);
	void Aspect(QString);
	void Vid(int);
	void Aid(int);
	void Sid(int);

	void AddSubtitleTrack(QString);
	void AddAudioTrack(QString);
	void ShowSubtitles(bool);
	void SubtitleScale(double scale, bool relative = false);

	void Deinterlace(bool);
	void Interpolate(bool);
	void Vo(QString);

	void MsgLevel(QString level);

	void ShowText(QString text, int duration = 1000);

	void LoadTracks();
	void LoadChapters();
	void LoadVideoParams();
	void LoadAudioParams();
	void LoadMetadata();

	void Command(const QStringList &strlist);
	void SetOption(QString key, QString val);

	void LoadFileInfo();

protected slots:
	void OpenFile(QString);

	void SetProperties();

	void AsyncCommand(const char *args[]);
	void Command(const char *args[]);
	void HandleErrorCode(int);

private slots:
	void setPlayState(Mpv::PlayState s) { emit playStateChanged(playState = s); }
	void setFile(QString s) { emit fileChanged(file = s); }
	void setVo(QString s) { emit voChanged(vo = s); }
	void setMsgLevel(QString s) { emit msgLevelChanged(msgLevel = s); }
	void setSpeed(double d) { emit speedChanged(speed = d); }
	void setTime(double i) { emit timeChanged(time = i); }
	void setMsecsTime(QString s) { emit msecsTimeChanged(s); }
	void setVolume(int i) { emit volumeChanged(volume = i); }
	void setIndex(int i) { emit indexChanged(index = i); }
	void setVid(int i) { emit vidChanged(vid = i); }
	void setAid(int i) { emit aidChanged(aid = i); }
	void setSid(int i) { emit sidChanged(sid = i); }
	void setSubtitleVisibility(bool b) { emit subtitleVisibilityChanged(subtitleVisibility = b); }
	void setMute(bool b) { if (mute != b) emit muteChanged(mute = b); }

signals:
	void fileInfoChanged(const Mpv::FileInfo&);
	void trackListChanged(const QList<Mpv::Track>&);
	void chaptersChanged(const QList<Mpv::Chapter>&);
	void videoParamsChanged(const Mpv::VideoParams&);
	void audioParamsChanged(const Mpv::AudioParams&);
	void playStateChanged(Mpv::PlayState);
	void fileChanging(int, int);
	void fileChanged(QString);
	void voChanged(QString);
	void msgLevelChanged(QString);
	void speedChanged(double);
	void timeChanged(double);
	void msecsTimeChanged(QString);
	void reachEndFile(QString file);
	void fileCannotOpen();
	void volumeChanged(int);
	void indexChanged(int);
	void vidChanged(int);
	void aidChanged(int);
	void sidChanged(int);
	void debugChanged(bool);
	void subtitleVisibilityChanged(bool);
	void muteChanged(bool);

	void messageSignal(QString m);
	void showStatusText(QString text, int duration);

	void onShowText(QString text, int duration);
private:
	//mpv_handle * m_mpv = nullptr;

	// variables
	Mpv::PlayState playState = Mpv::Idle;
	Mpv::FileInfo fileInfo;
	QString     file,
		path,
		suffix,
		vo,
		msgLevel;
	double      speed = 1;
	double         time = 0.0,
		lastTime = 0,
		volume = 100,
		index = 0,
		vid,
		aid,
		sid;
	double lastMsecsTime = 0.0;
	bool        init = false,
		playlistVisible = false,
		subtitleVisibility = true,
		mute = false;
	//
	QString subtitleFile;
	//
	bool m_isOpenGLInited = false;
	bool m_isUseOpenGL_CB = false;
};





