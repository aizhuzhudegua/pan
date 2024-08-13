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

    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer,SIGNAL(timeout()),
            this,SLOT(sendFileToClient()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)
{
    QDir dir;
    dir.mkdir(strDestDir);

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QString srcTmp;
    QString destTmp;
    for(int i=0;i<fileInfoList.size();i++)
    {
        qDebug() << "fileName: " << fileInfoList[i].fileName();
        if(fileInfoList[i].isFile())
        {
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            QFile::copy(srcTmp,destTmp);
        }
        else if(fileInfoList[i].isDir())
        {
            if(fileInfoList[i].fileName() == QString(".") ||
                    fileInfoList[i].fileName() == QString(".."))
            {
                continue;
            }
            srcTmp = strSrcDir + '/' + fileInfoList[i].fileName();
            destTmp = strDestDir + '/' + fileInfoList[i].fileName();
            // 递归拷贝
            copyDir(srcTmp,destTmp);
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if(!m_bUpload)
    {

        qDebug() << this->bytesAvailable();
        uint uiPDULen = 0;
        // 传入的是一个起始地址，并且是char*型，一次偏移一个字节
        this->read((char*)&uiPDULen,sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        // 将uiPDULen字段之后的所有数据读入
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));
        qDebug() << "mytcpsocket:40 " << pdu->uiMsgType;
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
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            qint64 fileSize;
            sscanf(pdu->caData,"%s %lld",caFileName,&fileSize);

            char *pPath = new char[pdu->uiMsgLen];
            strncpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);

            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            qDebug() << strPath;
            delete [] pPath;
            pPath = NULL;

            m_file.setFileName(strPath);
            if(m_file.open(QIODevice::WriteOnly))
            // 以只写方式创建文件，若文件不存在则创建文件
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iReceive = 0;
            }

            break;
        }
        case ENUM_MSG_TYPE_DEL_FILE_REQUEST:
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
                ret = false;
            }
            else if(fileInfo.isFile())  // 常规文件删除
            {
                QDir dir;
                ret = dir.remove(strPath);
            }
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType=ENUM_MSG_TYPE_DEL_FILE_RESPOND;
            if(ret)
            {
                memcpy(respdu->caData,DEL_FILE_OK,strlen(DEL_FILE_OK));
            }
            else
            {
                memcpy(respdu->caData,Del_FILE_FAILURED,strlen(Del_FILE_FAILURED));
            }
            write((char*) respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWLOAD_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            strcpy(caFileName,pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caFileName);
            delete []pPath;
            pPath= NULL;

            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWLOAD_FILE_RESPOND;
            sprintf(respdu->caData,"%s %lld",caFileName,fileSize);

            write((char*) respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;


            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caData,"%s %d",caSendName,&num);
            int size = num*32;

            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caData,caSendName);
            memcpy(respdu->caMsg,(char*)(pdu->caMsg)+size,pdu->uiMsgLen-size);


//            char *tmp = new char[pdu->uiMsgLen];
//            strncpy(tmp,(char*)pdu->caMsg,pdu->uiMsgLen);
//            QString strTmp = QString("%1").arg(tmp);
//            qDebug() << "resend:" << strTmp;
            char caRecvName[32] = {'\0'};
            for(int i=0;i<num;i++)
            {
                memcpy(caRecvName,(char*)(pdu->caMsg) + i*32,32);
                // qDebug() << "send to " << caRecvName;
                MyTcpServer::getInstance().resend(caRecvName,respdu);
            }
            free(respdu);
            respdu = NULL;

            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caData,"share file ok");
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {
            QString strRecvPath = QString("./users/%1").arg(pdu->caData);

            QString strShareFilePath = QString("%1").arg((char*)pdu->caMsg);
            QFileInfo fileInfo(strShareFilePath);
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);

            strRecvPath = strRecvPath + "/" + strFileName;
            qDebug()<<"strRecvPath: " << strRecvPath;
            if(fileInfo.isFile())
            {
                QFile::copy(strShareFilePath,strRecvPath);
            }
            else if(fileInfo.isDir())
            {
                copyDir(strShareFilePath,strRecvPath);
            }
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:
        {
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caData,"%d %d %s",&srcLen , &destLen,caFileName);

            char *pSrcPath = new char[srcLen+1];
            char *pDestPath = new char[destLen+1+32];
            // 清空
            memset(pSrcPath,'\0',srcLen+1);
            memset(pDestPath,'\0',destLen+1+32);

            memcpy(pSrcPath,pdu->caMsg,srcLen);
            memcpy(pDestPath,(char*)(pdu->caMsg) + srcLen+1,destLen);

            QFileInfo fileInfo(pDestPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir())
            {
                strcat(pDestPath,"/");
                strcat(pDestPath,caFileName);
                bool ret = QFile::rename(pSrcPath,pDestPath);
                if(ret)
                {
                    strcpy(respdu->caData,MOVE_FILE_OK);
                }
                else
                {
                    strcpy(respdu->caData,COMMON_ERR);
                }
            }
            else if(fileInfo.isFile())
            {
                strcpy(respdu->caData,MOVE_FILE_FAILURED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = NULL;
    }
    else
    // 接收文件二进制数据，不封装pdu
    {
        PDU *respdu =NULL;
        respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff = readAll();
        m_file.write(buff);
        m_iReceive += buff.size();
        qDebug() << QString("%1/%2").arg(m_iReceive).arg(m_iTotal);
        if(m_iReceive == m_iTotal)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_OK);
            write((char*)respdu,respdu->uiPDULen);
        }
        else if(m_iReceive > m_iTotal)
        {
            m_file.close();
            m_bUpload = false;
            strcpy(respdu->caData,UPLOAD_FILE_FAILURED);
            write((char*)respdu,respdu->uiPDULen);
        }

        free(respdu);
        respdu = NULL;

    }
}

void MyTcpSocket::clientOffLine()
{
    OpeDB::getInstance().handleOffLine(m_strName.toStdString().c_str());
    emit offline(this);
}

void MyTcpSocket::sendFileToClient()
{
    qDebug() << "begin to transfer...";
    m_pTimer->stop();
    char *pData = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = m_file.read(pData,4096);
        if(ret > 0 && ret <= 4096)
        {
            write(pData,ret);
        }
        else if(0 == ret)
        {
            m_file.close();
            break;
        }
        else if(ret < 0)
        {
            qDebug() << "发送文件内容给客户端过程中失败";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}
