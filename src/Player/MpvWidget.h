#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <mpv/client.h>

#include "MpvTypes.h"

#define MPV_REPLY_COMMAND 1
#define MPV_REPLY_PROPERTY 2

#define USE_MPV_OPENGL 1

#include <QtWidgets/QOpenGLWidget>
#include <mpv/opengl_cb.h>

class MpvWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	void Initialize();
public:
    MpvWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~MpvWidget();
	//
	void seekRelativeTime(double time);
	void setSubtitleFile(QString f);
	void setKeepAspect(bool isKeepAspect);
	void startABLoop(double start, double end);
	void stopABLoop();
	bool isOpenGLInited();
	//
	void setIsHwdec(bool isHwdec);
	//
	QString getRealTime();
Q_SIGNALS:
    void durationChanged(int value);
    void positionChanged(int value);
    void initializeFinished();
	void tryToGetTime();
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	//
	void handle_mpv_event(mpv_event *event);
	static void on_update(void *ctx);
private Q_SLOTS:
    void swapped();
    void maybeUpdate();
	void on_mpv_events();
private:
    mpv_handle* mpv = nullptr;
    mpv_opengl_cb_context *mpv_gl = nullptr;
	qreal m_dpr = 1;
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
	bool isFileExists(QString);

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

signals:
	void fileInfoChanged(const Mpv::FileInfo&);
	void trackListChanged(const QList<Mpv::Track>&);
	void chaptersChanged(const QList<Mpv::Chapter>&);
	void videoParamsChanged(const Mpv::VideoParams&);
	void audioParamsChanged(const Mpv::AudioParams&);
	void playStateChanged(Mpv::PlayState);
	void fileLoading(int, int);
	void fileChanged(QString);
	void voChanged(QString);
	void msgLevelChanged(QString);
	void speedChanged(double);
	void timeChanged(double);
	void reachEndFile(QString file);
	void fileCannotOpen();
	void volumeChanged(int);
	void vidChanged(int);
	void aidChanged(int);
	void sidChanged(int);
	void subtitleVisibilityChanged(bool);
	void muteChanged(bool);

	void messageSignal(QString m);

private slots:
	void setPlayState(Mpv::PlayState s);
	void setFile(QString s);
	void setVo(QString s);
	void setMsgLevel(QString s);
	void setSpeed(double d);
	void setTime(double i);
	void setVolume(int i);
	void setVid(int i);
	void setAid(int i);
	void setSid(int i);
	void setSubtitleVisibility(bool b);
	void setMute(bool b);

private:
	// variables
	Mpv::FileInfo m_fileInfo;
	Mpv::PlayState m_playState = Mpv::Idle;
	//
	QString m_file;
	QString m_path;
	QString m_vo;
	QString m_msgLevel;
	//
	QString m_subtitleFile;
	//
	double m_speed = 1;
	double m_lastTime = 0;
	double m_volume = 100;
	//
	double m_vid;
	double m_aid;
	double m_sid;
	//
	bool m_isSubtitleVisible = true;
	bool m_isMute = false;
	//
	bool m_isOpenGLInited = false;
	bool m_isUseOpenGL_CB = false;
};





