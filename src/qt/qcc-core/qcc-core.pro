#-------------------------------------------------
#
# Project created by QtCreator 2011-01-25T13:46:15
#
#-------------------------------------------------

QT = core network

TARGET = qcc-core
TEMPLATE = lib
CONFIG += staticlib create_prl

HEADERS += qccpacket.h \
           abstractuser.h

SOURCES += qccpacket.cpp \
           abstractuser.cpp
