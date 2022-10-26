#include "productitem.h"

/*ProductItem은 id, productname, price, category 총 4가지의 매개변수를 가지고 있다*/
ProductItem::ProductItem(int id, QString productname, int price, QString category)
{
    setText(0, QString::number(id));             //setText() 안에 들어가는 2번째 값은 QString 형태여야 하는데,
                                                 //id값은 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(1, productname);
    setText(2, QString::number(price));          //price값도 마찬가지로 int형이기 때문에, QString::number()함수를 이용해 QString 형태로 바꾸어 저장해준다
    setText(3, category);
}

QString ProductItem::getProductName() const      //상품 이름에 대한 getter
{
    return text(1);                              //첫 번째 매개변수에 들어감
}

void ProductItem::setProductName(QString& productname)  //상품 이름에 대한 setter
{
    setText(1, productname);                     //첫 번째 매개변수에 들어감
}

int ProductItem::getPrice() const                //상품 가격에 대한 getter
{
    return text(2).toInt();                      //두 번째 매개변수에 들어감
}
void ProductItem::setPrice(int& price)           //상품 가격에 대한 setter
{
    setText(2, QString::number(price));          //두 번째 매개변수에 들어감
}

QString ProductItem::getCategory() const         //상품 카테고리에 대한 getter
{
    return text(3);                              //세 번째 매개변수에 들어감
}
void ProductItem::setCategory(QString& category) //상품 카테고리에 대한 setter
{
    setText(3, category);                        //첫 번째 매개변수에 들어감
}

int ProductItem::id() const                      //상품 id에 대한 getter
{
    return text(0).toInt();                      //0 번째 매개변수에 들어감
}
