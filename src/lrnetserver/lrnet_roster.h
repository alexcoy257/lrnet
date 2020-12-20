#ifndef ROSTER_H
#define ROSTER_H

#include <QObject>
#include "auth_types.h"

class Member: public QObject{
    Q_OBJECT
    session_id_t s_id;
    QString netid;
    QString name;
    QString section;
public:
    explicit Member(QString & netid, session_id_t s_id, QObject *parent = nullptr);
    session_id_t getSessionID(){return s_id;}
    QString & getNetID(){return netid;}

signals:
};

class Roster : public QObject
{
    Q_OBJECT
    QVector<Member *> members;
public:
    explicit Roster(QObject *parent = nullptr);
    void addMember(QString & netid, session_id_t s_id);
    QVector<Member *> getMembers(){return members;}
    void removeMember(session_id_t s_id);

signals:

};

#endif // ROSTER_H
