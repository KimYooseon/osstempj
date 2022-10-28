#ifndef SHOPITEM_H
#define SHOPITEM_H

#include "clientitem.h"
#include "productitem.h"
#include <QTreeWidgetItem>
class ShopItem:public QTreeWidgetItem
{

public:
    /*ShopItem은 sid, cid, pid, date, count, price, totalprice, phonenumber, address 총 9가지의 매개변수를 가지고 있다*/
    explicit ShopItem(int sid = 0, QString cid= 0, QString pid = 0, QString date = " ", int count = 0, int price=0, int totalprice = 0, QString phonenumber="", QString address="");

    int SID() const;                //sid에 대한 getter

    QString getCID() const;         //cid에 대한 getter
    void setCID(QString&);          //cid에 대한 setter

    QString getPID() const;         //pid에 대한 getter
    void setPID(QString&);          //pid에 대한 setter

    QString getDate() const;        //구매날짜에 대한 getter
    void setDate(QString&);         //구매날짜에 대한 setter

    int getCount() const;           //count에 대한 getter
    void setCount(int&);            //count에 대한 setter

    int getPrice() const;           //상품 가격에 대한 getter

    QString getPhoneNum() const;    //고객 전화번호에 대한 getter

    QString getAddress() const;     //고객 주소에 대한 getter
};

#endif // SHOPITEM_H
