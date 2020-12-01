#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "lrnetclient.h"


Client::Client(const QString &host, int port):
oscOutStream( buffer, OUTPUT_BUFFER_SIZE )
{
    socket = new QSslSocket();
    connect(socket, SIGNAL(connected()), SLOT(waitForGreeting()));
    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    connect(socket, SIGNAL(disconnected()), qApp, SLOT(quit()));
    socket->connectToHost(host, port);
}

Client::~Client(){
    socket->disconnect();
}
    

void Client::waitForGreeting()
{   
    QString temp = "Hi";

    /*
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
    */
    char tryThing[34] = "acornell1000000000000000000000000";
    socket->write(tryThing, 33);
    qDebug("Connected; now waiting for the greeting");
}

void Client::sendPacket(){
    char outbuf[OUTPUT_BUFFER_SIZE + sizeof(Auth::session_id_t) + 1];
    memcpy(outbuf+1, &session, sizeof(Auth::session_id_t));
    memcpy(outbuf+sizeof(Auth::session_id_t)+1, oscOutStream.Data(), oscOutStream.Size());
    outbuf[0] = 's';
    socket->write(outbuf, oscOutStream.Size() + sizeof(Auth::session_id_t) + 1);
}

void Client::requestRoster(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage;
    sendPacket();
}

void Client::readResponse()
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
    osc::ReceivedPacket inPack(inbuf, bytesRead);
    osc::ReceivedBundle * inBundle = NULL;
    osc::ReceivedMessage * inMsg = NULL;
    qDebug() <<"Got communication";
    std::cout << "Got communication " << std::endl;
    

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
                args >> memName;
                std::cout << "Member " << memName << std::endl;
            }
            
        }
        std::cout <<"Address Pattern: " <<ap << endl;
    }
}


