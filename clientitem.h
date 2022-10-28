#ifndef CLIENTITEM_H
#define CLIENTITEM_H

#include <QTreeWidgetItem>

class ClientItem : public QTreeWidgetItem
{
public:
    /*clientitem은 고객 id, 고객이름, 고객 전화번호, 고객주소로 이루어져 있음*/
    explicit ClientItem(int id = 0, QString = "", QString = "", QString = "");

    int id() const;                     //고객 id에 대한 getter

    QString getName() const;            //고객 이름에 대한 getter
    void setName(QString&);             //고객 이름에 대한 setter

    QString getPhoneNumber() const;     //고객 전화번호에 대한 getter
    void setPhoneNumber(QString&);      //고객 전화번호에 대한 setter

    QString getAddress() const;         //고객 주소에 대한 getter
    void setAddress(QString&);          //고객 주소에 대한 setter

};

#endif // CLIENTITEM_H
