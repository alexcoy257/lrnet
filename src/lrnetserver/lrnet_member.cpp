#include "lrnet_roster.h"
#include "JackTripWorker.h"
#include <jacktrip/JackTrip.h>
#include <QDebug>
#include <sys/mman.h>

Member::serial_t Member::currentSerial=0;

Member::Member(QString & netid, session_id_t s_id, Roster * roster,  QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,serial(currentSerial++)
  ,netid(netid)
  ,mRoster(roster)
  ,mPort(roster?roster->getPort():0)
  ,cs(new ChannelStrip())
  ,ui(new ControlUI())
  ,audio(new jackaudio(false))
    {
    qDebug() <<"Member constructor " <<netid;
    mlock(cs, sizeof(ChannelStrip));

    cs->buildUserInterface(ui);
    audio->init(netid.toStdString().data(), cs);
    audio->start();

    assocThread = new JackTripWorker(serial, roster, 10, JackTrip::ZEROS, "JackTrip");
    assocThread->setBufferStrategy(1);
    assocThread->setPortCBAreas(fromPorts, toPorts, broadcastPorts, 2);
    {
        QMutexLocker lock(&roster->mMutex);

#ifdef ROSTER_TEST_NO_SERVER
#warning "Testing roster independently of the server."
#endif
#ifndef ROSTER_TEST_NO_SERVER
        assocThread->setJackTrip(roster->getActiveSessions()[s_id].lastSeenConnection->peerAddress().toString(),
                                        mPort,
                                        mPort,
                                        1,
                                        false
                                        ); /// \todo temp default to 1 channel
#endif
}

    QObject::connect(assocThread, &JackTripWorker::jackPortsReady, this,
        &Member::connectChannelStrip, Qt::QueuedConnection);

}

Member::Member(QObject * parent): QObject(parent)
  ,s_id(0)
  ,serial(0)
  ,netid("")
  ,mRoster(NULL)
  ,cs(NULL)
  ,ui(NULL)
    {
        qDebug()<<"Member null constructor called";
}

Member::~Member(){
    if(cs){
        munlock(cs, sizeof(ChannelStrip));
    }
    audio->stop();
    assocThread->stopThread();
    delete ui;
    delete audio;
    delete cs;
}


void Member::setName(QString & nname){
    qDebug() <<"Set name " <<nname;
    name = nname;
}

void Member::setSection(QString & nsection){
    if(mRoster->sections.isEmpty() || mRoster->sections.contains(nsection))
        section = nsection;
}

void Member::setControl(int out, float val){
    if (out < numControlValues){
        currentControlValues[out] = val;
        ui->decodeControl(currentControlValues, numControlValues);
    }
}

const float * Member::getCurrentControls(){
    return currentControlValues;
}

void Member::resetThread(){
    if (!assocThread)
        return;

    assocThread = new JackTripWorker(serial, mRoster, 10, JackTrip::ZEROS, "JackTrip");
    assocThread->setBufferStrategy(1);
    assocThread->setPortCBAreas(fromPorts, toPorts, broadcastPorts, 2);
    //{
    //    QMutexLocker lock(&mRoster->mMutex);

#ifdef ROSTER_TEST_NO_SERVER
#warning "Testing roster independently of the server."
#endif
#ifndef ROSTER_TEST_NO_SERVER
        assocThread->setJackTrip(mRoster->getActiveSessions()[s_id].lastSeenConnection->peerAddress().toString(),
                                        mPort,
                                        mPort,
                                        1,
                                        false
                                        ); /// \todo temp default to 1 channel
#endif
//}

    QObject::connect(assocThread, &JackTripWorker::jackPortsReady, this,
    &Member::connectChannelStrip, Qt::QueuedConnection);

}

/**
 *  Returns the port that goes to the network.
 */
audioPortHandle_t Member::getAudioInputPort(int n){
    n = n<0?0:n>1?1:n;
    return toPorts[n];
}

/**
 *  Disregards n and returns the port that comes from the channel strip.
 */
audioPortHandle_t Member::getAudioOutputPort(int n){
    if(audio)
        return audio->getOutputPort(0);
    return NULL;
}

void Member::connectChannelStrip(){
        //qDebug() << "Got jackPortsReady";
        qDebug() << "Emit readyToFan";
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[0]));
        jack_connect(mRoster->m_jackClient,
            jack_port_name(fromPorts[0]),
            jack_port_name(audio->getInputPort(0)));
        jack_connect(mRoster->m_jackClient,
            jack_port_name(audio->getOutputPort(0)),
            jack_port_name(toPorts[0]));
        jack_connect(mRoster->m_jackClient,
            jack_port_name(audio->getOutputPort(0)),
            jack_port_name(toPorts[1]));
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[1]));
        
        emit readyToFan(this);
    }