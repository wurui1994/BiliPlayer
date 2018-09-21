#include "Player.h"

#include <QtMath>
#include <QLibraryInfo>
#include <QMimeData>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QStackedLayout>

#include "Utils.h"
#include "Network.h"

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
	ui.seekBar->setStyleSheet(Utils::readFile(":/qss/seekbar.qss"));
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
		if (time != 0)
		{
			ui.stackedWidget->setCurrentIndex(0);
			emit advertPlayEnd();
			//
		}
	});

	// ui.mpvFrame
	connect(this, &Player::advertPlayEnd, [=]()
	{
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
		Q_UNUSED(trackList);
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
				ui.mpvFrame->PlayPause("");
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

	connect(ui.mpvFrame, &MpvWidget::pathChanged, [=]()
	{
		pathChanged = true;
	});

	connect(ui.mpvFrame, &MpvWidget::fileChanging, [=](int t, int l)
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

	connect(ui.mpvFrame, &MpvWidget::msecsTimeChanged, [=](QString s)
	{
		emit msecsTimeChanged(s);
		qint64 time = QTime::fromString(s, "hh:mm:ss.zzz").msecsSinceStartOfDay();
#if USE_DANMAKU

		if (m_window)
		{
			m_window->setDanmakuTime(time);
		}
#endif
		//
		if (m_lastSelect)
		{
			m_lastSelect->setTime(time,m_totalTime);
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
		//
		if (m_lastSelect)
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
		Q_UNUSED(sid);
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

	//connect(ui.mpvFrame, &MpvWidget::muteChanged, [=](bool b)
	//{
	//	if (b)
	//		ui.muteButton->setIcon(QIcon(":/img/default_mute.svg"));
	//	else
	//		ui.muteButton->setIcon(QIcon(":/img/default_unmute.svg"));
	//	ui.mpvFrame->ShowText(b ? tr("Muted") : tr("Unmuted"));
	//});

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
		ui.mpvFrame->PlayPause("");
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

void Player::setVideoTitle(QString title)
{
	ui.titleLabel->setText(title);
}

void Player::loadAdvert(QString path)
{
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

	if (time <= 0)
	{
		qDebug() << "retry play";
		QTimer::singleShot(500, [=]()
		{
			ui.mpvFrame->PlayPause("");
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
			ui.mpvAdFrame->PlayPause("");
			ui.mpvAdFrame->LoadFileInfo();
			ui.mpvAdFrame->Play();
		});
	}
	//
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
		m_window->showContextMenu(event->globalPos());
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
	m_window->showPreferDialog();
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
