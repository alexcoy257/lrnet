#include "lrnet_roster.h"
#include "JackTripWorker.h"
#include <jacktrip/JackTrip.h>
#include <QDebug>
#include <sys/mman.h>
#include <memory>

Member::serial_t Member::currentSerial=0;

class Member::csControlPair{
        public:
        ChannelStrip * cs = NULL;
        ControlUI * ui = NULL;
        jackaudio * audio = NULL;
        csControlPair(ChannelStrip * ncs = NULL,
        ControlUI * ncontrol = NULL,
        jackaudio * naudio = NULL):
        cs(ncs), ui(ncontrol), audio(naudio){
            mlock(cs, sizeof(ChannelStrip));
                qDebug() <<"Locked mem";

            cs->buildUserInterface(ui);
            qDebug() <<"Built UI";

        }
        ~csControlPair(){
            qDebug() <<"CS Control Pair: Destructor!";
            if (audio){
                qDebug() << "Try to stop audio";
                audio->stop();
            }
            if (cs){
                 munlock(cs, sizeof(ChannelStrip));
                 qDebug() << "Try to delete CS";
                delete cs;
            }
            if (ui){
                qDebug() << "Try to delete UI";
                delete ui;
            }
            if (audio){
                qDebug() << "Try to delete Audio";
                delete audio;
            }
            qDebug() <<"CC Control Pair Destructor End";
        }
    };

Member::Member(QString & nnetid, session_id_t s_id, Roster * roster,  QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,serial(currentSerial++)
  ,netid(nnetid)
  ,mRoster(roster)
  ,mPort(roster?roster->getPort():0)
  ,cs(NULL)
  ,ui(NULL)
  ,audio(NULL)
    {
    qDebug() <<"Member constructor " <<netid;

    //csControlPair excs = ;
    cses.push_back(
        std::unique_ptr<csControlPair>(
            new csControlPair(new ChannelStrip(), new ControlUI(), new jackaudio(false))));

    qDebug() <<"Pushed back.";
    //qDebug() <<"At 0:" <<cses.at(0)->cs <<cses.at(0)->ui <<cses.at(0)->audio;
    cses.at(0)->audio->init(netid.append("-%1").arg("CS-LM").toStdString().data(), cses.at(0)->cs);
    cses.at(0)->audio->start();
    setControl(MUTE, 1);

    //mlock(cs, sizeof(ChannelStrip));
    //cs->buildUserInterface(ui);
    
    //audio->init(netid.toStdString().data(), cs);
    //audio->start();

    assocThread = new JackTripWorker(serial, roster, 10,
        JackTrip::ZEROS,
        netid.arg("JT").toStdString().data());
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

    qDebug() << "Member destructor beginning";
    /*
    if(cs){
        munlock(cs, sizeof(ChannelStrip));
    }*/
    //audio->stop();
    assocThread->stopThread();
    mRoster->returnPort(mPort);
    //delete ui;
    //delete audio;
    //delete cs;

    qDebug() << "Member destructor end";
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
        cses.at(0)->ui->decodeControl(currentControlValues, numControlValues);
        if (cses.size() == 2){
            cses.at(1)->ui->decodeControl(currentControlValues, numControlValues);
        }
    }
}

const float * Member::getCurrentControls(){
    return currentControlValues;
}

void Member::resetThread(){
    if (!assocThread)
        return;

    assocThread = new JackTripWorker(serial, mRoster, 10, JackTrip::ZEROS,
        netid.arg("JT").toStdString().data());
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
    if (n<0) n=0;
    if (n>1) n=1;
    if (mNumChannels == 1){
        if(cses.at(0)->audio){
            return cses.at(0)->audio->getOutputPort(1);
        }
    }
    else if (mNumChannels == 2){
        if(cses.at(n)->audio){
            return cses.at(n)->audio->getOutputPort(1);
        }
    }
    //if(audio)
    //    return audio->getOutputPort(0);
    return NULL;
}

void Member::connectChannelStrip(){
        //qDebug() << "Got jackPortsReady";
        qDebug() << "Emit readyToFan";
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[0]));
        jack_connect(mRoster->m_jackClient,
            jack_port_name(fromPorts[0]),
            jack_port_name(cses.at(0)->audio->getInputPort(0)));
        if (mNumChannels == 2 && cses.at(1)->audio){
        jack_connect(mRoster->m_jackClient,
            jack_port_name(fromPorts[1]),
            jack_port_name(cses.at(1)->audio->getInputPort(0)));
        }

        
        //jack_connect(roster->m_jackClient, jack_port_name(fromPorts[0]), jack_port_name(toPorts[1]));
        
        emit readyToFan(this);
    }

void Member::setLoopback(bool lb){
    if(lb){
        jack_connect(mRoster->m_jackClient,
            jack_port_name(cses.at(0)->audio->getOutputPort(0)),
            jack_port_name(toPorts[0]));
        
        if (mNumChannels == 1){
        jack_connect(mRoster->m_jackClient,
            jack_port_name(cses.at(0)->audio->getOutputPort(0)),
            jack_port_name(toPorts[1]));
        }
        else if (mNumChannels==2){
            jack_connect(mRoster->m_jackClient,
            jack_port_name(cses.at(1)->audio->getOutputPort(0)),
            jack_port_name(toPorts[1]));
        }

    }
    else{
        jack_disconnect(mRoster->m_jackClient,
            jack_port_name(cses.at(0)->audio->getOutputPort(0)),
            jack_port_name(toPorts[0]));
        
        if (mNumChannels == 1){
        jack_disconnect(mRoster->m_jackClient,
            jack_port_name(cses.at(0)->audio->getOutputPort(0)),
            jack_port_name(toPorts[1]));
        }

        else if (mNumChannels==2){
            jack_disconnect(mRoster->m_jackClient,
            jack_port_name(cses.at(1)->audio->getOutputPort(0)),
            jack_port_name(toPorts[1]));
        }
    }
}

void Member::setNumChannels(int n){
    if (n<1) n=1;
    if (n>2) n=2;
    mNumChannels = n;
    if (cses.size() < n){
        cses.push_back(
        std::unique_ptr<csControlPair>(
            new csControlPair(new ChannelStrip(), new ControlUI(), new jackaudio(false))));

        qDebug() <<"Pushed back.";
        //qDebug() <<"At 0:" <<cses.at(0)->cs <<cses.at(0)->ui <<cses.at(0)->audio;
        cses.at(1)->audio->init(netid.arg("CS-R").toStdString().data(), cses.at(1)->cs);
        cses.at(1)->audio->start();
         setControl(MUTE, 1);
    }
}