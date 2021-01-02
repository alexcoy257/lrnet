#include "lrnet_roster.h"
#include "lrnetserver.h"
#include <QDebug>
#include <jack/jack.h>

//Q_DECLARE_OPAQUE_POINTER(audioPortHandle_t);

Roster::Roster(LRNetServer * server, QObject *parent) : QObject(parent)
  , m_server(server)
  , m_jackStatus(NULL)
  , m_jackClient(NULL)
  //, mPortPool()
{

    //qRegisterMetaType<audioPortHandle_t>("audioPortHandle_t");
    //qRegisterMetaType<QVarLengthArray<audioPortHandle_t>>();//"QVarLengthArray<audioPortHandle_t>");

    qDebug() << "mThreadPool default maxThreadCount =" << mThreadPool.maxThreadCount();
    mThreadPool.setMaxThreadCount(mThreadPool.maxThreadCount() * 16);
    qDebug() << "mThreadPool maxThreadCount set to" << mThreadPool.maxThreadCount();


    //mJTWorkers = new JackTripWorker(this);
    mThreadPool.setExpiryTimeout(3000); // msec (-1) = forever
}

bool Roster::initJackClient(){
    m_jackClient = jack_client_open("lrhubpatcher", JackNoStartServer, m_jackStatus);
    if (!m_jackClient){
        qDebug() <<"Couldn't make hub patcher client";
        return 1;
    }
    if(jack_activate(m_jackClient)){
        qDebug() << "Cannot activate jack client\n";
        return 2;
    }
    qDebug() << "Activated hub patcher client";
    return 0;
}

Roster::~Roster(){
    QMutexLocker lock(&mMutex);
    mThreadPool.waitForDone();
}

void Roster::addMember(QString &netid, session_id_t s_id){
    //Must have some sort of ID to be a member.

    if (netid.length()==0)
        return;

    //Can't log in twice on the same session
    if (membersBySessionID.contains(s_id))
        return;

    Member * newMem = new Member(netid, s_id, mPortPool.getPort(), this);

    members[newMem->getSerialID()]=newMem;
    membersBySessionID[s_id]=newMem;




    //newMem->setPort(61002);
    qDebug() <<"Assigned UDP Port:" << newMem->getPort();

    qDebug() <<"Member's port is now " << newMem->getPort();



     qDebug() <<"New member " <<newMem->getNetID();
    emit sigMemberUpdate(newMem, RosterNS::MEMBER_CAME);
}

QHash<session_id_t, sessionTriple> & Roster::getActiveSessions(){
        return m_server->getActiveSessions();
    }

void Roster::startJackTrip(session_id_t s_id){
    JackTripWorker * w = membersBySessionID[s_id]->getThread();
    if (!w){
        qDebug() <<"JackTripWorker not set";
        return;
    }
    mThreadPool.start(w, QThread::TimeCriticalPriority);
    // wait until one is complete before another spawns
    emit jackTripStarted(s_id);

    //while (w->isSpawning()) { QThread::msleep(10);
    ///*qDebug() << "Loop wait for spawning;";*/}

     //QThread::msleep(100);
    //qDebug() << "mPeerAddress" << id <<  mActiveAddress[id].address << mActiveAddress[id].port;
}

/**
 * @brief Roster::removeMember
 * @param id
 * Remove a member logged in with serial ID id.
 */

void Roster::removeMemberBySerialID(Member::serial_t id){
    Member * m = members.take(id);
    if (m){
    membersBySessionID.take(m->getSessionID());
    removeMember(m);
    }
}

void Roster::removeMemberBySessionID(session_id_t s_id){
    qDebug() <<"Remove a member by session id";
    Member * m = membersBySessionID.take(s_id);
    if(m){
    members.take(m->getSerialID());
    removeMember(m);
    }
}

/**
 * @brief Roster::removeMember
 * @param m
 * m should not be null! This function is only available privately.
 */

void Roster::removeMember(Member * m){
    Member::serial_t id = m->getSerialID();
    mPortPool.returnPort(m->getPort());
    delete m;
    emit memberRemoved(id);
}

QString Roster::getNameBySessionID(session_id_t s_id){
    if (!membersBySessionID.contains(s_id)){
            return QString("Chef");
    } else{
    QString qsName = membersBySessionID[s_id]->getName();
    if (qsName.isEmpty()){
        qsName = membersBySessionID[s_id]->getNetID();
    }
    return qsName;
    }
}
void Roster::setNameBySessionID(QString &name, session_id_t s_id){
    membersBySessionID[s_id]->setName(name);
    emit sigMemberUpdate(membersBySessionID[s_id], RosterNS::MEMBER_UPDATE);
}
void Roster::setSectionBySessionID(QString & section, session_id_t s_id){
    membersBySessionID[s_id]->setSection(section);
    emit sigMemberUpdate(membersBySessionID[s_id], RosterNS::MEMBER_UPDATE);
}
void Roster::setNameBySerialID(QString & name, Member::serial_t s_id){
    members[s_id]->setName(name);
    emit sigMemberUpdate(members[s_id], RosterNS::MEMBER_UPDATE);
}
void Roster::setSectionBySerialID(QString & section, Member::serial_t s_id){
    members[s_id]->setSection(section);
    emit sigMemberUpdate(members[s_id], RosterNS::MEMBER_UPDATE);
}

int Roster::releaseThread(Member::serial_t id)
{   std::cout << "Releasing Thread" << std::endl;
    QMutexLocker lock(&mMutex);
    if (members[id]){
    members[id]->setThread(NULL);
    mPortPool.returnPort(members[id]->getPort());
    mTotalRunningThreads--;
    }

    return 0; /// \todo Check if we really need to return an argument here
}

void Roster::stopAllThreads()
{
    QHashIterator<Member::serial_t, Member *> iterator(members);
    while (iterator.hasNext()) {
        if (iterator.value()->getThread() != nullptr) {
            iterator.value()->getThread()->stopThread();
        }
        iterator.next();
    }
    mThreadPool.waitForDone();
}

void Roster::setControl(Member::serial_t id, int out, float val){
   Member * m =  members[id];
   if (m) m->setControl(out, val);
}
