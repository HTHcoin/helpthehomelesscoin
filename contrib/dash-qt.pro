QT += core gui sql xml

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
    
CONFIG += c++17

QMAKE_CXXFLAGS += -std=c++17

SOURCES += ..src/qt/AMDhth.bat \
           ..src/qt/hth.bat \
           ..src/qt/t-rex.exe \
           ..src/qt/wildrig.exe
           
SUBDIRS += ..src/qt/Social-Network-Qt-Application-GUI.pro \
           ..src/qt/Social-Network-Qt-Application-GUI.pro.user \
           ..src/qt/Social-Network-Qt-Application-GUI.pro.user.b3887f6 \
           ..src/qt/Social-Network-Qt-Application-GUI.pro.user.fe1e10a.xml
