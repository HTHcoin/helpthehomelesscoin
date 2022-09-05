#ifndef WORLDPAGE_H
#define WORLDPAGE_H

#include <QWidget>
#include "annwidget.h"


#include <QDesktopServices>
#include <QProcess>
#include <QUrl>




namespace Ui {
class WorldPage;



}

class WorldPage : public QWidget
{
    Q_OBJECT

public:
    explicit WorldPage(QWidget *parent = 0);
    ~WorldPage();

private:
    Ui::WorldPage *ui;


private Q_SLOTS:
//    void on_annList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_pushButton_Website_clicked();
    void on_pushButton_Website_1_clicked();
    void on_pushButton_Website_2_clicked();
   /* void on_pushButton_Amazon_clicked(); */
    void on_pushButton_Website_4_clicked();
    void on_pushButton_Website_5_clicked();
    void on_pushButton_Website_6_clicked();
    void on_pushButton_Website_7_clicked();
   /* void on_pushButton_Website_8_clicked(); */
    void on_pushButton_Website_9_clicked();
    void on_pushButton_Website_10_clicked();

};

#endif // WORLDPAGE_H
