#include <iostream>
#include <QApplication>

#include "lrnetserver.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>
#include <cstdio>
#include <list>
#include <QRandomGenerator>

/**
 * \file SslServer.cpp
 * \author Alex Coy
 * \date November 2020
 */

#include "sslserver.h"
#include <QSslSocket>

using std::cout; using std::endl;

bool LRNetServer::sSigInt = false;

//*******************************************************************************
LRNetServer::LRNetServer(int server_port, int server_udp_port) :
    //mJTWorker(NULL),
    mTcpServer(this),
    mServerPort(server_port),
    mServerUdpPort(server_udp_port),//final udp base port number
    mRequireAuth(false),
    mStopped(false),
    mAuthCodeEnabled(false),
    mAuthCode(QString(getRandomString(12)))

    #ifdef WAIR // wair
    mWAIR(false),
    #endif // endwhere

    , mStimeoutTimer()
    , mCtimeoutTimer ()
    , cThread(new QThread())
    , m_connectDefaultAudioPorts(false)
    , mIOStatTimeout(0)

    , oscOutStream( buffer, OUTPUT_BUFFER_SIZE )
    , mRoster(new Roster(this, nullptr))

    , mLRdb(new LRdbClient(nullptr))
    , authorizer(mLRdb)

{

        mStimeoutTimer.setInterval(300000); //5 minutes for a session
        mStimeoutTimer.callOnTimeout([=](){
            //Single threaded event loop: mutex not required
            //QMutexLocker lock(&sMutex);
            QMutableHashIterator<session_id_t, sessionTriple> i(activeSessions);
            while (i.hasNext()){
                i.next();
                if (!(i.value().ShasCheckedIn)){

                    activeSessions.remove(i.key());
                    activeChefs.remove(i.key());
                    activeSuperChefs.remove(i.key());

                }
                else{
                i.value().ShasCheckedIn = false;
                }
            }
        });
        //mStimeoutTimer.start();

    mCtimeoutTimer.setInterval(20000); //20 seconds for a connection
    mCtimeoutTimer.callOnTimeout([=](){
        qDebug() <<"Checking for expired connections";
        //MutexLocker not required as this timer is in the main loop for now.
        //QMutexLocker lock(&cMutex);
        QMutableHashIterator<QSslSocket *, connectionPair> i(activeConnections);
        while (i.hasNext()){
            i.next();
            if (!(i.value().ChasCheckedIn)){
                if(i.key()){
                    i.key()->close();
                }
                //i.remove();
                //free i.value();
                qDebug() << "Closed a connection (timeout)";
            }
            else{
            i.value().ChasCheckedIn = false;
            }
        }
    });

    //mCtimeoutTimer->moveToThread(cThread);
    //mCtimeoutTimer.start();
    //cThread->exec();





    cout << "LRNet Server." << endl;

    QObject::connect(mRoster, &Roster::sigMemberUpdate, this, &LRNetServer::notifyChefsMemEvent);
    QObject::connect(mRoster, &Roster::sigMemberUpdate, this, &LRNetServer::sendMemberUdpPort);
    QObject::connect(mRoster, &Roster::sigNewChef, this, &LRNetServer::sendMemberUdpPort);
    QObject::connect(mRoster, &Roster::memberRemoved, this, &LRNetServer::notifyChefsMemLeft);
    QObject::connect(mRoster, &Roster::jackTripStarted, this, &LRNetServer::sendJackTripReady);
    QObject::connect(mRoster, &Roster::notifyChefsSessionJackTripStatus, this, &LRNetServer::notifyChefsSessionJackTripStatus);
    QObject::connect(mRoster, &Roster::sendKeyToClient, this, &LRNetServer::sendKeyToClient);
    QObject::connect(mRoster, &Roster::saveMemberControls, this, &LRNetServer::saveMemberControls);

    mBufferStrategy = 1;
    mBroadcastQueue = 0;



    jackServer.setDriver("dummy");
    {
    QString tmp = "rate";
    QVariant vtmp = 48000;
    jackServer.setDriverParameter(tmp,vtmp);
    }

    {
    QString tmp = "period";
    QVariant vtmp = 64;
    jackServer.setDriverParameter(tmp,vtmp);
    }

    if(!jackServer.start()){
       if(mRoster->initJackClient()){
           qDebug() <<"Couldn't connect hub patcher, quitting";
           QCoreApplication::quit();
       }

    }else{
        qDebug() << "jackServer.start() returned nonzero";
    }

    qDebug() << "Authorization code initialized to " << mAuthCode;

}


//*******************************************************************************
LRNetServer::~LRNetServer()
{
    //jackServer.stop();
    //delete jackServer;
    //delete mJTWorker;
}


//*******************************************************************************
// Now that the first handshake is with TCP server, if the addreess/peer port of
// the client is already on the thread pool, it means that a new connection is
// requested (the old was desconnected). So we have to remove that thread from
// the pool and then connect again.
void LRNetServer::start()
{
    mStopped = false;

    // Bind the TCP server
    // ------------------------------
    QObject::connect(&mTcpServer, &SslServer::newConnection, this, &LRNetServer::receivedNewConnection);
    if ( !mTcpServer.listen(QHostAddress::Any, mServerPort) ) {
        QString error_message = QString("TCP Socket Server on Port %1 ERROR: %2").arg(mServerPort).arg(mTcpServer.errorString());
        std::cerr << error_message.toStdString() << endl;
        emit signalError(error_message);
        return;
    }
    
    if (1) {
        cout << "LRNet Server: Enabling authentication" << endl;
        // Check that SSL is avaialable
        bool error = false;
        QString error_message;
        if (!QSslSocket::supportsSsl()) {
            error = true;
            error_message = "SSL not supported. Make sure you have the appropriate SSL libraries\ninstalled to enable authentication.";
        }
        
        if (mCertFile.isEmpty()) {
            error = true;
            error_message = "No certificate file specified.";
        } else if (mKeyFile.isEmpty()) {
            error = true;
            error_message = "No private key file specified.";
        }
        
        // Load our certificate and private key
        if (!error) {
            QFile certFile(mCertFile);
            if (certFile.open(QIODevice::ReadOnly)) {
                QSslCertificate cert(certFile.readAll());
                if (!cert.isNull()) {
                    mTcpServer.setCertificate(cert);
                } else {
                    error = true;
                    error_message = "Unable to load certificate file.";
                }
            } else {
                error = true;
                error_message = "Could not find certificate file.";
            }
        }
        
        if (!error) {
            QFile keyFile(mKeyFile);
            if (keyFile.open(QIODevice::ReadOnly)) {
                QSslKey key(&keyFile, QSsl::Rsa);
                if (!key.isNull()) {
                    mTcpServer.setPrivateKey(key);
                } else {
                    error = true;
                    error_message = "Unable to read RSA private key file.";
                }
            } else {
                error = true;
                error_message = "Could not find RSA private key file.";
            }
        }
        
        
        if (error) {
            std::cerr << "ERROR: " << error_message.toStdString() << endl;
            emit signalError(error_message);
            return;
        }
    }
    
    cout << "LRNet server: Waiting for client connections..." << endl;
    cout << "=======================================================" << endl;
    
    // Start our monitoring timer
    mStopCheckTimer.setInterval(200);
    connect(&mStopCheckTimer, &QTimer::timeout, this, &LRNetServer::stopCheck);
    mStopCheckTimer.start();
}


 /**
  * When the server receives a new connection, allocate a new streaming
  * buffer for it and set up cleanup on disconnect.
  *
  */   
void LRNetServer::receivedNewConnection()
{
    QSslSocket *clientSocket = static_cast<QSslSocket *>(mTcpServer.nextPendingConnection());

    OSCStreamingBuffer * cBuf = new OSCStreamingBuffer();
    if (cBuf){
        activeConnections.insert(clientSocket, {new QMutex(), 0, false, cBuf});
        connect(clientSocket, &QAbstractSocket::readyRead, this, &LRNetServer::receivedClientInfo);
        connect(clientSocket, &QAbstractSocket::disconnected, clientSocket,
                [=](){
            qDebug() <<"Client disconnected, must remove";
            //Single threaded event loop: mutex not required
            //if(cMutex.tryLock()){
                if (activeConnections.contains(clientSocket)){
                    connectionPair myConn = activeConnections[clientSocket];
                    activeSessions[myConn.assocSession].lastSeenConnection=NULL;
                    mRoster->removeMemberBySessionID(myConn.assocSession);
                    mRoster->removeChefBySessionID(myConn.assocSession);
                    activeChefs.remove(myConn.assocSession);
                    activeSuperChefs.remove(myConn.assocSession);
                    myConn.buffer->deleteLater();
                    activeConnections.remove(clientSocket);
                }
                clientSocket->deleteLater();
                //cMutex.unlock();
            //}

            });
        cout << "LRNet server: New Connection Received!" << endl;
    }
    else{
        clientSocket->close();
        clientSocket->deleteLater();
        cout << "Could not allocate buffer for new connection." << endl;
    }

}

void LRNetServer::receivedClientInfo()
{
    QSslSocket* clientConnection = static_cast<QSslSocket*>(QObject::sender());
    
    //QHostAddress PeerAddress = clientConnection->peerAddress();


    //If a connection has filled up our buffer with contents that don't
    //make a valid message, give up on that connection.
    OSCStreamingBuffer * cBuf = activeConnections[clientConnection].buffer;
    if (cBuf->remaining() == 0){
        cBuf->reset();
        //clientConnection->close();
    }

    int bytesRead = clientConnection->read(cBuf->head(), cBuf->remaining());
    cBuf->update(bytesRead);

    if (!cBuf->haveFullMessage())
        return;
    do{
    QScopedPointer<QByteArray> arr(cBuf->getMessage());
    if (!arr) return;

    qDebug() <<"Full message: " <<*arr;

    osc::ReceivedPacket * inPack = NULL;
    try{
        inPack = new osc::ReceivedPacket(arr->data(), (int32_t) arr->length());
    }
    catch(osc::MalformedPacketException e){
        qDebug() << "Malformed Packet";
        return;
    }

    if (!inPack)
        return;

    osc::ReceivedBundle * inBundle = NULL;
    osc::ReceivedMessage * inMsg = NULL;

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
            handleMessage(clientConnection, inMsg);
            //cBuf->reset();
        }
        catch(const osc::MalformedMessageException & e){
            qDebug() <<"Malformed Message " <<e.what();
        }

    }

    delete inBundle;
    delete inMsg;
    delete inPack;
    QThread::msleep(100);
    }while(cBuf->haveFullMessage());

}

void LRNetServer::handleMessage(QSslSocket * socket, osc::ReceivedMessage * msg){
    //cout <<"Address Pattern: " <<msg->AddressPattern() << endl;
    if (std::strcmp(msg->AddressPattern(), "/auth/newbykey") == 0){
        cout <<"/auth/newbykey" << endl;
        osc::ReceivedMessageArgumentStream args = msg->ArgumentStream();

        osc::Blob b;
        if (!args.Eos()){
            args >> b;
        }
        if (b.size == sizeof(auth_packet_t)){
            AuthPacket pkt(*reinterpret_cast<auth_packet_t *>(const_cast<void *>(b.data)));

            //QByteArray batmp = QByteArray::fromRawData((const char *)pkt.challenge, 214);
            //qDebug() <<"Have a challenge " <<batmp;
            //batmp = QByteArray::fromRawData((const char *)pkt.sig, 214);
            //qDebug() <<"Signed " <<batmp;

            qDebug() <<"Checking for " <<pkt.netid;
            temp_auth_type_t tat = authorizer.checkCredentials(pkt);
            if( tat.authType != NONE){
                sendAuthResponse(socket, {tat.session_id, tat.authType});
                qDebug() <<"Authenticated: Gave session id " <<tat.session_id;
                sessionTriple tt = {tat.session_id, tat.user_id, socket, true, tat.authType, NONE, ""};
                size_t nl = qMin(pkt.netid_length, (uint8_t)30);
                memcpy(tt.netid, pkt.netid, nl);
                tt.netid[nl] = 0;
                activeSessions.insert(tat.session_id, tt);
            }
            else
            {
                sendAuthFail(socket);
            }
        }


    }
    if (std::strcmp(msg->AddressPattern(), "/auth/newbycode") == 0){

        if (mAuthCodeEnabled){
            osc::ReceivedMessageArgumentStream args = msg->ArgumentStream();

            const char * authCode;
            if (!args.Eos()){
                try{
                args >> authCode;
                }catch(osc::WrongArgumentTypeException & e){
                    //Not a string.
                    authCode = NULL;
                }

                if (authCode){
                    qDebug() << "Got auth code " <<authCode;
                    QString qsAuthCode = QString::fromStdString(authCode);
                    qDebug() << "mAuthCode " << mAuthCode << " ... Got " << qsAuthCode;
                    if (mAuthCode.compare(qsAuthCode) == 0){
                        auth_type_t at = {authorizer.genSessionKey(), MEMBER};
                        sendAuthResponse(socket, at);
                        qDebug() <<"Authenticated: Gave session id " <<at.session_id;
                        sessionTriple tt = {at.session_id, -1, socket, true, at.authType, NONE, ""};
                        char nonetid[8] = "nonetid";
                        memcpy(tt.netid, nonetid, 7);
                        tt.netid[8] = 0;
                        activeSessions.insert(at.session_id, tt);
                    } else {
                        sendAuthCodeIncorrect(socket);
                       }
                }
            }
        } else {
            sendAuthCodeDisabled(socket);
        }
    }
    else
    {
        osc::ReceivedMessageArgumentStream args = msg->ArgumentStream();
        session_id_t tSess = checkForValidSession(args, socket);
        AuthTypeE role = activeSessions[tSess].role;
        if(tSess == 0)
            return;
        else if (std::strcmp(msg->AddressPattern(), "/get/roster") == 0){
            if (role & (SUPERCHEF | CHEF))
                sendRoster(socket);
        }
        else if (std::strcmp(msg->AddressPattern(), "/get/roles") == 0){
            if (role & (SUPERCHEF))
                sendRoles(socket);
        }
        else if (std::strcmp(msg->AddressPattern(), "/auth/storekey/send") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleStoreKey(args, tSess, socket);
        }
        else if (std::strcmp(msg->AddressPattern(), "/ping") == 0){
            //All sessions should have active connections. If not, pinging updates the connection.
            //tSess is in activeSessions at this point.
            if (activeConnections.contains(socket)){
                activeConnections[socket].ChasCheckedIn = true;
                activeSessions[activeConnections[socket].assocSession].ShasCheckedIn = true;
                QMutexLocker lock(activeConnections[socket].mutex); //Don't close connection after pong.
                sendPong(socket);
            } else{
                activeSessions[tSess].lastSeenConnection = socket;
            }
        }
        else if (std::strcmp(msg->AddressPattern(), "/sub/member") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER)){
                activeSessions[tSess].subcribedRole = MEMBER;
                qDebug() <<"Subscribed as member";
                handleNewMember(&args, tSess);
            }
        }

        else if (std::strcmp(msg->AddressPattern(), "/sub/chef") == 0){
            if (role & (SUPERCHEF | CHEF)){
                activeSessions[tSess].subcribedRole = CHEF;
                qDebug() <<"Subscribed as chef";
                handleNewChef(&args, tSess);
                //sendRoster(socket);
            }
        }

        else if (std::strcmp(msg->AddressPattern(), "/sub/superchef") == 0){
            if (role & (SUPERCHEF)){
                activeSessions[tSess].subcribedRole = SUPERCHEF;
                qDebug() <<"Subscribed as superchef";
                handleNewSuperChef(&args, tSess);
            }
        }

        else if (std::strcmp(msg->AddressPattern(), "/sub/unsubscribe") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleUnsubscribe(tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/send/chat") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                pushChatMessage(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/send/authcode") == 0){
            if (role & (SUPERCHEF | CHEF))
            handleAuthCodeUpdate(&args, tSess);
        }

        //Future refactor: use an association list of updatable parameters
        else if (std::strcmp(msg->AddressPattern(), "/update/name") == 0){
           if (role & (SUPERCHEF | CHEF | MEMBER))
                handleNameUpdate(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/section") == 0){
           if (role & (SUPERCHEF | CHEF | MEMBER))
                handleSectionUpdate(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/solo") == 0){
            if (role & (SUPERCHEF | CHEF))
                handleSoloUpdate(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/startjacktrip") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleStartJackTrip(args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/setredundancy") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleUpdateRedundancy(args, tSess);
        }


        else if (std::strcmp(msg->AddressPattern(), "/member/stopjacktrip") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                 mRoster->stopJackTrip(tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/sendmute") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleClientMute(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/control/member") == 0){
            if (role & (SUPERCHEF | CHEF))
                handleAdjustParams(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/joinmuted" ) == 0){
            if (role & (SUPERCHEF | CHEF))
                handleJoinMuted(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/section") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleSectionUpdate(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/permissions") == 0){
            if (role & (SUPERCHEF))
                handlePermissionUpdates(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/remove/users") == 0){
            if (role & (SUPERCHEF))
                removeUsers(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/chef/startjacktrip") == 0){
            if (role & (SUPERCHEF | CHEF ))
                handleStartJackTrip(args, tSess, CHEF);
        }

        else if (std::strcmp(msg->AddressPattern(), "/chef/stopjacktrip") == 0){
            if (role & (SUPERCHEF | CHEF ))
                mRoster->stopJackTrip(tSess);        
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/setselfloopback") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
                handleSelfLoopback(args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/setnumchannels") == 0){
            if (role & (SUPERCHEF | CHEF | MEMBER))
               handleSetNumChannels(args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/auth/setcodeenabled") == 0){
            if (role & (SUPERCHEF | CHEF))
                handleAuthCodeEnabled(&args);
        }

        else if (std::strcmp(msg->AddressPattern(), "/chef/startjacktripsec") == 0){
            if (role & (SUPERCHEF | CHEF))
                handleStartJackTripSec(tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/chef/stopjacktripsec") == 0){
            if (role & (SUPERCHEF | CHEF))
                handleStopJackTripSec(tSess);
        }
    }

}

/**
 * @brief LRNetServer::checkForValidSession
 * @param msgs
 * @return
 * the session id if it is active, zero otherwise.
 */

session_id_t LRNetServer::checkForValidSession(osc::ReceivedMessageArgumentStream & msgs, QSslSocket * socket){

    const session_id_t * tSess;
    osc::Blob tSess_b;
    try{
    if (!msgs.Eos()){
        msgs >> tSess_b;
    }}
    catch (osc::WrongArgumentTypeException e){
        qDebug() << "Not a blob type";
        return 0;
    }
    if (tSess_b.size != sizeof(session_id_t))
        return 0;
    tSess = reinterpret_cast<const session_id_t *>(tSess_b.data);

    if (!activeSessions.contains(*tSess)){
        qDebug() << "No session found for " <<tSess;
        return 0;
    }

    if (activeConnections[socket].assocSession != *tSess){
        qDebug() <<"Sessions differ on this connection. Updating.";
    }

    activeConnections[socket].assocSession = *tSess;
    /*
    qDebug() <<"Found session with role ";
    switch(activeSessions[*tSess].role){
    case CHEF:
        qDebug()<<"chef.";
        break;
    case SUPERCHEF:
        qDebug()<<"superchef.";
        break;
    case MEMBER:
        qDebug()<<"member.";
        break;
    case NONE:
        qDebug()<<"none.";
        break;
    default:
        qDebug()<<"unrecognized.";
        break;
    }
*/
    return *tSess;
}

void LRNetServer::sendAuthResponse(QSslSocket * socket, auth_type_t at){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/success" )
            << osc::Blob(&at, sizeof(at));
    /*
    switch (at.authType){
    case SUPERCHEF:
        oscOutStream<<"superchef";
        break;
    case CHEF:
        oscOutStream<<"chef";
        break;
    case MEMBER:
        oscOutStream<<"member";
        break;

    case NONE:
        qDebug() <<"Unrecognized auth type";
        oscOutStream<<"none";
        break;
    }*/

    oscOutStream << osc::EndMessage;
    writeStreamToSocket(socket);
    qDebug() <<"Sending Session ID bytes = ";// <<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendAuthFail(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/fail" )
            << 0 << osc::EndMessage;
    writeStreamToSocket(socket);
    qDebug() <<"Sending Auth Failed " ;//<<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendAuthCodeIncorrect(QSslSocket *socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/fail/wrongcode" )
                 << osc::EndMessage;
    writeStreamToSocket(socket);
    qDebug() << "Notifying user their auth code was incorrect";
}

void LRNetServer::sendAuthCodeDisabled(QSslSocket *socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/fail/codedisabled" )
                 << osc::EndMessage;
    writeStreamToSocket(socket);
    qDebug() << "Notifying user that auth code authentication is disabled";
}

void LRNetServer::sendPong(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/pong" )
            << 0 << osc::EndMessage;
    writeStreamToSocket(socket);
    qDebug() <<"Sending Pong " ;//<<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendRoster(QSslSocket * socket){
    qDebug() <<"Sending Roster " ;//<<socket->write(oscOutStream.Data(), oscOutStream.Size());
    for (Member * m:mRoster->getMembers()){
        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage( "/push/roster" );
        loadMemberFrame(m);
          //oscOutStream  << "James" <<"Sax" <<0 << "Coy" <<"Tbn" <<1;
        oscOutStream << osc::EndMessage;
    writeStreamToSocket(socket);
    }
}

void LRNetServer::sendRoles(QSslSocket * socket){
    qDebug() << "Requesting user roles";
    std::list<auth_roster_t> * userRoles = authorizer.getRoles();
    if (userRoles->empty())
        return;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/roles" );

    for (auth_roster_t user : *userRoles){
        qDebug() << "NetID: " << QString(user.netid.data()) << ", Role: " << user.authType;
        oscOutStream << user.netid.data();
        oscOutStream << int(user.authType);
    }
        oscOutStream << osc::EndMessage;
    writeStreamToSocket(socket);
}

void LRNetServer::notifyRolesUpdated(){
    qDebug() << "Recommending that super chefs request roles";
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/rolesupdated" )
                 << osc::EndMessage;
    broadcastToSuperChefs();
}

void LRNetServer::loadMemberFrame(Member * m){
    if (m){
        oscOutStream << m->getName().toStdString().c_str()
                     << m->getSection().toStdString().c_str()
                     << (int64_t)m->getSerialID()
                     << m->getIsClientMuted()
                     << m->getIsJackTripConnected();
    for (int i=0; i<Member::numControlValues; i++)
        oscOutStream<< (m->getCurrentControls())[i];
    }
    else{
        std::cerr << "loadMemberFrame called with null member!";
        }
}

void LRNetServer::stopCheck()
{
    if (mStopped || sSigInt) {
        cout << "LRNet Server: Stopped" << endl;
        mStopCheckTimer.stop();
        mRoster->stopAllThreads();
        mTcpServer.close();
        emit signalStopped();
    }
}



//*******************************************************************************
void LRNetServer::bindUdpSocket(QUdpSocket& udpsocket, int port)
{
    // QHostAddress::Any : let the kernel decide the active address
    if ( !udpsocket.bind(QHostAddress::Any,
                         port, QUdpSocket::DefaultForPlatform) ) {
        //std::cerr << "ERROR: could not bind UDP socket" << endl;
        //std::exit(1);
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    }
    else {
        cout << "UDP Socket Receiving in Port: " << port << endl;
    }
}


//*******************************************************************************
// check by comparing 32-bit addresses
int LRNetServer::isNewAddress(QString address, uint16_t port)
{ return 0;
    //QMutexLocker lock(&mMutex);
    bool busyAddress = false;
    int id = 0;

    /*
  while ( !busyAddress && (id<mThreadPool.activeThreadCount()) )
  {
    if ( address==mActiveAddress[id][0] &&  port==mActiveAddress[id][1]) { busyAddress = true; }
    id++;
  }
  */
    for (int i = 0; i<gMaxThreads; i++) {
        if ( address==mActiveAddress[i].address &&  port==mActiveAddress[i].port) {
            id = i;
            busyAddress = true;
        }
    }
    if ( !busyAddress ) {
        /*
    mActiveAddress[id][0] = address;
    mActiveAddress[id][1] = port;
  } else {
  */
        id = 0;
        bool foundEmptyAddress = false;
        while ( !foundEmptyAddress && (id<gMaxThreads) ) {
            if ( mActiveAddress[id].address.isEmpty() &&  (mActiveAddress[id].port == 0) ) {
                foundEmptyAddress = true;
                mActiveAddress[id].address = address;
                mActiveAddress[id].port = port;
            }  else {
                id++;
            }
        }
    }
    if (!busyAddress) {
        //mTotalRunningThreads++;
    }
    return ((busyAddress) ? -1 : id);
}


//*******************************************************************************
int LRNetServer::getPoolID(QString address, uint16_t port)
{
    //QMutexLocker lock(&mMutex);
    //for (int id = 0; id<mThreadPool.activeThreadCount(); id++ )
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( address==mActiveAddress[id].address &&  port==mActiveAddress[id].port)
        { return id; }
    }
    return -1;
}




// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit loop

void LRNetServer::handleNewMember(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    QString netid = QString::fromStdString(activeSessions[tSess].netid);
    db_controls_t controls = mLRdb->getControlsForUID(activeSessions[tSess].user_id);
    mRoster->addMember(netid, tSess, controls);
    while (!args->Eos()){
        const char * key;
        const char * value;
        try{
        *args >> key;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
        }
        if(!args->Eos()){
            try{
            *args >> value;
            }catch(osc::WrongArgumentTypeException & e){
                //Not a string.
            }
            QString qsKey = QString::fromStdString(key);
            QString qsValue = QString::fromStdString(value);
            if (qsKey.compare("name")==0){
                 mRoster->setNameBySessionID(qsValue, tSess);
            }
            if (qsKey.compare("section")==0){
                 mRoster->setSectionBySessionID(qsValue, tSess);
            }
        }

    }
}

void LRNetServer::handleNewChef(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    activeChefs.insert(tSess, tSess);
    QString netid = QString::fromStdString(activeSessions[tSess].netid);
    qDebug() << "Add chef netid " <<netid;
    db_controls_t controls = mLRdb->getControlsForUID(activeSessions[tSess].user_id);
    mRoster->addChef(netid, tSess, controls);
    sendRoster(activeSessions[tSess].lastSeenConnection);
    sendAuthCodeStatus(activeSessions[tSess].lastSeenConnection);
    sendJoinMutedStatus(activeSessions[tSess].lastSeenConnection);
    qDebug() << "Number of chefs: " << activeChefs.count();
}

void LRNetServer::handleNewSuperChef(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    activeSuperChefs.insert(tSess, tSess);
    QString netid = QString::fromStdString(activeSessions[tSess].netid);
//    mRoster->addSuperChef(netid,tSess);
    sendRoles(activeSessions[tSess].lastSeenConnection);
    qDebug() << "Number of super chefs: " << activeSuperChefs.count();
}

void LRNetServer::handleNameUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    if (!args->Eos()){
        const char * name = NULL;
        try{
        *args >> name;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
            name = NULL;
            qDebug() <<e.what();
        }
        if (name){
        QString qsName = QString::fromStdString(name);
             mRoster->setNameBySessionID(qsName, tSess);}
        }
}

void LRNetServer::handleClientMute(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    if (!args->Eos()){
        bool isMuted;
        try{
            *args >> isMuted;
            mRoster->setClientMutedBySessionID(tSess, isMuted);
            notifyChefsMemMute(mRoster->getSerialIDbySessionID(tSess), isMuted);
        }catch(osc::WrongArgumentTypeException & e){
            //Not a boolean.
        }
    }
}


void LRNetServer::handleUnsubscribe(session_id_t tSess){

    switch (activeSessions[tSess].subcribedRole){
        case NONE:
        case SUPERCHEF:
            activeSuperChefs.remove(tSess);
            qDebug() << "Number of super chefs: " << activeSuperChefs.count();
        case CHEF:
            activeChefs.remove(tSess);
            qDebug() << "Number of chefs: " << activeChefs.count();
        case MEMBER:
            mRoster->removeMemberBySessionID(tSess);
            activeSessions[tSess].subcribedRole = NONE;
    }
    activeSessions[tSess].subcribedRole = NONE;
}

void LRNetServer::handleSectionUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    if (!args->Eos()){
        const char * name;
        try{
        *args >> name;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
            name=NULL;
        }


        if(name){
        QString qsName = QString::fromStdString(name);
             mRoster->setSectionBySessionID(qsName, tSess);
        }
    }
}

void LRNetServer::handleSoloUpdate(osc::ReceivedMessageArgumentStream * args){
    int64_t id;
    bool isSolo;

    try{
        *args >> id;
        try{
            *args >> isSolo;

            oscOutStream.Clear();
            oscOutStream << osc::BeginMessage("/push/soloupdated")
                         << id
                         << isSolo
                         << osc::EndMessage;
            broadcastToChefs();

        }catch(osc::WrongArgumentTypeException & e){
            // not a bool
        }

    }catch(osc::WrongArgumentTypeException & e){
        // not an int
    }
}

void LRNetServer::handleJoinMuted(osc::ReceivedMessageArgumentStream * args){
    bool joinMuted;

    try{
        *args >> joinMuted;
        mRoster->setJoinMuted(joinMuted);

        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage("/push/joinmutedupdated")
                     << joinMuted
                     << osc::EndMessage;

        broadcastToChefs();

    }catch(osc::WrongArgumentTypeException & e){
        // not a boolean
    }
}

void LRNetServer::sendJoinMutedStatus(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage("/push/joinmutedupdated")
                 << mRoster->mJoinMuted
                 << osc::EndMessage;

    writeStreamToSocket(socket);

}

void LRNetServer::handlePermissionUpdates(osc::ReceivedMessageArgumentStream *args){
    if (!args->Eos()){
        osc::int32 authType;
        const char * netid;
        QList<QString> netidsSelected = QList<QString>();
        try{
            *args >> authType;
            try{
                while (!args->Eos()){
                    *args >> netid;
                    netidsSelected.append(QString(netid));
                }
            }catch(osc::WrongArgumentTypeException & e){
                // Not a string.
            }
            for (QString netid : netidsSelected){
                authorizer.updatePermission(netid, AuthTypeE(authType));
                for (unsigned long key : activeSessions.keys()){
                    if (netid == QString(activeSessions[key].netid)){
                        qDebug() << "Role before: " << activeSessions[key].role;
                        activeSessions[key].role = AuthTypeE(authType);
                        if (activeChefs.contains(key) && (AuthTypeE(authType) == MEMBER))
                            activeChefs.remove(key);
                        if (activeSuperChefs.contains(key) && (AuthTypeE(authType) & (CHEF | MEMBER)))
                            activeSuperChefs.remove(key);
                        qDebug() << "Role after: " << activeSessions[key].role;
                    }
                }
            }

            notifyRolesUpdated();

        }catch(osc::WrongArgumentTypeException & e){
            // Not an int.
        }
    }
}

void LRNetServer::removeUsers(osc::ReceivedMessageArgumentStream * args){
    if (!args->Eos()){
        const char * netid;
        QList<QString> netidsSelected = QList<QString>();
        try{
            while (!args->Eos()){
                *args >> netid;
                netidsSelected.append(QString(netid));
            }
        }catch(osc::WrongArgumentTypeException & e){
            // Not a string.
        }
        for (QString netid : netidsSelected){
            QVector<int> * uidsSelected = mLRdb->getIDsForNetid(netid);
            for (int uid : *uidsSelected){
                mLRdb->removeControlsForUID(uid);
            }
            mLRdb->removeUser(netid);
            for (unsigned long key : activeSessions.keys()){
                if (netid == QString(activeSessions[key].netid)){
                    qDebug() << "Removing current user: " << netid << "...";
                    activeSessions.remove(key);
                    if (activeChefs.contains(key))
                        activeChefs.remove(key);
                    if (activeSuperChefs.contains(key))
                        activeSuperChefs.remove(key);
                    qDebug() << "Successfully removed!";
                }
            }
        }

        notifyRolesUpdated();
    }
}

void LRNetServer::handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream * args){
    if (!args->Eos()){
        bool enabled;
        try{
            *args >> enabled;
        }catch(osc::WrongArgumentTypeException & e){
            // Not a boolean value.
            enabled = mAuthCodeEnabled;
        }
        if (mAuthCodeEnabled != enabled){
            mAuthCodeEnabled = enabled;

            qDebug() << "Auth code enabled set to " << mAuthCodeEnabled;

            oscOutStream.Clear();
            oscOutStream << osc::BeginMessage( "/push/authcodeenabled" )
                         << mAuthCodeEnabled
                         << osc::EndMessage;

            broadcastToChefs();
        }
    }
}

void LRNetServer::handleAuthCodeUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    if (!args->Eos()){
        const char * authCode;
        try{
            *args >> authCode;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
            authCode = NULL;
        }

        if (authCode){

            QString qsCode = QString::fromStdString(authCode);
            mAuthCode = qsCode;

            qDebug() << "Auth code updated to" << qsCode;
            oscOutStream.Clear();
            oscOutStream << osc::BeginMessage( "/push/authcodeupdated" )
                         << qsCode.toStdString().data()
                         << osc::EndMessage;

            broadcastToChefs();
        }
    }
}

void LRNetServer::saveAllControls(){
    for (Member *m : mRoster->getMembers()){
        session_id_t s_id = m->getSessionID();
        if (activeSessions.contains(s_id)){
            int u_id = activeSessions[s_id].user_id;
            const float * u_controls = m->getCurrentControls();
            db_controls_t db_controls = mLRdb->default_db_controls;
            db_controls.ratio = u_controls[Member::COMP_RATIO];
            db_controls.threshold = u_controls[Member::COMP_THRESHOLD];
            db_controls.attack = u_controls[Member::COMP_ATTACK];
            db_controls.release = u_controls[Member::COMP_RELEASE];
            db_controls.makeup = u_controls[Member::COMP_MAKEUP];
            db_controls.gain = u_controls[Member::INDIV_GAIN];
            mLRdb->updateControlsForUID(db_controls, u_id);
        }
    }
}

void LRNetServer::saveMemberControls(Member * m){
    if (mRoster->containsMember(m)){
        session_id_t s_id = m->getSessionID();
        if (activeSessions.contains(s_id)){
            int u_id = activeSessions[s_id].user_id;
            const float * u_controls = m->getCurrentControls();
            db_controls_t db_controls = mLRdb->default_db_controls;
            db_controls.ratio = u_controls[Member::COMP_RATIO];
            db_controls.threshold = u_controls[Member::COMP_THRESHOLD];
            db_controls.attack = u_controls[Member::COMP_ATTACK];
            db_controls.release = u_controls[Member::COMP_RELEASE];
            db_controls.makeup = u_controls[Member::COMP_MAKEUP];
            db_controls.gain = u_controls[Member::INDIV_GAIN];
            mLRdb->updateControlsForUID(db_controls, u_id);
        }
    }
}

void LRNetServer::sendAuthCodeStatus(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/authcodestatus" )
                 << mAuthCodeEnabled
                 << mAuthCode.toStdString().data()
                 << osc::EndMessage;

    writeStreamToSocket(socket);
}

void LRNetServer::notifyChefsMemLeft(Member::serial_t id){
    qDebug() <<"Generate mem left message";
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/roster/memberleft" );
    oscOutStream << (int64_t)id;
    oscOutStream << osc::EndMessage;
    broadcastToChefs();
}

void LRNetServer::notifyChefsMemEvent(Member * m, RosterNS::MemberEventE event){
    oscOutStream.Clear();
    switch (event){
    case RosterNS::MEMBER_CAME:
        oscOutStream << osc::BeginMessage( "/push/roster/newmember" );
        break;
    case RosterNS::MEMBER_UPDATE:
        oscOutStream << osc::BeginMessage( "/push/roster/updatemember" );
        break;
    }
        loadMemberFrame(m);
          oscOutStream << osc::EndMessage;
    broadcastToChefs();

}

void LRNetServer::notifyChefsMemMute(int serial_id, bool isClientMuted){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/clientmute")
                 << serial_id
                 << isClientMuted
                 << osc::EndMessage;

    broadcastToChefs();
}

void LRNetServer::sendMemberUdpPort(Member * m, RosterNS::MemberEventE event){
    if (event == RosterNS::MEMBER_CAME || event == RosterNS::CHEF_CAME){
        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage( "/config/udpport" )
        << m->getPort();
        oscOutStream << osc::EndMessage;
        writeStreamToSocket(activeSessions[m->getSessionID()].lastSeenConnection);
        //activeSessions[m->getSessionID()].lastSeenConnection->write(oscOutStream.Data(), oscOutStream.Size());
    }
}

void LRNetServer::pushChatMessage(osc::ReceivedMessageArgumentStream * args, session_id_t tSess) {
    if (!args->Eos()){
        const char * msg;
        try{
            *args >> msg;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
            qDebug() << "Wrong type of argument: pushChatMessage";
        }
        QString qsName = mRoster->getNameBySessionID(tSess);
        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage( "/push/chat" )
                     << qsName.toStdString().data()
                     << msg
                         << osc::EndMessage;
        broadcastToAll();
    }
}

void LRNetServer::broadcastToAll() {
    for (QSslSocket * socket : activeConnections.keys()) {
        writeStreamToSocket(socket);
        //socket->write(oscOutStream.Data(), oscOutStream.Size());
    }
}

void LRNetServer::broadcastToChefs(){
    //qDebug() <<"Will broadcast to chefs";
    for (session_id_t t:activeChefs.keys()){
        //qDebug() <<"Chef " <<t;
        QSslSocket * conn = activeSessions[t].lastSeenConnection;
         writeStreamToSocket(conn);
            //conn->write(oscOutStream.Data(), oscOutStream.Size());

    }
}

void LRNetServer::broadcastToSuperChefs(){
    for (session_id_t t:activeSuperChefs.keys()){
        QSslSocket * conn = activeSessions[t].lastSeenConnection;
        writeStreamToSocket(conn);
    }
}

void LRNetServer::notifyChefsSessionJackTripStatus(session_id_t session_id, bool jackTripStatus){
    qDebug() << "Sending Chefs Client's JackTrip Status: " << jackTripStatus;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/clientjacktripstatus" )
                 << mRoster->getSerialIDbySessionID(session_id)
                 << jackTripStatus
                 << osc::EndMessage;

    broadcastToChefs();
}

void LRNetServer::sendJackTripReady(session_id_t s_id){
    qDebug() << "Sending JackTrip Ready";
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/jacktripready" )
    << true;
    oscOutStream << osc::EndMessage;
    writeStreamToSocket(activeSessions[s_id].lastSeenConnection);
    //activeSessions[s_id].lastSeenConnection->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::handleAdjustParams(osc::ReceivedMessageArgumentStream * args){
    bool err;
    if (!args->Eos()){
        osc::int64 serial;
        err = false;
        try{
            *args >> serial;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a string.
            qDebug() << "Wrong type of argument: need member serial id";
        }
        if (err) return;
        while (!args->Eos()){
            osc::int32 paramNum;
            float paramVal;
            try{
                *args >> paramNum; *args >> paramVal;
            }catch(osc::WrongArgumentTypeException & e){
                //Not a string.
                qDebug() << "Wrong type of arguments for adjusting parameters. Needed int and float";
            }catch(osc::MissingArgumentException & e){
                qDebug() << "Wrong number of arguments for adjusting parameters. Needed int and float";
            }
            mRoster->setControl(serial, paramNum, paramVal);

        }

        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage( "/push/control/update" );
        loadMemberFrame(mRoster->getMembers()[serial]);
        oscOutStream << osc::EndMessage;

        broadcastToChefs();
    }

}

QString LRNetServer::getRandomString(int length){
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

   QString randomString;

   for(int i=0; i<length; ++i)
   {
       quint32 index = rGen.generate() % quint32(possibleCharacters.length());
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}

void LRNetServer::writeStreamToSocket(QSslSocket * socket){
    if(!socket)
       return;
    uint64_t s = oscOutStream.Size();
    socket->write((const char *)&s, sizeof(uint64_t));
    socket->write(oscOutStream.Data(), s);
}

void LRNetServer::handleStoreKey(
    osc::ReceivedMessageArgumentStream & args,
    session_id_t s_id,
    QSslSocket * socket)
    {
    osc::Blob b;
    osc::Blob c;
    bool err=false;
    try{
        args >> b >> c;
    }catch(osc::WrongArgumentTypeException & e){
        //Not a blob.
        qDebug() << "Wrong type of arguments for adjusting parameters. Needed two blobs";
        err = true;
    }catch(osc::MissingArgumentException & e){
        qDebug() << "Wrong number of arguments for adjusting parameters. Needed two blobs";
        err =true;
    }
    if(err || b.size!=451) {
        notifyStoreKeyResults(socket, false);
    }
    else if (c.size == sizeof(auth_packet_t)){
        qDebug() <<"Must verify sent key";
        AuthPacket pkt(*reinterpret_cast<auth_packet_t *>(const_cast<void *>(c.data)));
        if (!authorizer.addKey((const char *)b.data,pkt)){
            notifyRolesUpdated();
            notifyStoreKeyResults(socket, true);
            activeSessions[s_id].user_id = authorizer.getIDforKeyAndAuthPacket((const char *)b.data,pkt);
            size_t nl = qMin(pkt.netid_length, (uint8_t)30);
            memcpy(activeSessions[s_id].netid, pkt.netid, nl);
            activeSessions[s_id].netid[nl] = 0;
            mLRdb->addControlsForUID(mLRdb->default_db_controls, activeSessions[s_id].user_id);
        } else{
            notifyStoreKeyResults(socket, false);
        }
    }
}

void LRNetServer::notifyStoreKeyResults(QSslSocket *socket, bool success){
    qDebug() << "Notifying client: Store Key success ... " << success;
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/storekey/result" )
                 << success
                 << osc::EndMessage;
    writeStreamToSocket(socket);
}

void LRNetServer::handleUpdateRedundancy(
    osc::ReceivedMessageArgumentStream & args,
    session_id_t s_id)
{   
    osc::int32 n = 1;
    bool err=false;
    try{
        args >> n;
    }catch(osc::WrongArgumentTypeException & e){
        //Not a blob.
        qDebug() << "Wrong type of arguments for redundancy update. Need an int";
        err = true;
    }catch(osc::MissingArgumentException & e){
        qDebug() << "Wrong number of arguments for redundancy update. Need an int";
        err =true;
    }
    if(!err)
    mRoster->setRedundancyBySessionID(n, s_id);
}

void LRNetServer::handleStartJackTrip(
    osc::ReceivedMessageArgumentStream & args,
    session_id_t s_id,
    AuthTypeE role)
{   bool encrypt=false;
    try{
        args >> encrypt;
    }catch(osc::WrongArgumentTypeException & e){}
    catch(osc::MissingArgumentException & e){}
    mRoster->startJackTrip(s_id, encrypt);
}

void LRNetServer::sendKeyToClient(unsigned char * key, session_id_t s_id){
    qDebug() <<"Send key to client";
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/newencryptionkey" );
    if(key)
        oscOutStream << osc::Blob(key, 32);

    oscOutStream << osc::EndMessage;
    writeStreamToSocket(activeSessions[s_id].lastSeenConnection);
    delete[] key;
}

void LRNetServer::handleSetNumChannels(
    osc::ReceivedMessageArgumentStream & args,
    session_id_t session){
        osc::int32 nc = 1;
        bool err=false;
         try{
        args >> nc;
        }catch(osc::WrongArgumentTypeException & e){
            //Not a blob.
            qDebug() << "Wrong type of arguments for loopback. Need an int";
            err = true;
        }catch(osc::MissingArgumentException & e){
            qDebug() << "Wrong number of arguments for loopback. Need an int";
            err =true;
        }
        if(!err)
        mRoster->setNumChannelsBySessionID(nc, session);
    }

void LRNetServer::handleSelfLoopback(
    osc::ReceivedMessageArgumentStream & args,
    session_id_t session){

    bool lb = false;
    bool err=false;
    try{
        args >> lb;
    }catch(osc::WrongArgumentTypeException & e){
        //Not a blob.
        qDebug() << "Wrong type of arguments for loopback. Need an bool";
        err = true;
    }catch(osc::MissingArgumentException & e){
        qDebug() << "Wrong number of arguments for loopback. Need a bool";
        err =true;
    }
    if(!err)
        mRoster->setLoopbackBySessionID(lb, session);
    }

void LRNetServer::handleStartJackTripSec(session_id_t session){
    QString netid = QString::fromStdString(activeSessions[session].netid);
    db_controls_t controls = mLRdb->getControlsForUID(activeSessions[session].user_id);
    mRoster->addMember(netid, session, controls);
    mRoster->setNumChannelsBySessionID(2, session);
    //QString t = "Chef Alt";
    //mRoster->setNameBySessionID(t, session);
    //qDebug() <<"Set name by session ID";
    mRoster->startJackTrip(session, false, true);
}

void LRNetServer::handleStopJackTripSec(session_id_t session){
    mRoster->removeMemberBySessionID(session);
}
