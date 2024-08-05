#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>

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
            // 创建用户文件夹
            QDir dir;
            if(! dir.exists("./users")){
                dir.mkdir("./users");
            }
            qDebug() << "create dir: " << dir.mkdir(QString("./users/%1").arg(caName));
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
    case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
    {

        char caMyFriendName[32] = {'\0'};
        memcpy(caMyFriendName,pdu->caData+32,32);
        qDebug() << caMyFriendName;
        MyTcpServer::getInstance().resend(caMyFriendName,pdu);
        break;
    }
    case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData,32);
        QStringList olineFriend = OpeDB::getInstance().handleFlushFriend(caName);
        for(int i=0;i<olineFriend.size();i++){
            MyTcpServer::getInstance().resend(olineFriend[i].toStdString().c_str(),pdu);
        }
        break;
    }
    case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
    {
        QDir dir;
        QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
        qDebug() << strCurPath;
        bool ret = dir.exists(strCurPath);

        PDU* respdu = NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
        if(ret){ // 当前目录存在
            char caNewDir[32] = {'\0'};
            memcpy(caNewDir,pdu->caData+32,32);
            QString strNewPath = strCurPath+"/"+caNewDir;
            qDebug() << strNewPath;

            ret = dir.exists(strNewPath);
            // 新目录已存在
            if(ret){
                strcpy(respdu->caData,DIR_NAME_EXIST);
            }
            // 新目录不存在
            else
            {
                if(dir.mkdir(strNewPath))
                {
                    // 创建文件夹成功
                    strcpy(respdu->caData,CREATE_DIR_OK);
                }
                else
                {
                    // 未知错误
                    strcpy(respdu->caData,UNKNOW_ERROR);
                }
            }
        }
        else // 当前目录不存在
        {
            strcpy(respdu->caData,DIR_NOT_EXIST);
        }
        write((char*) respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
    {
        char *pCurPath = new char[pdu->uiMsgLen];
        memcpy(pCurPath,pdu->caMsg,pdu->uiMsgLen);
        QDir dir(pCurPath);
        QFileInfoList fileInfoList = dir.entryInfoList();
        int iFileCount = fileInfoList.size();
        PDU *respdu = mkPDU((iFileCount)*sizeof(FileInfo));
        respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
        FileInfo *pFileInfo = NULL;
        QString strFileName;
        for(int i=0;i<iFileCount;i++)
        {

            // 指向每一个FileInfo
            pFileInfo = (FileInfo*)respdu->caMsg + i;
            strFileName = fileInfoList[i].fileName();
            memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());


            if(fileInfoList[i].isDir()){
                pFileInfo->iFileType = 0;
            }
            if(fileInfoList[i].isFile()){
                pFileInfo->iFileType = 1;
            }
            qDebug() << fileInfoList[i].fileName()
                     << fileInfoList[i].size()
                     << " 文件夹：" << fileInfoList[i].isDir()
                     << " 常规文件：" <<  fileInfoList[i].isFile()
                     << pFileInfo->iFileType
                     << pFileInfo->caName;
        }
        write((char*) respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
    {
        char caName[32] = {'\0'};
        strcpy(caName,pdu->caData);
        char *pPath = new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        QString strPath = QString("%1/%2").arg(pPath).arg(caName);
        qDebug() << strPath;

        QFileInfo fileInfo(strPath);
        bool ret = false;
        if(fileInfo.isDir())
        {
            QDir dir;
            dir.setPath(strPath);
            // 递归删除
            ret = dir.removeRecursively();
        }
        else if(fileInfo.isFile())  // 常规文件不删除
        {
            ret = false;
        }
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_TYPE_DEL_DIR_RESPOND;
        if(ret)
        {
            memcpy(respdu->caData,DEL_DIR_OK,strlen(DEL_DIR_OK));
        }
        else
        {
            memcpy(respdu->caData,Del_DIR_FAILURED,strlen(Del_DIR_FAILURED));
        }
        write((char*) respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
    {
        char caOldName[32] = {'\0'};
        char caNewName[32] = {'\0'};
        strncpy(caOldName,pdu->caData,32);
        strncpy(caNewName,pdu->caData+32,32);

        char *pPath = new char[pdu->uiMsgLen];
        strncpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);

        QString OldPath = QString("%1/%2").arg(pPath).arg(caOldName);
        QString NewPath = QString("%1/%2").arg(pPath).arg(caNewName);

        qDebug() << OldPath;
        qDebug() << NewPath;

        QDir dir;
        bool ret = dir.rename(OldPath,NewPath);
        PDU *respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
        if(ret)
        {
            strcpy(respdu->caData,RENAME_FILE_OK);
        }
        else
        {
            strcpy(respdu->caData,RENAME_FILE_FAILUERD);
        }
        write((char*) respdu,respdu->uiPDULen);
        free(respdu);
        respdu = NULL;
        break;
    }
    case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
    {
        char caName[32] = {'\0'};
        strncpy(caName,pdu->caData,32);

        char *pPath = new char[pdu->uiMsgLen];
        strncpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);

        QString strPath = QString("%1/%2").arg(pPath).arg(caName);

        QFileInfo fileInfo(strPath);

        if(fileInfo.isDir())
        {
            QDir dir(strPath);
            QFileInfoList fileInfoList = dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            PDU *respdu = mkPDU((iFileCount)*sizeof(FileInfo));
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            for(int i=0;i<iFileCount;i++)
            {

                // 指向每一个FileInfo
                pFileInfo = (FileInfo*)respdu->caMsg + i;
                strFileName = fileInfoList[i].fileName();
                memcpy(pFileInfo->caName,strFileName.toStdString().c_str(),strFileName.size());


                if(fileInfoList[i].isDir()){
                    pFileInfo->iFileType = 0;
                }
                if(fileInfoList[i].isFile()){
                    pFileInfo->iFileType = 1;
                }
                qDebug() << fileInfoList[i].fileName()
                         << fileInfoList[i].size()
                         << " 文件夹：" << fileInfoList[i].isDir()
                         << " 常规文件：" <<  fileInfoList[i].isFile()
                         << pFileInfo->iFileType
                         << pFileInfo->caName;
            }
            write((char*) respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }
        else if(fileInfo.isFile())
        {
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
            strcpy(respdu->caData,ENTER_DIR_FAILURED);

            write((char*) respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
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
