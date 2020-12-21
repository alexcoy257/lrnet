#include "lrnet_roster.h"

Roster::Roster(QObject *parent) : QObject(parent)
{
}

void Roster::addMember(QString &netid, session_id_t s_id){
    //Must have some sort of ID to be a member.
    if (netid.length()==0)
        return;
    Member * newMem = new Member(netid, s_id);
    members[newMem->getSerialID()]=newMem;
    emit memberAdded(newMem);
}

/**
 * @brief Roster::removeMember
 * @param id
 * Remove a member logged in with serial ID id.
 */

void Roster::removeMember(Member::serial_t id){
    Member * m = members.take(id);
    delete m;
    emit memberRemoved(id);
}
