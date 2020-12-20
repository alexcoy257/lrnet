#ifndef LRDBCLIENT_H
#define LRDBCLIENT_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

class LRdbClient : public QObject
{
    Q_OBJECT
    QSqlDatabase readDb;
    bool madeConnection;
    bool connGood;
public:
    explicit LRdbClient(QString uname, QString pw, QString hostname = "localhost", QObject *parent = nullptr);
    ~LRdbClient();
    bool netidExists(QString& netid);
    bool addKeyToNetid(QByteArray& key, QString& netid);
    bool connIsGood() {return connGood;};
    QVector<int> * getIDsForNetid(QString& netid);
    QVector<int> * getIDsForNetid(char * netid, int len);
    QByteArray * getKeyForID(int id);
    QString * getRoleForID(int id);

signals:

};

#endif // LRDBCLIENT_H
