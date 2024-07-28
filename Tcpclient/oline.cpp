#include "oline.h"
#include "ui_oline.h"
#include <QDebug>
#include "tcpclient.h"
#include <QMessageBox>

Oline::Oline(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Oline)
{
    ui->setupUi(this);
}

Oline::~Oline()
{
    delete ui;
}

void Oline::showUsr(PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen/32;
    char caTmp[32];
    for (uint i= 0;i< uiSize; i++) {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        ui->online_lw->addItem(caTmp);
    }
}

void Oline::on_addFriend_pb_clicked()
{
    QListWidgetItem *pItem = ui->online_lw->currentItem();
    qDebug() << "addfriend:" <<pItem->text();
    QString strPerUsrName = pItem->text();
    QString strLoginName = TcpClient::getInstance().getLoginName();
    // 不能加本人为好友
    if(strPerUsrName == strLoginName){
        QMessageBox::information(this,"添加好友","请不要添加自己为好友");
        return;
    }
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,strPerUsrName.toStdString().c_str(),strPerUsrName.size());
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());
    // 发出好友请求
    TcpClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
}
