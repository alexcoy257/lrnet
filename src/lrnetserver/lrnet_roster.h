#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include <QHash>
#include <QThreadPool>
#include "auth_types.h"
#include "JackTripWorker.h"
#include "portpool.h"

namespace RosterNS {
enum MemberEventE{
    MEMBER_CAME,
    MEMBER_UPDATE,
};
}


#include "lrnet_member.h"
#include "lrnetserver_types.h"

using LRServer_types::sessionTriple;




class LRNetServer;
//class MemberNS::Member;



class Roster : public QObject
{
    friend class Member;
    Q_OBJECT
    QHash<Member::serial_t, Member *> members;
    QHash<session_id_t, Member *> membersBySessionID;

    QHash<Member::serial_t, Member *> chefs;
    QHash<session_id_t, Member *> chefsBySessionID;

    LRNetServer * m_server;
    jack_status_t * m_jackStatus;
    jack_client_t * m_jackClient;



    static const int gMaxThreads = 16;
    int mTotalRunningThreads; ///< Number of Threads running in the pool
    QThreadPool mThreadPool; ///< The Thread Pool

    QStringList sections = {"Piccolo",
                            "Flute",
                            "Oboe",
                            "Bassoon",
                            "Clarinet",
                            "Bass Clarinet",
                            "Horn",
                            "Trumpet",
                            "Trombone",
                            "Euphonium",
                            "Tuba",
                            "Guitar",
                            "Bass",
                            "Drum Kit",
                            "Percussion"
                            };

    void removeMember(Member * m);
    void fanNewMember(Member * m);
    void fanNewChef(Member * c);
    Member * addMemberOrChef(QString &netid,
        session_id_t s_id,
        QHash<session_id_t, Member *> & group,
        QHash<Member::serial_t, Member *> & sGroup);


public:

    explicit Roster(LRNetServer *server = nullptr, QObject *parent = nullptr);
    ///Assumes Jack is already running. Returns nonzero if failure.
    bool initJackClient();
    ~Roster();
    void addMember(QString & netid, session_id_t s_id);
    void addChef(QString & netid, session_id_t s_id);
    QHash<Member::serial_t, Member *>&  getMembers(){return members;}
    QStringList & getValidSections(){return sections;}
    void removeMemberBySerialID(Member::serial_t id);
    void removeMemberBySessionID(session_id_t s_id);
    void removeChefBySessionID(session_id_t s_id);
    QString getNameBySessionID(session_id_t s_id);
    void setNameBySessionID(QString & name, session_id_t s_id);
    void setSectionBySessionID(QString & section, session_id_t s_id);
    void setNameBySerialID(QString & name, Member::serial_t s_id);
    void setSectionBySerialID(QString & section, Member::serial_t s_id);
    void stopAllThreads();

    void setRedundancyBySessionID(int newRed, session_id_t s_id);
    void startJackTrip(session_id_t s_id, bool encrypt=false);
    void stopJackTrip(session_id_t s_id);
    void setControl(Member::serial_t id, int out, float val);
    void returnPort(int port){mPortPool.returnPort(port);}
    int getPort(){return mPortPool.getPort();}
    void setNumChannelsBySessionID(int newCh, session_id_t s_id);
    void setLoopbackBySessionID(bool lb, session_id_t s_id);

    QHash<session_id_t, sessionTriple> & getActiveSessions();


    QMutex mMutex;
    PortPool mPortPool;
    int releaseThread(Member::serial_t id);

signals:
    void jacktripRemoveMember(session_id_t s_id);
    void sigMemberUpdate(Member * member, RosterNS::MemberEventE);
    void memberRemoved(Member::serial_t id);
    void jackTripStarted(session_id_t s_id);
    void sendKeyToClient(unsigned char * key, session_id_t s_id);


};

#endif // ROSTER_H
