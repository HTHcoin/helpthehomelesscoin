#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QDialog>
#include <QDesktopServices>
#include <QUrl>

namespace Ui {
class AboutPage;
}

class AboutPage : public QDialog
{
    Q_OBJECT

public:
    explicit AboutPage(QWidget *parent = 0);
    ~AboutPage();

private slots:
    void on_visitTorPage_clicked();
    void on_visitCreatorsPage_pressed();

private:
    Ui::AboutPage *ui;
};

#endif // ABOUTPAGE_H
