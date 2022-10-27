#include "chatclientform.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDataStream>
#include <QTcpSocket>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

#define BLOCK_SIZE      1024

Widget::Widget(QWidget *parent) : QWidget(parent), isSent(false) {
    // 연결한 서버 정보 입력을 위한 위젯들
    name = new QLineEdit(this);

    //고객 이름 기본값 설정해주는 부분
    QSettings settings("ChatClient", "Chat Client");
    name->setText(settings.value("ChatClient/ID").toString());

    /*서버 주소를 127.0.0.1로 설정함*/
    serverAddress = new QLineEdit(this);
    serverAddress->setText("127.0.0.1");

    //serverAddress 값의 범위를 지정해줌
    QRegularExpression re("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
                          "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    QRegularExpressionValidator validator(re);
    serverAddress->setPlaceholderText("Server IP Address");
    serverAddress->setValidator(&validator);

    /*serverPort 설정*/
    serverPort = new QLineEdit(this);
    serverPort->setText(QString::number(PORT_NUMBER));      //포트 번호를 8000(PORT_NUMBER)로 설정
    serverPort->setInputMask("00000;_");                    //serverPort는 5글자로 설정하되, 입력하지 않았을 때는 '_'로 표시
    serverPort->setPlaceholderText("Server Port No");

    connectButton = new QPushButton(tr("Log In"), this);    //connectButton에 "Log In" 이라고 써넣음

    /*직선 레이아웃에 이름, 서버 주소, 포트번호, connectButton을 둔다*/
    QHBoxLayout *serverLayout = new QHBoxLayout;
    serverLayout->addWidget(name);
    serverLayout->addStretch(1);
    serverLayout->addWidget(serverAddress);
    serverLayout->addWidget(serverPort);
    serverLayout->addWidget(connectButton);

    /*서버에서 오는 메세지를 표시*/
    message = new QTextEdit(this);
    message->setReadOnly(true);

    /*서버로 보낼 메시지를 위한 위젯들. 메세지 작성하는 부분과 Send 버튼이 이에 해당*/
    inputLine = new QLineEdit(this);
    connect(inputLine, SIGNAL(returnPressed( )), SLOT(sendData( )));            //Enter키가 눌려졌을 때 데이터를 보내줌
    connect(inputLine, SIGNAL(returnPressed( )), inputLine, SLOT(clear( )));    //Enter키가 눌려졌을 때 inputLine을 비워줌
    sentButton = new QPushButton("Send", this);
    connect(sentButton, SIGNAL(clicked( )), SLOT(sendData( )));                 //sendButton이 눌려졌을 때 데이터를 보내줌
    connect(sentButton, SIGNAL(clicked( )), inputLine, SLOT(clear( )));         //sendButton이 눌려졌을 때 inputLine을 비워줌
    inputLine->setDisabled(true);                                               //처음에 창이 띄워졌을 때는 chat in 상태가 아니니까 inputLine과 sendButton을 비활성화함
    sentButton->setDisabled(true);

    /*inputLine과 sentButton을 위치시킬 ui 설정*/
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sentButton);

    /*fileButton 설정*/
    fileButton = new QPushButton("File Transfer", this);            //버튼에 "File Transfer"라고 적어줌
    connect(fileButton, SIGNAL(clicked( )), SLOT(sendFile( )));     //fileButton을 누르면 sendFile( )이 실행됨
    fileButton->setDisabled(true);                                  //처음에 창이 띄워졌을 때는 chat in 상태가 아니니까 fileButton을 비활성화함

    /*종료 기능*/
    QPushButton* quitButton = new QPushButton("Log Out", this);     //quitButton애 "Log Out"이라고 적어줌
    connect(quitButton, SIGNAL(clicked( )), this, SLOT(close( )));  //quitButton을 클릭하면 chatclient 위젯이 종료됨

    /*직선박스 레이아웃에 fileButton과 quitButton을 위치시킴*/
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(fileButton);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    /*mainLayout에 serverLayout과 message위젯과 inputLayout과 buttonLayout을 넣어줌*/
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(serverLayout);
    mainLayout->addWidget(message);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    /* 채팅을 위한 소켓 */
    clientSocket = new QTcpSocket(this);			// 클라이언트 소켓 생성. 서버에 연결될 때 소켓을 만듦
    connect(clientSocket, &QAbstractSocket::errorOccurred,                  //clientSocket에 에러 발생시에 errorString을 디버그로 띄운다
            [=]{ qDebug( ) << clientSocket->errorString( ); });
    connect(clientSocket, SIGNAL(readyRead( )), SLOT(receiveData( )));
    connect(clientSocket, SIGNAL(disconnected( )), SLOT(disconnect( )));

    /* 파일 전송을 위한 소켓 */
    fileClient = new QTcpSocket(this);
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));  //데이터를 보낼 준비가되면 다른 데이터를 보내고, 데이터를 다 보냈을 때는 데이터 전송을 끝냄

    /*progressDialog 객체를 생성*/
    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    /*connectButton이 클릭되었을 때*/
    connect(connectButton, &QPushButton::clicked,
            [=]{
        if(connectButton->text() == tr("Log In")) {                         //connectButton에 써진 텍스트가 Log In 이었을 때
            clientSocket->connectToHost(serverAddress->text( ),             //clientSocket에서 serverAddress와 serverPort에 해당하는 host에 연결
                                        serverPort->text( ).toInt( ));
            clientSocket->waitForConnected();                               //연결이 될 때까지 기다림
            sendProtocol(Chat_Login, name->text().toStdString().data());    //채팅 목적(Chat_Login)과 고객이름을 보내줌

            qDebug()<<name->text().toStdString().data();

            QString clientname = name->text().toStdString().data();         //clientname을 data에서 가져옴
            emit sendClientNameToServer(clientname);                        //clientname을 server로 보내줌

            qDebug() << "네임플래그: " << nameflag;
            if(nameflag == 0)                                               //nameflag가 0일 때(chatclient에서 입력한 고객 이름이 고객리스트에 없었을 때)
            {
                connectButton->setText(tr("Log In"));                       //고객리스트에 없는 고객은 로그인이 불가능하도록 해야 하기 때문에 로그인 버튼을 눌러도 로그인되지 않도록 connectButton을 계속해서 Log In으로 유지한다
                QMessageBox::warning(this, tr("Chatting Client"), \
                                      tr("No such name"));                  //그런 고객 이름은 없다는 경고창을 띄움
            }
            else                                                            //nameflag가 1일 때(chatclient에서 입력한 고객 이름이 고객리스트에 있을 때)
            {
            connectButton->setText(tr("Chat in"));                          //대기방에 입장시킨 상태(Chat in을 누르면 채팅방에 입장할 수 있는 상태)
            name->setReadOnly(true);                                        //Login을 한 상태이기 때문에 nameLineEdit에 이름을 적을 수 없도록 readOnly 상태로 변경해줌
            }
        } else if(connectButton->text() == tr("Chat in"))  {                //connectButton에 써진 텍스트가 Chat In 이었을 때
            sendProtocol(Chat_In, name->text().toStdString().data());       //채팅의 목적(Chat_In)과 고객이름을 프로토콜로 보낸다
            connectButton->setText(tr("Chat Out"));                         //connectButton에 적힌 텍스트를 Chat Out으로 변경해 준다(현재는 채팅방에 입장한 상태. Chat Out을 눌러주면 채팅방을 나가게 될 것임)
            inputLine->setEnabled(true);                                    //메세지를 적는 부분과 sentButton과 file전송 부분을 다 활성화시킴
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);
        } else if(connectButton->text() == tr("Chat Out"))  {               //connectButton에 써진 텍스트가 Chat Out 이었을 때
            sendProtocol(Chat_Out, name->text().toStdString().data());      //채팅의 목적(Chat_Out)과 고객이름을 프로토콜로 보낸다
            connectButton->setText(tr("Chat in"));                          //Chat Out을 누르면 채팅방에서 벗어나 대기실로 가기 때문에, 채팅방 입장이 가능하도록 만들어주기 위해 Chat In으로 버튼의 텍스트를 변경해준다
            inputLine->setDisabled(true);                                   //채팅방을 나간 상태이므로, 메세지를 입력하는 부분과 sendtButton과 file전송 버튼을 모두 비활성화시킴
            sentButton->setDisabled(true);
            fileButton->setDisabled(true);
        }
    } );

    setWindowTitle(tr("Chat Client"));
}

Widget::~Widget( )
{
    clientSocket->close( );                             //객체가 삭제될 때 소켓을 닫음
    QSettings settings("ChatClient", "Chat Client");
    settings.setValue("ChatClient/ID", name->text());   // 키 / 값
}

/* 창이 닫힐 때 서버에 연결 접속 메시지를 보내고 종료 */
void Widget::closeEvent(QCloseEvent*)
{
    sendProtocol(Chat_LogOut, name->text().toStdString().data());   //Chat_LogOut할 때 데이터를 보내줌
    clientSocket->disconnectFromHost();                             //clientSocket을 host로부터 연결을 끊어줌
    if(clientSocket->state() != QAbstractSocket::UnconnectedState)  //clientSockt의 상태가 연결되지 않은 상태라면
        clientSocket->waitForDisconnected();                        //clientSocket의 연결이 끊어질 때까지 기다려줌
}

/* 데이터를 받을 때 */
void Widget::receiveData( )
{
    QTcpSocket *clientSocket = dynamic_cast<QTcpSocket *>(sender( ));   //서버와 통신
    if (clientSocket->bytesAvailable( ) > BLOCK_SIZE) return;           //clientSocket의 사용가능한 바이트크기가 BLOCK_SIZE(1024)보다 클 때는 아무 일도 일어나지 않도록 만듦
    QByteArray bytearray = clientSocket->read(BLOCK_SIZE);              //clientSocket에서 BLOCK_SIZE만큼 읽어와 bytearray에 저장함

    Chat_Status type;               // 채팅의 목적(1024바이트 중 4바이트)
    char data[1020];                // 전송되는 메시지/데이터(1024바이트 중 1020바이트)
    memset(data, 0, 1020);          // data를 초기화 해 줌

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);           // 데이터를 처음부터 찾음
    in >> type;                     // 패킷의 타입
    in.readRawData(data, 1020);     // 실제 데이터

    switch(type) {
    case Chat_Talk:                             // 온 패킷의 타입이 대화이면
        if (flag == 0)                          // flag가 0일 때(초대가 된 상태일 때)
        {
            message->append(QString(data));     // 온메시지를 화면에 표시
            inputLine->setEnabled(true);        // 채팅의 목적인 Chat_Talk에 맞게 상태 변경. inputLine, sentButton, fileButton을 활성화
            sentButton->setEnabled(true);
            fileButton->setEnabled(true);
        }
        else if (flag == 1)                     // flag가 1일 때(강퇴가 된 상태일 때)
        {
            inputLine->setEnabled(false);       // 강퇴가 된 상태이기 때문에 inputLine, sentButton, fileButton을 비활성화
            sentButton->setEnabled(false);
            fileButton->setEnabled(false);
        }
        break;
    case Chat_KickOut:                                              // 강퇴면
        flag = 1;

        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Kick out from Server"));          //강퇴되었다는 경고 메세지박스를 띄움
        inputLine->setDisabled(true);                               // 버튼의 상태 변경(inputLine, sentButton, fileButton을 비활성화)
        sentButton->setDisabled(true);
        fileButton->setDisabled(true);
        connectButton->setDisabled(true);
        connectButton->setText("Chat in");
        name->setReadOnly(false);                                   // 메시지 입력 불가

        break;
    case Chat_Invite:                                               // 초대면
        flag = 0;
        QMessageBox::critical(this, tr("Chatting Client"), \
                              tr("Invited from Server"));           // 초대가 되었다는 메세지박스를 띄운다
        inputLine->setEnabled(true);                                // 버튼의 상태 변경(inputLine, sentButton, fileButton을 활성화)
        sentButton->setEnabled(true);
        fileButton->setEnabled(true);
        name->setReadOnly(true);                                    // 이미 초대된 상태이기 때문에 고객이름을 변경하지 못하도록 readOnly로 바꾸어준다
        connectButton->setEnabled(true);
        connectButton->setText(tr("Chat Out"));
        break;
    };
}

/* 연결이 끊어졌을 때 : 상태 변경 */
void Widget::disconnect( )
{
    QMessageBox::critical(this, tr("Chatting Client"), \
                          tr("Disconnect from Server"));        //연결이 끊어졌을 때 연결이 끊어졌다는 메세지박스를 띄운다
    inputLine->setEnabled(false);                               // 버튼의 상태 변경(inputLine, sentButton, fileButton을 비활성화)
    name->setReadOnly(false);
    sentButton->setEnabled(false);
    connectButton->setText(tr("Log in"));
}

/* 프로토콜을 생성해서 서버로 전송 */
void Widget::sendProtocol(Chat_Status type, char* data, int size)
{
    QByteArray dataArray;                                   // 소켓으로 보낼 데이터를 채우고
    QDataStream out(&dataArray, QIODevice::WriteOnly);
    out.device()->seek(0);                                  // 처음부터 찾는다
    out << type;                                            // 채팅의 목적을 출력
    out.writeRawData(data, size);                           // 데이터를 원시데이터 값을 출력함
    clientSocket->write(dataArray);                         // 서버로 전송
    clientSocket->flush();                                  // clientSocket 데이터를 초기화
    while(clientSocket->waitForBytesWritten());             // clientSocket에 다 써질 때까지 기다림
}

/* 메시지 보내기 */
void Widget::sendData(  )
{
    QString str = inputLine->text( );                           //inputLine에 적은 텍스트를 변수 str에 저장해 줌
    if(str.length( )) {                                         //메세지의 길이가 있으면 메세지를 보낼 준비를 함
        QByteArray bytearray;
        bytearray = str.toUtf8( );
        /* 화면에 표시 : 앞에 '나'라고 추가 */
        message->append("<font color=red>나</font> : " + str);   //자신이 보내는 메세지는 메세지 앞에 빨간 글씨로 '나 :' 라고 적어준다
        sendProtocol(Chat_Talk, bytearray.data());               //채팅의 목적(Chat_Talk)과 채팅 내용을 프로토콜로 보내준다
    }
}

/* 파일 전송시 여러번 나눠서 전송 */
void Widget::goOnSend(qint64 numBytes)      // 파일 전송을 시작
{
    /*파일의 전체 크기에서 numBytes씩만큼 나누어 전송*/
    byteToWrite -= numBytes; // 데이터 사이즈를 유지
    outBlock = file->read(qMin(byteToWrite, numBytes));
    fileClient->write(outBlock);

    /*progressDialog에서 totalSize를 최대로 설정해주고, 파일을 나누어 전송할 때마다 totalSize에서 byteToWrite만큼 삭제해줌*/
    progressDialog->setMaximum(totalSize);
    progressDialog->setValue(totalSize-byteToWrite);

    if (byteToWrite == 0) {                 // 전송이 완료되었을 때(이제 더이상 남은 파일 크기가 없을 때)
        qDebug("File sending completed!");
        progressDialog->reset();            // progressDialog를 초기화 시켜줌
    }
}

/* 파일 보내기 */
void Widget::sendFile() // 파일을 열고 파일 이름을 가져오는 부분 (including path)
{
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString filename = QFileDialog::getOpenFileName(this);
    if(filename.length()) {             //파일이름 선택할 때 파일이름의 길이가 1이상이면
        file = new QFile(filename);     //file 객체를 만들어 파일을 readOnly로 연다
        file->open(QFile::ReadOnly);

        qDebug() << QString("file %1 is opened").arg(filename);
        progressDialog->setValue(0); // Not sent for the first time

        if (!isSent) { // Only the first time it is sent, it happens when the connection generates the signal connect
            fileClient->connectToHost(serverAddress->text( ),
                                      serverPort->text( ).toInt( ) + 1);    //fileClient에서 serverAddress와 serverPort를 통해 host로 커넥트 함
            isSent = true;
        }

        // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

        byteToWrite = totalSize = file->size(); // The size of the remaining data
        loadSize = 1024; // The size of data sent each time

        QDataStream out(&outBlock, QIODevice::WriteOnly);
        out << qint64(0) << qint64(0) << filename << name->text();  //filename과 name의 크기가 현재는 어떤 상태인지 모르니까 일단 만들어놓음

        totalSize += outBlock.size(); // The total size is the file size plus the size of the file name and other information
        byteToWrite += outBlock.size();

        out.device()->seek(0); // 앞으로 이동해 일단 0으로 설정되어 있었던 totalsize와 byteToWrite의 제대로 된 값을 적어줌 // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
        out << totalSize << qint64(outBlock.size());

        fileClient->write(outBlock); // Send the read file to the socket    //서버로 보내줌

        progressDialog->setMaximum(totalSize);
        progressDialog->setValue(totalSize-byteToWrite);
        progressDialog->show();
    }
    qDebug() << QString("Sending file %1").arg(filename);
}


/*chatclientform에서 적은 고객이름이 clientList에 있는지 확인하기 위해 받아온 nameFlag1값을 멤버변수 nameflag에 넣어준다*/
void Widget::nameFlagSended(int nameflag1)
{
    nameflag = nameflag1;
    qDebug() << "디버깅중" <<nameflag;

}
