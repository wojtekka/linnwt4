######################################################################
# Automatically generated by qmake (2.01a) Do Dez 14 10:01:52 2006
######################################################################

TEMPLATE = app
TARGET = linnwt
INCLUDEPATH += .
QMAKE_CXXFLAGS = -ggdb

# Input
HEADERS += configfile.h \
           firmwidget.h \
           mainwindow.h \
           nwt7linux.h \
           optiondlg.h \
           fmarkedlg.h \
           widgetwobbeln.h \
           wkmanager.h \
           sondedlg.h \
           profildlg.h \
           konstdef.h
SOURCES += configfile.cpp \
           firmwidget.cpp \
           main.cpp \
           mainwindow.cpp \
           nwt7linux.cpp \
           optiondlg.cpp \
           fmarkedlg.cpp \
           widgetwobbeln.cpp \
           wkmanager.cpp \
           sondedlg.cpp \
           profildlg.cpp
RESOURCES += icon.qrc
DISTFILES += readme.txt \
 app_hu.qm \
 app_en.qm \
 app_nl.qm \
 app_ru.qm \
 app_es.qm
TRANSLATIONS += app_hu.ts \
 app_ru.ts \
 app_en.ts \
 app_nl.ts \
 app_es.ts \
 app_pl.ts

QT += widgets printsupport

!win32 {
     SOURCES += lin_modem.cpp
     HEADERS += lin_modem.h
 }
win32 {
     SOURCES += win_modem.cpp
     HEADERS += win_modem.h
 }

# Install

bin.files = linnwt
bin.path = /usr/local/bin/
icon.files = images/appicon.png
icon.path = /usr/share/icons/
INSTALLS += bin icon



