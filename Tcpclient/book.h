#ifndef BOOK_H
#define BOOK_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "protocol.h"
#include <QTimer>

class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(PDU *pdu);
    void clearEnterDir();
    QString getEnterDir();
    void setDownloadFlag(bool status);
    bool getDownloadFlag();
    QString getSaveFilePath();
    qint64 m_iTotal;
    qint64 m_iReceived;

    QString getShareFileName();

public slots:
    void createDir();
    void flushFile();
    void delDir();
    void renameFile();
    void enterDir(const QModelIndex &index);
    void returnPre();
    void uploadFile();
    void uploadFileData();
    void delRegFile();

    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDestDir();

signals:

private:
    QListWidget *m_pBookListW;
    QPushButton *m_pReturnPB;
    QPushButton *m_pCreateDirPB;
    QPushButton *m_pDelDirPB;
    QPushButton *m_pRenamePB;
    QPushButton *m_pFlushFilePB;
    QPushButton *m_pUploadPB;
    QPushButton *m_pDownloadPB;
    QPushButton *m_pDelFilePB;
    QPushButton *m_pShareFilePB;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;

    QString m_strEnterDir;
    QString m_strUploadFilePath;

    QTimer *m_pTimer;

    QString m_strSaveFilePath;
    bool m_bDownload;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDestDir;
};

#endif // BOOK_H
