#ifndef SHOPMANAGERFORM_H
#define SHOPMANAGERFORM_H

#include <QWidget>
#include <QHash>

class ShopItem;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ShopManagerForm;
}

class ShopManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ShopManagerForm(QWidget *parent = nullptr);
    ~ShopManagerForm();
    void loadData();        // 텍스트파일에 저장해 놓은 기존의 정보들을 불러오는 함수

public slots:
    void addClient(QString, int);                   // shopcidcomboBox에 고객이름과 cid를 추가함 (ex. 김유선(100) )
    void addProduct(QString, int, QString, int);    // shoppidcomboBox에 상품이름과 pid를 추가함 (ex. SA-100(10000) )
    void modifyClient(QString, int, int);           // shopcidcomboBox에서 고객이름과 cid를 변경함 (ex. 이정연(101) )
    void removeClient(int);                         // shopcidcomboBox에서 인덱스 i에 해당하는 정보를 삭제함
    void modifyProduct(QString, int, int);          // shoppidcomboBox에서 상품이름과 pid를 변경함 (ex. SA-110(10001) )
    void removeProduct(int);                        // shoppidcomboBox에서 인덱스 i에 해당하는 정보를 삭제함

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &);
    void removeItem();              /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    //void on_customer_Clicked();

    /*shoppidcomboBox를 클릭했을 때, 해당 정보에서 pid만 빼서 productmanager로 보내주는 부분*/
    void on_shoppidcomboBox_textActivated(const QString &arg1);
    /*pname, price, category 값을 productmanagerform에서 받아와 ProducttreeWidget에 띄워주는 부분*/
    void PInfoSended(QString, int, QString);

    /*shopcidcomboBox를 클릭했을 때, 해당 정보에서 cid만 빼서 customermanager로 보내주는 부분*/
    void on_shopcidcomboBox_textActivated(const QString &arg1);
    /*name, phonenumber, address 값을 customermanagerform에서 받아와 CustomertreeWidget에 띄워주는 부분*/
    void CInfoSended(QString, QString, QString);


signals:
    void orderAdded(QString);
    void sendpname(int);
    void SendPID(int);              // pid만 빼서 productmanager로 보내주는 부분
    void SendCID(int);              // cid만 빼서 customermanager로 보내주는 부분
private:
    int makeId();

    QMap<int, ShopItem*> shopList;  // 쇼핑 번호를 만드는 부분
    Ui::ShopManagerForm *ui;
    QMenu* menu;
};

#endif // SHOPMANAGERFORM_H
