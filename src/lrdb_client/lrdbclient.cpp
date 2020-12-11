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
    }
    else {
        std::cout << "Database Not okay \n ";
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

LRdbClient::~LRdbClient(){
    readDb.close();
   //std::cout << "Destructor called \n";
}
