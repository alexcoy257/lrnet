#include "lrdbclient.h"
#include <iostream>

LRdbClient::LRdbClient(QString uname, QString pw, QString hostname, QObject *parent) : QObject(parent)
{
    readDb = QSqlDatabase::addDatabase("QMYSQL");
    readDb.setHostName(hostname);
    readDb.setDatabaseName("lrnetdb");
    readDb.setUserName(uname);
    readDb.setPassword(pw);
    bool ok = readDb.open(); //bool ok

    if (ok){
        std::cout << "Database Okay \n ";
        connGood = true;
    }
    else {
        std::cout << "Database Not okay \n ";
        connGood = false;
    }
}

bool LRdbClient::netidExists(QString& netid){

    QSqlQuery query;
    query.prepare("SELECT netid FROM lrnetdb.users WHERE netid=?");
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
    query.prepare("SELECT netid FROM lrnetdb.users WHERE netid=? AND pubkey=?");
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

        query.prepare("INSERT INTO lrnetdb.users (netid, pubkey) VALUES (?,?)");
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
    query.prepare("SELECT id FROM lrnetdb.users WHERE netid=?");
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
    query.prepare("SELECT pubkey FROM lrnetdb.users WHERE id=?");
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

QString * LRdbClient::getRoleForID(int id){
    QSqlQuery query(readDb);
    QString * str = NULL;
    query.prepare("SELECT role FROM lrnetdb.users WHERE id=?");
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
