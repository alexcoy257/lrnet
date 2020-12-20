#include "lrnet_roster.h"

Roster::Roster(QObject *parent) : QObject(parent)
{

}

void Roster::addMember(QString &netid, session_id_t s_id){
    members.append(new Member(netid, s_id));
}

void Roster::removeMember(session_id_t s_id){
    QMutableVectorIterator<Member *> i(members);
    while (i.next()){
        if (i.value()->getSessionID() == s_id){
            i.remove();
        }
    }
}
