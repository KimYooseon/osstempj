#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "clientmanagerform.h"
#include "productmanagerform.h"
#include "shopmanagerform.h"
#include "chatserverform.h"
#include "chatclientform.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /*ui를 구성한다*/
    ui->setupUi(this);

    /*clientForm 생성*/
    clientForm = new ClientManagerForm(this);
    clientForm->setWindowTitle(tr("Client Info"));

    /*productForm 생성*/
    productForm = new ProductManagerForm(this);
    productForm->setWindowTitle(tr("Product Info"));

    /*orderForm 생성*/
    orderForm = new ShopManagerForm(this);
    orderForm->setWindowTitle(tr("Shop Info"));

    /*serverForm 생성*/
    serverForm = new ChatServerForm(this);
    serverForm->setWindowTitle(tr("Chatting Server"));


    /*객체가 파괴되기 전에 emit하고, 해당 객체는 나중에 삭제되도록 만듦*/
    connect(clientForm, SIGNAL(destroyed()),
            clientForm, SLOT(deleteLater()));
    connect(productForm, SIGNAL(destroyed()),
            productForm, SLOT(deleteLater()));
    connect(orderForm, SIGNAL(destroyed()),
            orderForm, SLOT(deleteLater()));


    /*clientForm에서 client의 id와 이름을 보내고, orderForm에서 해당 정보들을 받는다*/
    connect(clientForm, SIGNAL(clientAdded(QString,int)),
            orderForm, SLOT(addClient(QString,int)));
    /*clientForm에서 client의 id와 이름을 보내고, serverForm에서 해당 정보들을 받는다*/
    connect(clientForm, SIGNAL(clientAdded(QString, int)), serverForm, SLOT(addClientToServer(QString, int)));


    /*productForm에서 product의 id와 상품이름, 가격, 카테고리를 보내고, orderForm에서 해당 정보들을 받는다*/
    connect(productForm, SIGNAL(productAdded(QString, int, QString, int)),
            orderForm, SLOT(addProduct(QString, int, QString, int)));


    /*clientForm에서 특정 인덱스에 대한 client의 id와 이름을 변경하면 orderForm에서도 해당 정보들을 변경해준다*/
    connect(clientForm, SIGNAL(clientModified(QString,int, int)),
            orderForm, SLOT(modifyClient(QString,int, int)));
    /*clientForm에서 특정 인덱스에 대한 client의 id와 이름을 변경하면 serverForm에서도 해당 정보들을 변경해준다*/
    connect(clientForm, SIGNAL(clientModified(QString, int, int)),
            serverForm, SLOT(modifyClientFromServer(QString, int, int)));
    /*productForm에서 특정 인덱스에 대한 product의 id와 상품이름을 변경하면 orderForm에서도 해당 정보들을 변경해준다*/
    connect(productForm, SIGNAL(productModified(QString,int, int)),
            orderForm, SLOT(modifyProduct(QString,int, int)));


    /*clientForm에서 특정 인덱스에 있는 client의 정보를 삭제하면 orderForm에서도 해당 정보들을 삭제해준다*/
    connect(clientForm, SIGNAL(clientRemoved(int)),
            orderForm, SLOT(removeClient(int)));
    /*clientForm에서 특정 인덱스에 있는 client의 정보를 삭제하면 serverForm에서도 해당 정보들을 삭제해준다*/
    connect(clientForm, SIGNAL(clientRemoved(int)),
            serverForm, SLOT(removeClientFromServer(int)));
    /*clientForm에서 특정 인덱스에 있는 product의 정보를 삭제하면 orderForm에서도 해당 정보들을 삭제해준다*/
    connect(productForm, SIGNAL(productRemoved(int)),
            orderForm, SLOT(removeProduct(int)));


    /*orderForm에서 client의 id를 보내고, clientForm에서 해당 정보를 받는다*/
    connect(orderForm, SIGNAL(SendCID(int)), clientForm, SLOT(CIDsended(int)));
    /*clientForm에서 받았던 id에 해당하는 client의 정보들(고객이름, 전화번호, 주소)를 보내고, orderForm에서 해당 정보들을 받는다*/
    connect(clientForm, SIGNAL(sendCInfo(QString, QString, QString)), orderForm, SLOT(CInfoSended(QString, QString, QString)));

    /*orderForm에서 client의 id를 보내고, clientForm에서 해당 정보를 받는다*/
    connect(orderForm, SIGNAL(SendPID(int)), productForm, SLOT(PIDsended(int)));
    /*productForm에서 받았던 id에 해당하는 product의 정보들(상품이름, 가격, 카테고리)를 보내고, orderForm에서 해당 정보들을 받는다*/
    connect(productForm, SIGNAL(sendPInfo(QString, int, QString)), orderForm, SLOT(PInfoSended(QString, int, QString)));


    /*텍스토로 저장되어 있는 기존의 정보들을 불러온다*/
    clientForm->loadData();
    productForm->loadData();
    orderForm->loadData();


    /*mainwindow의 mdiArea애 clientForm, productForm, orderForm, serverForm을 추가한다*/
    QMdiSubWindow *cw = ui->mdiArea->addSubWindow(clientForm);
    ui->mdiArea->addSubWindow(productForm);
    ui->mdiArea->addSubWindow(orderForm);
    ui->mdiArea->addSubWindow(serverForm);
    ui->mdiArea->setActiveSubWindow(cw);
}

/*소멸자, 창을 꺼줌*/
MainWindow::~MainWindow()
{
    delete ui;
}

/*Client Info를 클릭했을 때 clientForm에 해당하는 화면이 나타남*/
void MainWindow::on_actionClient_triggered()
{
    if(clientForm != nullptr) {
        clientForm->setFocus();
    }
}

/*Product Info를 클릭했을 때 productForm에 해당하는 화면이 나타남*/
void MainWindow::on_actionProduct_triggered()
{
    if(productForm != nullptr) {
        productForm->setFocus();
    }
}

/*Shop Info를 클릭했을 때 orderForm에 해당하는 화면이 나타남*/
void MainWindow::on_actionOrder_triggered()
{
    if(orderForm != nullptr) {
        orderForm->setFocus();
    }
}

/*chat 액션을 클릭했을 때 chatclientform(wd)에 해당하는 화면이 나타남*/
void MainWindow::on_actionChat_triggered()
{
    wd = new Widget;
    wd->show();

    /*chatclient에서 입력한 고객 이름을 serverForm으로 보내줌*/
    connect(wd, SIGNAL(sendClientNameToServer(QString)),
            serverForm, SLOT(clientNameSended(QString)));
    /*serverForm에서 받았던 고객 이름이 chatclient에 있는지 확인해 고객리스트에 있다면 1 없다면 0인 nameflag값을 보내준다*/
    connect(serverForm, SIGNAL(sendNameFlag(int)),
            wd, SLOT(nameFlagSended(int)));
}

