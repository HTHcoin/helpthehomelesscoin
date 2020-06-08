#ifndef SETTINGSREADER_H
#define SETTINGSREADER_H

#include <QObject>
#include <QMap>
#include <QFile>

class SettingsReader : public QObject
{
    Q_OBJECT

private:
    QFile input_file;

public:
    SettingsReader();
    ~SettingsReader();
    void LoadFile(const char *filename);
    QMap<QString, QString> ReadAll();

signals:
    void Error(const QString &err_msg);
};

#endif // SETTINGSREADER_H
