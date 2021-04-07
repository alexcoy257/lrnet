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

LRdbClient::LRdbClient(const QString & filename, QObject * parent): QObject(parent){
    LRdbSettings temp(filename);
    if (temp.validFile()){
        init(temp.username, temp.password, temp.database, temp.host);
    }
}

LRdbClient::LRdbClient(QObject * parent): QObject(parent){
    LRdbSettings temp("/etc/lrnet/settings.json");
    if (temp.validFile()){
        init(temp.username, temp.password, temp.database, temp.host);
    }
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
    readDb.close();
}

bool LRdbClient::netidExists(QString& netid){
    readDb.open();
    QSqlQuery query;
    query.prepare("SELECT netid FROM users WHERE netid=?");
    query.bindValue(0, QVariant(netid));
    query.exec();
    bool nothing = false;
        while (query.next()) {
            nothing = true;
            QString country = query.value(0).toString();
        }

    readDb.close();
    return nothing;
}

bool LRdbClient::addKeyToNetid(QByteArray& key, QString& netid){
    readDb.open();
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

        query.prepare("INSERT INTO users (netid, pubkey, role) VALUES (?,?,?)");
        query.bindValue(0, QVariant(netid));
        query.bindValue(1, QVariant(key));
        query.bindValue(2, QVariant("user")); //May want to change this.
        if(query.exec())
            qDebug() << "Succeeded to add";
        else
            qDebug() << "Fail add";
    }

    readDb.close();
    return false;
    
}

void LRdbClient::removeUser(QString& netid){
    readDb.open();
    QSqlQuery query(readDb);
    query.prepare("DELETE FROM users WHERE netid=?");
    query.bindValue(0, QVariant(netid));

    if (query.exec())
        qDebug() << "Succeeded in removing " << netid;
    else
        qDebug() << "Failed to remove " << netid;
    readDb.close();
}

QVector<int> * LRdbClient::getIDsForNetid(QString &netid){
    readDb.open();
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
    else{
        qDebug() <<__FUNCTION__ <<__LINE__ << "Failed to get ids for netid " <<netid;
    }
    readDb.close();
    return vec;
}

QVector<int> * LRdbClient::getIDsForNetid(char * netid, int len){
    QString qs_netid = QString::fromLocal8Bit(netid, len);
    return getIDsForNetid(qs_netid);
}

QByteArray * LRdbClient::getKeyForID(int id){
    readDb.open();
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
    readDb.close();
    return arr;
}

int LRdbClient::getIDforKeyAndNetID(QByteArray& key, QString &netid){
    readDb.open();
    QSqlQuery query(readDb);
    int id = -1;
    query.prepare("SELECT id FROM users WHERE pubkey=? and netid=?");
    query.bindValue(0, QVariant(key));
    query.bindValue(1, QVariant(netid));
    if (query.exec()){
        qDebug() << "Succeeded to query getIDforKeyandNetID";
        if (query.next()){
            id = query.value(0).toInt();
        }
    }
    return id;
}

std::list<auth_roster_t> * LRdbClient::getRoles(){
    readDb.open();
    std::list<auth_roster_t> * stdAuthRoster = new std::list<auth_roster_t>();
    QSqlQuery query(readDb);
    query.prepare("SELECT DISTINCT netid, role FROM users");
    if (query.exec()){
        qDebug() << "Succeeded to query getRoles";

        while (query.next()){
            AuthTypeE authType;
            QString authTypeString = query.value(1).toString();
            if (authTypeString == "superchef")
                    authType = SUPERCHEF;
            else if (authTypeString == "chef")
                    authType = CHEF;
            else if (authTypeString == "user")
                    authType = MEMBER;
            else
                    authType = NONE;

            stdAuthRoster->push_back({query.value(0).toString().toStdString(),
                                    authType});
        }
    }
    readDb.close();
    return stdAuthRoster;
}

/**
 * You must delete the QString produced by this method.
 */
QString * LRdbClient::getRoleForID(int id){
    readDb.open();
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
    readDb.close();
    return str;
}

LRdbClient::~LRdbClient(){
    readDb.close();
   //std::cout << "Destructor called \n";
}

void LRdbClient::getPrivileges(){
    readDb.open();
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
    readDb.close();
}

bool LRdbClient::tryToMakeUsersSchema(){
    if (!connGood){
        return false;
    }
    readDb.open();
    QSqlQuery query(readDb);
    QString qBase = "CREATE TABLE users (\
    id int(11) NOT NULL AUTO_INCREMENT,\
    netid varchar(30) NOT NULL,\
    role enum('superchef','chef','user') DEFAULT NULL,\
    pubkey varchar(460) DEFAULT NULL,\
    PRIMARY KEY (id))";
    query.prepare(qBase);
    if(query.exec() && query.lastError().type() == QSqlError::NoError){
        readDb.close();
        return true;
    }
    readDb.close();
    return false;
}


bool LRdbClient::tryToMakeControlsSchema(){
    if (!connGood){
        return false;
    }
    readDb.open();
    QSqlQuery query(readDb);
    QString qBase = "CREATE TABLE controls (\
    id int NOT NULL,\
    c_ratio int,\
    c_threshold int,\
    c_attack int,\
    c_release int,\
    c_makeup int,\
    gain int,\
    PRIMARY KEY (id))";
    query.prepare(qBase);
    if (query.exec() && query.lastError().type() == QSqlError::NoError){
        readDb.close();
        return true;
    }
    readDb.close();
    return false;

}

void LRdbClient::setRoleForID(AuthTypeE role, int id){
    readDb.open();
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
    query.bindValue(1, QVariant(id));

    if(query.exec()){
        qDebug() << "Succeeded to query role";


    }
    readDb.close();
}

void LRdbClient::setRoleForNetID(AuthTypeE role, QString netid){
    readDb.open();
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
        qDebug() << "Succeeded to set role";


    }
    readDb.close();
}


void LRdbClient::addControlsForUID(db_controls_t controls, int uid){
    readDb.open();
    QSqlQuery query(readDb);
    query.prepare("SELECT id FROM controls WHERE id=?");
    query.bindValue(0, QVariant(uid));
    if(query.exec())
        qDebug() << "Succeeded to query";
    else
        qDebug() << "Fail query";
    bool nothing = true;
    while (query.next()) {
        nothing = false;
    }

    if (nothing){

        query.prepare("INSERT INTO controls (id, c_ratio, c_threshold, c_attack, c_release, c_makeup, gain) VALUES (?,?,?,?,?,?,?)");
        query.bindValue(0, QVariant(uid));
        query.bindValue(1, QVariant(controls.ratio));
        query.bindValue(2, QVariant(controls.threshold));
        query.bindValue(3, QVariant(controls.attack));
        query.bindValue(4, QVariant(controls.release));
        query.bindValue(5, QVariant(controls.makeup));
        query.bindValue(6, QVariant(controls.gain));
        if(query.exec())
            qDebug() << "Succeeded to add";
        else
            qDebug() << "Fail add";
    }

    readDb.close();
}


void LRdbClient::updateControlsForUID(db_controls_t controls, int uid){
    readDb.open();
    QSqlQuery query(readDb);
    query.prepare("SELECT id FROM controls WHERE id=?");
    query.bindValue(0, QVariant(uid));
    if(query.exec())
        qDebug() << "Succeeded to query";
    else
        qDebug() << "Fail query";
    bool something = false;
    while (query.next()) {
        something = true;
    }

    if (something){

        query.prepare("UPDATE controls SET c_ratio=?, c_threshold=?, c_attack=?, c_release=?, c_makeup=?, gain=? WHERE id=?");
        query.bindValue(0, QVariant(controls.ratio));
        query.bindValue(1, QVariant(controls.threshold));
        query.bindValue(2, QVariant(controls.attack));
        query.bindValue(3, QVariant(controls.release));
        query.bindValue(4, QVariant(controls.makeup));
        query.bindValue(5, QVariant(controls.gain));
        query.bindValue(6, QVariant(uid));
        if(query.exec())
            qDebug() << "Succeeded to update";
        else
            qDebug() << "Fail update";
    }

    readDb.close();
}

void LRdbClient::removeControlsForUID(int uid){
    readDb.open();
    QSqlQuery query(readDb);
    query.prepare("DELETE FROM controls WHERE id=?");
    query.bindValue(0, QVariant(uid));

    if (query.exec())
        qDebug() << "Succeeded in removing " << uid;
    else
        qDebug() << "Failed to remove " << uid;
    readDb.close();
}

db_controls_t LRdbClient::getControlsForUID(int uid){
    readDb.open();
    QSqlQuery query(readDb);
    db_controls_t controls = default_db_controls;
    query.prepare("SELECT c_ratio, c_threshold, c_attack, c_release, c_makeup, gain FROM controls WHERE id=?");
    query.bindValue(0,QVariant(uid));
    if (query.exec()){
        if (query.next()){
            controls.ratio = query.value(0).toInt();
            controls.threshold = query.value(1).toInt();
            controls.attack = query.value(2).toInt();
            controls.release = query.value(3).toInt();
            controls.makeup = query.value(4).toInt();
            controls.gain = query.value(5).toInt();
//            qDebug() << "Ratio: " << query.value(0).toInt();
//            qDebug() << "Threshold: " << query.value(1).toInt();
//            qDebug() << "Attack: " << query.value(2).toInt();
//            qDebug() << "Release: " << query.value(3).toInt();
//            qDebug() << "Makeup: " << query.value(4).toInt();
//            qDebug() << "Gain: " << query.value(5).toInt();
        }
    }

    return controls;

}

