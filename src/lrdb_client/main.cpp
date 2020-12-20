#include <QCoreApplication>

#include "lrdb_app.h"
#include "lrdbclient.h"
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <getopt.h>

typedef enum{
  ADD = 1<<0,
  VERIFY = 1<<1
} action_e;

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
        QVector<int> * idTemp = client.getIDsForNetid(temp);
        if(idTemp){
        QByteArray * arr = client.getKeyForID((*idTemp)[0]);
        qDebug() <<"Key " <<*arr;
        }
    }

/*
    static struct option long_options[] = {
                {"add",    no_argument,       0,  ADD },
                {"netid",  required_argument, 0,  'n' },
                {"key",    required_argument, 0,  'k' },
                {"verify", no_argument,       0,  VERIFY },
                {0,         0,                 0,  0 }
            };

    int c;
    unsigned int action = 0;


    while(1){
        int option_index = 0;
        c = getopt_long(argc, argv, "abc:d:012",
                         long_options, &option_index);
                if (c == -1)
                    break;

        switch (c) {
            case ADD:
            case VERIFY:
            action |= c;

            break;
            case 'n':

            break;
            case 'k':

            break;

        default:
                    printf("?? getopt returned character code 0%o ??\n", c);
        }
    }*/

    //qDebug() << myBytes << "\n Read: " <<myBytes.length();

    //client.~LRdbClient();

}
