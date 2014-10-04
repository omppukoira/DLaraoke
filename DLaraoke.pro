#-------------------------------------------------
#
# Project created by QtCreator 2012-04-01T13:19:55
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = DLaraoke
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    MediaFile.cpp \
    mediafileview.cpp \
    ctool.cpp \
    settingsedit.cpp \
    dlaraoke.cpp \
    medialibraryedit.cpp \
    mediafilterproxymodel.cpp \
    playlistedit.cpp \
    playlist.cpp \
    playlistselect.cpp

HEADERS  += mainwindow.h \
    MediaFile.h \
    mediafileview.h \
    ctool.h \
    settingsedit.h \
    dlaraoke.h \
    medialibraryedit.h \
    mediafilterproxymodel.h \
    playlistedit.h \
    playlist.h \
    playlistselect.h

FORMS    += mainwindow.ui \
    settingsedit.ui \
    medialibraryedit.ui \
    playlistedit.ui \
    playlistselect.ui
