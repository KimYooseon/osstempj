#include "chatserverform.h"
#include "ui_chatserverform.h"
#include "logthread.h"

#include <QPushButton>
#include <QBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ChatServerForm::ChatServerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatServerForm), totalSize(0), byteReceived(0)
{
    //현재 클래스에 ui를 구성한다
    ui->setupUi(this);

    //위젯의 사이즈 설정. 120은 전체 화면의 왼쪽에 해당하는 비중, 500은 전체 화면의 오른쪽에 해당하는 비중
    QList<int> sizes;
    sizes << 120 << 500;
    ui->splitter->setSizes(sizes);

    chatServer = new QTcpServer(this);

    //chatServer에서 새로운 연결이 있을 때 clientConnect를 실행시켜 연결한다
    connect(chatServer, SIGNAL(newConnection( )), SLOT(clientConnect( )));

    //chatServer에서 포트가 연결되지 않았을 때는 경고창과 함께 에러메세지를 내보낸다
    if (!chatServer->listen(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(chatServer->errorString( )));
        //close( );
        return;
    }

    //파일 서버를 만들어 새로운 연결이 있을 때 acceptConnection을 실행시켜 파일전송이 가능하도록 함
    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));

    //fileServer에서 포트가 연결되지 않았을 때는 경고창과 함께 에러메시지를 내보낸다
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {    //PORT_NUMBER+1인 이유: chatServer와 다른 포트번호를 사용하기 위함
        QMessageBox::critical(this, tr("Chatting Server"), \
                              tr("Unable to start the server: %1.") \
                              .arg(fileServer->errorString( )));
        //close( );
        return;
    }

    qDebug("Start listening ...");

    QAction* inviteAction = new QAction(tr("&Invite"));                 //채팅 초대 기능을 만듦
    inviteAction->setObjectName("Invite");                              //초대 기능의 이름을 Invite라고 만듦
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));   //invite를 클릭하면 고객초대 기능이 실행됨

    QAction* removeAction = new QAction(tr("&Kick out"));               //채팅 강퇴 기능을 만듦
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));        //강퇴 기능의 이름을 Invite라고 만듦

    //메뉴를 만듦
    menu = new QMenu;
    menu->addAction(inviteAction);                                      //메뉴에 채팅초대기능을 추가함
    menu->addAction(removeAction);                                      //메뉴에 채팅강퇴기능을 추가함
    ui->clientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);  //CustomContextMenu로 설정했기 때문에 현재의 위치에서 메뉴가 뜨게 된다

    progressDialog = new QProgressDialog(0);    //ProgressDialog는 작업 소요 시간을 알려주고 응용 프로그램이 중지되지 않았음을 보여주는 데 사용
    progressDialog->setAutoClose(true);         //다이얼로그가 reset에 의해 숨겨질 것인지 아닌지 판단함. true가 default값(숨겨짐)
    progressDialog->reset();                    //진행중인 다이얼로그를 리셋시킴(현재 다이얼로그는 autoClose()가 true이기 때문에 숨겨짐)

    logThread = new LogThread(this);            //로그 저장 기능을 만듦
    logThread->start();                         //start()를 통해 스레드 실행을 시작함

    connect(ui->savePushButton, SIGNAL(clicked()), logThread, SLOT(saveData()));   //save버튼을 눌렀을 때 logThread에서 데이터를 저장하게 만듦

    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort( ));

    ui->clientTreeWidget->hideColumn(2);
}

ChatServerForm::~ChatServerForm()
{
    delete ui;                          //소멸자가 호출될 때 ui를 삭제함

    logThread->terminate();             //로그스레드 기능을 중지시킴
    chatServer->close( );               //chatServer를 닫음
    fileServer->close( );               //fileServer를 닫음
}

/*고객 연결*/
void ChatServerForm::clientConnect( )
{
    QTcpSocket *clientConnection = chatServer->nextPendingConnection( );        //chatServer에서 다음에 기다리고 있는 연결을 연결해준다
    connect(clientConnection, SIGNAL(readyRead( )), SLOT(receiveData( )));      //clientConnection에서 readRead 시그널을 보내면 receiveData를 실행. 시그널을 보낼 객체가 sender가 됨
    connect(clientConnection, SIGNAL(disconnected( )), SLOT(removeClient()));   //clientConnection disconnected 시그널을 보내면 removeClient를 실행
    qDebug("new connection is established...");
}

/*데이터를 받아오는 기능*/
void ChatServerForm::receiveData( )
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));   //연결되어 있는 소켓이 client소켓
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1020];        // 전송되는 메시지/데이터
    memset(data, 0, 1020);  // 초기값을 설정하는 부분. 초기 데이터 값을 0으로 설정(쓰레기값이 들어가지 않도록 함)

    QDataStream in(&bytearray, QIODevice::ReadOnly);    // 데이터스트림으로 정보를 받아옴: in
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1020); //데이터를 씀

    QString ip = clientConnection->peerAddress().toString();    //고객이 연결되었을 때 peeraddress를 ip에 저장해 줌
    quint16 port = clientConnection->peerPort();                //peerPort를 port에 저장해 줌
    QString name = QString::fromStdString(data);                //받은 data를 name에 저장해 줌

    qDebug() << ip << " : " << type;

    switch(type) {      //채팅의 타입에 따라 case를 선택해 줌
    case Chat_Login:    //Chat_Login 상태일 때
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {    //foreach가 돌아가면서 clientTreeWidget에서 name을 검색함.
            if(item->text(2) != "-") {                      //현재 상태가 "-"가 아닐 때
                item->setText(2, "-");                      //현재 상태를 "-"를 설정해 줌
                item->setIcon(0, QIcon("loading.png"));
                clientList.append(clientConnection);        //QList<QTcpSocket*> clientList;
                clientSocketHash[name] = clientConnection;  //clientSocketHash로 name과 clientConnection을 이어준다. 이름에 소켓을 연결해줌
            }
        }
        break;
    case Chat_In:       //Chat_In 상태일 때
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {    //clientTreeWidget에서 name을 검색함
            if(item->text(2) != "O") {                      //현재 상태가 "O"가 아닐 때
                item->setText(2, "O");                      //현재 상태를 "O"를 설정해 줌
                item->setIcon(0, QIcon("connected.png"));
            }
            clientNameHash[port] = name;                    //clientNameHash로 port번호와 name를 이어준다
        }
        break;
    case Chat_Talk: {   //Chat_Talk 상태일 때
        foreach(QTcpSocket *sock, clientSocketHash.values()) {
            qDebug() << sock->peerPort();
            if(clientNameHash.contains(sock->peerPort()) && port != sock->peerPort()) { //나를 제외한 모든 사람들에게 메세지를 보내줌
                QByteArray sendArray;
                sendArray.clear();                                           //메모리를 비워준다
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;                                            //채팅내역을 내보내 줌
                sendArray.append("<font color=lightsteelblue>");             //자신이 아닌 peerport를 사용중인 채팅참여자의 이름 색깔을 lightsteelblue로 설정해준다
                sendArray.append(clientNameHash[port].toStdString().data()); //clientNameHash에 저장되어 있는 port에 해당하는 이름을 append 해준다
                sendArray.append("</font> : ");                              //':' 전까지 lightsteelblue로 설정
                sendArray.append(name.toStdString().data());                 //이름에 해당되는 데이터를 추가해준다
                sock->write(sendArray);                                      //sendArray에 해당하는 내용을 써준다
                qDebug() << sock->peerPort();
            }
        }

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);     //messageTreeWidget의 정보들을 담을 변수 item을 생성함
        item->setText(0, ip);                                                   //인덱스 0번은 ip로 설정
        item->setText(1, QString::number(port));                                //인덱스 1번은 port로 설정
        item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));  //인덱스 2번은 고객 id로 설정
        item->setText(3, clientNameHash[port]);                                 //인덱스 3번은 고객 이름으로 설정
        item->setText(4, QString(data));                                        //인덱스 4번은 채팅 내용(data)으로 설정
        item->setText(5, QDateTime::currentDateTime().toString());              //인덱스 5번은 채팅을 보낸 시간으로 설정
        item->setToolTip(4, QString(data));  //채팅 내용인 인덱스 4번에 커서를 가져다 대면 대화내용이 노란색 박스(툴팁)에 띄워진다(대화내용이 길어서 생략되어 나타날 경우, 4번 인덱스에 해당하는 위치에 커서를 가져다 대면 모든 대화내용을 확인할 수 있음
        ui->messageTreeWidget->addTopLevelItem(item);                           //messageTreeWidget에 item을 추가해 줌

        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)           //messageTreeWidget에서 column 개수까지 for문을 수행함
            ui->messageTreeWidget->resizeColumnToContents(i);                   //column의 사이즈를 현재 표시되는 메세지의 길이에 맞춘다

        logThread->appendData(item);                                            //item의 데이터를 logThread에 더해준다
    }
        break;
    case Chat_Out:      //Chat_Out 상태일 때
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {   //clientTreeWidget에서 name을 찾고 그에 해당하는 item을 변수로 삼음
            if(item->text(2) != "-") {      //item에 있는 0번째 인덱스가 "-"가 아니면
                item->setText(2, "-");      //item의 0번째 인덱스를 "-"로 바꾸어준다
                item->setIcon(0, QIcon("loading.png"));
            }
            clientNameHash.remove(port);    //clientNameHash의 port를 삭제해준다(쌍인 이름도 함께 삭제됨)
        }
        break;
    case Chat_LogOut:   //Chat_LogOut 상태일 때
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {   //clientTreeWidget에서 name을 찾고 그에 해당하는 item을 변수로 삼음
            if(item->text(2) != "X") {          //item에 있는 0번째 인덱스가 "X"가 아니면
                item->setText(2, "X");          //item의 0번째 인덱스를 "X"로 바꾸어준다
                item->setIcon(0, QIcon("disconnected.png"));
                clientList.removeOne(clientConnection);  // QList<QTcpSocket*> clientList;    //clientList에서 clientConnection을 삭제해준다
                clientSocketHash.remove(name);  //clientSocketHash에서 name을 삭제해준다
            }
        }
        break;
    }
}

/*고객 삭제 기능*/
void ChatServerForm::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));

    if(clientConnection != nullptr){
        QString name = clientNameHash[clientConnection->peerPort()];    //name에 peerPort에 해당하는 고객연결을 실행하고 name에 그 포트번호를 넣어주어 해시값들을 연결한다
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {   //clientTreeWidget에서 name을 찾고 그에 해당하는 item을 변수로 삼음
            item->setText(2, "X");                  //0번 인덱스를 X로 만든다(disconnect되었을 때)
            item->setIcon(0, QIcon("disconnected.png"));
        }

        clientList.removeOne(clientConnection);     //clientList에서 clientConnection을 없애준다
        clientConnection->deleteLater();            //clientConnection을 삭제하도록 예정시킴
    }
}

/*고객을 서버에 추가*/
void ChatServerForm::addClientToServer(QString name, int id)
{
    clientIDList << id;                                 //clientIDList에 id를 넣어줌
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);  //clientTreeWidget에 있는 정보들을 item에 저장함
    item->setIcon(0, QIcon("disconnected.png"));
    item->setText(2, "X");                              //고객을 추가한 상태에서는 일단 초기값으로 현재 고객 상태를 "X"로 만들어 줌(0번째 인덱스)-접속하지 않은 상태
    item->setText(1, name);                             //1번째 인덱스에는 받아온 이름을 저장해줌
    ui->clientTreeWidget->addTopLevelItem(item);        //item을 clientTreeWidget의 제일 윗 줄에 추가해준다
    clientIDHash[name] = id;                            //clientIDHash에서 name과 id를 쌍을 이루게 한다
    ui->clientTreeWidget->resizeColumnToContents(0);    //고객 상태(O,X)에 해당하는 열의 너비를 적힌 내용의 길이에 맞게 조정한다
}

/*clientTreeWidget에서 마우스 오른쪽 버튼을 눌렀을 때 메뉴의 Invite기능에 대한 설정*/
void ChatServerForm::on_clientTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    foreach(QAction *action, menu->actions()) {
        if(ui->clientTreeWidget->currentItem() != nullptr){
            if(action->objectName() == "Invite")
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "-");    //현재 상태가 "-"일 때만 Invite를 활성화 시켜줌
            else
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "O");    //현재 상태가 "O"일 때(대화방에 입장한 상태일 때, invite가 불가능하도록 만든다
        }
    }
    QPoint globalPos = ui->clientTreeWidget->mapToGlobal(pos);                          //clientTreeWidget에서 오른쪽버튼을 클릭한 마우스 커서의 위치를 구한다
    menu->exec(globalPos);                                                              //마우스 오른쪽 버튼을 클릭한 위치에서 menu를 띄운다
}

/*고객 강퇴 기능*/
void ChatServerForm::kickOut()
{
    QString name = ui->clientTreeWidget->currentItem()->text(1);    //변수 name에 clientTreeWidget에 있는 현재 아이템 중 첫 번째 인덱스에 있는 이름을 저장해 줌
    QTcpSocket* sock = clientSocketHash[name];                      //이름으로 소켓을 찾아 줌. sock에 clientSocketHash[name]을 저장해 줌(clientSockHash는 name과 socket이 짝을 이룸)

    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);              //쓰기 가능한 상태
    out << Chat_KickOut;                                            //채팅을 KickOut상태로 바꿔줌
    out.writeRawData("", 1020);                                     //원시 데이터를 빈 내용으로 초기화 함
    sock->write(sendArray);                                         //sendArray에 저장된 데이터를 소켓에 적어줌

    ui->clientTreeWidget->currentItem()->setText(2, "-");           //고객을 강퇴한 상태에서는 고객은 아예 연결이 끊어진 것이 아니고 대기방에 여전히 존재하는 상태임. 그러므로 현 상태는 X가 아닌 -임
    ui->clientTreeWidget->currentItem()->setIcon(0, QIcon("loading.png"));
}

/*고객 초대 기능*/
void ChatServerForm::inviteClient()
{
    if(ui->clientTreeWidget->topLevelItemCount()) {                     //clientTreeWidget에 고객이 1명 이상 있을 때(고객이 추가되어 있지 않으면 실행되지 않음)



        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);              //쓰기 가능한 상태
        out << Chat_Invite;                                             //채팅을 invite상태로 바꾸어줌
        out.writeRawData("", 1020);                                     //원시 데이터를 빈 내용으로 초기화 해 줌

        /* 소켓은 현재 선택된 아이템에 표시된 이름과 해쉬로 부터 가져온다. */
        QString name = ui->clientTreeWidget->currentItem()->text(1);    //clientTreeWidget의 현재 아이템 값에서 1번째 인덱스값인 고객이름을 name에 저장해준다.
        QTcpSocket* sock = clientSocketHash[name];                      //이름과 소켓을 짝을 지어줌

        // 로그인 안한사람 invite할 때 crashed나는 문제 해결
        if(sock == nullptr)
        {
            return;
        }

        sock->write(sendArray);                                         //sendArray에 저장된 데이터를 소켓에 적어줌

        quint16 port = sock->peerPort();    // invite를 했을 때 고객의 이름이 messagetreewidget에 뜨게 만들어주는 부분
        clientNameHash[port] = name;        //포트번호에 이름을 짝지어 넣어줌

        ui->clientTreeWidget->currentItem()->setText(2, "O");           //초대했으니 현재 고객 상태는 O가 되어야 함
        ui->clientTreeWidget->currentItem()->setIcon(0, QIcon("connected.png"));
    }
}

/* 파일 전송을 위한 소켓 생성 */
void ChatServerForm::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();           //receivedSocket에 fileServer에서 저장해두었던 다음 보류중인 연결을 연결해준다
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));     //받은 소켓에서 정보를 읽어 serverform에서 파일 전송이 가능하도록 만듦
}

/* 파일 전송 */
void ChatServerForm::readClient()
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));     //chatclientform에서 정보들을 받아와 receivedSocket에 넣어줌
    QString filename, name;

    if (byteReceived == 0) {            // 파일 전송 시작 : 파일에 대한 정보를 이용해서 QFile 객체 생성
        progressDialog->reset();        //진행중인 다이얼로그를 초기화해서
        progressDialog->show();         //보여줌

        QString ip = receivedSocket->peerAddress().toString();              //peerAddress를 변수 ip에 저장해준다
        quint16 port = receivedSocket->peerPort();                          //peerPort번호를 변수 port에 저장해준다
        qDebug() << ip << " : " << port;

        QDataStream in(receivedSocket);                                     //파일을 전송해줄 준비
        in >> totalSize >> byteReceived >> filename >> name;                //전체 사이즈와 받은 바이트 크기와 파일 이름과 고객 이름을 입력해줌
        progressDialog->setMaximum(totalSize);                              //진행중인 다이얼로그의 totalSize를 최대로 설정한다

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget); //messageTreeWidget에 위치해 있는 정보들에 대한 변수 item을 생성함
        item->setText(0, ip);                                               //messageTreeWidget의 0번째 인덱스에는 ip를 입력해 줌
        item->setText(1, QString::number(port));                            //messageTreeWidget의 1번째 인덱스에는 port번호를 입력해 줌
        item->setText(2, QString::number(clientIDHash[name]));              //messageTreeWidget의 2번째 인덱스에는 id를 입력해 줌. id값을 이름을 이용해 가져옴
        item->setText(3, name);                                             //messageTreeWidget의 3번째 인덱스에는 name을 입력해 줌
        item->setText(4, filename);                                         //messageTreeWidget의 4번째 인덱스에는 filename을 입력해 줌
        item->setText(5, QDateTime::currentDateTime().toString());          //messageTreeWidget의 5번째 인덱스에는 현재날짜를 입력해 줌
        item->setToolTip(4, filename);                                      //messageTreeWidget의 4번째 인덱스에 마우스 커서를 올렸을 때는 filename에 대한 정보를 툴팁을 통헤 보여줌

        /* 컨텐츠의 길이로 QTreeWidget의 헤더의 크기를 고정 */
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        ui->messageTreeWidget->addTopLevelItem(item);                       //item을 messageTreeWidget에 추가한다

        logThread->appendData(item);                                        //logThread에 item을 추가해 줌

        QFileInfo info(filename);
        QString currentFileName = info.fileName();                          //경로에서 이름을 뽑아옴
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);
    } else { // 파일 데이터를 읽어서 저장
        inBlock = receivedSocket->readAll();                                //받아온 소켓에 있는 정보를 모두 읽어 변수 inBlock에 저장해 줌

        byteReceived += inBlock.size();                                     //받은 바이트 크기에 변수 inBlock의 크기를 더해줌 => 파일을 저장할 때는 한번에 저장하는 것이 아니라 나누어 저장
        file->write(inBlock);                                               //file에 inBlock에 저장되어 있는 정보를 적어준다
        file->flush();                                                      //file을 비워줌
    }

    progressDialog->setValue(byteReceived);                                 //현재 진행중인 다이얼로그의 값을 byteReceived의 값으로 설정해 줌

    if (byteReceived == totalSize) {                                        //파일을 다 읽으면 QFile 객체를 닫고 삭제
        qDebug() << QString("%1 receive completed").arg(filename);

        /*초기화*/
        inBlock.clear();                                                    //파일을 다 읽은 상태니까 inBlock안의 정보들을 비워줌
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();
        file->close();
        delete file;
    }
}


/*client정보를 clientmanagerform에서 삭제했을 때 서버에서도 자동으로 고객이름이 지워지도록 만드는 부분*/
void ChatServerForm::removeClientFromServer(int i)
{
    ui->clientTreeWidget->takeTopLevelItem(i);
}

/*client정보를 clientmanagerform에서 변경했을 때 서버에서도 자동으로 고객이름이 변경되도록 만드는 부분*/
void ChatServerForm::modifyClientFromServer(QString name, int id, int i)
{
    ui->clientTreeWidget->topLevelItem(i)->setText(1, name);
    clientIDHash[name] = id;
}

/*고객이름을 chatclient에서 받아와 고객 리스트에 존재하는 이름을 입력했는지 확인하는 부분*/
void ChatServerForm::clientNameSended(QString name)
{
    int nameflag;

    if(ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1).length() == 0)    //고객리스트에 있는 이름인지 확인
        nameflag = 0;                                                                   //없는 이름을 입력했을 때 nameflag에 0을 넣어 emit으로 chatclient로 보내줌
    else
        nameflag = 1;                                                                   //있는 이름을 입력했을 때 nameflag에 1을 넣어 emit으로 chatclient로 보내줌

    emit sendNameFlag(nameflag);
}

