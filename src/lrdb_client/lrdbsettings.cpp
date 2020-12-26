#include "lrdbsettings.h"
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QJsonDocument>

using std::endl;

LRdbSettings::LRdbSettings(QObject *parent) : QObject(parent)
{

}

int LRdbSettings::loadSettingsFile(QString & filename){
    QFile jsonFile(filename);
    if (jsonFile.exists()){
        jsonFile.open(QIODevice::ReadOnly);
    } else{
        std::cerr <<"Settings file doesn't exist" <<endl;
        return 1;
    }

    const QByteArray jsonBytes = jsonFile.readAll();

    QJsonDocument jsonDoc= QJsonDocument::fromJson(jsonBytes);

    username = jsonDoc["username"].toString();
    password = jsonDoc["password"].toString();
    host = jsonDoc["host"].toString();
    database = jsonDoc["database"].toString();
    return 0;
}
