#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

#include <QDialog>

/* Including other modules */
#include "settings_reader.h"    // class SettingsReader (for reading settings)
#include "settings_writer.h"    // class SettingsWriter (for writting settings)

namespace Ui { class DialogSettings; }

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = nullptr);
    ~DialogSettings();

private slots:
    // Slot for handling 'Error' signal from the class SettingsReader and SettingsWriter
    void on_Error(const QString &err_msg);

    // Buttons slots
    void on_pushButtonSaveSettings_clicked();
    void on_pushButtonCancel_clicked();

private:
    Ui::DialogSettings *ui;
    SettingsWriter settings_writer;
    SettingsReader settings_reader;
};

#endif // DIALOG_SETTINGS_H
