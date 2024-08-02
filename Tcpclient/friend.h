#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QHBoxLayout>
#include "oline.h"



class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllOnlineUsr(PDU *pdu);
    void updateFriendList(PDU *pdu);
    void updateGroupMsg(PDU *pdu);

    QString m_strSearchName;


signals:

public slots:
    void showOnline();
    void searchUsr();
    void flushFriend();
    void delFriend();
    void privateChat();
    void groupChat();

private:
    QTextEdit* m_pShowMsgTE;
    QListWidget* m_pFriendListWidget;
    QLineEdit* m_pInputMsgLE;

    QPushButton* m_pDelFriendPB;
    QPushButton* m_pFlushFriendPB;
    QPushButton* m_pShowOnlineUsrPB;
    QPushButton* m_pSearchUsrPB;
    QPushButton* m_pMsgSendPB;
    QPushButton* m_pPrivateChatPB;

    Oline* m_pOnline;

};

#endif // FRIEND_H
