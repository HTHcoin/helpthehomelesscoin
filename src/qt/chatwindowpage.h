#ifndef CHATWINDOWPAGE_H
#define CHATWINDOWPAGE_H

#include <QMainWindow>
#include"servedev.h"
namespace Ui {
class ChatWindowPage;
}
class servedev;
class ChatWindowPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatWindowPage(QWidget *parent = 0);
    ~ChatWindowPage();
    void addMessage(QString Msg);
    servedev* m_pBoxServer;
private Q_SLOTS:
    void on_pushButtonStart_clicked();

private:
    Ui::ChatWindowPage *ui;
};

#endif // CHATWINDOWPAGE_H
