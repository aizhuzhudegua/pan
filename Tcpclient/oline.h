#ifndef OLINE_H
#define OLINE_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class Oline;
}

class Oline : public QWidget
{
    Q_OBJECT

public:
    explicit Oline(QWidget *parent = nullptr);
    ~Oline();
    void showUsr(PDU* pdu);

private slots:
    void on_addFriend_pb_clicked();

private:
    Ui::Oline *ui;
};

#endif // OLINE_H
