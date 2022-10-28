#ifndef CLIENTMANAGERFORM_H
#define CLIENTMANAGERFORM_H

#include <QWidget>
#include <QHash>

class ClientItem;
class QMenu;
class QTreeWidgetItem;

namespace Ui {
class ClientManagerForm;
}

class ClientManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ClientManagerForm(QWidget *parent = nullptr);
    ~ClientManagerForm();
    void loadData();                            // 텍스트파일에 저장해 놓은 기존의 정보들을 불러오는 함수

private slots:
    /* QTreeWidget을 위한 슬롯 */
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);  //treeWidget에 있는 아이템을 클릭했을 때 그 아이템에 해당하는 정보가 LineEdit에 값들이 뜸
    void showContextMenu(const QPoint &);
    void removeItem();              /* QAction을 위한 슬롯 */
    void on_addPushButton_clicked();
    void on_modifyPushButton_clicked();
    void on_searchPushButton_clicked();
    void CIDsended(int);                        //orderForm에서 보내온 client의 id를 받는 부분
signals:
    void clientAdded(QString, int);             // orderForm과 serverForm으로 client의 id와 이름을 보내주는 부분
    void clientModified(QString, int, int);     // client의 정보를 변경했을 때 orderForm과 serverForm으로 특정 인덱스에 대한 client의 id와 이름을 변경하는 부분
    void clientRemoved(int);                    // 고객이 삭제되고 나면 삭제된 해당 인덱스를 orderForm과 serverform에 보내줌
    void sendCInfo(QString, QString, QString);  // 고객의 정보를 orderForm으로 보내준다
private:
    int makeId();                               // 고객 번호를 만드는 부분

    QMap<int, ClientItem*> clientList;
    Ui::ClientManagerForm *ui;
    QMenu* menu;
};

#endif // CLIENTMANAGERFORM_H
