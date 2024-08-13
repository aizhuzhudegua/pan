#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"

Book::Book(QWidget *parent) : QWidget(parent)
{
    m_strEnterDir.clear();

    m_pTimer = new QTimer;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("返回");
    m_pCreateDirPB = new QPushButton("创建文件夹");
    m_pDelDirPB = new QPushButton("删除文件夹");
    m_pRenamePB = new QPushButton("重命名文件");
    m_pFlushFilePB = new QPushButton("刷新文件");
    m_pUploadPB = new QPushButton("上传文件");
    m_pDownloadPB = new QPushButton("下载文件");
    m_pDelFilePB = new QPushButton("删除文件");
    m_pShareFilePB = new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);


    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushFilePB);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadPB);
    pFileVBL->addWidget(m_pDownloadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookListW);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);
    connect(m_pCreateDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(createDir()));
    connect(m_pFlushFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(flushFile()));
    connect(m_pDelDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(delDir()));
    connect(m_pRenamePB,SIGNAL(clicked(bool))
            ,this,SLOT(renameFile()));
    connect(m_pBookListW,SIGNAL(doubleClicked(QModelIndex))
            ,this,SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB,SIGNAL(clicked(bool))
            ,this,SLOT(returnPre()));
    connect(m_pUploadPB,SIGNAL(clicked(bool))
            ,this,SLOT(uploadFile()));
    connect(m_pTimer,SIGNAL(timeout())
            ,this,SLOT(uploadFileData()));
    connect(m_pDelFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(delRegFile()));
    connect(m_pDownloadPB,SIGNAL(clicked(bool))
            ,this,SLOT(downloadFile()));
    connect(m_pShareFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(shareFile()));
    connect(m_pMoveFilePB,SIGNAL(clicked(bool))
            ,this,SLOT(moveFile()));
    connect(m_pSelectDirPB,SIGNAL(clicked(bool))
            ,this,SLOT(selectDestDir()));

    m_bDownload = false;
}

void Book::updateFileList(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }

    // ---这部分代码和clear效果同---
    /*QListWidgetItem *pItemTmp = NULL;
    int row = m_pBookListW->count()-1;
    while(row >= 0)
    {
        pItemTmp = m_pBookListW->item(row);
        m_pBookListW->removeItemWidget(pItemTmp);
        delete pItemTmp;
        row --;
    }*/
    // ---这部分代码和clear效果同---
    m_pBookListW->clear();

    FileInfo *pFileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for (int i=0;i<iCount; i++)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        QString fileName = QString(pFileInfo->caName);

        if(fileName==QString(".") || fileName==QString(".."))
        {
            continue;
        }
        QListWidgetItem *pItem = new QListWidgetItem;
        if( 0 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/dir.png")));
        }
        else if(1 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/map/file.png")));
        }
        pItem->setText(pFileInfo->caName);
        m_pBookListW->addItem(pItem);
    }

}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","文件夹名");
    if(! strNewDir.isEmpty()){
        if(strNewDir.size() > 32){
            QMessageBox::warning(this,"新建文件夹","文件夹名不能超过32字符");
        }
        else{
            QString strName = TcpClient::getInstance().getLoginName();
            QString strCurPath = TcpClient::getInstance().getCurPath();
            PDU* pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
            memcpy(pdu->caData+32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }

    }
    else
    {
        QMessageBox::warning(this,"新建文件夹","新文件夹名字不能为空");
    }


}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( NULL == pItem){
        QMessageBox::warning(this,"删除文件","请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;

    }

}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( NULL == pItem){
        QMessageBox::warning(this,"重命名文件","请选择要重命名的文件");
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this,"重命名文件","请输入新的文件名");
        if(!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            memcpy(pdu->caData,strOldName.toStdString().c_str(),strOldName.size());
            memcpy(pdu->caData+32,strNewName.toStdString().c_str(),strNewName.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu=NULL;
        }
        else
        {
            QMessageBox::warning(this,"重命名文件","新文件名不能为空");
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    qDebug() << strDirName;
    m_strEnterDir = strDirName;
    QString strCurPath = TcpClient::getInstance().getCurPath();
    qDebug() << "entering:"<<strCurPath<<"/"<<strDirName;
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    strncpy(pdu->caData,strDirName.toStdString().c_str(),strDirName.size());
    memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::returnPre()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    qDebug() << strCurPath;
    QString strRootPath = "./users/" +TcpClient::getInstance().getLoginName();
    qDebug() << strRootPath;
    if(strCurPath == strRootPath)
    {
        QMessageBox::warning(this,"返回","返回失败，无上级目录");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index,strCurPath.size() - index);
        qDebug() << "pre:" <<strCurPath;
        TcpClient::getInstance().setCurPath(strCurPath);

        clearEnterDir();
        flushFile();
    }
}

void Book::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();

    if(!m_strUploadFilePath.isEmpty())
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
        qDebug() << strFileName;

        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();

        PDU *pdu = mkPDU(strCurPath.size() + 1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        sprintf(pdu->caData,"%s %lld",strFileName.toStdString().c_str(),fileSize);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;

        // 1s后触发上传，防止粘包
        m_pTimer->start(1000);
    }
    else
    {
        QMessageBox::warning(this,"上传文件","上传文件名字不能为空");
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"上传文件","打开文件失败");
        return;
    }
    // 缓冲区
    char *pBuffer = new char[4096];
    // 实际读到的大小
    qint64 ret = 0;
    while (true)
    {
        ret = file.read(pBuffer,4096);
        if(ret > 0 && ret <=4096)
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer,ret);
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this,"上传文件","上传文件失败：读取文件失败");
            break;
        }

    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;

}

void Book::delRegFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( NULL == pItem){
        QMessageBox::warning(this,"删除文件","请选择要删除的文件");
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_FILE_REQUEST;
        strncpy(pdu->caData,strDelName.toStdString().c_str(),strDelName.size());
        memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*) pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( NULL == pItem){
        QMessageBox::warning(this,"下载文件","请选择要下载的文件");
    }
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileUrl().toLocalFile();
        if(strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this,"下载文件","请指定要保存的位置");
            m_strSaveFilePath.clear();
        }
        else
        {
            m_strSaveFilePath = strSaveFilePath;
        }

        if(!m_strSaveFilePath.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_DOWLOAD_FILE_REQUEST;
            QString strFileName = pItem->text();
            strcpy(pdu->caData,strFileName.toStdString().c_str());
            memcpy(pdu->caMsg,strCurPath.toStdString().c_str(),strCurPath.size());
            TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        }

    }

}

// 按钮事件
void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if( NULL == pItem){
        QMessageBox::warning(this,"分享文件","请选择要分享的文件");
        return;
    }
    m_strShareFileName = pItem->text();
    //刷新好友列表
    Friend *pFriend = OpeWidget::getInstance().getFrient();
    // 这里是异步的，需要改
    // pFriend->flushFriend();
    QListWidget *pFriendList = pFriend->getFriendList();
    // 更新到 sharefile的界面上
    ShareFile::getInstance().updateFriend(pFriendList);
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL != pCurItem)
    {
        m_strMoveFileName = pCurItem->text();
        QString curPath = TcpClient::getInstance().getCurPath();
        m_strMoveFilePath = curPath+'/'+m_strMoveFileName;
        m_pSelectDirPB->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");

    }
}

void Book::selectDestDir()
{
    QListWidgetItem *pCurItem = m_pBookListW->currentItem();
    if(NULL != pCurItem)
    {
        QString strDestDir = pCurItem->text();
        QString curPath = TcpClient::getInstance().getCurPath();
        m_strDestDir = curPath+'/'+strDestDir;

        int srcLen = m_strMoveFilePath.size();
        int destLen = m_strDestDir.size();
        PDU *pdu = mkPDU(destLen+srcLen+2);
        pdu->uiMsgType=ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
        sprintf(pdu->caData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());

        memcpy(pdu->caMsg,m_strMoveFilePath.toStdString().c_str(),srcLen);
        memcpy((char*)(pdu->caMsg)+srcLen+1,m_strDestDir.toStdString().c_str(),destLen);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this,"移动文件","请选择要移动的文件");

    }
    m_pSelectDirPB->setEnabled(false);
}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();
}

QString Book::getEnterDir()
{
    return m_strEnterDir;
}

void Book::setDownloadFlag(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadFlag()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}
