#include <iostream>
#include <QApplication>

#include "lrnetserver.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>
#include <cstdio>


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
    mStopped(false)
    #ifdef WAIR // wair
    mWAIR(false),
    #endif // endwhere

    , mTotalRunningThreads(0)
    , mStimeoutTimer()
    , mCtimeoutTimer ()
    , cThread(new QThread())
    , m_connectDefaultAudioPorts(false)
    , mIOStatTimeout(0)

    , oscOutStream( buffer, OUTPUT_BUFFER_SIZE )

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
    mCtimeoutTimer.start();
    //cThread->exec();



    qDebug() << "mThreadPool default maxThreadCount =" << mThreadPool.maxThreadCount();
    mThreadPool.setMaxThreadCount(mThreadPool.maxThreadCount() * 16);
    qDebug() << "mThreadPool maxThreadCount set to" << mThreadPool.maxThreadCount();

    //mJTWorkers = new JackTripWorker(this);
    mThreadPool.setExpiryTimeout(3000); // msec (-1) = forever

    cout << "LRNet Server." << endl;


    mBufferStrategy = 1;
    mBroadcastQueue = 0;
}


//*******************************************************************************
LRNetServer::~LRNetServer()
{
    QMutexLocker lock(&mMutex);
    mThreadPool.waitForDone();
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
    connect(clientSocket, &QAbstractSocket::readyRead, this, &LRNetServer::receivedClientInfo);
    connect(clientSocket, &QAbstractSocket::disconnected, clientSocket,
            [=](){
        qDebug() <<"Client disconnected, must remove";
        //Single threaded event loop: mutex not required
        //if(cMutex.tryLock()){
            if (activeConnections.contains(clientSocket)){
                delete activeConnections[clientSocket].buffer;
                activeConnections.remove(clientSocket);
            }
            clientSocket->deleteLater();
            //cMutex.unlock();
        //}

        });
    Buffer * cBuf = new Buffer();
    if (cBuf){
        activeConnections.insert(clientSocket, {new QMutex(), 0, false, cBuf});
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
    //cout << "JackTrip HUB SERVER: Client Connect Received from Address : "
    //     << PeerAddress.toString().toStdString() << endl;

    //Todo: Put this in a new area since TCP is a byte-oriented protocol and
    //we have variable length messages.

    ;


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
        catch(osc::MalformedBundleException e){
            qDebug() <<"Malformed Bundle " <<e.what();
        }
    }
    else{
        try{
            inMsg = new osc::ReceivedMessage(*inPack);
            handleMessage(clientConnection, inMsg);
            cBuf->reset();
        }
        catch(osc::MalformedMessageException e){
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
    if (std::strcmp(msg->AddressPattern(), "/auth/new") == 0){
        osc::ReceivedMessageArgumentStream args = msg->ArgumentStream();

        osc::Blob b;
        if (!args.Eos()){
            args >> b;
        }
        if (b.size == sizeof(auth_packet_t)){
            AuthPacket pkt(*reinterpret_cast<auth_packet_t *>(const_cast<void *>(b.data)));

            QByteArray batmp = QByteArray::fromRawData((const char *)pkt.challenge, 214);
            qDebug() <<"Have a challenge " <<batmp;
            batmp = QByteArray::fromRawData((const char *)pkt.sig, 214);
            qDebug() <<"Signed " <<batmp;

            auth_type_t at = authorizer.checkCredentials(pkt);
            if( at.authType != NONE){
                sendAuthResponse(socket, at);
                qDebug() <<"Authenticated: Gave session id " <<at.session_id;
                activeSessions.insert(at.session_id, {at.session_id, socket, true});
            }
            else
            {
                sendAuthFail(socket);
            }
        }


    }
    else
    {
        osc::ReceivedMessageArgumentStream args = msg->ArgumentStream();
        session_id_t tSess = checkForValidSession(args);
        if(tSess == 0)
            return;
        if (std::strcmp(msg->AddressPattern(), "/get/roster") == 0)
            sendRoster(socket);
        if (std::strcmp(msg->AddressPattern(), "/ping") == 0){
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
    }

}

/**
 * @brief LRNetServer::checkForValidSession
 * @param msgs
 * @return
 * the session id if it is active, zero otherwise.
 */

session_id_t LRNetServer::checkForValidSession(osc::ReceivedMessageArgumentStream msgs){

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
    oscOutStream << osc::BeginMessage( "/push/roster" ) 
            << "James" <<"Sax" <<0 << "Coy" <<"Tbn" <<1 << osc::EndMessage;
    qDebug() <<"Sending Roster " <<socket->write(oscOutStream.Data(), oscOutStream.Size());
}

void LRNetServer::stopCheck()
{
    if (mStopped || sSigInt) {
        cout << "JackTrip HUB SERVER: Stopped" << endl;
        mStopCheckTimer.stop();
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
{
    QMutexLocker lock(&mMutex);
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
        mTotalRunningThreads++;
    }
    return ((busyAddress) ? -1 : id);
}


//*******************************************************************************
int LRNetServer::getPoolID(QString address, uint16_t port)
{
    QMutexLocker lock(&mMutex);
    //for (int id = 0; id<mThreadPool.activeThreadCount(); id++ )
    for (int id = 0; id<gMaxThreads; id++ )
    {
        if ( address==mActiveAddress[id].address &&  port==mActiveAddress[id].port)
        { return id; }
    }
    return -1;
}


//*******************************************************************************
int LRNetServer::releaseThread(int id)
{
    QMutexLocker lock(&mMutex);
    mActiveAddress[id].address = "";
    mActiveAddress[id].port = 0;
    mTotalRunningThreads--;

    mActiveAddress[id].clientName = "";
    return 0; /// \todo Check if we really need to return an argument here
}


// TODO:
// USE bool QAbstractSocket::isValid () const to check if socket is connect. if not, exit loop
