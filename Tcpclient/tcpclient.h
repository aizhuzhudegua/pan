#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"
#include "opewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();
    QString getLoginName();
    QString getCurPath();

// 槽函数，信号处理
public slots:
    void showConnect();
    void recvMsg();
    static TcpClient &getInstance();
    QTcpSocket& getTcpSocket();

private slots:
//    void on_send_pd_clicked();

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;
    // ip和端口号
    QString m_strIP;
    quint16 m_usPort;
    // socket
    QTcpSocket m_tcpSocket;
    // 登录者的用户名
    QString m_strLoginName;
    // 当前所在路径
    QString m_strCurPath;
};
#endif // TCPCLIENT_H
