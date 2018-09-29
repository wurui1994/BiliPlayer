#include "Player.h"

#include <QtMath>
#include <QLibraryInfo>
#include <QMimeData>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QStackedLayout>

#include "Utils.h"
#include "Network.h"

#include "Setting.h"

#ifdef Q_OS_WIN
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

#ifdef __APPLE__
#include "OSXHideTitleBar.h"
#endif

Player::Player(QWidget *parent):
	QDialog(parent)
{
    ui.setupUi(this);
	//
	originTitle = ui.titleLabel->text();
	//
    setWindowFlag(Qt::FramelessWindowHint, true);
	//
	setupWindow();
	//
	setupShortcut();
	//
	setupConnect();
	//
#if 0
#ifdef __APPLE__
    ui.maxButton->setVisible(false);
    ui.minButton->setVisible(false);
    ui.closeButton->setVisible(false);
    //
    setWindowFlag(Qt::CustomizeWindowHint,true);
    setWindowFlag(Qt::WindowMinimizeButtonHint,true);
    setWindowFlag(Qt::WindowMaximizeButtonHint,true);
    //Then, hide the OS X title bar
    OSXHideTitleBar::HideTitleBar(winId());
#endif
#endif
}

Player::~Player()
{
    if(current != nullptr)
    {
        int t = ui.mpvFrame->getTime(),
            l = ui.mpvFrame->getFileInfo().length;
        if(t > 0.05*l && t < 0.95*l) // only save if within the middle 90%
            current->time = t;
        else
            current->time = 0;
    }
}

void Player::setupWindow()
{
	setStyleSheet(Utils::readFile(":/qss/playerbaka.qss"));
#ifdef Q_OS_MAC
	ui.seekBar->setStyleSheet(Utils::readFile(":/qss/seekbar_mac.qss"));
#else
	ui.seekBar->setStyleSheet(Utils::readFile(":/qss/seekbar.qss"));
#endif
	ui.widget->setStyleSheet(Utils::readFile(":/qss/input.qss"));
	ui.danmakuCheckBox->setStyleSheet(Utils::readFile(":/qss/danmaku.qss"));
	//
#if 1
	m_FramelessHelper = new FramelessHelper(this);
	m_FramelessHelper->activateOn(this);  
	m_FramelessHelper->setTitleHeight(ui.titleLabel->height());  
	m_FramelessHelper->setWidgetMovable(true);  
	m_FramelessHelper->setWidgetResizable(true);  
	m_FramelessHelper->setRubberBandOnMove(false); 
	m_FramelessHelper->setRubberBandOnResize(false); 
#endif
	//
	click_timer = new QTimer(this);
	//
	ui.sendButton->setEnabled(true);
	ui.sendButton->setChecked(true);
	//
	volumeMenu = new VolumeMenu(this);
	//
	ui.soundButton->setMenu(volumeMenu);
	//
	speedMenu = new SpeedMenu(this);
	//
	ui.speedButton->setMenu(speedMenu);
	//
	danmakuMenu = new DanmakuMenu(this);
	//
	ui.settingButton->setMenu(danmakuMenu);
	//
#if USE_DANMAKU
	m_window = new Window(ui.mpvFrame);
#endif
	m_recent = new RecentWidget(ui.mpvFrame);
	//
	m_advert = new AdvertWidget(ui.mpvFrame);
	m_advert->setStyleSheet(Utils::readFile(":/qss/seekbar.qss"));
	//
	m_advertInfo = new AdvertInfoWidget(ui.mpvAdFrame);
	//
	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(m_advertInfo);
	ui.mpvAdFrame->setLayout(vLayout);
	//
	QStackedLayout *stackedLayout = new QStackedLayout;
	//
	stackedLayout->addWidget(m_advert);
	stackedLayout->addWidget(m_recent);
#if USE_DANMAKU
	stackedLayout->addWidget(m_window);
#endif
	stackedLayout->setCurrentWidget(m_advert);
	stackedLayout->setStackingMode(QStackedLayout::StackAll);
	//
	ui.mpvFrame->setLayout(stackedLayout);
	//
	m_recent->setVisible(false);
	//
	m_advert->setVisible(false);
	//
}

void Player::setupConnect()
{
	// m_advertInfo
	connect(m_advertInfo, &AdvertInfoWidget::tryCloseAdvert, [=]()
	{
		ui.mpvAdFrame->Stop();
		//
		ui.stackedWidget->setCurrentIndex(0);
		//
		m_isAdvertClose = true;
		emit advertPlayEnd();
	});

	connect(m_advertInfo, &AdvertInfoWidget::tryMuteVolume, [=](bool isMute)
	{
		ui.mpvAdFrame->Mute(isMute);
	});

	// ui.mpvAdFrame
	connect(ui.mpvAdFrame, &MpvWidget::fileInfoChanged, [=](const Mpv::FileInfo &fileInfo)
	{
		onAdFileInfoChange(fileInfo);
	});
	//
	connect(ui.mpvAdFrame, &MpvWidget::fileCannotOpen, [=]()
	{
		ui.stackedWidget->setCurrentIndex(0);
		emit advertPlayEnd();
	});
	//
	connect(ui.mpvAdFrame, &MpvWidget::timeChanged, [=](int t)
	{
		Q_UNUSED(t);
		m_advertInfo->setRemainTime(ui.mpvAdFrame->getRemainTime());
	});
	//
	connect(ui.mpvAdFrame, &MpvWidget::reachEndFile, [=](QString file)
	{
		int time = ui.mpvAdFrame->getTime();
		if (time != 0 && !m_isAdvertClose)
		{
			ui.stackedWidget->setCurrentIndex(0);
			emit advertPlayEnd();
			//
		}
	});

	// ui.mpvFrame
	connect(this, &Player::advertPlayEnd, [=]()
	{
		//
		m_lastRecent = m_recent->getRecent(m_file);
		ui.playButton->setEnabled(true);
		ui.nextButton->setEnabled(true);
		ui.mpvFrame->LoadFile(m_file);
	});
	//
	connect(m_recent, &RecentWidget::tryClearList, [=]()
	{
		ui.mpvFrame->Stop();
		m_lastSelect = nullptr;
		m_totalTime = 0;
		m_lastRecent = Recent();
		//
		m_recent->clearList();
	});
	//
	connect(m_recent, &RecentWidget::tryOpenFile, [=](QString filePath)
	{
#if USE_DANMAKU
		m_window->tryLocal(filePath);
#endif
	});
	//
	connect(m_recent, &RecentWidget::tryOpenRecent, [=](Recent recent)
	{
		m_lastRecent = recent;
#if USE_DANMAKU
		m_window->tryLocal(recent.path);
#endif
	});
	//
	connect(speedMenu, &SpeedMenu::selectText, [=](QString text)
	{
		ui.speedButton->setText(text);
		double speed = text.replace("x","").toDouble();
		qDebug() << "speed:" << speed;
		ui.mpvFrame->Speed(speed);
	});
	//
	connect(ui.inputEdit, &QLineEdit::returnPressed, [=]()
	{
		on_sendButton_clicked();
	});
	//
	connect(ui.inputEdit, &QLineEdit::textChanged, [=]()
	{
		int len = ui.inputEdit->text().size();
		int maxLen = ui.inputEdit->maxLength();
		QString text = QString("%1/%2").arg(len).arg(maxLen);
		ui.wordLabel->setText(text);
	});
#if USE_DANMAKU
	//
	connect(m_window, &Window::modelReset, [=]()
	{
		//bool isEmpty = m_window->isDanmakuEmpty();
		//ui.sendButton->setEnabled(!isEmpty);
		ui.sendButton->setEnabled(true);
	});
	//
	connect(m_window, &Window::openMediaFile, [=](QString file)
	{
		Load(file);
	});
	//
	connect(m_window, &Window::settingChanged, [=]()
	{
		bool isChecked = Setting::getValue("/Video/Hwdec",false);
		bool isAutoPlay = Setting::getValue("/Video/AutoPlay", true);
		ui.mpvFrame->setIsHwdec(isChecked);
		m_recent->setIsAutoPlay(isAutoPlay);
	});
#endif
	//
	connect(click_timer, &QTimer::timeout, [this]()
	{
		click_timer->stop();
		emit clicked(m_last_pos);
		m_last_pos = QPoint();
	});

	// PlayerBaka

	connect(this, &Player::remainingChanged, [=]
	{
		SetRemainingLabels(ui.mpvFrame->getTime());
	});

	connect(this, &Player::hideAllControlsChanged, [=](bool b)
	{
		HideAllControls(b);
		blockSignals(true);
		blockSignals(false);
	});

	// ui.mpvFrame
	connect(ui.mpvFrame, &MpvWidget::fileInfoChanged, [=](const Mpv::FileInfo &fileInfo)
	{
		onFileInfoChange(fileInfo);
	});

	connect(ui.mpvFrame, &MpvWidget::trackListChanged, [=](const QList<Mpv::Track> &trackList)
	{
		onTrackListChanged(trackList);
	});

	connect(ui.mpvFrame, &MpvWidget::chaptersChanged, [=](const QList<Mpv::Chapter> &chapters)
	{
		QAction *action;
		QList<int> ticks;
		int n = 1, N = chapters.length();
		for (auto &ch : chapters)
		{
			action = new QAction(QString("%0: %1").arg(Utils::FormatNumberWithAmpersand(n, N), ch.title));

			connect(action, &QAction::triggered,[=]
			{
				ui.mpvFrame->Seek(ch.time);
			});
			//
			ticks.push_back(ch.time);
			n++;
		}
		//
		ui.seekBar->setTicks(ticks);
	});

	connect(ui.mpvFrame, &MpvWidget::playStateChanged, [=](Mpv::PlayState playState)
	{
		if (m_advert)
		{
			m_advert->setVisible(false);
		}
		setPlayState(playState);
		switch (playState)
		{
		case Mpv::Loaded:
			//ui.mpvFrame->ShowText("Loading...", 0);
			break;

		case Mpv::Started:
			if (!init) // will only happen the first time a file is loaded.
			{
				ui.playButton->setEnabled(true);
				ui.nextButton->setEnabled(true);
				init = true;
			}
			SetPlaybackControls(true);
			if (m_isPauseOnStart)
			{
				ui.mpvFrame->Play();
				ui.mpvFrame->Pause();
				SetPlayButtonIcon(true);
			}
			else
			{
				ui.mpvFrame->Play();
			}
			ui.mpvFrame->ShowText(QString(), 0);
		case Mpv::Playing:
			SetPlayButtonIcon(false);
//			if (onTop == "playing")
//				Util::SetAlwaysOnTop(winId(), true);
			break;

		case Mpv::Paused:
			//
			if (m_advert)
			{
				QString imgPath = Network::instance().imagePath();
				m_advert->setPixmap(QPixmap(imgPath));
				m_advert->setVisible(true);
			}
			break;
		case Mpv::Stopped:
			SetPlayButtonIcon(true);
//			if (onTop == "playing")
//				Util::SetAlwaysOnTop(winId(), false);
			if (playState == Mpv::Stopped && m_isRestartOnStop)
			{
				ui.mpvFrame->PlayPause();
				ui.mpvFrame->Pause();
				SetPlayButtonIcon(true);
			}

			break;

		case Mpv::Idle:
			if (init)
			{

			}
			break;

		}
	});

	connect(ui.mpvFrame, &MpvWidget::fileLoading, [=](int t, int l)
	{
		if (current != nullptr)
		{
			if (t > 0.05*l && t < 0.95*l) // only save if within the middle 90%
				current->time = t;
			else
				current->time = 0;
		}
	});


	connect(ui.mpvFrame, &MpvWidget::timeChanged, [=](int i)
	{
		const Mpv::FileInfo &fi = ui.mpvFrame->getFileInfo();
		// set the seekBar's location with NoSignal function so that it doesn't trigger a seek
		// the formula is a simple ratio seekBar's max * time/totalTime
		ui.seekBar->setValueNoSignal(ui.seekBar->maximum()*((double)i / fi.length));
		SetRemainingLabels(i);
	});

	connect(ui.mpvFrame, &MpvWidget::tryToGetTime, [=]()
	{
		if (ui.mpvFrame->getPlayState() != Mpv::Playing)
		{
			return;
		}
		qint64 time = ui.mpvFrame->getRealTime().toDouble() * 1000;
#if USE_DANMAKU
		if (m_window)
		{
			m_window->setDanmakuTime(time);
		}
#endif
		//
		if (m_lastSelect)
		{
			m_lastSelect->setTime(time, m_totalTime);
		}
	});

	//
	connect(ui.mpvFrame, &MpvWidget::reachEndFile, [=](QString file)
	{
		if (m_lastSelect && file != m_lastSelect->recent().path)
		{
			return;
		}
#if USE_DANMAKU
		if (m_window)
		{
			m_window->setDanmakuTime(m_totalTime);
		}
#endif
		bool isEnd = (m_totalTime - 1000*ui.mpvFrame->getTime()) < 1000;
		//
		if (m_lastSelect && isEnd)
		{
			m_lastSelect->setTime(m_totalTime, m_totalTime);
		}
	});

	connect(ui.mpvFrame, &MpvWidget::volumeChanged, [=](int volume)
	{
		volumeMenu->setValue(volume);
		emit volumeChanged(volume);
	});

	connect(ui.mpvFrame, &MpvWidget::speedChanged, [=](double speed)
	{
		static double last = 1;
		if (last != speed)
		{
			if (init)
				ui.mpvFrame->ShowText(tr("Speed: %0x").arg(QString::number(speed)));
			last = speed;
		}
	});

	connect(ui.mpvFrame, &MpvWidget::sidChanged, [=](int sid)
	{
		QList<QAction*> actions = menuSubtitle_Track->actions();
		for (auto &action : actions)
		{
			if (action->text().startsWith(QString::number(sid)))
			{
				action->setCheckable(true);
				action->setChecked(true);
			}
			else 
			{
				action->setChecked(false);
			}
		}
	});

	connect(ui.mpvFrame, &MpvWidget::aidChanged, [=](int aid)
	{
		Q_UNUSED(aid);
	});

	connect(ui.mpvFrame, &MpvWidget::subtitleVisibilityChanged, [=](bool b)
	{
		if (init)
			ui.mpvFrame->ShowText(b ? tr("Subtitles visible") : tr("Subtitles hidden"));
	});

	connect(ui.mpvFrame, &MpvWidget::muteChanged, [=](bool b)
	{
		ui.mpvFrame->ShowText(b ? tr("Muted") : tr("Unmuted"));
	});

	connect(ui.mpvFrame, &MpvWidget::voChanged, [=](QString vo)
	{

	});
	// ui

	// Playback: Seekbar clicked
	connect(ui.seekBar, &SeekBar::tryToSeek, [=](double t)
	{
		ui.mpvFrame->Seek(ui.mpvFrame->Relative(t), true);
	});

	// Playback: Rewind button
	//connect(ui.rewindButton, &QPushButton::clicked, [=]
	//{
	//	ui.mpvFrame->Rewind();
	//});

	// Playback: Previous button
	//connect(ui.previousButton, &QPushButton::clicked, [=]
	//{
	//	seekRelativeTime(-10);
	//});

	// Playback: Play/pause button
	connect(ui.playButton, &QPushButton::clicked, [=]
	{
		ui.mpvFrame->PlayPause();
	});

	// Playback: Next button
	connect(ui.nextButton, &QPushButton::clicked, [=]
	{
		//seekRelativeTime(10);
		m_recent->onNextVideo();
	});

#if 0
	connect(ui.soundButton, &QPushButton::clicked, [=]
	{
		ui.mpvFrame->Mute(!ui.mpvFrame->getMute());
	});
#endif

	// Playback: Volume slider adjusted
	connect(volumeMenu, &VolumeMenu::valueChanged, [=](int i)
	{
		ui.mpvFrame->Volume(i, true);
	});

}

void Player::setSubtitleFile(QString f)
{
	ui.mpvFrame->setSubtitleFile(f);
}

void Player::setupShortcut()
{
	//
	m_menu = new QMenu(this);
	m_menu->setWindowFlag(Qt::FramelessWindowHint, true);
	m_menu->setAttribute(Qt::WA_TranslucentBackground);
	m_menu->setStyleSheet(Utils::readFile(":/qss/menu.qss"));
	//快捷键函数映射表
	m_keyFuncMap = {
		// 视频前进和后退
		{ "OpenFile" ,[=]() { openFile(); } },
		{ "OpenUrl" ,[=]() { openUrl(); } },
		// 视频前进和后退
		{ "PlayPause" ,[=]() { ui.mpvFrame->PlayPause(); } },
		{ "Restart" ,[=]() { ui.mpvFrame->Restart(); } },
		{ "ForwardOneSecond" ,[=]() { ui.mpvFrame->seekRelativeTime(1); } },
		{ "BackwardOneSecond" ,[=]() { ui.mpvFrame->seekRelativeTime(-1); } },
		{ "ForwardFiveSecond" ,[=]() { ui.mpvFrame->seekRelativeTime(5); } },
		{ "BackwardFiveSecond" ,[=]() { ui.mpvFrame->seekRelativeTime(-5); } },
		{ "FrameBackStep" ,[=]() { ui.mpvFrame->FrameBackStep(); } },
		{ "FrameStep" ,[=]() { ui.mpvFrame->FrameStep(); } },
		//
		{ "AddSubtitle" ,[=]() {addSubtitle(); } },
		{ "ShowSubtitle" ,[=]() {showSubtitles(); } },
		// Video
		{ "NoAspect" ,[=]() {noKeepAspect(); } },
		// Audio
		{ "AudioMute" ,[=]() {audioMute(); } },
		//
		{ "HideTitle" ,[=]() {hideTitle(); } },
		{ "HideControl" ,[=]() {hideControl(); } },
		//
		{ "Setting" ,[=]() { m_window->showSettingDialog(); } },
		//
		{ "OpenFolder" ,[=]() { Utils::openAppDataFolder(); } },
	};
	//
	QMenu* openMenu = m_menu->addMenu(tr("Open"));
	openMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	openMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	genAction("OpenFile", tr("OpenFile"), QString("Ctrl+O"), openMenu);
	genAction("OpenUrl", tr("OpenUrl"), QString("Ctrl+U"), openMenu);
	//
	//m_menu->addSeparator();
	//
	QMenu* playMenu = m_menu->addMenu(tr("Play"));
	playMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	playMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	genAction("PlayPause", tr("PlayPause"), QString("Ctrl+P"), playMenu);
	genAction("Restart", tr("Restart"), QString("Ctrl+R"), playMenu);
	playMenu->addSeparator();
	genAction("BackwardOneSecond", tr("BackwardOneSecond"), QString("Left"), playMenu);
	genAction("ForwardOneSecond", tr("ForwardOneSecond"), QString("Right"), playMenu);
	playMenu->addSeparator();
	genAction("BackwardFiveSecond", tr("BackwardFiveSecond"), QString("Ctrl+Left"), playMenu);
	genAction("ForwardFiveSecond", tr("ForwardFiveSecond"), QString("Ctrl+Right"), playMenu);
	playMenu->addSeparator();
	genAction("FrameBackStep", tr("FrameStep"), QString("Shift+Left"), playMenu);
	genAction("FrameStep", tr("FrameBackStep"), QString("Shift+Right"), playMenu);
	//
	m_menu->addSeparator();
	//
	QMenu* subtitleMenu = m_menu->addMenu(tr("Subtitle"));
	subtitleMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	subtitleMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	QAction* showSubtitle = genAction("ShowSubtitle", tr("ShowSubtitle"), QString("Ctrl+W"), subtitleMenu);
	showSubtitle->setCheckable(true);
	showSubtitle->setChecked(true);
	//
	menuSubtitle_Track = subtitleMenu->addMenu(tr("Subtitle Tracks"));
	menuSubtitle_Track->setWindowFlag(Qt::FramelessWindowHint, true);
	menuSubtitle_Track->setAttribute(Qt::WA_TranslucentBackground);
	//
	QMenu* videoMenu = m_menu->addMenu(tr("Video"));
	videoMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	videoMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	QMenu* aspectMenu = videoMenu->addMenu(tr("Aspect"));
	aspectMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	aspectMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	action_NoAspect = genAction("NoAspect", tr("NoAspect"), QString("Ctrl+K"), aspectMenu);
	action_NoAspect->setCheckable(true);
	action_NoAspect->setChecked(false);
	//
	QMenu* audioMenu = m_menu->addMenu(tr("Audio"));
	audioMenu->setWindowFlag(Qt::FramelessWindowHint, true);
	audioMenu->setAttribute(Qt::WA_TranslucentBackground);
	//
	action_AudioMute = genAction("AudioMute", tr("AudioMute"), QString("Ctrl+M"), audioMenu);
	action_AudioMute->setCheckable(true);
	action_AudioMute->setChecked(false);
	//
	m_menu->addSeparator();
	//
	action_hideTitle = genAction("HideTitle", tr("HideTitle"), QString("Ctrl+T"), m_menu);
	action_hideTitle->setCheckable(true);
	action_hideTitle->setChecked(false);
	//
	action_hideControl = genAction("HideControl", tr("HideControl"), QString("Ctrl+B"), m_menu);
	action_hideControl->setCheckable(true);
	action_hideControl->setChecked(false);
	//
	m_menu->addSeparator();
	//
	genAction("Setting", tr("Setting"), QString("Ctrl+I"), m_menu);
	m_menu->addSeparator();
	//
	genAction("OpenFolder", tr("OpenFolder"), QString("Ctrl+L"), m_menu);
}

QAction* Player::genAction(QString const & key, QString const & actionName, QKeySequence shortCut, QMenu * menu)
{
	QAction* action = new QAction(actionName, menu);
	action->setShortcut(shortCut);
	//
	QShortcut* shortcut = new QShortcut(shortCut, this);
	connect(shortcut, &QShortcut::activated, [=]()
	{
		action->trigger();
	});
	//
	connect(action, &QAction::triggered, this, [=]()
	{
		if (m_keyFuncMap[key])
		{
			m_keyFuncMap[key]();
		}
	});
	menu->addAction(action);
	//
	return action;
}

void Player::setVideoTitle(QString title)
{
	ui.titleLabel->setText(title);
}

void Player::loadAdvert(QString path)
{
	m_isAdvertClose = false;
	//
	ui.mpvFrame->Stop();
	//
	ui.playButton->setDisabled(true);
	ui.nextButton->setDisabled(true);
	//
	ui.stackedWidget->setCurrentIndex(1);
	ui.mpvAdFrame->LoadFile(path);
	//
    ui.mpvAdFrame->setKeepAspect(false);
}

bool Player::isPlaying()
{
	return playState == Mpv::Playing;
}

bool Player::isPause()
{
	return playState == Mpv::Paused;
}

void Player::setPauseOnStart(bool isPauseOnStart)
{
	m_isPauseOnStart = isPauseOnStart;
}

void Player::restartOnStop(bool isRestartOnStop)
{
	m_isRestartOnStop = isRestartOnStop;
}

void Player::Load(QString file)
{
	if (file.isEmpty())
	{
		return;
	}
#if 1
	if (m_file != file)
	{
		m_file = file;
		QString adVideo = Network::instance().videoPath();
		if (adVideo.isEmpty())
		{
			// 等待json响应1000
			QTimer::singleShot(1000, [=]()
			{
				QString adVideo = Network::instance().videoPath();
				qDebug() << adVideo;
				if (adVideo.isEmpty())
				{
					ui.playButton->setDisabled(false);
					ui.nextButton->setDisabled(false);
					ui.mpvFrame->LoadFile(file);
				}
				else
				{
					loadAdvert(adVideo);
				}
			});
		}
		else
		{
			loadAdvert(adVideo);
		}
	}
	else
	{

		m_file = file;
		ui.playButton->setDisabled(false);
		ui.nextButton->setDisabled(false);
		ui.mpvFrame->LoadFile(file);
	}
#else
	m_file = file;
	ui.mpvFrame->LoadFile(file);
#endif
}

void Player::HideOrShowControls(bool isHide)
{
	if (isHide)
	{
		ui.seekBar->setVisible(false);
		ui.playbackLayoutWidget->setVisible(false);
	}
	else
	{
		ui.seekBar->setVisible(true);
		ui.playbackLayoutWidget->setVisible(true);
	}
}

void Player::seek(int t)
{
	ui.mpvFrame->Seek(t / 1000.0);
}

void Player::seekRelativeTime(double time)
{
	ui.mpvFrame->seekRelativeTime(time);
}

void Player::Pause()
{
	ui.mpvFrame->Pause();
}

void Player::onFileInfoChange(const Mpv::FileInfo & fileInfo)
{
	if (fileInfo.media_title == "")
	{
		setVideoTitle("Baka MPlayer");
	}
	else if (fileInfo.media_title == "-")
	{
		setVideoTitle("Baka MPlayer: stdin"); // todo: disable playlist?
	}
	else
	{
		setVideoTitle(fileInfo.media_title);
	}

	QString f = ui.mpvFrame->getFile(), file = ui.mpvFrame->getPath() + f;
	if (f != QString())
	{
		m_fileHash = Utils::fileHash(f);
		//
		Network::instance().setVidHash(m_fileHash);
		//
		Network::instance().getDanmaku(0);
		//
		QString title = (ui.mpvFrame->getPath() == QString() || !Utils::IsValidFile(file)) ?
			fileInfo.media_title : QString();
		Recent recent(file, title,0, fileInfo.length*1000);
		m_recentList.append(recent);
		current = &m_recentList.last();
		//
		m_lastSelect = m_recent->addRecent(recent);

	}

	// reset speed if length isn't known and we have a streaming video
	// todo: don't save this reset, put their speed back when a normal video comes on
	// todo: disable speed alteration during streaming media
	if (fileInfo.length == 0)
		if (ui.mpvFrame->getSpeed() != 1)
			ui.mpvFrame->Speed(1);

	ui.seekBar->setTracking(fileInfo.length);

	SetRemainingLabels(fileInfo.length);

	qint64 time = 1000 * fileInfo.length;
	qDebug() << "video file time:" << time;
	//
	if (time <= 0)
	{
		qDebug() << "retry play";
		QTimer::singleShot(500, [=]()
		{
			ui.mpvFrame->PlayPause();
			ui.mpvFrame->LoadFileInfo();
			ui.mpvFrame->Play();
			if (!m_lastRecent.path.isEmpty())
			{
				if (m_lastRecent.time == m_lastRecent.totalTime)
				{
					seek(0);
				}
				else
				{
					seek(m_lastRecent.time);
				}
				m_lastRecent = Recent();
			}
		});
	}
	else
	{
		if (!m_lastRecent.path.isEmpty())
		{
			if (m_lastRecent.time == m_lastRecent.totalTime)
			{
				seek(0);
			}
			else
			{
				seek(m_lastRecent.time);
			}
			m_lastRecent = Recent();
		}
	}
	//
	if (time > 0)
	{
		emit totalTime(time);
		m_totalTime = time;
	}
}

void Player::onAdFileInfoChange(const Mpv::FileInfo & fileInfo)
{
	QString f = ui.mpvAdFrame->getFile();
	QString file = ui.mpvAdFrame->getPath() + f;
	//
	if (fileInfo.length == 0) 
	{
		if (ui.mpvAdFrame->getSpeed() != 1)
		{
			ui.mpvAdFrame->Speed(1);
		}
	}

	qint64 time = 1000 * fileInfo.length;
	qDebug() << "ad video file time:" << time;

	if (time <= 0)
	{
		qDebug() << "retry play ad";
		QTimer::singleShot(500, [=]()
		{
			ui.mpvAdFrame->PlayPause();
			ui.mpvAdFrame->LoadFileInfo();
			ui.mpvAdFrame->Play();
		});
	}
	//
}

void Player::onTrackListChanged(const QList<Mpv::Track>& trackList)
{
	if (ui.mpvFrame->getPlayState() > 0)
	{
		menuSubtitle_Track->clear();
		action_Add_Subtitle_File = genAction("AddSubtitle", tr("AddSubtitle"), QString("Shift+S"), menuSubtitle_Track);
		menuSubtitle_Track->addAction(action_Add_Subtitle_File);
		//menuAudio_Tracks->clear();
		//menuAudio_Tracks->addAction(action_Add_Audio_File);
		for (auto &track : trackList)
		{
			if (track.type == "sub")
			{
				QString text = QString("%0: %1 (%2)")
					.arg(QString::number(track.id))
					.arg(track.title)
					.arg( track.lang + (track.external ? "*" : ""))
					.replace("&", "&&");
				addSubtitleAction(text, track.id);
			}
		}
	}
#if 0
			else if (track.type == "audio")
			{
				action = ui->menuAudio_Tracks->addAction(QString("%0: %1 (%2)").arg(QString::number(track.id), track.title, track.lang).replace("&", "&&"));
				connect(action, &QAction::triggered,
					[=]
				{
					if (mpv->getAid() != track.id) // don't allow selection of the same track
					{
						mpv->Aid(track.id);
						mpv->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Audio"), QString::number(track.id), track.title, track.lang));
					}
					else
						action->setChecked(true); // recheck the track
				});
			}
			else if (track.type == "video") // video track
			{
				if (!track.albumart) // isn't album art
					video = true;
				else
					albumArt = true;
			}
#endif


#if 0
		if (video)
		{
			// if we were hiding album art, show it--we've gone to a video
			if (ui->mpvFrame->styleSheet() != QString()) // remove filler album art
				ui->mpvFrame->setStyleSheet("");
			if (ui->action_Hide_Album_Art->isChecked())
				HideAlbumArt(false);
			ui->action_Hide_Album_Art->setEnabled(false);
			ui->menuSubtitle_Track->setEnabled(true);
			if (ui->menuSubtitle_Track->actions().count() > 1)
			{
				ui->menuFont_Si_ze->setEnabled(true);
				ui->actionShow_Subtitles->setEnabled(true);
				ui->actionShow_Subtitles->setChecked(mpv->getSubtitleVisibility());
			}
			else
			{
				ui->menuFont_Si_ze->setEnabled(false);
				ui->actionShow_Subtitles->setEnabled(false);
				ui->actionShow_Subtitles->setChecked(false);
			}
			ui->menuTake_Screenshot->setEnabled(true);
			ui->menuFit_Window->setEnabled(true);
			ui->menuAspect_Ratio->setEnabled(true);
			ui->action_Frame_Step->setEnabled(true);
			ui->actionFrame_Back_Step->setEnabled(true);
			ui->action_Deinterlace->setEnabled(true);
			ui->action_Motion_Interpolation->setEnabled(true);
		}
		else
		{
			if (!albumArt)
			{
				// put in filler albumArt
				if (ui->mpvFrame->styleSheet() == QString())
					ui->mpvFrame->setStyleSheet("background-image:url(:/img/album-art.png);background-repeat:no-repeat;background-position:center;");
			}
			ui->action_Hide_Album_Art->setEnabled(true);
			ui->menuSubtitle_Track->setEnabled(false);
			ui->menuFont_Si_ze->setEnabled(false);
			ui->actionShow_Subtitles->setEnabled(false);
			ui->actionShow_Subtitles->setChecked(false);
			ui->menuTake_Screenshot->setEnabled(false);
			ui->menuFit_Window->setEnabled(false);
			ui->menuAspect_Ratio->setEnabled(false);
			ui->action_Frame_Step->setEnabled(false);
			ui->actionFrame_Back_Step->setEnabled(false);
			ui->action_Deinterlace->setEnabled(false);
			ui->action_Motion_Interpolation->setEnabled(false);

			if (baka->sysTrayIcon->isVisible() && !hidePopup)
			{
				// todo: use {artist} - {title}
				baka->sysTrayIcon->showMessage("Baka MPlayer", mpv->getFileInfo().media_title, QSystemTrayIcon::NoIcon, 4000);
			}
		}

		if (ui->menuAudio_Tracks->actions().count() == 1)
			ui->menuAudio_Tracks->actions().first()->setEnabled(false);

		if (pathChanged && autoFit)
		{
			baka->FitWindow(autoFit, false);
			pathChanged = false;
		}
#endif
}

void Player::openFile()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("video file"));
	if (filename.isEmpty())
	{
		return;
	}
	Load(filename);
}

void Player::openUrl()
{
	bool ok = false;
	QString url = QInputDialog::getText(0, tr("Open Online Video."),tr("Video Url"), QLineEdit::Normal,"",&ok);
	if (!ok)
	{
		return;
	}
	//
	if (QUrl(url).isValid())
	{
		Load(url);
	}
	else
	{
		Utils::MessageShow(0, tr("Url is invalid."));
	}
}

void Player::addSubtitle()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("video file"));
	if (filename.isEmpty())
	{
		return;
	}
	ui.mpvFrame->AddSubtitleTrack(filename);
}

void Player::addSubtitleAction(QString text,int trackId)
{
	QAction* action = menuSubtitle_Track->addAction(text);
	connect(action, &QAction::triggered,
		[=]
	{
		// basically, if you uncheck the selected subtitle id, we hide subtitles
		// when you check a subtitle id, we make sure subtitles are showing and set it
		if (ui.mpvFrame->getSid() == trackId)
		{
			if (ui.mpvFrame->getSubtitleVisible())
			{
				ui.mpvFrame->ShowSubtitles(false);
				return;
			}
			else 
			{
				ui.mpvFrame->ShowSubtitles(true);
			}
		}
		else if (!ui.mpvFrame->getSubtitleVisible())
		{
			ui.mpvFrame->ShowSubtitles(true);
		}
		//
		ui.mpvFrame->Sid(trackId);
		//ui.mpvFrame->ShowText(QString("%0 %1: %2 (%3)").arg(tr("Sub"), QString::number(trackId), track.title, track.lang + (track.external ? "*" : "")));
	});
}

void Player::showSubtitles()
{
	bool isVis = ui.mpvFrame->getSubtitleVisible();
	ui.mpvFrame->ShowSubtitles(!isVis);
}

void Player::audioMute()
{
	bool isMute = ui.mpvFrame->getMute();
	ui.mpvFrame->Mute(!isMute);
}

void Player::noKeepAspect()
{
	bool isKeep = action_NoAspect->isChecked();
	ui.mpvFrame->setKeepAspect(!isKeep);
}

void Player::hideTitle()
{
	bool isHide = action_hideTitle->isChecked();
	ui.titleLabel->setVisible(!isHide);
}

void Player::hideControl()
{
	bool isHide = action_hideControl->isChecked();
	ui.playbackLayoutWidget->setVisible(!isHide);
	ui.seekBar->setVisible(!isHide);
}

void Player::contextMenuEvent(QContextMenuEvent * event)
{
	Q_UNUSED(event);
}


void Player::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls() || event->mimeData()->hasText())
	{
		event->acceptProposedAction();
	}
}

void Player::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls()) // urls
    {
        for(QUrl &url : mimeData->urls())
        {
			if (url.isLocalFile())
			{
				Load(url.toLocalFile());
			}
			else 
			{
				Load(url.url());
			}
        }
    }
	else if (mimeData->hasText()) // text
	{
		Load(mimeData->text());
	}
}
void Player::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_pos = event->pos();
		click_timer->start(qApp->doubleClickInterval());
		//
		m_bPressed = true;
		m_point = event->pos();
    }
	else if (event->button() == Qt::RightButton)
	{
#if USE_DANMAKU
		m_menu->exec(event->globalPos());
#endif
	}

    QWidget::mousePressEvent(event);
}


void Player::mouseReleaseEvent(QMouseEvent *event)
{
	m_bPressed = false;
	QWidget::mouseReleaseEvent(event);
}

void Player::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (m_bPressed)
		{
			move(event->pos() - m_point + pos());
		}		
		event->accept();
	}
	QWidget::mouseMoveEvent(event);
}

void Player::wheelEvent(QWheelEvent *event)
{
#if 0
    if(event->delta() > 0)
        ui.mpvFrame->Volume(ui.mpvFrame->getVolume()+5, true);
    else
        ui.mpvFrame->Volume(ui.mpvFrame->getVolume()-5, true);
#endif
	QWidget::wheelEvent(event);
}

void Player::keyPressEvent(QKeyEvent *event)
{
    QString key = QKeySequence(event->modifiers()|event->key()).toString();
	
	if (key == "Esc")
	{
		emit escape();
		return;
	}
}

void Player::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
}

void Player::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton 
		&& ui.mpvFrame->geometry().contains(event->pos())) // if mouse is in the mpvFrame
    {
		click_timer->stop();

		emit doubleClicked();
        event->accept();
    }
#ifdef Q_OS_MAC
    QWidget::mouseDoubleClickEvent(event);
#endif
}


void Player::SetPlaybackControls(bool enable)
{
    // playback controls
    ui.seekBar->setEnabled(enable);
}

void Player::HideAllControls(bool w, bool s)
{
	Q_UNUSED(w);
	Q_UNUSED(s);
	return;
}

void Player::FullScreen(bool fs)
{
    if(fs)
    {
#ifdef Q_OS_WIN
		QWindowsWindowFunctions::setHasBorderInFullScreen(this->windowHandle(), true);
#endif
        setWindowState(windowState() | Qt::WindowFullScreen);
        if(!hideAllControls)
            HideAllControls(true, false);
    }
    else
    {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        if(!hideAllControls)
            HideAllControls(false, false);
    }
}


void Player::SetPlayButtonIcon(bool play)
{
	ui.playButton->setChecked(play);
}

void Player::SetRemainingLabels(int time)
{
    // todo: move setVisible functions outside of this function which gets called every second and somewhere at the start of a video
    const Mpv::FileInfo &fi = ui.mpvFrame->getFileInfo();
    if(fi.length == 0)
    {
        ui.durationLabel->setText(Utils::FormatTime(time));
    }
    else
    {
        ui.durationLabel->setText(Utils::FormatTime(time));
        QString text = "" + Utils::FormatTime(fi.length);
        ui.totalLabel->setText(text);
    }
}

void Player::on_settingsButton_clicked()
{
#if USE_DANMAKU
	m_window->showSettingDialog();
#endif
}

void Player::on_sendButton_clicked()
{
	QString text = ui.inputEdit->text();
	int size = danmakuMenu->isSmallFont() ? 16 : 25;
	QColor color = danmakuMenu->selectColor();
#if USE_DANMAKU
	m_window->sendDanmaku(m_fileHash,text,color,size);
#endif
	ui.inputEdit->clear();
}

void Player::on_fullscreenButton_clicked(bool isChecked)
{
	Q_UNUSED(isChecked);
	if (true)
	{
#ifdef Q_OS_WIN
		QWindowsWindowFunctions::setHasBorderInFullScreen(this->windowHandle(), true);
#endif
	}
	setWindowState(windowState() ^ Qt::WindowFullScreen);
}

void Player::on_danmakuCheckBox_clicked(bool isChecked)
{
#if USE_DANMAKU
	m_window->setDanmakuVisiable(isChecked);
#endif
}

void Player::on_localCheckBox_clicked(bool isChecked)
{
	m_recent->setVisible(isChecked);
}

void Player::on_minButton_clicked()
{
	setWindowState(windowState() ^ Qt::WindowMinimized);
}

void Player::on_maxButton_clicked(bool isChecked)
{
	Q_UNUSED(isChecked);
	setWindowState(windowState() ^ Qt::WindowMaximized);
	if (windowState().testFlag(Qt::WindowFullScreen))
	{
		setWindowState(windowState() & ~Qt::WindowFullScreen);
	}
}

void Player::on_closeButton_clicked()
{
	qApp->quit();
}
