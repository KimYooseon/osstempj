#ifndef LOGTHREAD_H
#define LOGTHREAD_H

#include <QThread>
#include <QList>

class QTreeWidgetItem;

class LogThread : public QThread
{
    Q_OBJECT
public:
    explicit LogThread(QObject *parent = nullptr);

private:
    void run();

    QList<QTreeWidgetItem*> itemList;
    QString filename;

signals:
    void send(int data);

public slots:
    void appendData(QTreeWidgetItem*);  //데이터들을 itemList에 추가해줌
    void saveData();                    //채팅로그를 저장
};

#endif // LOGTHREAD_H

