#include "lrnet_roster.h"
#include "lrnetserver.h"
#include <QDebug>
#include <jack/jack.h>
#include <openssl/rand.h>
#include <QDateTime>
#include <QDir>

QString findLogFN(){
  if (!QDir(QDir::homePath().append("/.lrlogs")).exists()){
    QDir(QDir::homePath()).mkdir(".lrlogs");
    qDebug() << "Made Log Directory";
  }
  QString toRet = QDir::homePath().append("/.lrlogs/")
  .append(QDateTime::currentDateTime().toString("yyyyMMdd-hhmm"));
  //QSharedPointer<QString> toRet = QSharedPointer<QString>(new QString("No"));
  return toRet;
}


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
    m_iostatOutStream = new std::ofstream();
    m_iostatOutStream->open(findLogFN().toStdString());
}
/**
 * Try to make the Roster's JACK client. Return 0 if successful, 1
 * if the JACK client creation failed, and 2 if the JACK
 * client activation failed.
 */
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
    qDebug() <<"addMember called with" <<membersBySessionID.size();
    Member * newMem = addMemberOrChef(netid, s_id, membersBySessionID, members);
    if (!newMem)
        return;

    //newMem->setPort(61002);
    qDebug() <<"New member " <<newMem->getNetID() <<"Assigned UDP Port:" << newMem->getPort();

    QObject::connect(newMem, &Member::readyToFan,
        this, &Roster::fanNewMember);
    emit sigMemberUpdate(newMem, RosterNS::MEMBER_CAME);
}

void Roster::addChef(QString &netid, session_id_t s_id){
    qDebug() <<"addChef called";
    
    Member * newMem = addMemberOrChef(netid, s_id, chefsBySessionID, chefs);
    if (!newMem)
        return;

     qDebug() <<"New chef " <<newMem->getNetID() <<"Assigned UDP Port:" << newMem->getPort();
    QObject::connect(newMem, &Member::readyToFan,
        this, &Roster::fanNewChef);
    emit sigNewChef(newMem, RosterNS::CHEF_CAME);
    //emit sigMemberUpdate(newMem, RosterNS::MEMBER_CAME);
}

Member * Roster::addMemberOrChef(QString &netid,
        session_id_t s_id,
        QHash<session_id_t, Member *> & group,
        QHash<Member::serial_t, Member *> & sGroup){
            //Must have some sort of ID to be a member.
    if (netid.length()==0)
        return NULL;
    //Can't log in twice on the same session
    if (group.contains(s_id)){
        qDebug() << "Can't log in twice";
        return NULL;
    }
    Member * newMem = new Member(netid, s_id, this);
    newMem->setControl(Member::MUTE, (float)mJoinMuted);
    group[s_id]=newMem;
    sGroup[newMem->getSerialID()]=newMem;    
    return newMem;
}


QHash<session_id_t, sessionTriple> & Roster::getActiveSessions(){
        return m_server->getActiveSessions();
    }

void Roster::startJackTrip(session_id_t s_id, bool encrypt, bool hint_member){
    Member * m = NULL;
     if (membersBySessionID.contains(s_id)){
        m = membersBySessionID[s_id];
    }
    else if (!hint_member && chefsBySessionID.contains(s_id))
        m = chefsBySessionID[s_id];
    

    if (!m)
        return;

    JackTripWorker * w = m->getThread();
    
    if (!w){
        qDebug() <<"JackTripWorker not set";
        return;
    }

    if (encrypt){
        qDebug() << "todo: enable encryption stuff here";
        
        unsigned char * initKey = new unsigned char[32];
        
        if (initKey){/*
            QObject::connect(w, &JackTripWorker::canFreePrimedKey, this, [=](){
            qDebug() <<"Freeing init key";
            free(initKey);
            });*/
            RAND_bytes(initKey, 32);
            //w->primeEncryptionKey(initKey);
            w->setEncryptionKey(initKey);
            w->enableEncryption();
            emit sendKeyToClient(initKey, s_id);
        }
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

/**
 * @brief Roster::removeMember
 * @param id
 * Remove a member logged in with Session ID s_id.
 */

void Roster::removeMemberBySessionID(session_id_t s_id){
    qDebug() <<"Remove a member by session id";
    if(!membersBySessionID.contains(s_id))
        return;
    Member * m = membersBySessionID.take(s_id);
    if(m){
        {
            QHashIterator<Member::serial_t, Member *> iterator(members);
        
        while (iterator.hasNext()) {
        iterator.next();
        qDebug() <<"Member " <<iterator.value();
         }
        }
    members.take(m->getSerialID());

    
    {
            QHashIterator<Member::serial_t, Member *> iterator(members);
        
        while (iterator.hasNext()) {
        iterator.next();
        qDebug() <<"Member " <<iterator.value();
         }
        }
    qDebug() <<members.size() <<" member(s) remain...";

    removeMember(m);
    }
}

void Roster::removeChefBySessionID(session_id_t s_id){
    qDebug() <<"Remove a chef by session id";
    if(!chefsBySessionID.contains(s_id))
        return;
    Member * m = chefsBySessionID.take(s_id);
    if (m){
    chefs.take(m->getSerialID());
    delete m;
    }
}

/**
 * @brief Roster::removeMember
 * @param m
 * m should not be null! This function is only available privately.
 */

void Roster::removeMember(Member * m){
    Member::serial_t id = m->getSerialID();
    m->deleteLater();
    qDebug() <<"Deleted member successfully";
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
    Member * m = NULL;
    if (membersBySessionID.contains(s_id))
        m = membersBySessionID[s_id];
    else if (chefsBySessionID.contains(s_id)){
        m = chefsBySessionID[s_id];
    }
    m->setName(name);
    emit sigMemberUpdate(m, RosterNS::MEMBER_UPDATE);
}
void Roster::setSectionBySessionID(QString & section, session_id_t s_id){
    Member * m = NULL;
    if (membersBySessionID.contains(s_id))
        m = membersBySessionID[s_id];
    else if (chefsBySessionID.contains(s_id)){
        m = chefsBySessionID[s_id];
    }
    m->setSection(section);
    emit sigMemberUpdate(m, RosterNS::MEMBER_UPDATE);
}
void Roster::setNameBySerialID(QString & name, Member::serial_t s_id){
    if (members.contains(s_id)){
        members[s_id]->setName(name);
        emit sigMemberUpdate(members[s_id], RosterNS::MEMBER_UPDATE);
    }
}
void Roster::setSectionBySerialID(QString & section, Member::serial_t s_id){
    if (members.contains(s_id)){
        members[s_id]->setSection(section);
        emit sigMemberUpdate(members[s_id], RosterNS::MEMBER_UPDATE);
    }
}

void Roster::setJoinMuted(bool joinMuted){
    mJoinMuted = joinMuted;
}

int Roster::releaseThread(Member::serial_t id)
{   std::cout << "Releasing Thread" << std::endl;
    QMutexLocker lock(&mMutex);
    if (members.contains(id)){
    //members[id]->setThread(NULL);
    members[id]->resetThread();
    //mPortPool.returnPort(members[id]->getPort());
    mTotalRunningThreads--;
    }

    if (chefs.contains(id)){
    //members[id]->setThread(NULL);
    chefs[id]->resetThread();
    //mPortPool.returnPort(members[id]->getPort());
    mTotalRunningThreads--;
    }

    return 0; /// \todo Check if we really need to return an argument here
}

void Roster::stopAllThreads()
{   {
    QHashIterator<Member::serial_t, Member *> iterator(members);
    qDebug() <<members.size() <<"members to stop.";
    while (iterator.hasNext()) {
        iterator.next();
        qDebug() <<iterator.value();
        
        if (iterator.value()->getThread() != nullptr) {
            iterator.value()->getThread()->stopThread();
        }
    }
}
{
    QHashIterator<session_id_t, Member *> iterator(chefsBySessionID);
    qDebug() <<chefsBySessionID.size() <<"chefs to stop.";
    while (iterator.hasNext()) {
        iterator.next();
        qDebug() <<iterator.value();
        
        if (iterator.value()->getThread() != nullptr) {
            iterator.value()->getThread()->stopThread();
        }
    }
}
    mThreadPool.waitForDone();
}

void Roster::setControl(Member::serial_t id, int out, float val){
    if (members.contains(id)){
        Member * m =  members[id];
        qDebug() << __FUNCTION__ << "to " << val << " for field " << out;
        if (m) m->setControl(out, val);
    }
}

void Roster::stopJackTrip(session_id_t s_id, bool hint_member){
    Member * m = NULL;
    if (hint_member ){
        m = membersBySessionID.value(s_id);
    }
    if (!m)
        m = chefsBySessionID.value(s_id);
    if (!m)
        return;
    qDebug() <<"Stop jacktrip member" <<m->getName();
    m->getThread()->stopThread();
    m->resetThread();
}

void Roster::fanNewMember(Member * member){
    qDebug() <<"Fanning new member"
    <<member->getName();

    fanMemberToGroup(member, members);
    fanMemberToGroup(member, chefs);

;    
}

void Roster::fanMemberToGroup(Member * member, QHash<Member::serial_t, Member *>& group){
    for (Member * m:group){
        qDebug() << "Looking at" <<m->getName();
        //Quick fix: lone members don't have JackTrips, don't try to fan out
        //to them. Other ports end up connected instead.
        if(!m->getAudioInputPort(0)){
            qDebug() <<"Member has no JackTrip port yet";
        continue;
        }
        if(m != member){
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(0)),
                jack_port_name(member->getAudioInputPort(0)));
            qDebug()<< "Connect other "
                <<jack_port_name(m->getAudioOutputPort(0))
                <<" to new member (R)"
                <<jack_port_name(member->getAudioInputPort(0))
                ;
            
            if (m->getNumChannels()==2){
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(1)),
                jack_port_name(member->getAudioInputPort(1)));
            }
            else{
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(0)),
                jack_port_name(member->getAudioInputPort(1)));
                qDebug()<< "Connect other (LM) "
                <<jack_port_name(m->getAudioOutputPort(0))
                <<" to new member (R)"
                <<jack_port_name(member->getAudioInputPort(1))
                ;
            }

            jack_connect(m_jackClient,
                jack_port_name(member->getAudioOutputPort(0)),
                jack_port_name(m->getAudioInputPort(0)));

                qDebug()<< "Connect new member (LM) "
                <<jack_port_name(member->getAudioOutputPort(0))
                <<" to other (L) "
                <<jack_port_name(m->getAudioInputPort(0))
                ;

            if (member->getNumChannels()==2){
            jack_connect(m_jackClient,
                jack_port_name(member->getAudioOutputPort(1)),
                jack_port_name(m->getAudioInputPort(1)));
            }
            else{

            //Fan 1 channel to member's R
            jack_connect(m_jackClient,
                jack_port_name(member->getAudioOutputPort(0)),
                jack_port_name(m->getAudioInputPort(1)));

                qDebug()<< "Connect new member (LM) "
                <<jack_port_name(member->getAudioOutputPort(0))
                <<" to other (R) "
                <<jack_port_name(m->getAudioInputPort(1))
                ;
            }
        }
    }
}

void Roster::fanChefToGroup(Member * chef, QHash<Member::serial_t, Member *>& group){
    for (Member * m:group){
        qDebug() << "Looking at" <<m->getName();
        //Quick fix: lone members don't have JackTrips, don't try to fan out
        //to them. Other ports end up connected instead.
        if(!m->getAudioInputPort(0)){
            qDebug() <<"Member has no JackTrip port yet";
        continue;
        }
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(0)),
                jack_port_name(chef->getAudioInputPort(0)));
            qDebug()<< "Connect other "
                <<jack_port_name(m->getAudioOutputPort(0))
                <<" to new member (R)"
                <<jack_port_name(chef->getAudioInputPort(0))
                ;
            
            if (m->getNumChannels()==2){
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(1)),
                jack_port_name(chef->getAudioInputPort(1)));
            }
            else{
            jack_connect(m_jackClient,
                jack_port_name(m->getAudioOutputPort(0)),
                jack_port_name(chef->getAudioInputPort(1)));
                qDebug()<< "Connect other (LM) "
                <<jack_port_name(m->getAudioOutputPort(0))
                <<" to new member (R)"
                <<jack_port_name(chef->getAudioInputPort(1))
                ;
            }

            jack_connect(m_jackClient,
                jack_port_name(chef->getAudioOutputPort(0)),
                jack_port_name(m->getAudioInputPort(0)));

                qDebug()<< "Connect new member (LM) "
                <<jack_port_name(chef->getAudioOutputPort(0))
                <<" to other (L) "
                <<jack_port_name(m->getAudioInputPort(0))
                ;

            {

            //Fan 1 channel to member's R
            jack_connect(m_jackClient,
                jack_port_name(chef->getAudioOutputPort(0)),
                jack_port_name(m->getAudioInputPort(1)));

                qDebug()<< "Connect new member (LM) "
                <<jack_port_name(chef->getAudioOutputPort(0))
                <<" to other (R) "
                <<jack_port_name(m->getAudioInputPort(1))
                ;
            }
        
    }

    }

void Roster::fanNewChef(Member * chef){
   qDebug() <<"Fanning new chef"
    <<chef->getName();
    fanChefToGroup(chef, members);
    fanChefToGroup(chef, chefs);
}

void Roster::setRedundancyBySessionID(int newRed, session_id_t s_id){
    Member * m = membersBySessionID[s_id];
    if (!m)
        return;
    JackTripWorker * t = m->getThread();
    if (t)
        t->setRedundancy(newRed);
}

void Roster::setNumChannelsBySessionID(int newCh, session_id_t s_id){
    Member * m = membersBySessionID[s_id];
    if (!m)
        return;
    m->setNumChannels(newCh);
}

void Roster::setLoopbackBySessionID(bool lb, session_id_t s_id){
    Member * m = membersBySessionID[s_id];
    if (!m){
        m = chefsBySessionID[s_id];
    }
    if (!m)
        return;
    m->setLoopback(lb);
}
