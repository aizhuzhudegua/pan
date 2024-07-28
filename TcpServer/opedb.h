#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    bool handleOffLine(const char *name);
    QStringList handAllOline();
    int handleSearchUsr(const char* name);
    int handleAddFriend(const char*pername, const char *name);
    bool handleAgreeFriend(const char*pername, const char *name);

signals:

public slots:

private:
    // 用于连接数据库
    QSqlDatabase m_db;

};

#endif // OPEDB_H
