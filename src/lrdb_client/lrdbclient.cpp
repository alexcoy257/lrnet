#include "lrdbclient.h"
#include <iostream>
#include <QSqlError>

LRdbClient::LRdbClient(QString & uname, QString & pw, QString & database, QString & hostname, QObject *parent) : QObject(parent)
{
    init(uname, pw, database, hostname);
}

LRdbClient::LRdbClient(LRdbSettings & set, QObject * parent): QObject(parent){
    init(set.username, set.password, set.database, set.host);
}

LRdbClient::LRdbClient(const char* uname, const char* pw, const char* database, const char* hostname, QObject *parent): QObject(parent){
    QString un = QString::fromStdString(uname);
    QString pwd = QString::fromStdString(pw);
    QString db = QString::fromStdString(database);
    QString hn = QString::fromStdString(hostname);
    init(un, pwd, db, hn);
}

void LRdbClient::init(QString & uname, QString & pw, QString & database, QString & hostname){
    readDb = QSqlDatabase::addDatabase("QMYSQL");
    readDb.setHostName(hostname);
    readDb.setDatabaseName(database);
    readDb.setUserName(uname);
    readDb.setPassword(pw);
    bool ok = readDb.open(); //bool ok

    if (ok){
        std::cout << "Database Okay \n ";
        connGood = true;
        //getPrivileges();
    }
    else {
        std::cout << "Database Not okay \n ";
        connGood = false;
    }
}

bool LRdbClient::netidExists(QString& netid){

    QSqlQuery query;
    query.prepare("SELECT netid FROM users WHERE netid=?");
    query.bindValue(0, QVariant(netid));
    query.exec();
    bool nothing = false;
        while (query.next()) {
            nothing = true;
            QString country = query.value(0).toString();
        }
    return nothing;
}

bool LRdbClient::addKeyToNetid(QByteArray& key, QString& netid){
    QSqlQuery query(readDb);
    query.prepare("SELECT netid FROM users WHERE netid=? AND pubkey=?");
    query.bindValue(0, QVariant(netid));
    query.bindValue(1, QVariant(key));
    if(query.exec())
        qDebug() << "Succeeded to query";
    else
        qDebug() << "Fail query";
    bool nothing = true;
    while (query.next()) {
        nothing = false;
        QString country = query.value(0).toString();
    }

    if (nothing){

        query.prepare("INSERT INTO users (netid, pubkey) VALUES (?,?)");
        query.bindValue(0, QVariant(netid));
        query.bindValue(1, QVariant(key));
        if(query.exec())
            qDebug() << "Succeeded to add";
        else
            qDebug() << "Fail add";
    }

    return false;
}

QVector<int> * LRdbClient::getIDsForNetid(QString &netid){
    QSqlQuery query(readDb);
    QVector<int> * vec = new QVector<int>();
    query.prepare("SELECT id FROM users WHERE netid=?");
    query.bindValue(0, QVariant(netid));
    if(query.exec()){
        qDebug() << "Succeeded to query";
        while (query.next()){
            qDebug() << "ID: " <<query.value(0).toInt();
            vec->append(query.value(0).toInt());
        }

    }
    return vec;
}

QVector<int> * LRdbClient::getIDsForNetid(char * netid, int len){
    QString qs_netid = QString::fromLocal8Bit(netid, len);
    return getIDsForNetid(qs_netid);
}

QByteArray * LRdbClient::getKeyForID(int id){
    QSqlQuery query(readDb);
    QByteArray * arr = NULL;
    query.prepare("SELECT pubkey FROM users WHERE id=?");
    query.bindValue(0, QVariant(id));
    if(query.exec()){
        qDebug() << "Succeeded to query getkey";
        if (query.next()){
            //qDebug() << "ID: " <<query.value(0).toInt();
            arr = new QByteArray(query.value(0).toByteArray());
        }

    }
    return arr;
}

/**
 * You must delete the QString produced by this method.
 */
QString * LRdbClient::getRoleForID(int id){
    QSqlQuery query(readDb);
    QString * str = NULL;
    query.prepare("SELECT role FROM users WHERE id=?");
    query.bindValue(0, QVariant(id));
    if(query.exec()){
        qDebug() << "Succeeded to query role";
        if (query.next()){
            //qDebug() << "ID: " <<query.value(0).toInt();
            str = new QString(query.value(0).toString());
        }

    }
    return str;
}

LRdbClient::~LRdbClient(){
    readDb.close();
   //std::cout << "Destructor called \n";
}

void LRdbClient::getPrivileges(){
    QSqlQuery query(readDb);
    QString * str = NULL;
    query.prepare("SHOW GRANTS;");
    if(query.exec()){
        qDebug() << "Got grants";
        while(query.next()){
            str = new QString(query.value(0).toString());
            qDebug() << *str;
            delete str;
        }
    }
}

bool LRdbClient::tryToMakeSchema(){
    if (!connGood){
        return false;
    }
    QSqlQuery query(readDb);
    QString qBase = "CREATE TABLE users (\
    id int(11) NOT NULL AUTO_INCREMENT,\
    netid varchar(30) NOT NULL,\
    role enum('superchef','chef','user') DEFAULT NULL,\
    pubkey varchar(460) DEFAULT NULL,\
    PRIMARY KEY (id))";
    query.prepare(qBase);
    if(query.exec() && query.lastError().type() == QSqlError::NoError){
        return true;
    }
    return false;
}

void LRdbClient::setRoleForID(AuthTypeE role, int id){
    QSqlQuery query(readDb);
    QString sRole = QString();
    query.prepare("UPDATE users SET role=? WHERE id=?");
    switch(role){
        case SUPERCHEF:
            sRole = "superchef";
        break;
    case CHEF:
        sRole = "chef";
        break;
    case MEMBER:
        sRole = "member";
        break;
    case NONE:
        break;
    }
    if(!sRole.isNull()){
        query.bindValue(0, QVariant(sRole));
    }
    else{
        query.bindValue(0, QVariant(QString()));
    }
    query.bindValue(1, QVariant(id));

    if(query.exec()){
        qDebug() << "Succeeded to query role";


    }
}

void LRdbClient::setRoleForNetID(AuthTypeE role, QString & netid){
    QSqlQuery query(readDb);
    QString sRole = QString();
    query.prepare("UPDATE users SET role=? WHERE netid=?");
    switch(role){
        case SUPERCHEF:
            sRole = "superchef";
        break;
    case CHEF:
        sRole = "chef";
        break;
    case MEMBER:
        sRole = "user";
        break;
    case NONE:
        break;
    }
    if(!sRole.isNull()){
        query.bindValue(0, QVariant(sRole));
    }
    else{
        query.bindValue(0, QVariant(QString()));
    }
    query.bindValue(1, QVariant(netid));

    if(query.exec()){
        qDebug() << "Succeeded to query role";


    }
}
