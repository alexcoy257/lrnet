#include "lrnet_roster.h"
#include "JackTripWorker.h"
#include <jacktrip/JackTrip.h>
#include <QDebug>
#include <sys/mman.h>

Member::serial_t Member::currentSerial=0;

Member::Member(QString & netid, session_id_t s_id, int port, Roster * roster,  QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,serial(currentSerial++)
  ,netid(netid)
  ,mPort(port)
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
                                        port,
                                        port,
                                        1,
                                        false
                                        ); /// \todo temp default to 1 channel
#endif
}

    QObject::connect(assocThread, &JackTripWorker::jackPortsReady, this, [=](){
        qDebug() << "Got jackPortsReady";
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[0]));
        jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(audio->getInputPort(0)));
        jack_connect(roster->m_jackClient, jack_port_name(audio->getOutputPort(0)), jack_port_name(toPorts[0]));
        jack_connect(roster->m_jackClient, jack_port_name(audio->getOutputPort(0)), jack_port_name(toPorts[1]));
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[1]));

    }, Qt::QueuedConnection);

}

Member::Member(QObject * parent): QObject(parent)
  ,s_id(0)
  ,serial(0)
  ,netid("")
  ,mRoster(NULL)
  ,cs(NULL)
  ,ui(NULL)
    {
}

Member::~Member(){
    if(cs){
        munlock(cs, sizeof(ChannelStrip));
    }
    audio->stop();
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
