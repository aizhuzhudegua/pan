#include "sharefile.h"
#include <QDebug>
#include "tcpclient.h"
#include "opewidget.h"

ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectPB = new QPushButton("取消选择");

    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");

    m_pSA = new  QScrollArea;
    m_pFriendW = new QWidget;
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    m_pButtonGroup->setExclusive(false);

    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);


    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);

    connect(m_pCancelSelectPB,SIGNAL(clicked(bool))
            ,this,SLOT(cancelSelect()));
    connect(m_pSelectAllPB,SIGNAL(clicked(bool))
            ,this,SLOT(selectAll()));
    connect(m_pOKPB,SIGNAL(clicked(bool))
            ,this,SLOT(ok()));
    connect(m_pCancelPB,SIGNAL(clicked(bool))
            ,this,SLOT(cancel()));


}

ShareFile &ShareFile::getInstance()
{
    static ShareFile instance;
    return instance;
}

void ShareFile::test()
{
    // 设置m_pFriendW的布局，构造函数内的参数是父widget
    QVBoxLayout *p = new QVBoxLayout(m_pFriendW);
    QCheckBox *pCB = NULL;
    for(int i=0;i<15;i++)
    {
        pCB = new QCheckBox("jack");
        p->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);

    }
    // 设置滚动视窗的widget
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::updateFriend(QListWidget *pFriendList)
{
    if(NULL == pFriendList)
    {
        return;
    }
    // 获取所有的checkbox
    QList<QAbstractButton*> preFriendList =  m_pButtonGroup->buttons();
    QAbstractButton *tmp = NULL;
    for(int i=0;i<preFriendList.size();i++)
    {
        tmp = preFriendList[i];
        m_pFriendWVBL->removeWidget(tmp);
        m_pButtonGroup->removeButton(tmp);
        preFriendList.removeOne(tmp);
        delete tmp;
        tmp = NULL;
    }
    qDebug()<< "pFriendList:" << pFriendList->count();

    QCheckBox *pCB = NULL;
    for(int i=0;i<pFriendList->count();i++)
    {
        pCB = new QCheckBox(pFriendList->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);

    }
    // 设置滚动视窗的widget
    m_pSA->setWidget(m_pFriendW);

}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0;i<cbList.count();i++) {
        if(cbList[i]->isChecked())
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0;i<cbList.count();i++) {
        if(!cbList[i]->isChecked())
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::ok()
{
    QString strName = TcpClient::getInstance().getLoginName();
    QString strCurPath = TcpClient::getInstance().getCurPath();
    QString strShareFileName = OpeWidget::getInstance().getBook()->getShareFileName();

    QString strPath = strCurPath+"/"+strShareFileName;
    // 选项数
    int num = 0;
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i=0;i<cbList.count();i++) {
        if(cbList[i]->isChecked())
        {
            num ++;
        }
    }
    PDU *pdu = mkPDU(32*num + strPath.size() + 1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),num);
    int j = 0;
    for (int i=0;i<cbList.count();i++) {
        if(cbList[i]->isChecked())
        {
           memcpy((char*)pdu->caMsg+j*32,cbList[i]->text().toStdString().c_str(),cbList[i]->text().size());
           j++;
        }
    }
    memcpy((char*)(pdu->caMsg)+j*32,strPath.toStdString().c_str(),strPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);

    free(pdu);
    pdu = NULL;
    hide();
}

void ShareFile::cancel()
{
    hide();
}
