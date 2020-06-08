// We need to include a couple Qt classes that we'll use:
#include <QMainWindow>
#include <QTcpSocket>

// This is the include file that Qt generates for us from the
// GUI we built in Designer  
#include "ChatWindowPage.ui"

/*
 * This is the ChatWindowPage class that we have told to inherit from
 * our Designer ChatWindowPage (ui::ChatWindowPage)
 */
class ChatWindowPage : public QMainWindow, public Ui::ChatWindowPage
{
    Q_OBJECT

    public:

        // Every QWidget needs a constructor, and they should allow
        // the user to pass a parent QWidget (or not).
        ChatWindowPage(QWidget *parent=0);

    private Q_SLOTS:

        // This function gets called when a user clicks on the
        // loginButton on the front page (which you placed there
        // with Designer)
        void on_loginButton_clicked();

        // This gets called when you click the sayButton on
        // the chat page.
        void on_sayButton_clicked();

        // This is a function we'll connect to a socket's readyRead()
        // signal, which tells us there's text to be read from the chat
        // server.
        void readyRead();

        // This function gets called when the socket tells us it's connected.
        void connected();

    private:

        // This is the socket that will let us communitate with the server.
        QTcpSocket *socket;
};
