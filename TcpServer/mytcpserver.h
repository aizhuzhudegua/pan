#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    // 重写虚函数，只要有客户端连接到服务器，就会自动调用
    void incomingConnection(qintptr socketDescriptor);

    static MyTcpServer& getInstance();

    // 转发
    void resend(const char *pername,const PDU* pdu);

public slots:
    void deleteSocket(MyTcpSocket *mysocket);

private:
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MY TCPSERVER_H
