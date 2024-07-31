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

    resize(500,250);

    loadConfig();

    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));
    // 关联m_tcpSocket发出的读信号
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));

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

QString TcpClient::getLoginName()
{
    return m_strLoginName;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TcpClient::recvMsg()
{
    qDebug() << m_tcpSocket.bytesAvailable();
    uint uiPDULen = 0;
    // 传入的是一个起始地址，并且是char*型，一次偏移一个字节
    m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
    uint uiMsgLen = uiPDULen - sizeof(PDU);
    PDU *pdu = mkPDU(uiMsgLen);
    // 将uiPDULen字段之后的所有数据读入
    m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

    switch (pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_RESPOND:
    {
        if(0 == strcmp(pdu->caData,REGIST_OK)){
            QMessageBox::information(this,"注册",REGIST_OK);
        }
        else if(0 == strcmp(pdu->caData,REGIST_FAILED))
        {
            QMessageBox::information(this,"注册",REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_RESPOND:
    {
        if(0 == strcmp(pdu->caData,LOGIN_OK)){
            QMessageBox::information(this,"登录",LOGIN_OK);
            OpeWidget::getInstance().show();
            hide();

        }
        else if(0 == strcmp(pdu->caData,LOGIN_FAILED))
        {
            QMessageBox::information(this,"登录",LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
    {
        OpeWidget::getInstance().getFrient()->showAllOnlineUsr(pdu);
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
    {
        if(0 == strcmp(SEARCH_USR_NO,pdu->caData)){
            QMessageBox::information(this,"搜索",QString("%1: not exist").arg(OpeWidget::getInstance().getFrient()->m_strSearchName));
        }
        else if (0 == strcmp(SEARCH_USR_ONLINE,pdu->caData)){
            QMessageBox::information(this,"搜索",QString("%1: online").arg(OpeWidget::getInstance().getFrient()->m_strSearchName));
        }
        else if (0 == strcmp(SEARCH_USR_OFFLINE,pdu->caData)) {
            QMessageBox::information(this,"搜索",QString("%1: offline").arg(OpeWidget::getInstance().getFrient()->m_strSearchName));
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        // 获取请求源用户
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData+32,32);
        int ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend").arg(caName)
                                 ,QMessageBox::Yes,QMessageBox::No);
        PDU *respdu = mkPDU(0);
        // 把双方用户名带上
        memcpy(respdu->caData,pdu->caData,32);
        memcpy(respdu->caData+32,pdu->caData+32,32);
        qDebug() << pdu->caData;
        if(ret == QMessageBox::Yes){
            // 同意
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
        }
        else{
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
        }
        m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        // 请求目标
        char caPerName[32] = {'\0'};
        strncpy(caPerName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",QString("%1 agree you request").arg(caPerName));
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        // 请求目标
        char caPerName[32] = {'\0'};
        strncpy(caPerName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",QString("%1 refuse you request").arg(caPerName));
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
    {
        OpeWidget::getInstance().getFrient()->updateFriendList(pdu);
        break;
    }
    // 这是服务器转发过来的提示
    case ENUM_MSG_TYPE_DEL_FRIEND_REQUEST:
    {
        char caFriendName[32] = {'\0'};
        memcpy(caFriendName,pdu->caData,32);
        QMessageBox::information(this,"删除好友",QString("%1 已将你从好友列表中删除").arg(caFriendName));
        break;
    }
    case ENUM_MSG_TYPE_DEL_FRIEND_RESPOND:
    {
        QMessageBox::information(this,"删除好友",pdu->caData);
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;

}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}


//void TcpClient::on_send_pd_clicked()
//{
//    QString strMsg = ui->lineEdit->text();
//    if(!strMsg.isEmpty()){
//        PDU* pdu = mkPDU(strMsg.size());
//        pdu->uiMsgType = 8888;
//        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());
//        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
//        free(pdu);
//        pdu = NULL;
//    }
//    else{
//        QMessageBox::warning(this,"信息发送","发送的信息为空");
//    }
//}

void TcpClient::on_login_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        m_strLoginName = strName;
        // 发送登录请求
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        // caData的前32B存放用户名，后32B存放密码
        strncpy(pdu->caData, strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this,"登录","登陆失败：用户名或密码为空！");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        // 发送注册请求
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        // caData的前32B存放用户名，后32B存放密码
        strncpy(pdu->caData, strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this,"注册","注册失败：用户名或密码为空！");
    }
}

void TcpClient::on_cancel_pb_clicked()
{

}
