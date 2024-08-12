#include "tcpclient.h"
#include "opewidget.h"
#include <QApplication>
#include "sharefile.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TcpClient::getInstance().show();
//    ShareFile w;
//    w.show();

    return a.exec();
}
