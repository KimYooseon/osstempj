#include "shopmanagerform.h"
#include "ui_shopmanagerform.h"
#include "shopitem.h"
#include "productitem.h"


#include <QFile>
#include <QMenu>
#include <QDebug>

ShopManagerForm::ShopManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShopManagerForm)
{
    //ui 설정 부분
    ui->setupUi(this);

    //위젯의 사이즈 설정. 540은 전체 화면의 왼쪽에 해당하는 비중, 650은 전체 화면의 오른쪽에 해당하는 비중
    QList<int> sizes;
    sizes << 540 << 650;
    ui->splitter->setSizes(sizes);

    //트리위젯의 컬럼별 너비 설정
    ui->treeWidget->setColumnWidth(0,50);
    ui->treeWidget->setColumnWidth(1,80);
    ui->treeWidget->setColumnWidth(2,130);
    ui->treeWidget->setColumnWidth(3,80);
    ui->treeWidget->setColumnWidth(4,50);
    ui->treeWidget->setColumnWidth(5,70);

    //remove 액션 추가. remove를 클릭했을 때 아이템이 삭제되도록 만듦
    QAction* removeAction = new QAction(tr("&Remove"));
    connect(removeAction, SIGNAL(triggered()), SLOT(removeItem()));

    //메뉴를 생성하고 remove 액션을 메뉴에 추가
    menu = new QMenu;
    menu->addAction(removeAction);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    connect(ui->searchLineEdit, SIGNAL(returnPressed()),
            this, SLOT(on_searchPushButton_clicked()));

    //CustomertreeWidgetn


}

// 저장된 텍스트를 가져오는 부분
void ShopManagerForm::loadData()
{

    QFile file("shoplist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {                       //파일이 끝까지 읽힐 때까지 반복함
        QString line = in.readLine();
        QList<QString> row = line.split(", ");  //", "을 기준으로 데이터를 구분해 데이터를 저장함
        if(row.size()) {                        //행이 0개가 아닐 때
            int sid = row[0].toInt();           //스트링값을 toInt() 함수를 통해 정수값으로 변환
            //int cid = row[1].toInt();
            //int pid = row[2].toInt();
            int count = row[4].toInt();         //스트링값을 toInt() 함수를 통해 정수값으로 변환
            int price = row[5].toInt();         //스트링값을 toInt() 함수를 통해 정수값으로 변환
            int totalprice = count*price;

            ShopItem* s = new ShopItem(sid, row[1], row[2], row[3], count, price, totalprice, row[7], row[8]);
            ui->treeWidget->addTopLevelItem(s); //ShopItem을 저장함
            shopList.insert(sid, s);            //정보 추가

        }
    }
    file.close( );
}


ShopManagerForm::~ShopManagerForm()
{
    delete ui;

    //파일을 저장함
    QFile file("shoplist.txt");          //shoplist.txt를 열음
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);              //파일이 있을 때
    for (const auto& v : shopList) {     //for문을 통해 productlist의 값을 하나씩 가져옴
        ShopItem* s = v;                 //s에 각각의 값들을 저장해 줌
        out << s->SID() << ", ";
        out << s->getCID() << ", ";
        out << s->getPID() << ", ";
        out << s->getDate() << ", ";
        out << s->getCount() << ", ";
        out << s->getPrice() << ", ";
        out << (s->getPrice() * s->getCount()) << ", ";
        out << s->getPhoneNum() << ", ";
        out <<s->getAddress() << "\n";

    }
    file.close( );
}

/*쇼핑아이디 만들기*/
int ShopManagerForm::makeId( )
{
    if(shopList.size( ) == 0) {          //상품아이디가 없으면 20000번부터 고객아이디를 부여
        return 20000;
    } else {
        auto sid = shopList.lastKey();   //shopList의 마지막 키 값 + 1로 다음 키 값을 설정해 줌
        return ++sid;
    }
}

/*아이템 삭제*/
void ShopManagerForm::removeItem()                  //treeWidget에 추가할 아이템을 만들어 줌
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    if(item != nullptr) {                           //아이템이 있을 때
        shopList.remove(item->text(0).toInt());     //쇼핑리스트에서 아이템의 첫번째 컬럼에 해당하는 아이템을 삭제해 줌
        ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));    //해당하는 상품을 treeWidget에서도 삭제해줌
        ui->treeWidget->update();                   //treeWidget 업데이트
    }
}

/*treeWidget 영역에서 마우스 오른쪽버튼 클릭했을 때 메뉴가 나오게 해주는 부분*/
void ShopManagerForm::showContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);    //위치를 구해줌
    menu->exec(globalPos);                                  //그 위치에 메뉴가 뜨게 만들어 줌
}

/*search 버튼 클릭 시 해당하는 정보가 searchTreeWidget에 뜨게 만들어 줌*/
void ShopManagerForm::on_searchPushButton_clicked()
{
    ui->searchTreeWidget->clear();                                         //search 버튼 다시 누르기 전에 저장되어 있었던 정보를 지워줌
    int i = ui->searchComboBox->currentIndex();                            //i에 검색콤보박스의 현재인덱스 번호를 저장해준다
    //MatchCaseSensitive: 대소문자 구별, MatchContains: 포함하는지 확인
    auto flag = (i==0||i==4)? Qt::MatchCaseSensitive    //flag값을 지정해 줌. i가 0이나 4일 때는 정확한 값을 입력했을 때만 검색이 되도록 만듦
                   : Qt::MatchCaseSensitive|Qt::MatchContains;  //i가 0이나 4가 아닐 때는 입력한 값이 포함되면 검색이 되도록 만듦
    {
        auto items = ui->treeWidget->findItems(ui->searchLineEdit->text(), flag, i);//flag와 i값에 해당하는 정보를 searchLineEdit에 입력한 텍스트를 찾고, items에 해당 값을 저장해준다

        foreach(auto i, items) {                                           //아이템들을 하나씩 꺼내옴
            ShopItem* s = static_cast<ShopItem*>(i);                       //i의 자료형을 ProductItem이라는 형식으로 변환하고 고정
            int sid = s->SID();
            QString cid = s->getCID();
            QString pid = s->getPID();
            QString date = s->getDate();
            int count = s->getCount();
            int price = s->getPrice();
            int totalprice = s->getCount()*s->getPrice();
            QString phonenumber = s->getPhoneNum();
            QString address = s->getAddress();

            //sid, cid, pid, date, count, price, totalprice, address, phonenumber를 item에 넣어줌
            ShopItem* item = new ShopItem(sid, cid, pid, date, count, price, totalprice, address, phonenumber);
            ui->searchTreeWidget->addTopLevelItem(item);                   //item을 searchTreeWidget의 맨 위에 추가해 줌
        }
    }
}

/*수정버튼이 눌렸을 때*/
void ShopManagerForm::on_modifyPushButton_clicked()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();      //item에 treeWidget의 현재 아이템을 넣어줌

    if(item != nullptr) {                                       //item이 있을 때
        int key = item->text(0).toInt();                        //item의 0번째 인덱스 값인 쇼핑번호를 key에 저장함
//        int cid = item->text(1).toInt();
//        int pid = item->text(2).toInt();
//        int count = item->text(4).toInt();

        ShopItem* s = shopList[key];                            //shopList[key]에 해당하는 값을 s에 저장해준다

        //id productname price category
        QString date, cid, pid;
        int count;
        //cid = ui->shopcidLineEdit->text().toInt();
        //pid = ui->shoppidLineEdit->text().toInt();
        date = ui->dateLineEdit->text();                        //dateLineEdit에 입력한 값을 변수 date에 저장해준다
        count = ui->countLineEdit->text().toInt();              //countLineEdit에 입력한 값을 변수 count에 저장해준다


        s->setCID(cid);
        s->setPID(pid);
        s->setDate(date);
        s->setCount(count);
        shopList[key] = s;                                      //shopList[key]에 s를 넣어준다
    }

}

/*add 버튼을 클릭했을 때*/
void ShopManagerForm::on_addPushButton_clicked()
{
    QString date, cid, pid, address, phonenumber;
    int count, price, totalprice;
    int sid = makeId( );                        //makeId()를 통해 새로운 상품 id 값을 추가해 줌

    //정보를 입력하지 않은 채로 add버튼을 눌렀을 때
    if(ui->CustomertreeWidget->topLevelItemCount() == 0 || ui->ProducttreeWidget->topLevelItemCount() == 0 || date == "" ||count == 0)
    {
        return;                                 //그냥 리턴해줘서 아무 일도 일어나지 않게끔 해 줌                                                   //그냥 리턴해줘서 아무 일도 일어나지 않게끔 해 줌
    }

    cid = ui->shopcidcomboBox->currentText();   //shopcidcomboBox에서 선택한 값을 변수 cid에 저장해준다
    pid = ui->shoppidcomboBox->currentText();   //shoppidcomboBox에서 선택한 값을 변수 pid에 저장해준다
    date = ui->dateLineEdit->text();            //dateLineEdit에 입력한 값을 변수 date에 저장해준다
    count = ui->countLineEdit->text().toInt();  //countLineEdit에 입력한 값을 변수 count에 저장해준다

    price = ui->ProducttreeWidget->topLevelItem(0)->text(1).toInt();      //ProducttreeWidget의 첫번째 줄에 있는 값의 인덱스 1번 값을 변수 price에 저장해준다
    phonenumber =ui->CustomertreeWidget->topLevelItem(0)->text(1);        //CustomertreeWidget의 첫번째 줄에 있는 값의 인덱스 1번 값을 변수 phonenumber에 저장해준다
    address = ui->CustomertreeWidget->topLevelItem(0)->text(2);           //CustomertreeWidget의 첫번째 줄에 있는 값의 인덱스 2번 값을 변수 address에 저장해준다
    totalprice = price * count;

    if(date.length()) {                                                   //날짜 길이가 1이상일 때만 if문 실행
        ShopItem* s = new ShopItem(sid, cid, pid, date, count, price, totalprice, address, phonenumber);
        shopList.insert(sid, s);                                          //shopList에 정보들을 넣어줌
        ui->treeWidget->addTopLevelItem(s);                               //treeWidget에 s값을 추가해 줌
    }
}

/*treeWidget에 있는 item을 클릭했을 때*/
void ShopManagerForm::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);                                       //매개변수인 column을 사용하지 않았을 때 오류가 생기지 않게 해 줌
    ui->shopsidLineEdit->setText(item->text(0));            //item의 인덱스 0번 값에 쇼핑id를 저장해 줌
    ui->shopcidcomboBox->setCurrentText(item->text(1));     //item의 인덱스 1번 값에 고객id를 저장해 줌
    ui->shoppidcomboBox->setCurrentText(item->text(2));     //item의 인덱스 2번 값에 상품id를 저장해 줌

    ui->dateLineEdit->setText(item->text(3));               //item의 인덱스 3번 값에 날짜를 저장해 줌
    ui->countLineEdit->setText(item->text(4));              //item의 인덱스 4번 값에 구매수량을 저장해 줌

    emit SendPID(item->text(2).right(6).left(5).toInt());   //pid만을 뽑아내어 productmanager로 보내줌
    emit SendCID(item->text(1).right(4).left(3).toInt());   //cid만을 뽑아내어 clientmanager로 보내줌

    ui->toolBox->setCurrentIndex(0);                        //treeWidget에 있는 아이템을 선택했을 때 toolBox의 0번째 인덱스 값인 Input을 현재 화면으로 설정
}


/*shopcidcomboBox에 고객이름과 cid를 추가함 (ex. 김유선(100) )*/
void ShopManagerForm::addClient(QString name, int cid)
{
    ui->shopcidcomboBox->addItem(name+"(" + QString::number(cid)+")");
}

/*shoppidcomboBox에 상품이름과 pid를 추가함 (ex. SA-100(10000) )*/
void ShopManagerForm::addProduct(QString productname, int price, QString category, int pid)
{
    ui->shoppidcomboBox->addItem(productname+"(" + QString::number(pid)+")");
}

/*shopcidcomboBox에서 고객이름과 cid를 변경함 (ex. 이정연(101) )*/
void ShopManagerForm::modifyClient(QString name, int cid, int i)
{
    ui->shopcidcomboBox->setItemText(i, name+"(" + QString::number(cid)+")");
}

/*shopcidcomboBox에서 인덱스 i에 해당하는 정보를 삭제함*/
void ShopManagerForm::removeClient(int i)
{
    ui->shopcidcomboBox->removeItem(i);
}

/*shoppidcomboBox에서 상품이름과 pid를 변경함 (ex. SA-110(10001) )*/
void ShopManagerForm::modifyProduct(QString productname, int pid, int i)
{
    ui->shoppidcomboBox->setItemText(i, productname+"(" + QString::number(pid)+")");
}

/*shoppidcomboBox에서 인덱스 i에 해당하는 정보를 삭제함*/
void ShopManagerForm::removeProduct(int i)
{
    ui->shoppidcomboBox->removeItem(i);
}

/*shoppidcomboBox를 클릭했을 때, 해당 정보에서 pid만 빼서 productmanager로 보내주는 부분*/
void ShopManagerForm::on_shoppidcomboBox_textActivated(const QString &arg1)
{
    int ID = arg1.right(6).left(5).toInt();
    emit SendPID(ID);
}

/*pname, price, category 값을 productmanagerform에서 받아와 ProducttreeWidget에 띄워주는 부분*/
void ShopManagerForm::PInfoSended(QString pname, int price, QString category)
{
    ui->ProducttreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, pname);
    item->setText(1, QString::number(price));
    item->setText(2, category);

    ui->ProducttreeWidget->addTopLevelItem(item);
}

/*shopcidcomboBox를 클릭했을 때, 해당 정보에서 cid만 빼서 clientmanager로 보내주는 부분*/
void ShopManagerForm::on_shopcidcomboBox_textActivated(const QString &arg1)
{
    int ID = arg1.right(4).left(3).toInt();
    emit SendCID(ID);
}

/*name, phonenumber, address 값을 customermanagerform에서 받아와 CustomertreeWidget에 띄워주는 부분*/
void ShopManagerForm::CInfoSended(QString name, QString phonenumber, QString address){
    ui->CustomertreeWidget->clear();
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, name);
    item->setText(1, phonenumber);
    item->setText(2, address);
    ui->CustomertreeWidget->addTopLevelItem(item);
}


