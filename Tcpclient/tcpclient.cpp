#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QHostAddress>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();

    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));

    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    // 规定写法:
    QFile file(":/client.config");
    // 只读打开
    if(file.open(QIODevice::ReadOnly)){
        QByteArray baData = file.readAll();
        // 转换成char*型
        QString strData = baData.toStdString().c_str();
        file.close();
        strData.replace("\r\n"," ");
        QStringList strList = strData.split(" ");
        // 设置IP和host
        m_strIP = strList[0];
        m_usPort = strList[1].toUShort();
        qDebug() << m_strIP;
        qDebug() << m_usPort;

    }else{
        QMessageBox::critical(this,"open config","open config failed");
    }

}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}


void TcpClient::on_send_pd_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if(!strMsg.isEmpty()){
        PDU* pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else{
        QMessageBox::warning(this,"信息发送","发送的信息为空");
    }
}
