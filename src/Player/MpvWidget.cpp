#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>
#include <QCoreApplication>

#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>

#include <clocale>
#include <stdexcept>

#include "MpvWidget.h"
#include "Utils.h"
#include "Setting.h"

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) 
{
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{
	//
    std::setlocale(LC_NUMERIC, "C");
    mpv = mpv_create();
    if (!mpv)
    {
        qDebug() << "could not create mpv context";
        return;
    }

    if (mpv_initialize(mpv) < 0)
    {
        qDebug() << "could not initialize mpv context";
        return;
    }
	//
	mpv_set_option_string(mpv, "input-cursor", "no");   // no mouse handling
	mpv_set_option_string(mpv, "cursor-autohide", "no");// no cursor-autohide, we handle that
	mpv_set_option_string(mpv, "ytdl", "yes"); // youtube-dl support
	//mpv_set_option_string(mpv, "sub-auto", "no"); // youtube-dl support

	setIsHwdec(getSettingValue("/Video/Hwdec", true));

	m_isUseOpenGL_CB = true;
	//
	if (m_isUseOpenGL_CB)
	{
		mpv_set_option_string(mpv, "vo", "opengl-cb");
	}

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
    {
        qDebug() << "OpenGL not compiled in";
    }
    else
    {
	    mpv_opengl_cb_set_update_callback(mpv_gl, MpvWidget::on_update, (void *)this);
	    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));
    }

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
	//
	// get updates when these properties change
	mpv_observe_property(mpv, 0, "playback-time", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "ao-volume", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "sid", MPV_FORMAT_INT64);
	mpv_observe_property(mpv, 0, "aid", MPV_FORMAT_INT64);
	mpv_observe_property(mpv, 0, "sub-visibility", MPV_FORMAT_FLAG);
	mpv_observe_property(mpv, 0, "ao-mute", MPV_FORMAT_FLAG);
	mpv_observe_property(mpv, 0, "core-idle", MPV_FORMAT_FLAG);
	mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
	//
	mpv_observe_property(mpv, 0, "ab-loop-a", MPV_FORMAT_DOUBLE);
	mpv_observe_property(mpv, 0, "ab-loop-b", MPV_FORMAT_DOUBLE);

	//
    mpv_set_wakeup_callback(mpv, wakeup, this);
}

MpvWidget::~MpvWidget()
{
    if (mpv_gl)
    {
    	qDebug() << "release mpv_gl:" << mpv_gl;
    	makeCurrent();
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);

	    // Until this call is done, we need to make sure the player remains
	    // alive. This is done implicitly with the mpv::qt::Handle instance
	    // in this class.
    	//mpv_opengl_cb_uninit_gl(mpv_gl);
    }
    if (mpv)
    {
#if 0
    	qDebug() << "release mpv:" << mpv;
    	mpv_terminate_destroy(mpv);
#endif
    } 
}

void MpvWidget::initializeGL()
{
	if(!mpv_gl)
	{
		return;
	}
	//
    int r = mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
    {
        qDebug() << "could not initialize OpenGL";
        return;
    }
    else
    {
		m_isOpenGLInited = true;
        emit initializeFinished();
    }
}

void MpvWidget::resizeGL(int w, int h)
{
	m_dpr = devicePixelRatioF();
	QOpenGLWidget::resizeGL(w, h);
}

void MpvWidget::paintGL()
{
	if(!mpv_gl)
	{
		return;
	}
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), m_dpr*width(), -m_dpr*height());
    //
    //update();
#if 0
	emit tryToGetTime();
#endif
}

void MpvWidget::swapped()
{
	if(!mpv_gl)
	{
		return;
	}
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) 
	{
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) 
		{
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) 
	{
    case MPV_EVENT_PROPERTY_CHANGE: 
	{
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) 
		{
            if (prop->format == MPV_FORMAT_DOUBLE) 
			{
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) 
		{
            if (prop->format == MPV_FORMAT_DOUBLE) 
			{
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        } else if (strcmp(prop->name, "ab-loop-b") == 0) 
		{
            if (prop->format == MPV_FORMAT_DOUBLE) 
			{
                //double time = *(double *)prop->data;
                //Q_EMIT durationChanged(time);
            }
        }
		if (QString(prop->name) == "playback-time") // playback-time does the same thing as time-pos but works for streaming media
		{
			if (prop->format == MPV_FORMAT_DOUBLE)
			{
				double secs = *(double*)prop->data;
				secs = qAbs(secs);
				//
				setTime(secs);
			}
		}
		else if (QString(prop->name) == "ao-volume")
		{
			if (prop->format == MPV_FORMAT_DOUBLE)
				setVolume((int)*(double*)prop->data);
		}
		else if (QString(prop->name) == "sid")
		{
			if (prop->format == MPV_FORMAT_INT64)
				setSid(*(int*)prop->data);
		}
		else if (QString(prop->name) == "aid")
		{
			if (prop->format == MPV_FORMAT_INT64)
				setAid(*(int*)prop->data);
		}
		else if (QString(prop->name) == "sub-visibility")
		{
			if (prop->format == MPV_FORMAT_FLAG)
			{
				bool isVisible = (bool)*(unsigned*)prop->data;
				if (isVisible != m_isSubtitleVisible)
				{
					setSubtitleVisibility(isVisible);
				}
			}
		}
		else if (QString(prop->name) == "ao-mute")
		{
			if (prop->format == MPV_FORMAT_FLAG)
				setMute((bool)*(unsigned*)prop->data);
		}
		else if (QString(prop->name) == "core-idle")
		{
			if (prop->format == MPV_FORMAT_FLAG)
			{
#if 0
				if ((bool)*(unsigned*)prop->data && playState == Mpv::Playing)
					ShowText(tr("Buffering..."), 0);
				else
					ShowText(QString(), 0);
#endif
			}
		}
		else if (QString(prop->name) == "paused-for-cache")
		{
			if (prop->format == MPV_FORMAT_FLAG)
			{
#if 0
				if ((bool)*(unsigned*)prop->data && playState == Mpv::Playing)
					ShowText(tr("Your network is slow or stuck, please wait a bit"), 0);
				else
					ShowText(QString(), 0);
#endif
			}
		}
		break;
	}
	case MPV_EVENT_IDLE:
		m_fileInfo.length = 0;
		setTime(0);
		setPlayState(Mpv::Idle);
		break;
		// these two look like they're reversed but they aren't. the names are misleading.
	case MPV_EVENT_START_FILE:
		setPlayState(Mpv::Loaded);
		break;
	case MPV_EVENT_FILE_LOADED:
		setPlayState(Mpv::Started);
		LoadFileInfo();
		SetProperties();
		//
		if (!m_subtitleFile.isEmpty())
		{
			setSubtitleFile(m_subtitleFile);
		}
	case MPV_EVENT_UNPAUSE:
		setPlayState(Mpv::Playing);
		break;
	case MPV_EVENT_PAUSE:
		setPlayState(Mpv::Paused);
		ShowText(QString(), 0);
		break;
	case MPV_EVENT_END_FILE:
		//
		if (m_playState == Mpv::Loaded) 
		{
			ShowText(tr("File couldn't be opened"));
			emit fileCannotOpen();
		}
		//
		if (m_playState == Mpv::Playing)
		{
			//	
			setPlayState(Mpv::Stopped);
			emit reachEndFile(m_file);
		}
		else
		{
			setPlayState(Mpv::Stopped);
		}
		break;
	case MPV_EVENT_SHUTDOWN:
		//QCoreApplication::quit();
		break;
	case MPV_EVENT_LOG_MESSAGE:
	{
		mpv_event_log_message *message = static_cast<mpv_event_log_message*>(event->data);
		if (message != nullptr)
			emit messageSignal(message->text);
		break;
	}
	default: // unhandled events
		break;
	}
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
	update();
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}

Mpv::FileInfo MpvWidget::getFileInfo()
{
	return m_fileInfo;
}

Mpv::PlayState MpvWidget::getPlayState()
{
	return m_playState;
}

QString MpvWidget::getFile()
{
	return m_file;
}

QString MpvWidget::getPath()
{
	return m_path;
}

QString MpvWidget::getVo()
{
	return m_vo;
}

QString MpvWidget::getMsgLevel()
{
	return m_msgLevel;
}

double MpvWidget::getSpeed()
{
	return m_speed;
}

int MpvWidget::getTime()
{
	return m_lastTime;
}

int MpvWidget::getRemainTime()
{
	return m_fileInfo.length - m_lastTime;
}
int MpvWidget::getVolume()
{
	return m_volume;
}

int MpvWidget::getVid()
{
	return m_vid;
}

int MpvWidget::getAid()
{
	return m_aid;
}

int MpvWidget::getSid()
{
	return m_sid;
}

bool MpvWidget::getSubtitleVisible()
{
	return m_isSubtitleVisible;
}

bool MpvWidget::getMute()
{
	return m_isMute;
}


bool MpvWidget::isFileExists(QString f)
{
	if (Utils::IsValidUrl(f)) // web url
		return true;
	return QFile(f).exists();
}

void MpvWidget::LoadFile(QString file)
{
	if (file.startsWith("http://")
		|| file.startsWith("https://"))
	{
		PlayFile(file);
		return;
	}
	//
	QFileInfo info(file);
	if (info.isFile() && info.exists())
	{
		QString filePath = info.absoluteFilePath();
		qDebug() << "Load file:" << filePath;
		PlayFile(filePath);
	}
	else
	{
		Utils::MessageShow(0, tr("File is invalid or not exist."));
	}
}


bool MpvWidget::PlayFile(QString f)
{
	if (f.isEmpty()) // ignore if file doesn't exist
	{
		return false;
	}
	//
	if (m_path.isEmpty()) // web url
	{
		OpenFile(f);
		setFile(f);
		return true;
	}
	//
	QFile qf(m_path + f);
	if (qf.exists())
	{
		OpenFile(m_path + f);
		setFile(f);
		Play();
	}
	else
	{
		ShowText(tr("File no longer exists")); // tell the user
		return false;
	}
	return true;
}

void MpvWidget::Play()
{
	if (m_playState > 0 && mpv)
	{
		int flag = 0;
		mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "pause", MPV_FORMAT_FLAG, &flag);
	}
}

void MpvWidget::Pause()
{
	if (m_playState > 0 && mpv)
	{
		int flag = 1;
		mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "pause", MPV_FORMAT_FLAG, &flag);
	}
}

void MpvWidget::Stop()
{
	const char *args[] = { "stop", NULL };
	AsyncCommand(args);
}

void MpvWidget::StopUnload()
{
	const char *args[] = { "stop", NULL };
	Command(args);
}

void MpvWidget::PlayPause()
{
	if (m_playState < 0) // not playing, play plays the selected playlist file
	{
		PlayFile(m_file);
	}
	else
	{
		const char *args[] = { "cycle", "pause", NULL };
		AsyncCommand(args);
	}
}

void MpvWidget::Restart()
{
	Seek(0);
	Play();
}

void MpvWidget::Rewind()
{
	// if user presses rewind button twice within 3 seconds, stop video
	if (m_lastTime < 3)
	{
		Stop();
	}
	else
	{
		if (m_playState == Mpv::Playing)
			Restart();
		else
			Stop();
	}
}

void MpvWidget::Mute(bool m)
{
	if (m_playState > 0)
	{
		const char *args[] = { "set", "ao-mute", m ? "yes" : "no", NULL };
		AsyncCommand(args);
	}
	else
	{
		setMute(m);
	}
}

void MpvWidget::Seek(double pos, bool relative, bool osd)
{
	if (m_playState > 0)
	{
		if (relative)
		{
			const QByteArray tmp = (((pos >= 0) ? "+" : QString()) + QString::number(pos)).toUtf8();
			if (osd)
			{
				const char *args[] = { "osd-msg", "seek", tmp.constData(), NULL };
				AsyncCommand(args);
			}
			else
			{
				const char *args[] = { "seek", tmp.constData(), NULL };
				AsyncCommand(args);
			}
		}
		else
		{
			const QByteArray tmp = QString::number(pos).toUtf8();
			if (osd)
			{
				const char *args[] = { "osd-msg", "seek", tmp.constData(), "absolute", NULL };
				AsyncCommand(args);
			}
			else
			{
				const char *args[] = { "seek", tmp.constData(), "absolute", NULL };
				AsyncCommand(args);
			}
		}
	}
}

int MpvWidget::Relative(int pos)
{
	int ret = pos - m_lastTime;
	m_lastTime = pos;
	return ret;
}

void MpvWidget::FrameStep()
{
	const char *args[] = { "frame_step", NULL };
	AsyncCommand(args);
}

void MpvWidget::FrameBackStep()
{
	const char *args[] = { "frame_back_step", NULL };
	AsyncCommand(args);
}

void MpvWidget::Chapter(int c)
{
	mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "chapter", MPV_FORMAT_INT64, &c);
}

void MpvWidget::NextChapter()
{
	const char *args[] = { "add", "chapter", "1", NULL };
	AsyncCommand(args);
}

void MpvWidget::PreviousChapter()
{
	const char *args[] = { "add", "chapter", "-1", NULL };
	AsyncCommand(args);
}

void MpvWidget::Volume(int level, bool osd)
{
	if (level > 100) level = 100;
	else if (level < 0) level = 0;

	double v = level;

	mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "ao-volume", MPV_FORMAT_DOUBLE, &v);
	if (osd)
		ShowText(tr("Volume: %0%").arg(QString::number(level)));
}

void MpvWidget::Speed(double d)
{
	if (m_playState > 0)
	{
		const QByteArray tmp = QString::number(d).toUtf8();
		const char *args[] = { "set", "speed", tmp.constData(), NULL };
		Command(args);
	}
	//mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &d);
	setSpeed(d);
}

void MpvWidget::Aspect(QString aspect)
{
	const QByteArray tmp = aspect.toUtf8();
	const char *args[] = { "set", "video-aspect", tmp.constData(), NULL };
	AsyncCommand(args);
}


void MpvWidget::Vid(int vid)
{
	const QByteArray tmp = QString::number(vid).toUtf8();
	const char *args[] = { "set", "vid", tmp.constData(), NULL };
	AsyncCommand(args);
}

void MpvWidget::Aid(int aid)
{
	const QByteArray tmp = QString::number(aid).toUtf8();
	const char *args[] = { "set", "aid", tmp.constData(), NULL };
	AsyncCommand(args);
}

void MpvWidget::Sid(int sid)
{
	const QByteArray tmp = QString::number(sid).toUtf8();
	const char *args[] = { "set", "sid", tmp.constData(), NULL };
	AsyncCommand(args);
}

void MpvWidget::AddSubtitleTrack(QString f)
{
	if (f == QString())
		return;
	const QByteArray tmp = f.toUtf8();
#if 0
	const char *args_remove[] = { "sub-remove", "1" , NULL};
	Command(args_remove);
#endif
	const char *args[] = { "sub-add", tmp.constData(), NULL };
	Command(args);
	// this could be more efficient if we saved tracks in a bst
	auto old = m_fileInfo.tracks; // save the current track-list
	LoadTracks(); // load the new track list
	auto current = m_fileInfo.tracks;
	for (auto track : old)
	{
		// remove the old tracks in current
		current.removeOne(track);
	}
	//
	if(current.size() > 1 )
	{
		Mpv::Track &track = current.first();
		ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), 
			track.title, track.external ? "external" : track.lang));
	}
}

void MpvWidget::AddAudioTrack(QString f)
{
	if (f == QString())
		return;
	const QByteArray tmp = f.toUtf8();
	const char *args[] = { "audio-add", tmp.constData(), NULL };
	Command(args);
	auto old = m_fileInfo.tracks;
	LoadTracks();
	auto current = m_fileInfo.tracks;
	for (auto track : old)
		current.removeOne(track);
	Mpv::Track &track = current.first();
	ShowText(QString("%0: %1 (%2)").arg(QString::number(track.id), 
		track.title, track.external ? "external" : track.lang));
}

void MpvWidget::ShowSubtitles(bool b)
{
	const char *args[] = { "set", "sub-visibility", b ? "yes" : "no", NULL };
	AsyncCommand(args);
}

void MpvWidget::SubtitleScale(double scale, bool relative)
{
	const QByteArray tmp = QString::number(scale).toUtf8();
	const char *args[] = { relative ? "add" : "set", "sub-scale", tmp.constData(), NULL };
	AsyncCommand(args);
}

void MpvWidget::Deinterlace(bool deinterlace)
{
	HandleErrorCode(mpv_set_property_string(mpv, "deinterlace", deinterlace ? "yes" : "auto"));
	ShowText(tr("Deinterlacing: %0").arg(deinterlace ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Interpolate(bool interpolate)
{
	if (m_vo == QString())
		m_vo = mpv_get_property_string(mpv, "current-vo");
	QStringList vos = m_vo.split(',');
	for (auto &o : vos)
	{
		int i = o.indexOf(":interpolation");
		if (interpolate && i == -1)
			o.append(":interpolation");
		else if (i != -1)
			o.remove(i, QString(":interpolation").length());
	}
	setVo(vos.join(','));
	SetOption("vo", m_vo);
	ShowText(tr("Motion Interpolation: %0").arg(interpolate ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Vo(QString o)
{
	setVo(o);
	SetOption("vo", m_vo);
}

void MpvWidget::MsgLevel(QString level)
{
	QByteArray tmp = level.toUtf8();
	mpv_request_log_messages(mpv, tmp.constData());
	setMsgLevel(level);
}

void MpvWidget::ShowText(QString text, int duration)
{
	if (!text.isEmpty())
	{
		Utils::MessageShow(this, text, QSize(200, 80), duration);
	}
}

void MpvWidget::LoadFileInfo()
{
	// get media-title
	m_fileInfo.media_title = mpv_get_property_string(mpv, "media-title");
	// get length
	QString lenString = mpv_get_property_string(mpv, "duration");
	m_fileInfo.length = lenString.toDouble();

	LoadTracks();
	LoadChapters();
	LoadVideoParams();
	LoadAudioParams();
	LoadMetadata();

	emit fileInfoChanged(m_fileInfo);
}

void MpvWidget::LoadTracks()
{
	m_fileInfo.tracks.clear();
	mpv_node node;
	mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_ARRAY)
	{
		for (int i = 0; i < node.u.list->num; i++)
		{
			if (node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
			{
				Mpv::Track track;
				for (int n = 0; n < node.u.list->values[i].u.list->num; n++)
				{
					if (QString(node.u.list->values[i].u.list->keys[n]) == "id")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
							track.id = node.u.list->values[i].u.list->values[n].u.int64;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "type")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							track.type = node.u.list->values[i].u.list->values[n].u.string;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "src-id")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64)
							track.src_id = node.u.list->values[i].u.list->values[n].u.int64;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "title")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							track.title = node.u.list->values[i].u.list->values[n].u.string;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "lang")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							track.lang = node.u.list->values[i].u.list->values[n].u.string;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "albumart")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
							track.albumart = node.u.list->values[i].u.list->values[n].u.flag;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "default")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
							track._default = node.u.list->values[i].u.list->values[n].u.flag;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "external")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_FLAG)
							track.external = node.u.list->values[i].u.list->values[n].u.flag;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "external-filename")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							track.external_filename = node.u.list->values[i].u.list->values[n].u.string;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "codec")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							track.codec = node.u.list->values[i].u.list->values[n].u.string;
					}
				}
				m_fileInfo.tracks.push_back(track);
			}
		}
	}

	emit trackListChanged(m_fileInfo.tracks);
}

void MpvWidget::LoadChapters()
{
	m_fileInfo.chapters.clear();
	mpv_node node;
	mpv_get_property(mpv, "chapter-list", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_ARRAY)
	{
		for (int i = 0; i < node.u.list->num; i++)
		{
			if (node.u.list->values[i].format == MPV_FORMAT_NODE_MAP)
			{
				Mpv::Chapter ch;
				for (int n = 0; n < node.u.list->values[i].u.list->num; n++)
				{
					if (QString(node.u.list->values[i].u.list->keys[n]) == "title")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING)
							ch.title = node.u.list->values[i].u.list->values[n].u.string;
					}
					else if (QString(node.u.list->values[i].u.list->keys[n]) == "time")
					{
						if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_DOUBLE)
							ch.time = (int)node.u.list->values[i].u.list->values[n].u.double_;
					}
				}
				m_fileInfo.chapters.push_back(ch);
			}
		}
	}
	emit chaptersChanged(m_fileInfo.chapters);
}

void MpvWidget::LoadVideoParams()
{
	m_fileInfo.video_params.codec = mpv_get_property_string(mpv, "video-codec");
	//
	mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &m_fileInfo.video_params.width);
	mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &m_fileInfo.video_params.height);
	mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &m_fileInfo.video_params.dwidth);
	mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &m_fileInfo.video_params.dheight);
	// though this has become useless, removing it causes a segfault--no clue:
	mpv_get_property(mpv, "video-aspect", MPV_FORMAT_INT64, &m_fileInfo.video_params.aspect);

	emit videoParamsChanged(m_fileInfo.video_params);
}

void MpvWidget::LoadAudioParams()
{
	m_fileInfo.audio_params.codec = mpv_get_property_string(mpv, "audio-codec");
	mpv_node node;
	mpv_get_property(mpv, "audio-params", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_MAP)
	{
		for (int i = 0; i < node.u.list->num; i++)
		{
			if (QString(node.u.list->keys[i]) == "samplerate")
			{
				if (node.u.list->values[i].format == MPV_FORMAT_INT64)
					m_fileInfo.audio_params.samplerate = node.u.list->values[i].u.int64;
			}
			else if (QString(node.u.list->keys[i]) == "channel-count")
			{
				if (node.u.list->values[i].format == MPV_FORMAT_INT64)
					m_fileInfo.audio_params.channels = node.u.list->values[i].u.int64;
			}
		}
	}

	emit audioParamsChanged(m_fileInfo.audio_params);
}

void MpvWidget::LoadMetadata()
{
	m_fileInfo.metadata.clear();
	mpv_node node;
	mpv_get_property(mpv, "metadata", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_MAP)
	{
		for (int n = 0; n < node.u.list->num; n++)
		{
			if (node.u.list->values[n].format == MPV_FORMAT_STRING)
			{
				m_fileInfo.metadata[node.u.list->keys[n]] = node.u.list->values[n].u.string;
			}
		}
	}
}

void MpvWidget::Command(const QStringList &strlist)
{
	const QByteArray tmp = strlist.join(" ").toUtf8();
	mpv_command_string(mpv, tmp.constData());
}

void MpvWidget::SetOption(QString key, QString val)
{
	HandleErrorCode(mpv_set_option_string(mpv, key.toUtf8().constData(), val.toUtf8().constData()));
}

void MpvWidget::OpenFile(QString f)
{
	if (!m_isUseOpenGL_CB)
	{
		int64_t wid = this->winId();
		mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);
	}

	const QByteArray tmp = f.toUtf8();
	const char *args[] = { "loadfile", tmp.constData(), NULL };
	Command(args);

	emit fileLoading(m_lastTime, m_fileInfo.length);
}

void MpvWidget::SetProperties()
{
	Volume(m_volume);
	Speed(m_speed);
	Mute(m_isMute);
}

void MpvWidget::AsyncCommand(const char *args[])
{
	mpv_command_async(mpv, MPV_REPLY_COMMAND, args);
}

void MpvWidget::Command(const char *args[])
{
	HandleErrorCode(mpv_command(mpv, args));
}

void MpvWidget::HandleErrorCode(int error_code)
{
	if (error_code >= 0)
		return;
	QString error = mpv_error_string(error_code);
	if (error != QString())
	{
		emit messageSignal(error + "\n");
	}
}

void MpvWidget::setFile(QString s)
{
	emit fileChanged(m_file = s);
}

void MpvWidget::setVo(QString s)
{
	emit voChanged(m_vo = s);
}

void MpvWidget::setMsgLevel(QString s)
{
	emit msgLevelChanged(m_msgLevel = s);
}

void MpvWidget::setSpeed(double d)
{
	emit speedChanged(m_speed = d);
}

void MpvWidget::setTime(double i)
{
	emit timeChanged(m_lastTime = i);
	m_lastTime = m_lastTime;
}

void MpvWidget::setVolume(int i)
{
	emit volumeChanged(m_volume = i);
}

void MpvWidget::setVid(int i)
{
	emit vidChanged(m_vid = i);
}

void MpvWidget::setAid(int i)
{
	emit aidChanged(m_aid = i);
}

void MpvWidget::setSid(int i)
{
	emit sidChanged(m_sid = i);
}

void MpvWidget::setSubtitleVisibility(bool b)
{
	emit subtitleVisibilityChanged(m_isSubtitleVisible = b);
}

void MpvWidget::setMute(bool b)
{
	if (m_isMute != b) emit muteChanged(m_isMute = b);
}

QString MpvWidget::getRealTime()
{
	QString realTime = mpv_get_property_string(mpv, "playback-time");
	return realTime;
}

void MpvWidget::seekRelativeTime(double time)
{
	double t = m_lastTime + time;
	if (t <= 0.0)
		Seek(0);
	else if (t > m_fileInfo.length)
		Seek(m_fileInfo.length);
	else
		Seek(t);
}

void MpvWidget::setSubtitleFile(QString f)
{
	m_subtitleFile = f;
	AddSubtitleTrack(f);
}

void MpvWidget::startABLoop(double start, double end)
{
	mpv_set_property_string(mpv, "ab-loop-a", QString::number(start).toLocal8Bit());
	mpv_set_property_string(mpv, "ab-loop-b", QString::number(end).toLocal8Bit());

	Seek(start);
}

void MpvWidget::stopABLoop()
{
	mpv_set_property_string(mpv, "ab-loop-a", "-1");
	mpv_set_property_string(mpv, "ab-loop-b", "-1");
}

void MpvWidget::setKeepAspect(bool isKeepAspect)
{
	SetOption("keepaspect", isKeepAspect ? "yes":"no");
}

bool MpvWidget::isOpenGLInited()
{
	return m_isOpenGLInited;
}

void MpvWidget::setIsHwdec(bool isHwdec)
{
	// Request hw decoding, just for testing.
	mpv_set_option_string(mpv, "hwdec", isHwdec ? "auto" : "no");
}

// Speed
void MpvWidget::incSpeed10() 
{
	qDebug("MpvWidget::incSpeed10");
	Speed((double)m_speed + 0.1);
}

void MpvWidget::decSpeed10() 
{
	qDebug("MpvWidget::decSpeed10");
	Speed((double)m_speed - 0.1);
}


void MpvWidget::doubleSpeed() {
	qDebug("MpvWidget::doubleSpeed");
	Speed((double)m_speed * 2);
}

void MpvWidget::halveSpeed() 
{
	qDebug("MpvWidget::halveSpeed");
	Speed((double)m_speed / 2);
}

void MpvWidget::normalSpeed() 
{
	Speed(1);
}

void MpvWidget::setRotate(int angle)
{
	const QByteArray tmp = QString::number(angle).toUtf8();
	const char *args[] = { "set", "video-rotate", tmp.constData(), NULL };
	AsyncCommand(args);
}

void MpvWidget::setPlayState(Mpv::PlayState s)
{ 
	emit playStateChanged(m_playState = s); 
}