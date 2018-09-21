TEMPLATE = app
TARGET = BiliPlayer
INCLUDEPATH += . src src/Common src/Danmaku src/Player src/Widget

QT += widgets network

macx:LIBS += -L/usr/local/lib -lmpv

mac {
    # Only include / compile these files on OS X
    OBJECTIVE_SOURCES += \
        src/Common/OSXHideTitleBar.mm
    HEADERS  +=\
        src/Common/OSXHideTitleBar.h

    # Additionally include Cocoa for OS X code
    LIBS += -framework Foundation -framework Cocoa
    INCLUDEPATH += /System/Library/Frameworks/Foundation.framework/Versions/C/Headers
}

# Input
HEADERS += mpv/client.h \
           mpv/opengl_cb.h \
           mpv/qthelper.hpp \
           mpv/render.h \
           mpv/render_gl.h \
           mpv/stream_cb.h \
           src/Application.h \
           src/Common/Common.h \
           src/Common/Setting.h \
           src/Common/Logger.h \
           src/Common/Utils.h \
           src/Danmaku/Attribute.h \
           src/Danmaku/Danmaku.h \
           src/Danmaku/Graphic.h \
           src/Danmaku/Parse.h \
           src/Danmaku/Sprite.h \
           src/Player/MpvTypes.h \
           src/Player/MpvWidget.h \
           src/Widget/ARender.h \
           src/Widget/Player.h \
           src/Widget/Prefer.h \
           src/Widget/SeekBar.h \
           src/Widget/SpeedMenu.h \
           src/Widget/VolumeMenu.h \
           src/Widget/Window.h \
    src/Widget/DanmakuMenu.h \
    src/Widget/RecentWidget.h \
    src/Widget/RecentWidgetItem.h \
    src/Widget/AdvertInfoWidget.h \
    src/Widget/AdvertWidget.h \
    src/Common/FramelessHelper.h \
    src/Common/Network.h
FORMS += src/Widget/Player.ui src/Widget/SpeedMenu.ui src/Widget/VolumeMenu.ui \
    src/Widget/DanmakuMenu.ui \
    src/Widget/RecentWidget.ui \
    src/Widget/RecentWidgetItem.ui \
    src/Widget/AdvertInfoWidget.ui \
    src/Widget/AdvertWidget.ui
SOURCES += main.cpp \
           src/Application.cpp \
           src/Common/Setting.cpp \
           src/Common/Logger.cpp \
           src/Common/Utils.cpp \
           src/Danmaku/Attribute.cpp \
           src/Danmaku/Danmaku.cpp \
           src/Danmaku/Graphic.cpp \
           src/Danmaku/Parse.cpp \
           src/Danmaku/Sprite.cpp \
           src/Player/MpvWidget.cpp \
           src/Widget/ARender.cpp \
           src/Widget/Player.cpp \
           src/Widget/Prefer.cpp \
           src/Widget/SeekBar.cpp \
           src/Widget/SpeedMenu.cpp \
           src/Widget/VolumeMenu.cpp \
           src/Widget/Window.cpp \
    src/Widget/DanmakuMenu.cpp \
    src/Widget/RecentWidget.cpp \
    src/Widget/RecentWidgetItem.cpp \
    src/Widget/AdvertInfoWidget.cpp \
    src/Widget/AdvertWidget.cpp \
    src/Common/FramelessHelper.cpp \
    src/Common/Network.cpp
RESOURCES += src/Resource/player.qrc
TRANSLATIONS += src/Resource/lang/biliplayer_zh.ts
