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
        startHandshake();
    });
    connect(socket, &QSslSocket::disconnected, this, [=](){
        qDebug() <<__FILE__ <<__LINE__ <<"Disconnected";
        m_timeoutTimer.stop();
        connect(&m_timeoutTimer, &QTimer::timeout, this, &LRNetClient::connectionTimedOut);
        emit disconnected();
    });

    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    //connect(socket, SIGNAL(disconnected()), qApp, SLOT(quit()));

}


void LRNetClient::connectionTimedOut(){
    socket->abort();
    emit timeout();
}

void LRNetClient::tryConnect(const QString &host, int port){
    qDebug() <<"Connecting to "<<host <<"on " <<port;
    socket->connectToHost(host, port);
    m_timeoutTimer.start();
}

void LRNetClient::disconnectFromHost(){
    qDebug() << "Disconnecting from host";
    socket->disconnectFromHost();
}

LRNetClient::~LRNetClient(){
    disconnectFromHost();
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
    //socket->write(oscOutStream(), oscOutStream.Size());
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

void LRNetClient::requestRoles(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/get/roles" )
            << osc::Blob(&session, sizeof(session))
            << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::readResponse()
{

    int bytesRead = socket->read(buffer.head(), buffer.remaining());
    buffer.update(bytesRead);

    if (!buffer.haveFullMessage())
        return;
    do{
    osc::ReceivedPacket * inPack = NULL;
    QScopedPointer<QByteArray> msg(buffer.getMessage());

    if (!msg) return;

    try{

    inPack = new osc::ReceivedPacket(msg->data(), (int32_t)msg->length());
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
            //buffer.reset();
        }
        catch(const osc::MalformedMessageException & e){
            qDebug() <<"Malformed Message " <<e.what();
        }
    }
    delete inBundle;
    delete inMsg;
    delete inPack;
    }while (buffer.haveFullMessage());
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

    else if (std::strcmp(ap, "/push/roles") == 0){
        handleRoles(args);
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

    else if (std::strcmp(ap, "/auth/fail/wrongcode") == 0){
        emit authCodeIncorrect();
    }

    else if (std::strcmp(ap, "/auth/fail/codedisabled") == 0){
        emit authCodeDisabled();
    }

    else if (std::strcmp(ap, "/auth/storekey/result") == 0){
        handleStoreKeyResult(args);
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

    else if (std::strcmp(ap, "/push/clientmute") == 0){
        handleClientMute(args);
    }

    else if (std::strcmp(ap, "/push/clientjacktripstatus") == 0){
        handleClientJackTripStatus(args);
    }

    else if (std::strcmp(ap, "/config/udpport") == 0){
          handleNewUdpPort(args);
    }

    else if (std::strcmp(ap, "/member/newencryptionkey") == 0){
          handleNewEncryptionKey(args);
    }

    else if (std::strcmp(ap, "/member/jacktripready") == 0){
        if(!mEncryptionEnabled)
          emit serverJTReady();
    }

    else if (std::strcmp(ap, "/push/chat") == 0){
        handleNewChat(args);
    }

    else if (std::strcmp(ap, "/push/authcodestatus") == 0){
        handleAuthCodeStatus(args);
    }

    else if (std::strcmp(ap, "/push/rolesupdated") == 0){
        requestRoles();
    }

    else if (std::strcmp(ap, "/push/soloupdated") == 0){
        handleSoloUpdate(args);
    }

    else if (std::strcmp(ap, "/push/joinmutedupdated") == 0){
        handleJoinMutedUpdated(args);
    }

    else if (std::strcmp(ap, "/push/control/update") == 0){
        handleControlUpdate(args);
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

void LRNetClient::subSuperChef(){
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

void LRNetClient::unsubscribe(){
    QString handle = "/sub/unsubscribe";
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
    bool isClientMuted;
    bool isJackTripConnected;
    QVector<float> currControls(9); //ChannelStrip::numControlValues

    try{
        args >> memName;
        args >> memSect;
        args >> id;
        args >> isClientMuted;
        args >> isJackTripConnected;



        std::cout << "Member " <<id <<": " << memName <<"-" <<memSect << std::endl;
        switch (type){
        case MEMBER_ADD:
            for (int i=0; i<9; i++){ // ChannelStrip::numControlValues
                args >> (currControls.data())[i];
            }
            emit newMember(QString(memName), QString(memSect), QVector<float>(currControls), id, isClientMuted, isJackTripConnected);
            break;
        case MEMBER_UPDATE:
            emit updateMember(QString(memName), QString(memSect), id, isClientMuted, isJackTripConnected);
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
    osc::int64 id;
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
    osc::int32 port;
    try{
    args >> port;
    std::cout << "New UDP Port " <<port  << std::endl;
    emit gotUdpPort((int32_t)port);
    }
    catch (osc::WrongArgumentTypeException & e){
        qDebug() <<"UDP Port was a bad type";
    }
    catch (osc::MissingArgumentException & e){
        qDebug() <<"Not enough data for new UDP port";
    }
}

void LRNetClient::handleRoles(osc::ReceivedMessageArgumentStream & args){
    QList<AuthRoster> * authRoster = new QList<AuthRoster>();
    const char * netid;
    osc::int32 authType;
    while (!args.Eos()){
        try{
            args >> netid;
            try{
                args >> authType;
                authRoster->append({QString(netid), AuthTypeE(authType)});

            }catch(osc::WrongArgumentTypeException & e){
                // Not an int.
            }
        }catch(osc::WrongArgumentTypeException & e){
            // Not a const char *.
        }
    }

    if (!authRoster->empty())
        emit rolesReceived(authRoster);
}

void LRNetClient::handleSoloUpdate(osc::ReceivedMessageArgumentStream & args){
    osc::int64 id;
    bool isSolo;

    try{
        args >> id;
        try{
            args >> isSolo;

            emit handleSoloResponse(id, isSolo);

        }catch(osc::WrongArgumentTypeException & e){
            // not a boolean
        }
    }catch(osc::WrongArgumentTypeException & e){
        // not an int
    }
}

void LRNetClient::updatePermissions(QList<QString> *netidsSelected, AuthTypeE authType){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/permissions" )
                 << osc::Blob(&session, sizeof(session))
                 << int(authType);
    for (QString netid : *netidsSelected)
        oscOutStream << netid.toStdString().data();

    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::removeUsers(QList<QString> *netidsSelected){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/remove/users" )
                 << osc::Blob(&session, sizeof(session));
    for (QString netid : *netidsSelected)
        oscOutStream << netid.toStdString().data();

    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::startJackTrip(AuthTypeE role){
    oscOutStream.Clear();
    switch(role){
    case MEMBER:
        oscOutStream << osc::BeginMessage( "/member/startjacktrip" );
        break;
    case CHEF:
        oscOutStream << osc::BeginMessage( "/chef/startjacktrip" );
        break;
    default:
        qDebug() << "Role doesn't have jacktrip!";
        return;
    }
    oscOutStream << osc::Blob(&session, sizeof(session))
    << mEncryptionEnabled
    << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::stopJackTrip(AuthTypeE role){
    oscOutStream.Clear();
    switch(role){
    case MEMBER:
        oscOutStream << osc::BeginMessage( "/member/stopjacktrip" );
        break;
    case CHEF:
        oscOutStream << osc::BeginMessage( "/chef/stopjacktrip" );
        break;
    default:
        qDebug() << "Role doesn't have jacktrip!";
        return;
    }
    oscOutStream << osc::Blob(&session, sizeof(session))
    << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendClientMute(bool isMuted){
    qDebug() << "Sending muted set to :" << isMuted;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/sendmute")
                 << osc::Blob(&session, sizeof(session))
                 << isMuted
                 << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::handleClientMute(osc::ReceivedMessageArgumentStream & args){
    osc::int32 serial_id;
    bool isMuted;
    try{
        args >> serial_id;
        try{
            args >> isMuted;
            qDebug() << "Received client mute set to: " << isMuted;
            emit clientMuteReceived(serial_id, isMuted);
        }catch(osc::WrongArgumentTypeException & e){
            //Not a boolean
        }
    } catch(osc::WrongArgumentTypeException & e){
        //Not an int
    }
}

void LRNetClient::handleClientJackTripStatus(osc::ReceivedMessageArgumentStream & args){
    osc::int32 serial_id;
    bool isJackTripConnected;
    try{
        args >> serial_id;
        try{
            args >> isJackTripConnected;
            qDebug() << "Received client jacktrip connection: " << isJackTripConnected;
            emit clientJackTripStatusReceived(serial_id, isJackTripConnected);
        }catch(osc::WrongArgumentTypeException & e){
            //Not a boolean
        }
    } catch(osc::WrongArgumentTypeException & e){
        //Not an int
    }
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

void LRNetClient::handleAuthCodeStatus(osc::ReceivedMessageArgumentStream & args){
    bool enabled;
    const char * authCode;
    try{
        args >> enabled;

        try{
            args >> authCode;

            emit updateAuthCodeStatus(enabled, QString(authCode));

        }catch(osc::WrongArgumentTypeException & e){
            // Not a string
            authCode = NULL;
        }
    }catch(osc::WrongArgumentTypeException & e){
        // Not a boolean
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
        emit serverUpdatedAuthCode(QString(authCode));
    }
}

void LRNetClient::handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream & args){
    bool enabled;
    try{
        args >> enabled;

        qDebug() << "Auth code enabled on server: " << enabled;

        emit serverUpdatedAuthCodeEnabled(enabled);

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

    writeStreamToSocket();
}

void LRNetClient::sendControlUpdate(int64_t id, QVector<float> & controls){
    qDebug() << "Sending control update for "<<id << controls;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/control/member" )
    << osc::Blob(&session, sizeof(session));
    oscOutStream << id;
    for (int i=0; i<9; i++)
        oscOutStream << i << controls[i];
    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
    //socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetClient::sendSoloUpdate(int64_t id, bool isSolo){
    qDebug() << "Sending solo update for " << id << ": " << isSolo;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/solo" )
                 << osc::Blob(&session, sizeof(session))
                 << id
                 << isSolo
                 << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::sendJoinMutedUpdate(bool joinMuted){
    qDebug() << "Sending join muted updated to " << joinMuted;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/update/joinmuted" )
                 << osc::Blob(&session, sizeof(session))
                 << joinMuted
                 << osc::EndMessage;

    writeStreamToSocket();
}

void LRNetClient::handleJoinMutedUpdated(osc::ReceivedMessageArgumentStream & args){
    bool joinMuted;

    try{
        args >> joinMuted;

        emit handleJoinMutedResponse(joinMuted);

    }catch (osc::WrongArgumentTypeException & e){
        // not a boolean
    }
}

void LRNetClient::handleControlUpdate(osc::ReceivedMessageArgumentStream & args){
    const char * memName;
    const char * memSect;
    int64_t id;
    QVector<float> currControls(9); //ChannelStrip::numControlValues

    try{
        args >> memName;
        args >> memSect;
        args >> id;

        for (int i=0; i<9; i++){ //ChannelStrip::numControlValues
            args >> (currControls.data())[i];
        }

        emit updateMemberControls(currControls, id);

    }
    catch (osc::WrongArgumentTypeException & e){
        qDebug() <<"Member control argument was a bad type";
    }
    catch (osc::MissingArgumentException & e){
        qDebug() <<"Not enough data for member controls";
    }
}

void LRNetClient::writeStreamToSocket(){
    if(!socket)
       return;
    size_t s = oscOutStream.Size();
    socket->write((const char *)&s, sizeof(size_t));
    socket->write(oscOutStream.Data(), s);
}

void LRNetClient::sendPublicKey(){

    if(authKey){
        char t_arr[452];
        size_t len;
        BIO * pubkey_out = BIO_new(BIO_s_mem());
        PEM_write_bio_RSA_PUBKEY(pubkey_out, authKey);
        len = BIO_pending(pubkey_out);

        AuthPacket pck(netid);
        unsigned int retlen;
        RAND_bytes(pck.challenge, 214);
        RSA_sign(NID_sha256, pck.challenge, 214, pck.sig, &retlen, authKey);
        if(RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, 256, authKey)){
            qDebug() <<"Verified at send";
        }else{
            qDebug() <<"Failed to verify at send";
        }
        auth_packet_t dpck;
        pck.pack(dpck);


        if (len == 451){
            BIO_read(pubkey_out, t_arr, len);
            t_arr[451] = 0;
            qDebug() << "Sending public key";
            qDebug() << QString(t_arr);
            oscOutStream.Clear();
            oscOutStream << osc::BeginMessage( "/auth/storekey/send" )
            << osc::Blob(&session, sizeof(session))
            << osc::Blob(&t_arr, 451)
            << osc::Blob(reinterpret_cast<const char*>(const_cast<const auth_packet_t *>(&dpck)), sizeof(auth_packet_t));
            oscOutStream << osc::EndMessage;
            writeStreamToSocket();

        }
    }

}

void LRNetClient::handleStoreKeyResult(osc::ReceivedMessageArgumentStream & args){
    bool success;
    try{
        args >> success;
        emit storeKeyResultReceived(success);
    }catch(osc::WrongArgumentTypeException & e){
        // Not a boolean
    }
}

void LRNetClient::setRedundancy(int newRed){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/setredundancy" )
    << osc::Blob(&session, sizeof(session))
    << newRed;
    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::handleNewEncryptionKey(osc::ReceivedMessageArgumentStream & args){
    osc::Blob b;
    bool err = false;
    try{
        args >> b;

    } catch(osc::WrongArgumentTypeException & e){
        // Not a blob
        qDebug() <<"Key argument wrong type";
        err = true;
    }
    catch(osc::MissingArgumentException & e){
            // Missing blob
        qDebug() <<"Missing key argument for encryption";
        err = true;
        }
    if (!err && b.size == 32){
        char * key = new char[32];
        if (!key){
            qDebug() << "Could not allocate key pointer";
            return;
        }

        memcpy(key, b.data, 32);
        emit gotEncryptionKey(key);
    }
}


void LRNetClient::setjtSelfLoopback(bool e){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/setselfloopback" )
    << osc::Blob(&session, sizeof(session))
    << e;
    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::setNumChannels(int n){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/setnumchannels" )
    << osc::Blob(&session, sizeof(session))
    << n;
    oscOutStream << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::startJackTripSec(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/chef/startjacktripsec" );
    oscOutStream << osc::Blob(&session, sizeof(session))
    << mEncryptionEnabled
    << osc::EndMessage;
    writeStreamToSocket();
}

void LRNetClient::stopJackTripSec(){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/chef/stopjacktripsec" );
    oscOutStream << osc::Blob(&session, sizeof(session))
    << osc::EndMessage;
    writeStreamToSocket();
}
