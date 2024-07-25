#include "mytcpsocket.h"
#include <QDebug>

MyTcpSocket::MyTcpSocket()
{
    // 本身发出的readyRead信号由本身的槽函数处理
    connect(this,SIGNAL(readyRead())
            ,this,SLOT(recvMsg()));
}

void MyTcpSocket::recvMsg()
{
    qDebug() << this->bytesAvailable();
    uint uiPDULen = 0;
    // 传入的是一个起始地址，并且是char*型，一次偏移一个字节
    this->read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    // 将uiPDULen字段之后的所有数据读入
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
    qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
}
