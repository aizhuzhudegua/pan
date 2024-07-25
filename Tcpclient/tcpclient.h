#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "protocol.h"

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

// 槽函数，信号处理
public slots:
    void showConnect();

private slots:
    void on_send_pd_clicked();

private:
    Ui::TcpClient *ui;
    // ip和端口号
    QString m_strIP;
    quint16 m_usPort;
    // socket
    QTcpSocket m_tcpSocket;
};
#endif // TCPCLIENT_H
