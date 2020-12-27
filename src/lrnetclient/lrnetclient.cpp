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

    tryToAuthenticate();




    qDebug() <<__FILE__ <<__LINE__ <<"Connected; authenticating";
}

void LRNetClient::tryToAuthenticate(){
    if (authMethod == KEY){
        AuthPacket pck(netid);
        //qDebug() <<"Netid is: " <<netid;
        unsigned int retlen;
        RAND_bytes(pck.challenge, 214);

        RSA_sign(NID_sha256, pck.challenge, 214, pck.sig, &retlen, authKey);

        //QByteArray batmp = QByteArray::fromRawData((const char *)pck.challenge, 214);
        //qDebug() <<"Challenging with challenge " <<batmp;
        //batmp = QByteArray::fromRawData((const char *)pck.sig, 214);
        //qDebug() <<"Signed " <<batmp;

        if(RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, 256, authKey)){
            qDebug() <<"Verified at send";
        }else{
            qDebug() <<"Failed to verify at send";
        }

        auth_packet_t dpck;
        pck.pack(dpck);

        sendKeyAuthPacket(dpck);
    }
    else if (authMethod==CODE){
        sendCodeAuthPacket();
    }
}


void LRNetClient::sendKeyAuthPacket(auth_packet_t & pck){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/newbykey" );

    osc::Blob b(reinterpret_cast<const char*>(const_cast<const auth_packet_t *>(&pck)), sizeof(auth_packet_t));
    oscOutStream << b << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendCodeAuthPacket(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/newbycode" );
    std::string code = tempCode.toStdString();
    oscOutStream << code.data() << osc::EndMessage;
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
    catch(const osc::MalformedPacketException & e){
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
        catch(const osc::MalformedBundleException & e){
            qDebug() <<"Malformed Bundle " <<e.what();
        }
    }
    else{
        try{
            inMsg = new osc::ReceivedMessage(*inPack);
            handleMessage(inMsg);
            buffer.reset();
        }
        catch(const osc::MalformedMessageException & e){
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


    else if (std::strcmp(ap, "/auth/success") == 0){
        osc::ReceivedMessageArgumentStream args = inMsg->ArgumentStream();

            const auth_type_t *t_at = NULL;
            osc::Blob t_at_b;
            try{
            if (!args.Eos()){
                args >> t_at_b;
            }
            }
            catch (const osc::WrongArgumentTypeException & e){
                qDebug() << "Auth success: Bad return type";
                return;
            }
            if (t_at_b.size != sizeof(auth_type_t))
                return;
            t_at = reinterpret_cast<const auth_type_t *>(t_at_b.data);
            session = t_at->session_id;
            authType = t_at->authType;

            emit authenticated(authType);

            m_timeoutTimer.setSingleShot(false);
            m_timeoutTimer.setInterval(2000);
            m_timeoutTimer.callOnTimeout([=](){sendPing();});
            m_timeoutTimer.start();
    }

     else if (std::strcmp(ap, "/auth/fail") == 0){
            emit authFailed();
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

void LRNetClient::sendSmallMessage(QString & handle){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( handle.toStdString().data() )
    << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::subSuperchef(){
    QString handle = "/sub/superchef";
    sendSmallMessage(handle);
}

void LRNetClient::subChef(){
    QString handle = "/sub/chef";
    sendSmallMessage(handle);
}

void LRNetClient::subMember(){
    QString handle = "/sub/member";
    sendSmallMessage(handle);
}

void LRNetClient::setNetid(const QString & nnetid){
    QByteArray temp = nnetid.toLocal8Bit();
    size_t len = qMax(29, nnetid.length());
    std::memcpy(netid, temp.data(), len);
    netid[len] = 0;
}

void LRNetClient::updateName(const QString & nname){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/name" )
    << osc::Blob(&session, sizeof(session))
    << nname.toStdString().data()
            << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::updateSection(const QString & nsection){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/section" )
    << osc::Blob(&session, sizeof(session))
    << nsection.toStdString().data()
            << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}
