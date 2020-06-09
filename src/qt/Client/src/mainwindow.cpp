#include "../qt/Client/include/mainwindow.h"
#include "../qt/Client/ui_mainwindow.h"

/* Including other forms */
#include "../qt/Client/include/dialog_settings.h"    // DialogSettings class (form with a connection settings)

#include <QMessageBox>

static constexpr const char *settings_filename = "settings.json";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&socket_client, &SocketClient::outInfo, this, &MainWindow::on_OutInfo);
    connect(&socket_client, &SocketClient::Error, this, &MainWindow::on_Error);
    connect(&socket_client, &SocketClient::connected, this, &MainWindow::on_Connected);
    connect(&socket_client, &SocketClient::disconnected, this, &MainWindow::on_Disconnected);


    connect(&settings_reader, &SettingsReader::Error, this, &MainWindow::on_Error);

    // Loading file for reading
    settings_reader.LoadFile(settings_filename);

    // Reading settings from file
    const auto settings = settings_reader.ReadAll();
    remote_address  = settings["remote_ip-address"];
    remote_port     = settings["remote_port"].toUInt();
    nickname        = settings["nickname"];
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_OutInfo(const QString &message)
{
    ui->textEditOutput->append(message);
}

void MainWindow::on_Error(const QString &message)
{
     QMessageBox::critical(this, "Error!", message);
}

void MainWindow::on_Connected()
{
    ui->pushButtonDisconnectFromServer->setEnabled(true);
    ui->pushButtonSendMessage->setEnabled(true);
    ui->lineEditInputMessageToSend->setEnabled(true);
}

void MainWindow::on_Disconnected()
{
    ui->pushButtonDisconnectFromServer->setEnabled(false);
    ui->pushButtonSendMessage->setEnabled(false);
    ui->lineEditInputMessageToSend->setEnabled(false);
    ui->lineEditInputMessageToSend->clear();
}

/* TODO:
 * Check remote_address and nickname
 * Set address and port for connection with a server
 * Start the client for sendding and reciving messages
*/
void MainWindow::on_pushButtonConnectToServer_clicked()
{
    if (remote_address.isEmpty()) {
        QMessageBox::critical(this, "Error!", "Enter IP-address of the server!");
        return;
    }

    if (nickname.isEmpty()) {
        QMessageBox::critical(this, "Error!", "Enter your nickname!");
        return;
    }

    socket_client.setAddress(remote_address);
    socket_client.setPort(remote_port);
    socket_client.start();
}

/* TODO:
 * Read message from lineEditInputMessageToSend
 * Check this message
 * Create data to send (concatenate nickname with a message)
 * Call method for send data to server
*/
void MainWindow::on_pushButtonSendMessage_clicked()
{
    const QString message = ui->lineEditInputMessageToSend->text();

    if (message.isEmpty()) {
        QMessageBox::critical(this, "Error!", "Enter a message to send!");
        return;
    }

    socket_client.SendToServer(nickname, message);

    ui->lineEditInputMessageToSend->clear();
}

/* TODO:
 * Check textEditOutput (is empty or not)
 * If not empty --> call 'clear()' method
*/
void MainWindow::on_pushButtonClearOutput_clicked()
{
    if (not ui->textEditOutput->toPlainText().isEmpty()) {
        ui->textEditOutput->clear();
    }
}

/* TODO:
 * Show the 'DialogSettings' form
*/
void MainWindow::on_actionSettings_triggered()
{
    DialogSettings settings_dialog;
    settings_dialog.exec();
}

void MainWindow::on_pushButtonDisconnectFromServer_clicked()
{
    socket_client.stop();
}
