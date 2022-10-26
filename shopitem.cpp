#include "shopitem.h"

/*ShopItem은 sid, cid, pid, date, count, price, totalprice, phonenumber, address 총 9가지의 매개변수를 가지고 있다*/
ShopItem::ShopItem(int sid, QString cid, QString pid, QString date, int count, int price, int totalprice, QString phonenumber, QString address)
{
    setText(0, QString::number(sid));           //setText() 안에 들어가는 2번째 값은 QString 형태여야 하는데,
                                                //sid값은 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(1, cid);
    setText(2, pid);
    setText(3, date);
    setText(4, QString::number(count));         //count값도 마찬가지로 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(5, QString::number(price));         //price값도 마찬가지로 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(6, QString::number(price*count));   //price*count값도 마찬가지로 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(7, phonenumber);
    setText(8, address);
}

QString ShopItem::getCID() const        //cid에 대한 getter
{
    return text(1);                     //첫 번째 매개변수에 들어감
}

void ShopItem::setCID(QString& cid)     //cid에 대한 setter
{
    setText(1, cid);                    //첫 번째 매개변수에 들어감
}

QString ShopItem::getPID() const        //pid에 대한 getter
{
    return text(2);
}

void ShopItem::setPID(QString& pid)     //pid에 대한 setter
{
    setText(2, pid);
}


QString ShopItem::getDate() const       //구매날짜에 대한 getter
{
    return text(3);
}
void ShopItem::setDate(QString& date)   //구매날짜에 대한 setter
{
    setText(3, date);
}

int ShopItem::getCount() const          //count에 대한 getter
{
    return text(4).toInt();
}

void ShopItem::setCount(int& count)     //count에 대한 setter
{
    setText(4, QString::number(count));
}

int ShopItem::SID() const               //sid에 대한 getter
{
    return text(0).toInt();
}


int ShopItem::getPrice() const          //상품 가격에 대한 getter
{
    return text(5).toInt();
}

QString ShopItem::getPhoneNum() const{  //고객 전화번호에 대한 getter
    return text(7);
}
QString ShopItem::getAddress() const{   //고객 주소에 대한 getter
    return text(8);
}
