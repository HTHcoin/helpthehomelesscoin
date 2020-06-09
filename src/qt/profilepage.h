#ifndef PROFILEPAGE_H
#define PROFILEPAGE_H

#include <QDialog>

namespace Ui {
class profilepage;
}

class profilepage : public QDialog
{
    Q_OBJECT

public:
    explicit profilepage(QWidget *parent = 0);
    ~profilepage();

private:
    Ui::profilepage *ui;
};

#endif // PROFILEPAGE_H
