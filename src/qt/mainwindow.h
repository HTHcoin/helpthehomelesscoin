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
    MainWindow(int userID );
    int id;
    ~MainWindow();

private Q_SLOTS:
    void on_registrButton_clicked();

    void on_loginButton_clicked();
    
    void on_forgotButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
