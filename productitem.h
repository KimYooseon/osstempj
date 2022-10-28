#ifndef PRODUCTITEM_H
#define PRODUCTITEM_H

#include <QTreeWidgetItem>

class ProductItem: public QTreeWidgetItem
{
public:
    /*productitem은 상품 id, 상품이름, 상품가격, 상품카테고리로 이루어져 있음*/
    explicit ProductItem(int id = 0, QString = "", int price = 0, QString = " ");

    int id() const;                     //상품 id에 대한 getter

    QString getProductName() const;     //상품 이름에 대한 getter
    void setProductName(QString&);      //상품 이름에 대한 setter

    int getPrice() const;               //상품 가격에 대한 getter
    void setPrice(int&);                //상품 가격에 대한 setter

    QString getCategory() const;        //상품 카테고리에 대한 getter
    void setCategory(QString&);         //상품 카테고리에 대한 setter
};

#endif // PRODUCTITEM_H
