#ifndef CHATSERVERFORM_H
#define CHATSERVERFORM_H
#include "chatclientform.h"
#include <QWidget>
#include <QList>
#include <QHash>

class QLabel;
class QTcpServer;
class QTcpSocket;

namespace Ui {
class ChatServerForm;
}


//typedef enum {
//    Chat_Login,
//    Chat_In,
//    Chat_Talk,
//    Chat_Close,
//    Chat_LogOut,
//    Chat_Invite,
//    Chat_KickOut,
//    Chat_FileTransform
//} Chat_Status;

typedef struct {
    Chat_Status type;
    char data[1020];
} chatProtocolType;

class ChatServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ChatServerForm(QWidget *parent = nullptr);
    ~ChatServerForm();

private:
    const int BLOCK_SIZE = 1024;
    Ui::ChatServerForm *ui;
    QTcpServer *tcpServer;
    QList<QTcpSocket*> clientList;
    QList<int> clientIDList;
    QHash<QString, QString> clientNameHash;
    QHash<QString, int> clientIDHash;
    QMenu* menu;

private slots:
    void clientConnect( );                       /* 채팅 서버 */
    void receiveData( );
    void removeClient( );
    void addClientToServer(QString, int);
    void inviteClient();
    void kickOut();
    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);

    void on_savePushButton_clicked();


    void removeClientFromServer(int);
    void modifyClientFromServer(QString, int, int);
};

#endif // CHATSERVERFORM_H
