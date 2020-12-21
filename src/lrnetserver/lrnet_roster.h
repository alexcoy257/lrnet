#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include <QHash>
#include "auth_types.h"

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
public:

    explicit Member(QObject *parent = nullptr);
    explicit Member(QString & netid, session_id_t s_id, QObject *parent = nullptr);
    session_id_t getSessionID(){return s_id;}
    uint64_t getSerialID(){return serial;}
    QString & getNetID(){return netid;}

signals:
};

class Roster : public QObject
{
    Q_OBJECT
    QHash<Member::serial_t, Member *> members;
public:
    explicit Roster(QObject *parent = nullptr);
    void addMember(QString & netid, session_id_t s_id);
    QHash<Member::serial_t, Member *>&  getMembers(){return members;}
    void removeMember(session_id_t s_id);

signals:
    void jacktripRemoveMember(session_id_t s_id);
    void memberAdded(Member * member);
    void memberRemoved(Member::serial_t id);

};

#endif // ROSTER_H
