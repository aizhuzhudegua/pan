#include "mytcpserver.h"
#include <QDebug>

MyTcpServer::MyTcpServer()
{

}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "new client connected!";
    MyTcpSocket *pTcpSocket = new MyTcpSocket();
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),
            this,SLOT(deleteSocket(MyTcpSocket*)));
}

MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return  instance;
}

void MyTcpServer::resend(const char *pername, const PDU *pdu)
{
    if(NULL == pername || NULL == pdu){
        return;
    }
    for(int i=0;i<m_tcpSocketList.size();i++){
        if(pername == m_tcpSocketList[i]->getName()){
            m_tcpSocketList[i]->write((char*)pdu,pdu->uiPDULen);
        }
    }
}

void MyTcpServer::deleteSocket(MyTcpSocket *mysocket)
{
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(;iter < m_tcpSocketList.end(); iter++){
        if(mysocket == *iter){
            // delete *iter;
            (*iter) -> deleteLater(); // 释放空间
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }
    for(int i=0;i<m_tcpSocketList.size();i++){
        qDebug() << "name:" <<m_tcpSocketList[i]->getName();
    }
}
