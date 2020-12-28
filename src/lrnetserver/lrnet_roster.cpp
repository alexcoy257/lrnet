#include "lrnet_roster.h"
#include <QDebug>

Roster::Roster(QObject *parent) : QObject(parent)
{
}

void Roster::addMember(QString &netid, session_id_t s_id){
    //Must have some sort of ID to be a member.

    if (netid.length()==0)
        return;
    Member * newMem = new Member(netid, s_id, this);
    members[newMem->getSerialID()]=newMem;
    membersBySessionID[s_id]=newMem;
     qDebug() <<"New member " <<newMem->getNetID();
    emit sigMemberUpdate(newMem, MEMBER_CAME);
}

/**
 * @brief Roster::removeMember
 * @param id
 * Remove a member logged in with serial ID id.
 */

void Roster::removeMember(Member::serial_t id){
    Member * m = members.take(id);
    if (m){
    membersBySessionID.take(m->getSessionID());
    delete m;
    emit memberRemoved(id);
    }
}

void Roster::removeMember(session_id_t s_id){
    qDebug() <<"Remove a member by session id";
    Member * m = membersBySessionID.take(s_id);
    if(m){
    emit memberRemoved((Member::serial_t)m->getSerialID());
    members.take(m->getSerialID());
    delete m;
    }
}

void Roster::setNameBySessionID(QString &name, session_id_t s_id){
    membersBySessionID[s_id]->setName(name);
    emit sigMemberUpdate(membersBySessionID[s_id], MEMBER_UPDATE);
}
void Roster::setSectionBySessionID(QString & section, session_id_t s_id){
    membersBySessionID[s_id]->setSection(section);
    emit sigMemberUpdate(membersBySessionID[s_id], MEMBER_UPDATE);
}
void Roster::setNameBySerialID(QString & name, Member::serial_t s_id){
    members[s_id]->setName(name);
    emit sigMemberUpdate(members[s_id], MEMBER_UPDATE);
}
void Roster::setSectionBySerialID(QString & section, Member::serial_t s_id){
    members[s_id]->setSection(section);
    emit sigMemberUpdate(members[s_id], MEMBER_UPDATE);
}
