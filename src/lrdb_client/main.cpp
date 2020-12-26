#include <QCoreApplication>

#include "lrdb_app.h"
#include "lrdbclient.h"
#include <QFile>
#include <QDebug>
#include <QSqlQuery>
#include <getopt.h>
#include <iostream>

#include "lrdbsettings.h"

typedef enum{
  ADD = 1<<1,
  VERIFY = 1<<2,
  MAKE_SCHEMA = 1<<3,
  REMOVE = 1<<4,
  ROLE_SUPERCHEF = 1<<18,
  ROLE_CHEF = 1 << 17,
  ROLE_MEMBER = 1 << 16
} action_e;

int main(int argc, char** argv){
    //LRdbApp app(argc, argv);
    //return app.exec();



    //QString temp = QString::fromStdString("ac2456");
    //client.getIDsForNetid(temp);

    /*
    QByteArray myBytes;
    if (argc>1){
    QFile myFile(argv[1]);
    myFile.open(QFile::ReadOnly | QFile::Text);
    myBytes = myFile.read(451);
    }

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
*/

    static struct option long_options[] = {
                {"add",    no_argument,       0,  ADD },
                {"remove",    no_argument,       0,  REMOVE },
                {"netid",  required_argument, 0,  'n' },
                {"keyfile",    required_argument, 0,  'k' },
                {"role",    required_argument, 0,  'k' },
                {"verify", no_argument,       0,  VERIFY },
                {"make-schema", no_argument,       0,  MAKE_SCHEMA },
                {"superchef", no_argument,       0,  ROLE_SUPERCHEF },
                {"chef", no_argument,       0,  ROLE_CHEF },
                {"member", no_argument,       0,  ROLE_MEMBER },
                {"settingsfile", required_argument,       0,  's' },
                {0,         0,                 0,  0 }
            };

    int c;
    unsigned int action = 0;
    QString fn = QString::fromStdString("settings.json");
    QString netid = QString();
    QString kfn = QString();
    AuthTypeE role = NONE;

    while(1){
        int option_index = 0;
        c = getopt_long(argc, argv, "abc:d:s:n:k:012",
                         long_options, &option_index);
                if (c == -1)
                    break;

        switch (c) {
            case ADD:
            case VERIFY:
            case MAKE_SCHEMA:
            action |= c;

            break;
            case 'n':
            netid = QString::fromStdString(optarg);
            break;
            case 'k':
            kfn = QString::fromStdString(optarg);
            break;

            case 's':
            fn = QString::fromStdString(optarg);
            break;

        case ROLE_SUPERCHEF:
            role = SUPERCHEF;
            break;
        case ROLE_MEMBER:
            role = MEMBER;
            break;
        case ROLE_CHEF:
            role = CHEF;
            break;

        default:
                    printf("?? getopt returned character code 0%o ??\n", c);
                    exit(1);
        }
    }

    //std::cout <<"Action: " <<action <<std::endl;
    if (!action){
        exit(0);
    }

    if ((action & (action-1))){
        std::cerr <<"Error! Can't select multiple add/remove/create-schema" <<std::endl;
        exit(1);
    }

    LRdbSettings s;


    s.loadSettingsFile(fn);


    LRdbClient client(s);

    if (action & MAKE_SCHEMA){
        bool success = client.tryToMakeSchema();
        if(!success)
            std::cerr << "Couldn't make schema. Maybe it exists?" <<std::endl;
        else
            std::cout << "Established schema successfully." <<std::endl;
    }

    if (action & ADD){
        if (netid.isNull()){
            std::cerr << "Error: netid was not set. \n";
            exit(1);
        }
        if (kfn.isNull()){
            std::cerr << "Error: keyfile was not set. \n";
            exit(1);
        }

        QByteArray myBytes;
        QFile myFile(kfn);
        myFile.open(QFile::ReadOnly | QFile::Text);
        myBytes = myFile.read(451);
        client.addKeyToNetid(myBytes, netid);

        client.setRoleForNetID(role, netid);
    }

    //qDebug() << myBytes << "\n Read: " <<myBytes.length();

    //client.~LRdbClient();


}
