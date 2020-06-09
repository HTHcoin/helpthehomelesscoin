#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "socket_server.h"

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
    // Slots for handling signals from 'SocketServer' class
    void on_OutInfo(const QString &message);
    void on_Error(const QString &error_message);

    // Buttons slots
    void on_pushButtonStartServer_clicked();
    void on_pushButtonStopServer_clicked();

private:
    Ui::MainWindow *ui;
    SocketServer socket_server;
    quint16 line_counter;
};
#endif // MAINWINDOW_H
