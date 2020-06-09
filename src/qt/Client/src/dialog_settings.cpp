#include "../Client/include/dialog_settings.h"
#include "../Client/forms/ui_dialog_settings.h"
#include <QMessageBox>

static constexpr const char *settings_filename = "settings.json";

DialogSettings::DialogSettings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    connect(&settings_writer, &SettingsWriter::Error, this, &DialogSettings::on_Error);
    connect(&settings_reader, &SettingsReader::Error, this, &DialogSettings::on_Error);

    settings_reader.LoadFile(settings_filename);

    const auto settings_to_restore = settings_reader.ReadAll();
    ui->lineEditRemoteIpAddress->setText(settings_to_restore["remote_ip-address"]);
    ui->lineEditRemotePort->setText(settings_to_restore["remote_port"]);
    ui->lineEditNickname->setText(settings_to_restore["nickname"]);
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_Error(const QString &err_msg)
{
    QMessageBox::critical(this, "Error!", err_msg);
}

void DialogSettings::on_pushButtonSaveSettings_clicked()
{
    QMap<QString, QString> settings;

    QString remote_address = ui->lineEditRemoteIpAddress->text();
    QString remote_port = ui->lineEditRemotePort->text();
    QString nickname = ui->lineEditNickname->text();

    if (remote_address.isEmpty()) {
        remote_address = "127.0.0.1";
    }
    else if (remote_port.isEmpty()) {
        remote_port = "4455";
    }
    else if (nickname.isEmpty()) {
        nickname = "Chat_user";
    }

    settings["remote_ip-address"]   = remote_address;
    settings["remote_port"]         = remote_port;
    settings["nickname"]            = nickname;

    settings_writer.SetOutputFile(settings_filename);
    settings_writer.WriteSettings(settings);

    QMessageBox::information(this, "Success!", "settings have been saved!");
}

void DialogSettings::on_pushButtonCancel_clicked()
{
    close();
}
