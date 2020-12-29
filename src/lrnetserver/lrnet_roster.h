#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include <QHash>
#include <QThreadPool>
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
    JackTripWorker * getThread(){return assocThread;}

signals:
};

class Roster : public QObject
{
    friend class Member;
    Q_OBJECT
    QHash<Member::serial_t, Member *> members;
    QHash<session_id_t, Member *> membersBySessionID;


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


public:
    enum MemberEventE{
        MEMBER_CAME,
        MEMBER_UPDATE,
    };
    explicit Roster(QObject *parent = nullptr);
    ~Roster();
    void addMember(QString & netid, session_id_t s_id);
    QHash<Member::serial_t, Member *>&  getMembers(){return members;}
    QStringList & getValidSections(){return sections;}
    void removeMemberBySerialID(Member::serial_t id);
    void removeMemberBySessionID(session_id_t s_id);
    void setNameBySessionID(QString & name, session_id_t s_id);
    void setSectionBySessionID(QString & section, session_id_t s_id);
    void setNameBySerialID(QString & name, Member::serial_t s_id);
    void setSectionBySerialID(QString & section, Member::serial_t s_id);
    void stopAllThreads();


    QMutex mMutex;
    int releaseThread(Member::serial_t id);

signals:
    void jacktripRemoveMember(session_id_t s_id);
    void sigMemberUpdate(Member * member, MemberEventE);
    void memberRemoved(Member::serial_t id);

};

#endif // ROSTER_H
