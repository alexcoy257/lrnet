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
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendCodeAuthPacket(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/newbycode" );
    std::string code = tempCode.toStdString();
    oscOutStream << code.data() << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}


///Todo: Refactor to merge old and new names.
void LRNetClient::sendPacket(){
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::requestRoster(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roster" ) 
            << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::readResponse()
{

    int bytesRead = socket->read(buffer.head(), buffer.remaining());
    buffer.update(bytesRead);

    if (!buffer.haveFullMessage())
        return;

    osc::ReceivedPacket * inPack = NULL;
    QScopedPointer<QByteArray> msg(buffer.getMessage());

    if (!msg) return;

    try{
    inPack = new osc::ReceivedPacket(msg->data(), msg->length());
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
    osc::ReceivedMessageArgumentStream args = inMsg->ArgumentStream();

    if (std::strcmp(ap, "/push/roster") == 0){

        while (!args.Eos()){
            handleMemberGroup(args, MEMBER_ADD);
        }
    }


    else if (std::strcmp(ap, "/auth/success") == 0){


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

            /*
            m_timeoutTimer.setSingleShot(false);
            m_timeoutTimer.setInterval(2000);
            m_timeoutTimer.callOnTimeout([=](){sendPing();});
            m_timeoutTimer.start();*/
    }

     else if (std::strcmp(ap, "/auth/fail") == 0){
            emit authFailed();
     }

    else if (std::strcmp(ap, "/push/roster/newmember") == 0){
        handleMemberGroup(args, MEMBER_ADD);
    }

    else if (std::strcmp(ap, "/push/roster/updatemember") == 0){
            handleMemberGroup(args, MEMBER_UPDATE);
        }
    else if (std::strcmp(ap, "/push/roster/memberleft") == 0){
          handleRemoveMember(args);
    }

    else if (std::strcmp(ap, "/config/udpport") == 0){
          handleNewUdpPort(args);
    }

    else if (std::strcmp(ap, "/member/jacktripready") == 0){
          emit serverJTReady();
    }

    else if (std::strcmp(ap, "/push/chat") == 0){
        handleNewChat(args);
    }

    else if (std::strcmp(ap, "/push/authcodeupdated") == 0){
        handleAuthCodeUpdated(args);
    }

    else if (std::strcmp(ap, "/push/authcodeenabled") == 0){
        handleAuthCodeEnabled(args);
    }
}

void LRNetClient::sendPing(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/ping" )
            << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::setRSAKey(RSA * key){
    authKey = key;
}

void LRNetClient::sendSmallMessage(QString & handle){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( handle.toStdString().data() )
    << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
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
    size_t len = qMin(29, nnetid.length());
    std::memcpy(netid, temp.data(), len);
    netid[len] = 0;
    qDebug() <<"netid is now" <<netid;
}

void LRNetClient::updateName(const QString & nname){
    qDebug() <<"Update Name";
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/name" )
    << osc::Blob(&session, sizeof(session))
    << nname.toStdString().data()
            << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::updateSection(const QString & nsection){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/section" )
    << osc::Blob(&session, sizeof(session))
    << nsection.toStdString().data()
            << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::handleMemberGroup(osc::ReceivedMessageArgumentStream & args, MemberInfoTypeE type){
    const char * memName;
    const char * memSect;
    int64_t id;
    QVector<float> currControls(8);

    try{
    args >> memName;
    args >> memSect;
    args >> id;


    std::cout << "Member " <<id <<": " << memName <<"-" <<memSect << std::endl;
    switch (type){
    case MEMBER_ADD:
        for (int i=0; i<8; i++){
            args >> (currControls.data())[i];
        }
    emit newMember(QString(memName), QString(memSect), QVector<float>(currControls), id);
        break;
    case MEMBER_UPDATE:
    emit updateMember(QString(memName), QString(memSect), id);
        break;
    }
    }
    catch (osc::WrongArgumentTypeException & e){
        qDebug() <<"Member group argument was a bad type";
    }
    catch (osc::MissingArgumentException & e){
        qDebug() <<"Not enough data for member group";
    }
}

void LRNetClient::handleRemoveMember(osc::ReceivedMessageArgumentStream & args){
    int64_t id;
    try{
    args >> id;
    std::cout << "Remove member " <<id  << std::endl;
    emit lostMember(id);
    }
    catch (osc::WrongArgumentTypeException & e){
        qDebug() <<"Member remove ID was a bad type";
    }
    catch (osc::MissingArgumentException & e){
        qDebug() <<"Not enough data for removing a member";
    }
}

void LRNetClient::handleNewUdpPort(osc::ReceivedMessageArgumentStream & args){
    int32_t port;
    try{
    args >> port;
    std::cout << "New UDP Port " <<port  << std::endl;
    emit gotUdpPort(port);
    }
    catch (osc::WrongArgumentTypeException & e){
        qDebug() <<"UDP Port was a bad type";
    }
    catch (osc::MissingArgumentException & e){
        qDebug() <<"Not enough data for new UDP port";
    }
}

void LRNetClient::startJackTrip(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/startjacktrip" )
    << osc::Blob(&session, sizeof(session))
    << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendChat(const QString &chatMsg){
    qDebug() << "Sending chat: " << chatMsg;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/send/chat" )
    << osc::Blob(&session, sizeof(session))
    << chatMsg.toStdString().data()
        << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::handleNewChat(osc::ReceivedMessageArgumentStream & args){
    const char * chatName;
    const char * chatMsg;
    try{
        args >> chatName;
    }catch(osc::WrongArgumentTypeException & e){
        // Not a string.
        chatName = NULL;
    }

    if (chatName){
        try{
        args >> chatMsg;
        } catch(osc::WrongArgumentTypeException & e){
            // Not a string.
            chatMsg = NULL;
        }

        if (chatMsg){
            qDebug() << "Name <" <<chatName <<">: " << chatMsg;
            emit chatReceived(QString(chatName), QString(chatMsg));
        }
    }
}

void LRNetClient::handleAuthCodeUpdated(osc::ReceivedMessageArgumentStream & args){
    const char * authCode;
    try{
        args >> authCode;
    } catch(osc::WrongArgumentTypeException & e){
        // Not a string.
        authCode = NULL;
    }

    if (authCode){
        std::cout << "Authorization code changed to " << authCode  << std::endl;
    }
}

void LRNetClient::handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream & args){
    bool enabled;
    try{
        args >> enabled;

        if (enabled){
            std::cout << "Authorization code enabled on server" << std::endl;
        } else{
            std::cout << "Authorization code disabled on server" << std::endl;
        }

    } catch(osc::WrongArgumentTypeException & e){
        // Not a boolean.
    }
}


void LRNetClient::sendAuthCode(const QString &authCode){
    qDebug() << "Sending authorization code: " << authCode;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/send/authcode" )
    << osc::Blob(&session, sizeof(session))
    << authCode.toStdString().data()
        << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::updateAuthCodeEnabled(bool enabled){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/setcodeenabled" )
    << osc::Blob(&session, sizeof(session))
    << enabled
    << osc::EndMessage;

    socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendControlUpdate(int64_t id, QVector<float> & controls){
    qDebug() << "Sending control update for "<<id << controls;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/control/member" )
    << osc::Blob(&session, sizeof(session));
    oscOutStream << id;
    for (int i=0; i<8; i++)
        oscOutStream << i << controls[i];
    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::writeStreamToSocket(){
    if(!socket)
       return;
    size_t s = oscOutStream.Size();
    socket->write((const char *)&s, sizeof(size_t));
    socket->write(oscOutStream.Data(), s);
}

void LRNetClient::updateAuthCodeEnabled(bool enabled){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/setcodeenabled" )
    << osc::Blob(&session, sizeof(session))
    << enabled
    << osc::EndMessage;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
}
