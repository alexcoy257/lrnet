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
public:
    explicit LRdbClient(QString uname, QString pw, QString hostname = "localhost", QObject *parent = nullptr);
    ~LRdbClient();
    bool netidExists(QString& netid);
    bool addKeyToNetid(QByteArray& key, QString& netid);

signals:

};

#endif // LRDBCLIENT_H
