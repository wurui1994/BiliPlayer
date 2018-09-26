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

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

#if USE_MPV_OPENGL
MpvWidget::MpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
#else
MpvWidget::MpvWidget(QWidget *parent)
	: QWidget(parent)
#endif
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
	mpv_set_option_string(mpv, "sub-auto", "no"); // youtube-dl support

#if USE_MPV_OPENGL
	m_isUseOpenGL_CB = true;
#else
	m_isUseOpenGL_CB = false;
#endif
	//
	if (m_isUseOpenGL_CB)
	{
		mpv_set_option_string(mpv, "vo", "opengl-cb");
	}

    // Request hw decoding, just for testing.
	mpv_set_option_string(mpv, "hwdec", "auto");

#if USE_MPV_OPENGL
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
#endif

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
#if USE_MPV_OPENGL 
    if (mpv_gl)
    {
#if 1
    	qDebug() << "release mpv_gl:" << mpv_gl;
    	makeCurrent();
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);

	    // Until this call is done, we need to make sure the player remains
	    // alive. This is done implicitly with the mpv::qt::Handle instance
	    // in this class.
    	//mpv_opengl_cb_uninit_gl(mpv_gl);
#endif
    }
#endif
    if (mpv)
    {
#if 0
    	qDebug() << "release mpv:" << mpv;
    	mpv_terminate_destroy(mpv);
#endif
    } 
}

#if USE_MPV_OPENGL
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
}

void MpvWidget::swapped()
{
	if(!mpv_gl)
	{
		return;
	}
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}
#endif

void MpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void MpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        } else if (strcmp(prop->name, "ab-loop-b") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                //double time = *(double *)prop->data;
                //Q_EMIT durationChanged(time);
            }
        }
		if (QString(prop->name) == "playback-time") // playback-time does the same thing as time-pos but works for streaming media
		{
			if (prop->format == MPV_FORMAT_DOUBLE)
			{
				double secs = *(double*)prop->data;
				//
				bool negative = (secs < 0);
				secs = qAbs(secs);

				double t = secs;
				int hours = (int)t / 3600;
				t -= hours * 3600;
				int minutes = (int)t / 60;
				t -= minutes * 60;
				int seconds = t;
				t -= seconds;
				int milliseconds = t * 1000;

				QString s = QString("%1%2:%3:%4.%5").arg(negative ? "-" : "")
					.arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'))
					.arg(seconds, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));
				//ShowText(s, 100);
				setMsecsTime(s);
				//
				setTime((int)secs);
				lastMsecsTime = secs;
				lastTime = time;
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
				if (isVisible != subtitleVisibility)
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
		fileInfo.length = 0;
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
		if (!subtitleFile.isEmpty())
		{
			setSubtitleFile(subtitleFile);
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
		if (playState == Mpv::Loaded) 
		{
			ShowText(tr("File couldn't be opened"));
			emit fileCannotOpen();
		}
		//
		if (playState == Mpv::Playing)
		{
			emit reachEndFile(file);
		}
		//	
		setPlayState(Mpv::Stopped);
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

#if USE_MPV_OPENGL
// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void MpvWidget::maybeUpdate()
{
	update();
}

void MpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((MpvWidget*)ctx, "maybeUpdate");
}
#endif


QString MpvWidget::getMediaInfo()
{
	QFileInfo fi(path + file);

	double avsync, fps, vbitrate, abitrate;

	mpv_get_property(mpv, "avsync", MPV_FORMAT_DOUBLE, &avsync);
	mpv_get_property(mpv, "estimated-vf-fps", MPV_FORMAT_DOUBLE, &fps);
	mpv_get_property(mpv, "video-bitrate", MPV_FORMAT_DOUBLE, &vbitrate);
	mpv_get_property(mpv, "audio-bitrate", MPV_FORMAT_DOUBLE, &abitrate);
	QString current_vo = mpv_get_property_string(mpv, "current-vo"),
		current_ao = mpv_get_property_string(mpv, "current-ao"),
		hwdec_active = mpv_get_property_string(mpv, "hwdec-active");

	int vtracks = 0,
		atracks = 0;

	for (auto &track : fileInfo.tracks)
	{
		if (track.type == "video")
			++vtracks;
		else if (track.type == "audio")
			++atracks;
	}

	const QString outer = "%0: %1\n", inner = "    %0: %1\n";

	QString out = outer.arg(tr("File"), fi.fileName()) +
		inner.arg(tr("Title"), fileInfo.media_title) +
		inner.arg(tr("File size"), Utils::HumanSize(fi.size())) +
		inner.arg(tr("Date created"), fi.created().toString()) +
		inner.arg(tr("Media length"), Utils::FormatTime(fileInfo.length)) + '\n';
	if (fileInfo.video_params.codec != QString())
		out += outer.arg(tr("Video (x%0)").arg(QString::number(vtracks)), fileInfo.video_params.codec) +
		inner.arg(tr("Video Output"), QString("%0 (hwdec %1)").arg(current_vo, hwdec_active)) +
		inner.arg(tr("Resolution"), QString("%0 x %1 (%2)").arg(QString::number(fileInfo.video_params.width),
			QString::number(fileInfo.video_params.height),
			Utils::Ratio(fileInfo.video_params.width, fileInfo.video_params.height))) +
		inner.arg(tr("FPS"), QString::number(fps)) +
		inner.arg(tr("A/V Sync"), QString::number(avsync)) +
		inner.arg(tr("Bitrate"), tr("%0 kbps").arg(vbitrate / 1000)) + '\n';
	if (fileInfo.audio_params.codec != QString())
		out += outer.arg(tr("Audio (x%0)").arg(QString::number(atracks)), fileInfo.audio_params.codec) +
		inner.arg(tr("Audio Output"), current_ao) +
		inner.arg(tr("Sample Rate"), QString::number(fileInfo.audio_params.samplerate)) +
		inner.arg(tr("Channels"), QString::number(fileInfo.audio_params.channels)) +
		inner.arg(tr("Bitrate"), tr("%0 kbps").arg(abitrate)) + '\n';

	if (fileInfo.chapters.length() > 0)
	{
		out += outer.arg(tr("Chapters"), QString());
		int n = 1;
		for (auto &chapter : fileInfo.chapters)
			out += inner.arg(QString::number(n++), chapter.title);
		out += '\n';
	}

	if (fileInfo.metadata.size() > 0)
	{
		out += outer.arg(tr("Metadata"), QString());
		for (auto data = fileInfo.metadata.begin(); data != fileInfo.metadata.end(); ++data)
			out += inner.arg(data.key(), *data);
		out += '\n';
	}

	return out;
}

void MpvWidget::AddOverlay(int id, int x, int y, QString file, int offset, int w, int h)
{
	QByteArray tmp_id = QString::number(id).toUtf8(),
		tmp_x = QString::number(x).toUtf8(),
		tmp_y = QString::number(y).toUtf8(),
		tmp_file = file.toUtf8(),
		tmp_offset = QString::number(offset).toUtf8(),
		tmp_w = QString::number(w).toUtf8(),
		tmp_h = QString::number(h).toUtf8(),
		tmp_stride = QString::number(4 * w).toUtf8();

	const char *args[] = { "overlay_add",
		tmp_id.constData(),
		tmp_x.constData(),
		tmp_y.constData(),
		tmp_file.constData(),
		tmp_offset.constData(),
		"bgra",
		tmp_w.constData(),
		tmp_h.constData(),
		tmp_stride.constData(),
		NULL };
	Command(args);
}

void MpvWidget::RemoveOverlay(int id)
{
	QByteArray tmp = QString::number(id).toUtf8();
	const char *args[] = { "overlay_remove", tmp.constData(), NULL };
	AsyncCommand(args);
}

bool MpvWidget::FileExists(QString f)
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
	}
	else
	{
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
}

QString MpvWidget::LoadPlaylist(QString f)
{
	if (f == QString()) // ignore empty file name
		return QString();

	if (f == "-")
	{
		setPath("");
		//setPlaylist({ f });
	}
	else if (Utils::IsValidUrl(f)) // web url
	{
		setPath("");
		//setPlaylist({ f });
	}
	else // local file
	{
		QFileInfo fi(f);
		if (!fi.exists()) // file doesn't exist
		{
			ShowText(tr("File does not exist")); // tell the user
			return QString(); // don't do anything more
		}
		else if (fi.isDir()) // if directory
		{
			setPath(QDir::toNativeSeparators(fi.absoluteFilePath() + "/")); // set new path
			return PopulatePlaylist();
		}
		else if (fi.isFile()) // if file
		{
			setPath(QDir::toNativeSeparators(fi.absolutePath() + "/")); // set new path
			PopulatePlaylist();
			return fi.fileName();
		}
	}
	return f;
}

bool MpvWidget::PlayFile(QString f)
{
	if (f == QString()) // ignore if file doesn't exist
		return false;

	if (path == QString()) // web url
	{
		OpenFile(f);
		setFile(f);
	}
	else
	{
		QFile qf(path + f);
		if (qf.exists())
		{
			OpenFile(path + f);
			setFile(f);
			Play();
		}
		else
		{
			ShowText(tr("File no longer exists")); // tell the user
			return false;
		}
	}
	return true;
}

void MpvWidget::Play()
{
	if (playState > 0 && mpv)
	{
		int f = 0;
		mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "pause", MPV_FORMAT_FLAG, &f);
	}
}

void MpvWidget::Pause()
{
	if (playState > 0 && mpv)
	{
		int f = 1;
		mpv_set_property_async(mpv, MPV_REPLY_PROPERTY, "pause", MPV_FORMAT_FLAG, &f);
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
	if (playState < 0) // not playing, play plays the selected playlist file
	{
		PlayFile(file);
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
	if (time < 3)
	{
		Stop();
	}
	else
	{
		if (playState == Mpv::Playing)
			Restart();
		else
			Stop();
	}
}

void MpvWidget::Mute(bool m)
{
	if (playState > 0)
	{
		const char *args[] = { "set", "ao-mute", m ? "yes" : "no", NULL };
		AsyncCommand(args);
	}
	else
		setMute(m);
}

void MpvWidget::Seek(double pos, bool relative, bool osd)
{
	if (playState > 0)
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
	int ret = pos - lastTime;
	lastTime = pos;
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
	//    const QByteArray tmp = QString::number(c).toUtf8();
	//    const char *args[] = {"set", "chapter", tmp.constData(), NULL};
	//    AsyncCommand(args);
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
	if (playState > 0)
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

void MpvWidget::Screenshot(bool withSubs)
{
	const char *args[] = { "screenshot", (withSubs ? "subtitles" : "video"), NULL };
	AsyncCommand(args);
}

void MpvWidget::ScreenshotFormat(QString s)
{
	SetOption("screenshot-format", s);
	setScreenshotFormat(s);
}

void MpvWidget::ScreenshotTemplate(QString s)
{
	SetOption("screenshot-template", s);
	setScreenshotTemplate(s);
}

void MpvWidget::ScreenshotDirectory(QString s)
{
	SetOption("screenshot-directory", s);
	setScreenshotDir(s);
}

void MpvWidget::AddSubtitleTrack(QString f)
{
	if (f == QString())
		return;
	const QByteArray tmp = f.toUtf8();
	const char *args_remove[] = { "sub-remove", "1" , NULL};
	Command(args_remove);
	const char *args[] = { "sub-add", tmp.constData(), NULL };
	Command(args);
	// this could be more efficient if we saved tracks in a bst
	auto old = fileInfo.tracks; // save the current track-list
	LoadTracks(); // load the new track list
	auto current = fileInfo.tracks;
	for (auto track : old) // remove the old tracks in current
		current.removeOne(track);
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
	auto old = fileInfo.tracks;
	LoadTracks();
	auto current = fileInfo.tracks;
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
	if (vo == QString())
		vo = mpv_get_property_string(mpv, "current-vo");
	QStringList vos = vo.split(',');
	for (auto &o : vos)
	{
		int i = o.indexOf(":interpolation");
		if (interpolate && i == -1)
			o.append(":interpolation");
		else if (i != -1)
			o.remove(i, QString(":interpolation").length());
	}
	setVo(vos.join(','));
	SetOption("vo", vo);
	ShowText(tr("Motion Interpolation: %0").arg(interpolate ? tr("enabled") : tr("disabled")));
}

void MpvWidget::Vo(QString o)
{
	setVo(o);
	SetOption("vo", vo);
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
		//emit onShowText(text, duration);
		Utils::MessageShow(this, text, QSize(200, 80), duration);
	}
}

void MpvWidget::LoadFileInfo()
{
	// get media-title
	fileInfo.media_title = mpv_get_property_string(mpv, "media-title");
	// get length
	double len;
	mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &len);
	fileInfo.length = len;

	LoadTracks();
	LoadChapters();
	LoadVideoParams();
	LoadAudioParams();
	LoadMetadata();

	emit fileInfoChanged(fileInfo);
}

void MpvWidget::LoadTracks()
{
	fileInfo.tracks.clear();
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
				fileInfo.tracks.push_back(track);
			}
		}
	}

	emit trackListChanged(fileInfo.tracks);
}

void MpvWidget::LoadChapters()
{
	fileInfo.chapters.clear();
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
				fileInfo.chapters.push_back(ch);
			}
		}
	}
	emit chaptersChanged(fileInfo.chapters);
}

void MpvWidget::LoadVideoParams()
{
	fileInfo.video_params.codec = mpv_get_property_string(mpv, "video-codec");
	mpv_get_property(mpv, "width", MPV_FORMAT_INT64, &fileInfo.video_params.width);
	mpv_get_property(mpv, "height", MPV_FORMAT_INT64, &fileInfo.video_params.height);
	mpv_get_property(mpv, "dwidth", MPV_FORMAT_INT64, &fileInfo.video_params.dwidth);
	mpv_get_property(mpv, "dheight", MPV_FORMAT_INT64, &fileInfo.video_params.dheight);
	// though this has become useless, removing it causes a segfault--no clue:
	mpv_get_property(mpv, "video-aspect", MPV_FORMAT_INT64, &fileInfo.video_params.aspect);

	emit videoParamsChanged(fileInfo.video_params);
}

void MpvWidget::LoadAudioParams()
{
	fileInfo.audio_params.codec = mpv_get_property_string(mpv, "audio-codec");
	mpv_node node;
	mpv_get_property(mpv, "audio-params", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_MAP)
	{
		for (int i = 0; i < node.u.list->num; i++)
		{
			if (QString(node.u.list->keys[i]) == "samplerate")
			{
				if (node.u.list->values[i].format == MPV_FORMAT_INT64)
					fileInfo.audio_params.samplerate = node.u.list->values[i].u.int64;
			}
			else if (QString(node.u.list->keys[i]) == "channel-count")
			{
				if (node.u.list->values[i].format == MPV_FORMAT_INT64)
					fileInfo.audio_params.channels = node.u.list->values[i].u.int64;
			}
		}
	}

	emit audioParamsChanged(fileInfo.audio_params);
}

void MpvWidget::LoadMetadata()
{
	fileInfo.metadata.clear();
	mpv_node node;
	mpv_get_property(mpv, "metadata", MPV_FORMAT_NODE, &node);
	if (node.format == MPV_FORMAT_NODE_MAP)
		for (int n = 0; n < node.u.list->num; n++)
			if (node.u.list->values[n].format == MPV_FORMAT_STRING)
				fileInfo.metadata[node.u.list->keys[n]] = node.u.list->values[n].u.string;
}

void MpvWidget::LoadOsdSize()
{
	mpv_get_property(mpv, "osd-width", MPV_FORMAT_INT64, &osdWidth);
	mpv_get_property(mpv, "osd-height", MPV_FORMAT_INT64, &osdHeight);
}

void MpvWidget::Command(const QStringList &strlist)
{
	// convert input string into char array
	int len = strlist.length();
	char **data = new char*[len + 1];
	for (int i = 0; i < len; ++i)
	{
		data[i] = new char[strlist[i].length() + 1];
		memcpy(data[i], QByteArray(strlist[i].toUtf8()).begin(), strlist[i].length() + 1);
	}
	data[len] = NULL;
	AsyncCommand(const_cast<const char**>(data));
	for (int i = 0; i < len; ++i)
		delete[] data[i];
	delete[] data;

	//    const QByteArray tmp = str.toUtf8();
	//    mpv_command_string(m_mpv, tmp.constData());
}

void MpvWidget::SetOption(QString key, QString val)
{
	QByteArray tmp1 = key.toUtf8(),
		tmp2 = val.toUtf8();
	HandleErrorCode(mpv_set_option_string(mpv, tmp1.constData(), tmp2.constData()));
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

	emit fileChanging(time, fileInfo.length);
}

QString MpvWidget::PopulatePlaylist()
{
	if (path != "")
	{
		QStringList playlist;
		QDir root(path);
		QStringList filter = Mpv::media_filetypes;
		if (path != QString() && file != QString())
			filter.append(QString("*.%1").arg(file.split(".").last()));
		QFileInfoList flist;
		flist = root.entryInfoList(filter, QDir::Files);
		for (auto &i : flist)
			playlist.push_back(i.fileName()); // add files to the list
		//setPlaylist(playlist);
		if (playlist.empty())
			return QString();
		return playlist.first();
	}
	return QString();
}

void MpvWidget::SetProperties()
{
	Volume(volume);
	Speed(speed);
	Mute(mute);
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
		emit messageSignal(error + "\n");
}

void MpvWidget::seekRelativeTime(double time)
{
	double t = lastMsecsTime + time;
	if (t <= 0.0)
		Seek(0);
	else if (t > fileInfo.length)
		Seek(fileInfo.length);
	else
		Seek(t);
}

void MpvWidget::setSubtitleFile(QString f)
{
	subtitleFile = f;
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

// Speed
void MpvWidget::incSpeed10() 
{
	qDebug("MpvWidget::incSpeed10");
	Speed((double)speed + 0.1);
}

void MpvWidget::decSpeed10() 
{
	qDebug("MpvWidget::decSpeed10");
	Speed((double)speed - 0.1);
}


void MpvWidget::doubleSpeed() {
	qDebug("MpvWidget::doubleSpeed");
	Speed((double)speed * 2);
}

void MpvWidget::halveSpeed() 
{
	qDebug("MpvWidget::halveSpeed");
	Speed((double)speed / 2);
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
