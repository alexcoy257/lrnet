#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "lrnetclient.h"


LRNetClient::LRNetClient(RSA * k):
authKey(k)
,oscOutStream( buffer, OUTPUT_BUFFER_SIZE )
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
    char netid[30] = "ac2456";
    AuthPacket pck(netid);
    unsigned int retlen;
    RAND_bytes(pck.challenge, 214);

    //unsigned char challenge[214]; //256-42 = 214
    //unsigned char sig[256];

    RSA_sign(NID_sha256, pck.challenge, 214, pck.sig, &retlen, authKey);
    //char tryThing[34] = "acornell1000000000000000000000000";

    QByteArray batmp = QByteArray::fromRawData((const char *)pck.challenge, 214);
    qDebug() <<"Challenging with challenge " <<batmp;
    batmp = QByteArray::fromRawData((const char *)pck.sig, 214);
    qDebug() <<"Signed " <<batmp;

    if(RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, 256, authKey)){
        qDebug() <<"Verified at send";
    }else{
        qDebug() <<"Failed to verify at send";
    }
    auth_packet_t dpck;
    pck.pack(dpck);
    char buf [sizeof(auth_packet_t) + 1] = {'a'};
    std::memcpy(buf + 1, &dpck, sizeof(auth_packet_t));
    socket->write(buf, sizeof(auth_packet_t) + 1);
    qDebug() <<__FILE__ <<__LINE__ <<"Connected; authenticating";
}

void LRNetClient::sendPacket(){
    char outbuf[OUTPUT_BUFFER_SIZE + sizeof(session_id_t) + 1];
    memcpy(outbuf+1, &session, sizeof(session_id_t));
    memcpy(outbuf+sizeof(session_id_t)+1, oscOutStream.Data(), oscOutStream.Size());
    outbuf[0] = 's';
    socket->write(outbuf, oscOutStream.Size() + sizeof(session_id_t) + 1);
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
        memcpy(&session, inbuf+1, sizeof(session_id_t));
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

void LRNetClient::setRSAKey(RSA * key){
    authKey = key;
}
