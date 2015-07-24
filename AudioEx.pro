#-------------------------------------------------
#
# Project created by QtCreator 2014-07-28T22:03:30
#
#-------------------------------------------------
#LIBS += /usr/share/qtcreator/qbs/share/qbs/modules/Qt/phonon/phonon.qbs



QT       += core gui network sql phonon webkit webkitwidgets
CONFIG += qt designer


TARGET = AudioEx
TEMPLATE = app


SOURCES +=\
    aex_main.cpp \
    connect.cpp \
    add_music_single.cpp \
    player.cpp \
    dialog.cpp \
    addgenre.cpp \
    addjingle.cpp \
    add_pub.cpp \
    manage_db.cpp \
    main.cpp \
    optionsdialog.cpp \
    youtubedownloader.cpp \
    commonFunctions.cpp

HEADERS  += \
    aex_main.h \
    connect.h \
    add_music_single.h \
    player.h \
    dialog.h \
    addgenre.h \
    addjingle.h \
    audioclass.h \
    qvumeter.h \
    qvumeterplugin.h \
    add_pub.h \
    manage_db.h \
    optionsdialog.h \
    youtubedownloader.h \
    commonFunctions.h

FORMS    += \
    aex_main.ui \
    add_music_single.ui \
    player.ui \
    dialog.ui \
    addgenre.ui \
    addjingle.ui \
    add_pub.ui \
    manage_db.ui \
    optionsdialog.ui \
    youtubedownloader.ui


RESOURCES += \
    resources.qrc \
    jquery.qrc

LIBS += -L/usr/lib/qt4/plugins/designer -lqvumeterplug
LIBS += /home/fred/cpp/AudioEx/libqvumeterplug.so
