#include <iostream>
#include <QApplication>

#include "lrnetserver.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>
#include <cstdio>
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

                }
                else{
                i.value().ShasCheckedIn = false;
                }
            }
        });
        mStimeoutTimer.start();

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
    QObject::connect(mRoster, &Roster::memberRemoved, this, &LRNetServer::notifyChefsMemLeft);
    QObject::connect(mRoster, &Roster::jackTripStarted, this, &LRNetServer::sendJackTripReady);

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
    jackServer.stop();

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
    
void LRNetServer::receivedNewConnection()
{
    QSslSocket *clientSocket = static_cast<QSslSocket *>(mTcpServer.nextPendingConnection());

    Buffer * cBuf = new Buffer();
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
                    activeChefs.remove(myConn.assocSession);
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
    Buffer * cBuf = activeConnections[clientConnection].buffer;
    if (cBuf->remaining() == 0){
        clientConnection->close();
    }

    int bytesRead = clientConnection->read(cBuf->head(), cBuf->remaining());
    activeConnections[clientConnection].buffer->update(bytesRead);
    osc::ReceivedPacket * inPack = NULL;
    try{
    inPack = new osc::ReceivedPacket(cBuf->base(), cBuf->filled());
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
            cBuf->reset();
        }
        catch(const osc::MalformedMessageException & e){
            qDebug() <<"Malformed Message " <<e.what();
        }

    }

    delete inBundle;
    delete inMsg;
    delete inPack;
    QThread::msleep(100);


}

void LRNetServer::handleMessage(QSslSocket * socket, osc::ReceivedMessage * msg){
    cout <<"Address Pattern: " <<msg->AddressPattern() << endl;
    if (std::strcmp(msg->AddressPattern(), "/auth/newbykey") == 0){
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
            auth_type_t at = authorizer.checkCredentials(pkt);
            if( at.authType != NONE){
                sendAuthResponse(socket, at);
                qDebug() <<"Authenticated: Gave session id " <<at.session_id;
                sessionTriple tt = {at.session_id, socket, true, at.authType, ""};
                size_t nl = qMin(pkt.netid_length, (uint8_t)30);
                memcpy(tt.netid, pkt.netid, nl);
                tt.netid[nl] = 0;
                activeSessions.insert(at.session_id, tt);
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
                        sessionTriple tt = {at.session_id, socket, true, at.authType, ""};
                        char nonetid[8] = "nonetid";
                        memcpy(tt.netid, nonetid, 7);
                        tt.netid[8] = 0;
                        activeSessions.insert(at.session_id, tt);
                    } else {
                        sendAuthFail(socket);
                       }
                }
            }
        } else {
            sendAuthFail(socket);
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
            if (role & (SUPERCHEF | CHEF | MEMBER))
                qDebug() <<"Subscribed as member";
            handleNewMember(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/sub/chef") == 0){
            if (role & (SUPERCHEF | CHEF)){
                qDebug() <<"Subscribed as chef";
                handleNewChef(&args, tSess);
                //sendRoster(socket);
            }
        }

        else if (std::strcmp(msg->AddressPattern(), "/sub/superchef") == 0){
            if (role & (SUPERCHEF))
                qDebug() <<"Subscribed as superchef";
        }

        else if (std::strcmp(msg->AddressPattern(), "/send/chat") == 0){
            pushChatMessage(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/send/authcode") == 0){
            handleAuthCodeUpdate(&args, tSess);
        }

        //Future refactor: use an association list of updatable parameters
        else if (std::strcmp(msg->AddressPattern(), "/update/name") == 0){
           handleNameUpdate(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/update/section") == 0){
           handleSectionUpdate(&args, tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/member/startjacktrip") == 0){
           mRoster->startJackTrip(tSess);
        }

        else if (std::strcmp(msg->AddressPattern(), "/auth/setcodeenabled") == 0){
            handleAuthCodeEnabled(&args, tSess);
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
    qDebug() <<"Sending Session ID bytes = " <<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendAuthFail(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/auth/failed" )
            << 0 << osc::EndMessage;
    qDebug() <<"Sending Auth Failed " <<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendPong(QSslSocket * socket){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/pong" )
            << 0 << osc::EndMessage;
    qDebug() <<"Sending Pong " <<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::sendRoster(QSslSocket * socket){
    
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/push/roster" );
    for(Member * m:mRoster->getMembers()){
        oscOutStream << m->getName().toStdString().c_str();
        oscOutStream << m->getSection().toStdString().c_str();
        oscOutStream << (int64_t)m->getSerialID();
}
          //oscOutStream  << "James" <<"Sax" <<0 << "Coy" <<"Tbn" <<1;
          oscOutStream << osc::EndMessage;
    qDebug() <<"Sending Roster " <<socket->write(oscOutStream.Data(), oscOutStream.Size());
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
    mRoster->addMember(netid, tSess);
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
    sendRoster(activeSessions[tSess].lastSeenConnection);
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

void LRNetServer::handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream * args, session_id_t tSess){
    if (!args->Eos()){
        bool enabled;
        try{
            *args >> enabled;
        }catch(osc::WrongArgumentTypeException & e){
            // Not a boolean value.
            enabled = mAuthCodeEnabled;
        }

        if (activeChefs.contains(tSess)){
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
            if (activeChefs.contains(tSess)){
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
        oscOutStream << m->getName().toStdString().c_str();
        oscOutStream << m->getSection().toStdString().c_str();
        oscOutStream << (int64_t)m->getSerialID();

          //oscOutStream  << "James" <<"Sax" <<0 << "Coy" <<"Tbn" <<1;
          oscOutStream << osc::EndMessage;
    broadcastToChefs();

}

void LRNetServer::sendMemberUdpPort(Member * m, RosterNS::MemberEventE event){
    if (event == RosterNS::MEMBER_CAME){
        oscOutStream.Clear();
        oscOutStream << osc::BeginMessage( "/config/udpport" )
        << m->getPort();
        oscOutStream << osc::EndMessage;
        activeSessions[m->getSessionID()].lastSeenConnection->write(oscOutStream.Data(), oscOutStream.Size());
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
        socket->write(oscOutStream.Data(), oscOutStream.Size());
    }
}

void LRNetServer::broadcastToChefs(){
    for (session_id_t t:activeChefs.keys()){
        QSslSocket * conn = activeSessions[t].lastSeenConnection;
        if (conn){
            qDebug() <<"Sending Update " <<conn->write(oscOutStream.Data(), oscOutStream.Size());
        }

    }
}

void LRNetServer::sendJackTripReady(session_id_t s_id){
    oscOutStream.Clear();
    oscOutStream << osc::BeginMessage( "/member/jacktripready" )
    << true;
    oscOutStream << osc::EndMessage;
    activeSessions[s_id].lastSeenConnection->write(oscOutStream.Data(), oscOutStream.Size());
}

// Function modified from here: https://stackoverflow.com/questions/18862963/qt-c-random-string-generation/18866593
QString LRNetServer::getRandomString(int length){
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");

   QString randomString;
   QRandomGenerator *generator = QRandomGenerator::system();
   for(int i=0; i<length; ++i)
   {
       quint32 index = generator->generate() % quint32(possibleCharacters.length());
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}
