#-------------------------------------------------
#
# Project created by QtCreator 2011-01-23T16:09:56
#
#-------------------------------------------------

QT = core network

TARGET = qcc-server
TEMPLATE = app
CONFIG += console link_prl
CONFIG -= app_bundle

QCC_CORE_PREFIX = ../qcc-core

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/debug
} else {
    DEFINES += DEBUG
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/release
}

INCLUDEPATH += ../qcc-core
LIBS += -L$$QCC_CORE_PREFIX -lqcc-core

HEADERS += server.h \
           user.h

SOURCES += main.cpp \
           server.cpp \
           user.cpp
