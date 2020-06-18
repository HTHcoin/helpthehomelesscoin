#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void on_signUpButton_clicked();

    void on_logInButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
