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


class LRNetServer;
class Member;



class Roster : public QObject
{
    friend class Member;
    Q_OBJECT
    QHash<Member::serial_t, Member *> members;
    QHash<session_id_t, Member *> membersBySessionID;

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


public:

    explicit Roster(LRNetServer *server = nullptr, QObject *parent = nullptr);
    ///Assumes Jack is already running. Returns nonzero if failure.
    bool initJackClient();
    ~Roster();
    void addMember(QString & netid, session_id_t s_id);
    QHash<Member::serial_t, Member *>&  getMembers(){return members;}
    QStringList & getValidSections(){return sections;}
    void removeMemberBySerialID(Member::serial_t id);
    void removeMemberBySessionID(session_id_t s_id);
    QString getNameBySessionID(session_id_t s_id);
    void setNameBySessionID(QString & name, session_id_t s_id);
    void setSectionBySessionID(QString & section, session_id_t s_id);
    void setNameBySerialID(QString & name, Member::serial_t s_id);
    void setSectionBySerialID(QString & section, Member::serial_t s_id);
    void stopAllThreads();

    void startJackTrip(session_id_t s_id);


    QMutex mMutex;
    PortPool mPortPool;
    int releaseThread(Member::serial_t id);

signals:
    void jacktripRemoveMember(session_id_t s_id);
    void sigMemberUpdate(Member * member, RosterNS::MemberEventE);
    void memberRemoved(Member::serial_t id);
    void jackTripStarted(session_id_t s_id);


};

#endif // ROSTER_H
