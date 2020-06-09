#include "../include/settings_reader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

SettingsReader::SettingsReader()
{}

SettingsReader::~SettingsReader()
{}

void SettingsReader::LoadFile(const char *filename)
{
    input_file.setFileName(filename);
}

QMap<QString, QString> SettingsReader::ReadAll()
{
    QMap<QString, QString> settings;
    if (not input_file.open(QIODevice::ReadOnly)) {
        Q_EMIT Error(QString("Could not find the settings file!"));
        return settings;
    }

    QByteArray data = input_file.readAll();
    QJsonDocument json_doc = QJsonDocument::fromJson(data);
    QJsonObject json_obj = json_doc.object();

    settings["remote_ip-address"]   = json_obj["remote_ip-address"].toString();
    settings["remote_port"]         = json_obj["remote_port"].toString();
    settings["nickname"]            = json_obj["nickname"].toString();

    input_file.close();
    return settings;
}
