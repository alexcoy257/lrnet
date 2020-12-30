#ifndef LRNET_MEMBER_H
#define LRNET_MEMBER_H
#include <QObject>
#include "auth_types.h"
#include "JackTripWorker.h"

class Roster;

class Member: public QObject{
    Q_OBJECT
public:
    typedef quint64 serial_t;
private:
    session_id_t s_id;
    static serial_t currentSerial;
    serial_t serial;
    QString netid;
    QString name;
    QString section;
    Roster * mRoster;
    JackTripWorker * assocThread = NULL;
    int mPort;

public:

    explicit Member(QObject *parent = nullptr);
    explicit Member(QString & netid, session_id_t s_id, Roster * roster, QObject *parent = nullptr);
    session_id_t getSessionID(){return s_id;}
    uint64_t getSerialID(){return serial;}
    QString & getNetID(){return netid;}
    QString & getName(){return name;}
    QString & getSection(){return section;}
    void setName(QString & nname);
    void setSection(QString & nsection);
    void setThread(JackTripWorker * thread){assocThread=thread;}
    void setPort(int port){mPort = port;}
    int getPort(){return mPort;}
    JackTripWorker * getThread(){return assocThread;}

signals:
};
#endif // LRNET_MEMBER_H
