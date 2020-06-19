  
#include "mainwindow.h"
#include "formlogin.h"
#include <QApplication>
 
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow* mainWindow = new MainWindow();
    FormLogin* formLogin = new FormLogin(mainWindow);
    formLogin->show();
    mainWindow->show();
 
    return a.exec();
}
