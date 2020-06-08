#include "chatwindowpage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChatWindowPage w;
    w.show();

    return a.exec();
}
