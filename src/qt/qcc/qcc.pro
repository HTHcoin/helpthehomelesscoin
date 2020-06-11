#-------------------------------------------------
#
# Project created by QtCreator 2011-01-23T16:10:20
#
#-------------------------------------------------

QT = core gui network

TARGET = qcc
TEMPLATE = app
CONFIG += link_prl crypto

QCC_CORE_PREFIX = ../qcc-core

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/debug
} else {
    DEFINES += RELEASE
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/release
}

INCLUDEPATH += ../qcc-core
LIBS += -L$$QCC_CORE_PREFIX -lqcc-core

HEADERS += mainwindow.h \
           registerdialog.h \
           messagewindow.h \
           messagepage.h \
           contact.h \
           contactlistmodel.h

SOURCES += main.cpp \
           mainwindow.cpp \
           registerdialog.cpp \
           messagewindow.cpp \
           messagepage.cpp \
           contact.cpp \
           contactlistmodel.cpp

FORMS += mainwindow.ui \
         registerdialog.ui \
         messagewindow.ui \
         messagepage.ui

RESOURCES += icons.qrc

win32:RC_FILE = qcc.rc

OTHER_FILES += qcc.rc
