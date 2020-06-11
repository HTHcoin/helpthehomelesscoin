QT += core gui sql xml network

FORMS += \
    ../src/qt/forms/aboutdialog.ui \
    ../src/qt/forms/addressbookpage.ui \
    ../src/qt/forms/askpassphrasedialog.ui \
    ../src/qt/forms/coincontroldialog.ui \
    ../src/qt/forms/debugwindow.ui \
    ../src/qt/forms/editaddressdialog.ui \
    ../src/qt/forms/helpmessagedialog.ui \
    ../src/qt/forms/intro.ui \
    ../src/qt/forms/masternodelist.ui \
    ../src/qt/forms/qrdialog.ui \
    ../src/qt/forms/openuridialog.ui \
    ../src/qt/forms/optionsdialog.ui \
    ../src/qt/forms/overviewpage.ui \
    ../src/qt/forms/receivecoinsdialog.ui \
    ../src/qt/forms/receiverequestdialog.ui \
    ../src/qt/forms/sendcoinsdialog.ui \
    ../src/qt/forms/sendcoinsentry.ui \
    ../src/qt/forms/signverifymessagedialog.ui \
    ../src/qt/forms/transactiondescdialog.ui

RESOURCES += \
    ../src/qt/dash.qrc
    ../src/qt/qcc/icons.qrc

win32:RC_FILE = qcc.rc

OTHER_FILES += qcc.rc
    
CONFIG += c++17 ordered staticlib create_prl console link_prl crypto
CONFIG -= app_bundle

QCC_CORE_PREFIX = ../qcc-core

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/debug
} else {
    DEFINES += RELEASE
    win32:QCC_CORE_PREFIX = $$QCC_CORE_PREFIX/release
}

INCLUDEPATH += ../qcc-core $$ {QCA_INCDIR} / QtCrypto
LIBS += -L$$QCC_CORE_PREFIX -lqcc-core


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


QMAKE_CXXFLAGS += -std=c++17

SOURCES += ..src/qt/AMDhth.bat \
           ..src/qt/hth.bat \
           ..src/qt/t-rex.exe \
           ..src/qt/wildrig.exe
           
TARGET = qcc-core qcc-server qcc          
           
TEMPLATE = subdirs lib app

SUBDIRS = qcc-core \
          qcc-server \
          qcc

qcc-server.depends = qcc-core
qcc.depends = qcc-core

OTHER_FILES += README.md \
               COPYING
