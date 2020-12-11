#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "lrnetclient.h"


LRNetClient::LRNetClient():
oscOutStream( buffer, OUTPUT_BUFFER_SIZE )
,m_timeoutTimer(this)
{
    socket = new QSslSocket();
    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.setInterval(5000);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &LRNetClient::connectionTimedOut);
    connect(socket, &QSslSocket::connected, this,
            [=](){
        disconnect(&m_timeoutTimer, &QTimer::timeout, this, &LRNetClient::connectionTimedOut);
        m_timeoutTimer.stop();
        m_timeoutTimer.setSingleShot(false);
        m_timeoutTimer.setInterval(2000);
        m_timeoutTimer.callOnTimeout([=](){
            char temp = 'p';
             qDebug() << "Sending ping";
            socket->write(&temp,1);

        });
        m_timeoutTimer.start();
        connect(socket, &QSslSocket::disconnected, this, [=](){
            qDebug() <<__FILE__ <<__LINE__ <<"Disconnected";
            m_timeoutTimer.stop();
        });
        waitForGreeting();
    });
    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    //connect(socket, SIGNAL(disconnected()), qApp, SLOT(quit()));

}


void LRNetClient::connectionTimedOut(){
    emit timeout();
}

void LRNetClient::tryConnect(const QString &host, int port){
    qDebug() <<"Connecting to "<<host <<"on " <<port;
    socket->connectToHost(host, port);
    m_timeoutTimer.start();
}

LRNetClient::~LRNetClient(){
    socket->disconnect();
}
    

void LRNetClient::waitForGreeting()
{   
    emit connected();

    /*
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
    */
    char tryThing[34] = "acornell1000000000000000000000000";
    socket->write(tryThing, 33);
    qDebug() <<__FILE__ <<__LINE__ <<"Connected; authenticating";
}

void LRNetClient::sendPacket(){
    char outbuf[OUTPUT_BUFFER_SIZE + sizeof(Auth::session_id_t) + 1];
    memcpy(outbuf+1, &session, sizeof(Auth::session_id_t));
    memcpy(outbuf+sizeof(Auth::session_id_t)+1, oscOutStream.Data(), oscOutStream.Size());
    outbuf[0] = 's';
    socket->write(outbuf, oscOutStream.Size() + sizeof(Auth::session_id_t) + 1);
}

void LRNetClient::requestRoster(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    sendPacket();
}

void LRNetClient::readResponse()
{
    char inbuf[1024+33];
    int bytesRead = socket->read(inbuf, 1024);
    if (inbuf[0] == 'f'){
        qDebug() <<"Login failed";
        return;
    }
    else if (inbuf[0] == 's'){
        qDebug() <<"Login success";
        memcpy(&session, inbuf+1, sizeof(Auth::session_id_t));
        qDebug() <<"Got a session " <<session;
        requestRoster();
        return;
    }
    else if (inbuf[0] == 'p'){
        qDebug() << "Pong";
        return;
    }
    osc::ReceivedPacket inPack(inbuf, bytesRead);
    osc::ReceivedBundle * inBundle = NULL;
    osc::ReceivedMessage * inMsg = NULL;
    qDebug() <<"Got communication";
    

    if (inPack.IsBundle()){
        inBundle = new osc::ReceivedBundle(inPack);
    }
    else{
        inMsg = new osc::ReceivedMessage(inPack);
        std::cout << "Got message " <<inMsg->AddressPattern() <<std::endl;
        const char * ap = inMsg->AddressPattern();
        if (std::strcmp(ap, "/push/roster") == 0){
            osc::ReceivedMessageArgumentStream args = inMsg->ArgumentStream();

            while (!args.Eos()){
                const char * memName;
                const char * memSect;
                int id;
                args >> memName; args >> memSect; args >> id;
                std::cout << "Member " <<id <<": " << memName <<"-" <<memSect << std::endl;
                emit newMember(QString(memName), QString(memSect), id);
            }
        }
        std::cout <<"Address Pattern: " <<ap << std::endl;
    }
}


