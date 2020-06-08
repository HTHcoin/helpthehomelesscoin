#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

/* Including other modules */
#include "socket_client.h"      // class SocketClient   (the base class of a client side)
#include "settings_reader.h"    // class SettingsReader (for reading settings)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    // Slots for handling signals from the class SocketClient
    void on_OutInfo(const QString &message);
    void on_Error(const QString &message);
    void on_Connected();
    void on_Disconnected();

    // Buttons slots
    void on_pushButtonConnectToServer_clicked();
    void on_pushButtonSendMessage_clicked();
    void on_pushButtonClearOutput_clicked();
    void on_pushButtonDisconnectFromServer_clicked();

    // Menu bar actions (slots)
    void on_actionSettings_triggered();

private:
    Ui::MainWindow *ui;
    SocketClient socket_client;
    SettingsReader settings_reader;
    QString remote_address;
    quint16 remote_port;
    QString nickname;
};

#endif // MAINWINDOW_H
