#include "../qt/Server/include/mainwindow.h"
#include "../qt/Server/forms/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , line_counter(0)
{
    ui->setupUi(this);
    setWindowTitle("Socket server");

    connect(&socket_server, &SocketServer::OutInfo, this, &MainWindow::on_OutInfo);
    connect(&socket_server, &SocketServer::Error, this, &MainWindow::on_Error);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_OutInfo(const QString &message)
{
    ui->textEditServerOutput->append(message);
    line_counter++;
}

void MainWindow::on_Error(const QString &error_message)
{
    ui->textEditServerOutput->append(error_message);
    line_counter++;

    if (line_counter == 450) {
        ui->textEditServerOutput->clear();
    }
}

void MainWindow::on_pushButtonStartServer_clicked()
{
    socket_server.setPort(4455);
    socket_server.start();
}

void MainWindow::on_pushButtonStopServer_clicked()
{
    socket_server.stop();
    if (QMessageBox::question(this, "Question", "Do you want to close application?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        QCoreApplication::quit();
    }
}
