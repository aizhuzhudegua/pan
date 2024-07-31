#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"

MyTcpSocket::MyTcpSocket()
{
    // 本身发出的readyRead信号由本身的槽函数处理
    connect(this,SIGNAL(readyRead())
            ,this,SLOT(recvMsg()));

    connect(this,SIGNAL(disconnected()),
            this,SLOT(clientOffLine()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
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

    PDU *respdu;
    switch (pdu->uiMsgType) {
    case ENUM_MSG_TYPE_REGIST_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleRegist(caName,caPwd);

        // 返回注册信息
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
        if(ret){
            strcpy(respdu->caData,REGIST_OK);
        }
        else
        {
            strcpy(respdu->caData,REGIST_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_LOGIN_REQUEST:
    {
        char caName[32] = {'\0'};
        char caPwd[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        strncpy(caPwd,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleLogin(caName,caPwd);

        // 返回登录信息
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
        if(ret){
            strcpy(respdu->caData,LOGIN_OK);
            m_strName = caName;
        }
        else
        {
            strcpy(respdu->caData,LOGIN_FAILED);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
    {
        qDebug() << "收到请求" << ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        QStringList ret = OpeDB::getInstance().handAllOline();
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
        for(int i = 0;i<ret.size();i++){
            memcpy((char*)(respdu->caMsg)+i*32,ret[i].toStdString().c_str(),ret[i].size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
    {
        int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
        if(-1 == ret){
            strcpy(respdu->caData,SEARCH_USR_NO);
        }
        else if(1 == ret){
            strcpy(respdu->caData,SEARCH_USR_ONLINE);
        }
        else if(0 == ret){
            strcpy(respdu->caData,SEARCH_USR_OFFLINE);
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
    {
        char caPerName[32] = {'\0'};
        char caName[32] = {'\0'};
        strncpy(caPerName,pdu->caData,32);
        strncpy(caName,pdu->caData+32,32);
        int ret = OpeDB::getInstance().handleAddFriend(caPerName,caName);

        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
        if(-1 == ret){
            strcpy(respdu->caData,UNKNOW_ERROR);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(0 == ret)
        {
            strcpy(respdu->caData,EXISTED_FRIEND);
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(1 == ret)
        {
            // 转发好友请求
            MyTcpServer::getInstance().resend(caPerName,pdu);
        }
        else if(2 == ret)
        {
             strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
             write((char*)respdu, respdu->uiPDULen);
             free(respdu);
             respdu = NULL;
        }
        else if(3 == ret)
        {
             strcpy(respdu->caData,ADD_FRIEND_NOT_EXIST);
             write((char*)respdu, respdu->uiPDULen);
             free(respdu);
             respdu = NULL;
        }

        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
    {
        // 录入数据库
        // 目标用户
        char caPerName[32] = {'\0'};
        // 源用户
        char caName[32] = {'\0'};
        strncpy(caPerName,pdu->caData,32);
        strncpy(caName,pdu->caData+32,32);
        bool ret = OpeDB::getInstance().handleAgreeFriend(caPerName,caName);
        if(ret){
            // 插入成功
            strncpy(caName,pdu->caData+32,32);
            qDebug() << "转发答复:" << caName;
            MyTcpServer::getInstance().resend(caName,pdu);
        }
        else
        {
            // 插入失败，未知错误
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            strcpy(respdu->caData,UNKNOW_ERROR);
            MyTcpServer::getInstance().resend(caName,respdu);
            free(respdu);
            respdu=NULL;
        }
        break;
    }
    case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
    {
        // 转发答复
        // 源用户
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData+32,32);
         qDebug() << "转发答复:" << caName;
        MyTcpServer::getInstance().resend(caName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        QStringList ret = OpeDB::getInstance().handleFlushFriend(caName);
        uint uiMsgLen = ret.size()*32;
        PDU *respdu = mkPDU(uiMsgLen);
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
        for(int i=0;i<ret.size();i ++){
            memcpy((char*)(respdu->caMsg)+i*32,ret[i].toStdString().c_str(),ret[i].size());
        }
        write((char*)respdu, respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DEL_FRIEND_REQUEST:
    {
        char caMyselfName[32] = {'\0'};
        char caMyFriendName[32] = {'\0'};
        memcpy(caMyselfName,pdu->caData,32);
        memcpy(caMyFriendName,pdu->caData+32,32);
        qDebug() << "handle：" << caMyFriendName;
        bool ret = OpeDB::getInstance().handleDelFriend(caMyselfName,caMyFriendName);
        if( ret ){
            // 给发送方的回复
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_FRIEND_RESPOND;
            strcpy(respdu->caData,DELETE_FRIEND_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            // 给好友的通知
            MyTcpServer::getInstance().resend(caMyFriendName,pdu);
        }
        break;
    }
    default:
        break;
    }
    free(pdu);
    pdu = NULL;

}

void MyTcpSocket::clientOffLine()
{
    OpeDB::getInstance().handleOffLine(m_strName.toStdString().c_str());
    emit offline(this);
}
