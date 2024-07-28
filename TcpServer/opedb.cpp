#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("E:\\pan\\TcpServer\\cloud.db");

    if(m_db.open()){
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while(query.next()){
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }

}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if(NULL == name || NULL == pwd){
        return false;
    }
    QString data = QString("insert into usrInfo(name,pwd) values('%1','%2')").arg(name).arg(pwd);
    qDebug() << data;
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL == name || NULL == pwd){
        return false;
    }
    QString data = QString("select * from usrInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        data = QString("update usrInfo set online=1 where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
        qDebug() << data;
        QSqlQuery query;
        return query.exec(data);
    }
    else
    {
        return false;
    }
}

bool OpeDB::handleOffLine(const char *name)
{
    if(NULL == name ){
        return false;
    }
    QString data = QString("update usrInfo set online=0 where name=\'%1\' and online=1").arg(name);
    qDebug() << data;
    QSqlQuery query;
    return query.exec(data);
}

QStringList OpeDB::handAllOline()
{
    QString data = QString("select name from usrInfo where online = 1");
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    QStringList res;
    res.clear();

    while(query.next()){
        res.push_back(query.value(0).toString());
    }
    return res;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if(NULL == name){
        return -1;
    }
    QString data = QString("select online from usrInfo where name = '%1'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next())
    {
        int ret = query.value(0).toInt();
        return ret;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if(NULL == pername || NULL == name){
        return -1;
    }
    QString data = QString("select * from friend where \
                            (id=(select id from usrInfo where name = '%1') and friendId=(select id from usrInfo where name = '%2')) \
                         or (id=(select id from usrInfo where name = '%2') and friendId=(select id from usrInfo where name = '%1'))")
                           .arg(pername).arg(name);
    qDebug() << data;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        return  0; // 已经是好友
    }
    else {
        QString data = QString("select online from usrInfo where name = '%1'").arg(pername);
        QSqlQuery query;
        query.exec(data);
        if(query.next())
        {
            int ret = query.value(0).toInt();
            if(1 == ret){
                return 1; // 对方在线
            }
            else if(0 == ret){
                return 2; // 对方不在线
            }
        }
        else
        {
            return 3; // 不存在用户
        }
    }
}

bool OpeDB::handleAgreeFriend(const char *pername, const char *name)
{
    if(NULL == pername || NULL == name){
        return false;
    }
    QString data = QString("insert into friend values((select id from usrInfo where name = '%1'),(select id from usrInfo where name = '%2'))").arg(name).arg(pername);
    qDebug() << data;
    QSqlQuery query;
    return query.exec(data);
}
