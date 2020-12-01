//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file Patcher.cpp
 * \author Aaron Wyatt
 * \date September 2020
 */

#include "Patcher.h"

Patcher::Patcher()
{
    m_jackClient = jack_client_open("lrhubpatcher", JackNoStartServer, &m_status);
    if (m_jackClient == NULL) {
        qDebug() << "Unable to start patcher JACK client: Patching disabled\n";
    return;
    }
    if (jack_set_port_registration_callback(m_jackClient,
        Patcher::s_cp_patch, (void *)this))
        {
        qDebug() << "Unable to set callback\n";
    return;
    }
    if(jack_activate(m_jackClient)){
        qDebug() << "Cannot activate jack client\n";
    }

    connect(this, SIGNAL(newPort(jack_port_id_t,int)), SLOT(doNewPort(jack_port_id_t,int)), Qt::QueuedConnection);
    qDebug() << "Constructed Patcher\n";
}



void Patcher::s_cp_patch(jack_port_id_t port, int reg, void * p){
    emit((Patcher *) p)->newPort(port, reg);
    Q_UNUSED(port);
    Q_UNUSED(reg);
}

void Patcher::doNewPort(jack_port_id_t p_id, int reg){
    //qDebug() << "Called patch callback";
    QMutexLocker locker(&m_connectionMutex);


    const char **outPorts, **inPorts;
    outPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsOutput);
    inPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsInput);

    const jack_port_t * port = jack_port_by_id(m_jackClient, p_id);
    QString pName = jack_port_name(port);
    QString cName = pName.section(":", 0, 0);
    QString test = cName.section("-", 1, 1);
    if(test != NULL && reg > 0){
        if(!m_clients.contains(cName)){
            m_clients.insert(cName, new EnsMember(cName, m_jackClient));
            qDebug() << "Adding " <<cName;
        }
        EnsMember * cMember = m_clients[cName];
        cMember->regPort(port);
        fanInOut(m_clients[cName]);

    }
    else if (test != NULL){
        if(m_clients.contains(cName)){
            EnsMember * cMember = m_clients[cName];
            if(! cMember->deregPort(port)){
                m_clients.remove(cName);
                delete cMember;
                qDebug() << "Removed" <<cName;
            }
        }
        
    }

    //qDebug() << "Port for " <<pName <<(reg?" registered ":" deregistered ") <<"\n";

    /*
    for (int i = 0; outPorts[i]; i++) {
        QString clientName = QString(outPorts[i]).section(":", 0, 0);
        qDebug() << "Port for " <<clientName <<"\n";
        if(!m_clients.contains(clientName)){
            qDebug() << "Adding " <<clientName;
        }
    }
    */

   jack_free(outPorts);
   jack_free(inPorts);
}

void Patcher::fanInOut(EnsMember * mem){
    qDebug() <<"Start fanout. \n";
    for (EnsMember * client : m_clients){
        if (mem==NULL || client == NULL){
            qDebug() <<"Client or member null";
            break;
        }
        
        if (client != mem){
            qDebug() <<"Fanning " <<mem->name <<" to " << client->name <<"\n";
            const jack_port_t * opc = client->outPort(0);
            const jack_port_t * ipc = mem->inPort(0);
            if(opc && ipc){
                jack_connect(m_jackClient, jack_port_name(opc), jack_port_name(ipc));
            }
            opc = mem->outPort(0);
            ipc = client->inPort(0);
            if(opc && ipc){
                jack_connect(m_jackClient, jack_port_name(opc), jack_port_name(ipc));
            }
        }
    }
}

/*
void Patcher::registerClient(const QString &clientName)
{
    QMutexLocker locker(&m_connectionMutex);


    const char **outPorts, **inPorts;
    outPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsOutput);
    inPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsInput);
    
    //Start with our receiving ports.
    for (int i = 0; outPorts[i]; i++) {
        QString client = QString(outPorts[i]).section(":", 0, 0);
        if (client == clientName) {
            QString channel = QString(outPorts[i]).section("_", -1, -1);
            for (int j = 0; inPorts[j]; j++) {
                //First check if this is one of our other clients. (Fan out/in and full mix.)
                if (false) {
                    if (m_clients.contains(QString(inPorts[j]).section(":", 0, 0)) 
                        && QString(inPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[i], inPorts[j]);
                    }
                }
                //Then check if it's our registering client. (Client Echo and full mix.)
                if (false) {
                    if (QString(inPorts[j]).section(":", 0, 0) == clientName 
                        && QString(inPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[i], inPorts[j]);
                    }
                }
            }
        }
    }
    
    //Then our sending ports. We only need to check for other clients here.
    //(Any loopback connections will have been made in the previous loop.)
    if (false) {
        for (int i = 0; inPorts[i]; i++) {
            QString client = QString(inPorts[i]).section(":", 0, 0);
            if (client == clientName) {
                QString channel = QString(inPorts[i]).section("_", -1, -1);
                for (int j = 0; outPorts[j]; j++) {
                    if (m_clients.contains(QString(outPorts[j]).section(":", 0, 0)) 
                        && QString(outPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[j], inPorts[i]);
                    }
                }
            }
        }
    }
    
    m_clients.append(clientName);
    jack_free(outPorts);
    jack_free(inPorts);
}

void Patcher::unregisterClient(const QString &clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    m_clients.removeAll(clientName);
}
*/

Patcher::~Patcher()
{
    jack_deactivate(m_jackClient);
    jack_client_close(m_jackClient);
}
