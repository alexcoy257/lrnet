#include <QCoreApplication>

#include "lrdb_app.h"
#include "lrdbclient.h"
#include <QFile>
#include <QDebug>
#include <QSqlQuery>

int main(int argc, char** argv){
    //LRdbApp app(argc, argv);
    //return app.exec();

    QByteArray myBytes;
    if (argc>1){
    QFile myFile(argv[1]);
    myFile.open(QFile::ReadOnly | QFile::Text);
    myBytes = myFile.read(451);
    }
    LRdbClient client("lrnetwrite", "lrnetwritepw");

    if (argc>2){
        QString temp = QString(argv[2]);
        if (client.netidExists(temp)){
            qDebug() <<"Netid exists";
        } else{
            qDebug() <<"Not Found";
        }
        client.addKeyToNetid(myBytes, temp);
    }

    //qDebug() << myBytes << "\n Read: " <<myBytes.length();

    //client.~LRdbClient();

}
