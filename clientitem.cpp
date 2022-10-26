#include "clientitem.h"

/*ClientItem은 id, name, phoneNumber, address 총 4가지의 매개변수를 가지고 있다*/
ClientItem::ClientItem(int id, QString name, QString phoneNumber, QString address)
{
    setText(0, QString::number(id));        //setText() 안에 들어가는 2번째 값은 QString 형태여야 하는데,
                                            //id값은 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(1, name);
    setText(2, phoneNumber);
    setText(3, address);
}

QString ClientItem::getName() const         //고객 이름에 대한 getter
{
    return text(1);                         //첫 번째 매개변수에 들어감
}

void ClientItem::setName(QString& name)     //고객 이름에 대한 setter
{
    setText(1, name);                       //첫 번째 매개변수에 들어감
}

QString ClientItem::getPhoneNumber() const  //전화번호에 대한 getter
{
    return text(2);                         //두 번째 매개변수에 들어감
}

void ClientItem::setPhoneNumber(QString& phoneNumber)   //전화번호에 대한 setter
{
    setText(2, phoneNumber);                            //두 번째 매개변수에 들어감
}

QString ClientItem::getAddress() const          //주소에 대한 getter
{
    return text(3);                             //세 번째 매개변수에 들어감
}

void ClientItem::setAddress(QString& address)   //주소에 대한 setter
{
    setText(3, address);                        //세 번째 매개변수에 들어감
}

int ClientItem::id() const                      //고객id에 대한 getter
{
    return text(0).toInt();                     //0 번째 매개변수에 들어감
}


