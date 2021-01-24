#ifndef LRDBCLIENT_H
#define LRDBCLIENT_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "lrdbsettings.h"
#include "../lrnetserver/auth_types.h"

class LRdbClient : public QObject
{
    Q_OBJECT
    QSqlDatabase readDb;
    bool madeConnection;
    bool connGood;
public:
    explicit LRdbClient(QString & uname, QString & pw, QString & database, QString & hostname, QObject *parent = nullptr);
    explicit LRdbClient(const char* uname, const char* pw, const char* database, const char* hostname, QObject *parent = nullptr);
    explicit LRdbClient(LRdbSettings & set, QObject *parent = nullptr);
    explicit LRdbClient(const QString & filename="/etc/lrnet/settings.json", QObject *parent = nullptr);
    explicit LRdbClient(QObject *parent = nullptr);
    ~LRdbClient();
    bool netidExists(QString& netid);
    bool addKeyToNetid(QByteArray& key, QString& netid);
    void removeUser(QString& netid);
    bool connIsGood() {return connGood;};
    QVector<int> * getIDsForNetid(QString& netid);
    QVector<int> * getIDsForNetid(char * netid, int len);
    QByteArray * getKeyForID(int id);
    QString * getRoleForID(int id);
    std::list<auth_roster_t> * getRoles();
    void updatePermission(QString netid, AuthTypeE authType);
    void setRoleForID(AuthTypeE role, int id);
    void setRoleForNetID(AuthTypeE role, QString netid);
    bool tryToMakeSchema();
signals:

private:
    void init(QString & uname, QString & pw, QString & database, QString & hostname);
    void getPrivileges();
};

#endif // LRDBCLIENT_H
