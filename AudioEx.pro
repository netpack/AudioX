#-------------------------------------------------
#
# Project created by QtCreator 2014-07-28T22:03:30
#
#-------------------------------------------------

QT       += core gui webkit network phonon sql network xml


TARGET = AudioEx
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    aex_main.cpp \
    add_music.cpp \
    connect.cpp \
    add_music_single.cpp \
    player.cpp \
    mainwindowplayer.cpp \
    dialog.cpp \
    addgenre.cpp \
    addjingle.cpp \
    qvumeterplugin.cpp \
    qvumeter.cpp \
    add_pub.cpp \
    add_pub_select_date_time.cpp

HEADERS  += mainwindow.h \
    aex_main.h \
    add_music.h \
    connect.h \
    add_music_single.h \
    player.h \
    mainwindowplayer.h \
    dialog.h \
    addgenre.h \
    addjingle.h \
    audioclass.h \
    qvumeter.h \
    qvumeterplugin.h \
    add_pub.h \
    add_pub_select_date_time.h

FORMS    += mainwindow.ui \
    aex_main.ui \
    add_music.ui \
    add_music_single.ui \
    player.ui \
    mainwindowplayer.ui \
    dialog.ui \
    addgenre.ui \
    addjingle.ui \
    add_pub.ui \
    add_pub_select_date_time.ui

CONFIG += designer

OTHER_FILES += \
    ../../Documents/NETPACK/NetPackimgs/novo logo_12.png += \
    ./48x48.png

RESOURCES += \
    resources.qrc \
    jquery.qrc
