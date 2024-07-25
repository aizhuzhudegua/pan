#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QHostAddress>

TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    // 规定写法:
    QFile file(":/server.config");
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



