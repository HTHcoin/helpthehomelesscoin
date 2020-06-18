#ifndef MAINWINDOW_H
#define MAINWINDOW_H
 
#include <QMainWindow>
#include "secdialog.h"
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

    void on_pushButton_Login_clicked();
 
private:
    Ui::MainWindow *ui;
    SecDialog *secDialog;
};
 
#endif // MAINWINDOW_H
