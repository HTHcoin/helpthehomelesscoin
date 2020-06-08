#include "../include/settings_writer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariant>

SettingsWriter::SettingsWriter()
{}

SettingsWriter::~SettingsWriter()
{}

void SettingsWriter::SetOutputFile(const char *filename)
{
    output_file.setFileName(filename);
}

void SettingsWriter::WriteSettings(QMap<QString, QString> &settings)
{
    if (not output_file.open(QIODevice::WriteOnly)) {
        emit Error(QString("Could not find the settings file!"));
        return;
    }

    QVariantMap data_to_write;

    data_to_write["remote_ip-address"]  = settings["remote_ip-address"];
    data_to_write["remote_port"]        = settings["remote_port"];
    data_to_write["nickname"]           = settings["nickname"];

    output_file.write(QJsonDocument(QJsonObject::fromVariantMap(data_to_write)).toJson());
    output_file.close();
}
