#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QMainWindow>
#include "statistics.h"
namespace Ui {
class AdminWindow;
}

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = 0);
    ~AdminWindow();

private slots:
    void on_showStatistics_clicked();

    void makePlot(QVector<Statistics> *plotData);
private:
    Ui::AdminWindow *ui;
};

#endif // ADMINWINDOW_H
