#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "lrnetclient.h"


LRNetClient::LRNetClient(RSA * k):
authKey(k)
,oscOutStream( oBuffer, OUTPUT_BUFFER_SIZE )
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
        connect(socket, &QSslSocket::disconnected, this, [=](){
            qDebug() <<__FILE__ <<__LINE__ <<"Disconnected";
            m_timeoutTimer.stop();
        });
        startHandshake();
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
    

void LRNetClient::startHandshake()
{   
    emit connected();

    char netid[30] = "ac2456";


    AuthPacket pck(netid);
    unsigned int retlen;
    RAND_bytes(pck.challenge, 214);

    RSA_sign(NID_sha256, pck.challenge, 214, pck.sig, &retlen, authKey);

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

    sendAuthPacket(dpck);

    qDebug() <<__FILE__ <<__LINE__ <<"Connected; authenticating";
}


void LRNetClient::sendAuthPacket(auth_packet_t & pck){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/new" );

    osc::Blob b(reinterpret_cast<const char*>(const_cast<const auth_packet_t *>(&pck)), sizeof(auth_packet_t));
    oscOutStream << b << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendPacket(){
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::requestRoster(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::readResponse()
{

    int bytesRead = socket->read(buffer.head(), buffer.remaining());
    buffer.update(bytesRead);

    osc::ReceivedPacket * inPack = NULL;
    try{
    inPack = new osc::ReceivedPacket(buffer.base(), buffer.filled());
    }
    catch(osc::MalformedPacketException e){
        qDebug() << "Malformed Packet";
        return;
    }

    if (!inPack)
        return;


    osc::ReceivedBundle * inBundle = NULL;
    osc::ReceivedMessage * inMsg = NULL;
    qDebug() <<"Got communication";
    

    if (inPack->IsBundle()){
        try{
            inBundle = new osc::ReceivedBundle(*inPack);
        }
        catch(osc::MalformedBundleException e){
            qDebug() <<"Malformed Bundle " <<e.what();
        }
    }
    else{
        try{
            inMsg = new osc::ReceivedMessage(*inPack);
            handleMessage(inMsg);
            buffer.reset();
        }
        catch(osc::MalformedMessageException e){
            qDebug() <<"Malformed Message " <<e.what();
        }
    }
    delete inBundle;
    delete inMsg;
    delete inPack;
}

void LRNetClient::handleMessage(osc::ReceivedMessage * inMsg){
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


    if (std::strcmp(ap, "/auth/success") == 0){
        osc::ReceivedMessageArgumentStream args = inMsg->ArgumentStream();

            const session_id_t *tSess = NULL;
            osc::Blob tSess_b;
            try{
            if (!args.Eos()){
                args >> tSess_b;
            }}
            catch (osc::WrongArgumentTypeException e){
                qDebug() << "Not a blob type";
                return;
            }
            if (tSess_b.size != sizeof(session_id_t))
                return;
            tSess = reinterpret_cast<const session_id_t *>(tSess_b.data);
            session = *tSess;
            m_timeoutTimer.setSingleShot(false);
            m_timeoutTimer.setInterval(2000);
            m_timeoutTimer.callOnTimeout([=](){sendPing();});
            m_timeoutTimer.start();
    }

}

void LRNetClient::sendPing(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/ping" )
            << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::setRSAKey(RSA * key){
    authKey = key;
}
