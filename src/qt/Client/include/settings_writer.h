#ifndef SETTINGSWRITER_H
#define SETTINGSWRITER_H

#include <QObject>
#include <QMap>
#include <QFile>

class SettingsWriter : public QObject
{
    Q_OBJECT

private:
    QFile output_file;

public:
    SettingsWriter();
    ~SettingsWriter();
    void SetOutputFile(const char *filename);
    void WriteSettings(QMap<QString, QString> &settings);

Q_SIGNALS:
    void Error(const QString &err_msg);
};

#endif // SETTINGSWRITER_H
