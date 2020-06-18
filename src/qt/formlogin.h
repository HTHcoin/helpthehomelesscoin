#ifndef FORMLOGIN_H
#define FORMLOGIN_H
 
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
 
class FormLogin : public QDialog
{
    Q_OBJECT
 
public:
    FormLogin(QWidget* parent = 0);
    ~FormLogin();
 
private Q_SLOTS:
    void OnQuit();
    void OnLogin();
 
private:
    void reject();
 
    QLabel* userLabel;
    QLabel* passLabel;
    QLineEdit* userLineEdit;
    QLineEdit* passLineEdit;
    QPushButton* loginButton;
    QPushButton* quitButton;
};
 
#endif // FORMLOGIN_H
