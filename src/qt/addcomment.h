#ifndef ADDCOMMENT_H
#define ADDCOMMENT_H

#include <QDialog>

namespace Ui {
class AddComment;
}

class AddComment : public QDialog
{
    Q_OBJECT

public:
    explicit AddComment(QWidget *parent = 0);

    ~AddComment();

public Q_SLOTS:
    QString getCommentBody();


private Q_SLOTS:
    void on_AddComment_accepted();


private:
    Ui::AddComment *ui;

    QString commentBody;
};

#endif // ADDCOMMENT_H
