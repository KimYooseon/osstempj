#ifndef CHATSERVERFORM_H
#define CHATSERVERFORM_H
#include "chatclientform.h"
#include <QWidget>
#include <QList>
#include <QHash>

class QLabel;
class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;
class LogThread;
class QTreeWidgetItem;

namespace Ui {
class ChatServerForm;
}

class ChatServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatServerForm(QWidget *parent = nullptr);
    ~ChatServerForm();

private:
    const int BLOCK_SIZE = 1024;
    const int PORT_NUMBER = 8000;

    Ui::ChatServerForm *ui;
    QTcpServer *chatServer;
    QTcpServer *fileServer;
    QList<QTcpSocket*> clientList;
    QList<int> clientIDList;
    QHash<quint16, QString> clientNameHash;
    QHash<QString, QTcpSocket*> clientSocketHash;
    QHash<QString, int> clientIDHash;
    QMenu* menu;
    QFile* file;
    QProgressDialog* progressDialog;
    qint64 totalSize;
    qint64 byteReceived;
    QByteArray inBlock;
    LogThread* logThread;                               // 서버에서 이 부분을 통해 채팅 로그를 저장함


private slots:
    void acceptConnection();                            // 파일 서버
    void readClient();

    void clientConnect( );                              // 채팅 서버
    void receiveData( );
    void removeClient( );
    void addClientToServer(QString, int);               // client의 이름과 id를 server에 추가
    void inviteClient();
    void kickOut();
    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);     //마우스 좌표값을 통해 그 위치에 메뉴를 띄우는 부분
    void removeClientFromServer(int);                   // client정보를 clientmanagerform에서 삭제했을 때 서버에서도 자동으로 고객이름이 지워지도록 만드는 부분
    void modifyClientFromServer(QString, int, int);     // client정보를 clientmanagerform에서 변경했을 때 서버에서도 자동으로 고객이름이 변경되도록 만드는 부분
    void clientNameSended(QString);                     // 고객이름을 chatclient에서 받아와 고객 리스트에 존재하는 이름을 입력했는지 확인하는 부분

signals:
    void sendNameFlag(int);                             // 고객 리스트에 존재하는 이름을 입력했는지 확인하고 없는 이름이면 nameflag=0, 있으면 nameflag=1로 설정해 줌
};

#endif // CHATSERVERFORM_H
