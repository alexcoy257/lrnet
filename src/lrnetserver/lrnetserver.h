/**
 * \file LRNetServer.h
 * \author Alex Coy
 * \date Novemer 2020
 */

#ifndef __LRNetServer_H__
#define __LRNetServer_H__

#include <iostream>
#include <stdexcept>
#include <fstream>

#include <QThread>
#include <QThreadPool>
#include <QUdpSocket>
#include <QHostAddress>
#include "sslserver.h"
#include <QMutex>
#include <QTimer>
#include <QSslSocket>
#include <QFile>
#include <QtEndian>
#include <QUdpSocket>
#include <osc/OscReceivedElements.h>
#include <osc/OscPrintReceivedElements.h>
#include <osc/OscOutboundPacketStream.h>
#include "auth.h"
#include "lrnet_roster.h"
#include "lrnet_member.h"
#include "../JackServerTest/jackinterface.h"
#include "lrnetserver_types.h"
#include <QRandomGenerator>
#include "../common/oscstreambuffer.h"

#define OUTPUT_BUFFER_SIZE 1024
#define INPUT_BUFFER_SIZE 1024

extern int gVerboseFlag;

typedef struct {
    QString address;
    int16_t port;
    QString clientName;
} addressPortNameTriple;

class Roster;
class Member;

using LRServer_types::sessionTriple;

class LRNetServer : public QObject
{
    Q_OBJECT;


/*
    class Buffer: public QObject{
        char _base[INPUT_BUFFER_SIZE];
        char * _head = _base;
        size_t _remaining = INPUT_BUFFER_SIZE;
    public:
        Buffer(QObject * parent=nullptr):QObject(parent){};
        void update(size_t bytesRead){
            _head += bytesRead;
            _remaining -= bytesRead;
        }
        void reset(){
            _head = _base;
            _remaining = INPUT_BUFFER_SIZE;
        }
        char * head(){
            return _head;
        }
        char * base(){
            return _base;
        }
        size_t remaining(){
            return _remaining;
        }
        size_t filled(){
            return INPUT_BUFFER_SIZE - _remaining;
        }
    };*/

    /**
      * This type handles data associated with a particular
      * connection. As connections are kept alive with /ping
      * messages, the associated session can be kept alive as
      * well.
      */
    typedef struct {
        QMutex * mutex;
        session_id_t assocSession;
        bool ChasCheckedIn;
        OSCStreamingBuffer * buffer;
    }connectionPair;

public:
    LRNetServer(int server_port = 4463, int server_udp_port = 0);
    virtual ~LRNetServer();

    /// \brief Starts the TCP server
    void start();

    /// \brief Stops the execution of the Thread
    void stop() { mStopped = true; }

    int releaseThread(int id);

    
    static void sigIntHandler(__attribute__((unused)) int unused)
    { std::cout << std::endl << "Shutting Down..." << std::endl; sSigInt = true; }

private slots:
    void testReceive()
    { std::cout << "========= TEST RECEIVE SLOT ===========" << std::endl; }
    void receivedNewConnection();
    void receivedClientInfo();
    void stopCheck();
    void handleMessage(QSslSocket * socket, osc::ReceivedMessage * msg);
    session_id_t checkForValidSession(osc::ReceivedMessageArgumentStream & msgs, QSslSocket * socket);
    void sendAuthResponse(QSslSocket * socket, auth_type_t at);
    void sendAuthFail(QSslSocket * socket);
    void sendAuthCodeIncorrect(QSslSocket *socket);
    void sendAuthCodeDisabled(QSslSocket *socket);
    void sendRoster(QSslSocket * socket);
    void sendRoles(QSslSocket * socket);
    void sendAuthCodeStatus(QSslSocket * socket);
    void sendJoinMutedStatus(QSslSocket * socket);
    void notifyRolesUpdated();
    void sendPong(QSslSocket * socket);
    void sendJackTripReady(session_id_t s_id);
    void handleNewMember(osc::ReceivedMessageArgumentStream * args, session_id_t session);
    void handleUpdateRedundancy(osc::ReceivedMessageArgumentStream & args, session_id_t session);
    void handleStartJackTrip(osc::ReceivedMessageArgumentStream & args, session_id_t s_id, AuthTypeE role=MEMBER);
    void sendKeyToClient(unsigned char * key, session_id_t s_id);
    void handleUnsubscribe(session_id_t);
    void sendMemberUdpPort(Member * m, RosterNS::MemberEventE event);
    void notifyChefsMemEvent(Member * member, RosterNS::MemberEventE event);
    void notifyChefsMemLeft(Member::serial_t id);
    void notifyChefsMemMute(int serial_id, bool isClientMuted);
    void notifyChefsSessionJackTripStatus(session_id_t session_id, bool jackTripStatus);
    void broadcastToSuperChefs();
    void broadcastToChefs();
    void broadcastToAll();
    void handleNewSuperChef(osc::ReceivedMessageArgumentStream * args, session_id_t tSess);
    void handleNewChef(osc::ReceivedMessageArgumentStream * args, session_id_t tSess);
    void handleNameUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t session);
    void handleSectionUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t session);
    void handleSoloUpdate(osc::ReceivedMessageArgumentStream * args);
    void handleJoinMuted(osc::ReceivedMessageArgumentStream * args);
    void handleClientMute(osc::ReceivedMessageArgumentStream * args, session_id_t session);
    void handlePermissionUpdates(osc::ReceivedMessageArgumentStream * args);
    void removeUsers(osc::ReceivedMessageArgumentStream * args);
    void handleAdjustParams(osc::ReceivedMessageArgumentStream * args);
    void loadMemberFrame(Member * m);
    void handleAuthCodeUpdate(osc::ReceivedMessageArgumentStream * args, session_id_t session);
    void handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream * args);
    void saveAllControls();
    void saveMemberControls(Member * m);
    void pushChatMessage(osc::ReceivedMessageArgumentStream * args, session_id_t tSess);
    void writeStreamToSocket(QSslSocket * socket);
    void handleStoreKey(osc::ReceivedMessageArgumentStream & args, session_id_t session, QSslSocket * socket);
    void notifyStoreKeyResults(QSslSocket *socket, bool success);
    void handleSetNumChannels(osc::ReceivedMessageArgumentStream & args, session_id_t session);
    void handleSelfLoopback(osc::ReceivedMessageArgumentStream & args, session_id_t session);
    void handleStartJackTripSec(session_id_t session);
    void handleStopJackTripSec(session_id_t session);



signals:
    void Listening();
    void ClientAddressSet();
    void signalRemoveThread(int id);
    void signalStopped();
    void signalError(const QString &errorMessage);

private:
    /** \brief Binds a QUdpSocket. It chooses the available (active) interface.
   * \param udpsocket a QUdpSocket
   * \param port Port number
   */
    static void bindUdpSocket(QUdpSocket& udpsocket, int port);


    /** \brief Send the JackTripWorker to the thread pool. This will run
   * until it's done. We still have control over the prototype class.
   * \param id Identification Number
   */
    //void sendToPoolPrototype(int id);

    /** \brief Check if address is already handled, if not add to array
   * \param address as string (IPv4 or IPv6)
   * \return -1 if address is busy, id number if not
   */
    int isNewAddress(QString address, uint16_t port);

    /** \brief Returns the ID of the client in the pool. If the client
    * is not in the pool yet, returns -1.
    */
    int getPoolID(QString address, uint16_t port);

    void manageTimeouts(){

    }




    QRandomGenerator rGen = QRandomGenerator::securelySeeded();
    QString getRandomString(int length);

    //QUdpSocket mUdpHubSocket; ///< The UDP socket
    //QHostAddress mPeerAddress; ///< The Peer Address



    SslServer mTcpServer;
    int mServerPort; //< Server known port number
    int mServerUdpPort; //< Server udp base port number
    int mBasePort;
    static const int gMaxThreads = 2;
    addressPortNameTriple mActiveAddress[gMaxThreads]; ///< Active address pool addresses
    //QHash<QString, uint16_t> mActiveAddressPortPair;
    
    bool mRequireAuth;
    bool mAuthCodeEnabled;
    QString mCertFile;
    QString mKeyFile;
    QString mCredsFile;
    QString mAuthCode;

    /// Boolean stop the execution of the thread
    volatile bool mStopped;
    static bool sSigInt;
    QTimer mStopCheckTimer;


    QMutex sMutex;
    QMutex cMutex;

    QTimer mStimeoutTimer;
    QTimer mCtimeoutTimer;
    QThread * cThread;

    int mBufferQueueLength;
    
    bool m_connectDefaultAudioPorts;

    int mIOStatTimeout;
    QSharedPointer<std::ofstream> mIOStatStream;

    int mBufferStrategy;
    int mBroadcastQueue;
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream oscOutStream;
    //QVector<QSslSocket *>activeConnections;

    /**
     *  a hash table of sessions that have checked
     *  in between 30 and 60 minutes ago. Associated
     *  with mStimeoutTimer.
     */
    QHash<session_id_t, sessionTriple>activeSessions;

    /**
     *  a hash table of chefs who have checked
     *  in between 30 and 60 minutes ago.
     *  Used for publishing new member events.
     */
    QHash<session_id_t, session_id_t>activeChefs;

    /**
     *  a hash table of super chefs who have checked
     *  in between 30 and 60 minutes ago.
     *  Used for publishing authorization updates.
     */
    QHash<session_id_t, session_id_t>activeSuperChefs;

    /**
     *  a hash table of connections that are still alive (and
     *  have TCP buffers). Associated connections die after
     *  20 seconds of no activity.
     */
    QHash<QSslSocket *, connectionPair>activeConnections;

    LRdbClient * mLRdb;

    Auth authorizer;
    Roster * mRoster;
    
    JackInterface jackServer;

public :
    void setRequireAuth(bool requireAuth) { mRequireAuth = requireAuth; }
    void setCertFile(QString certFile) { mCertFile = certFile; }
    void setKeyFile(QString keyFile) { mKeyFile = keyFile; }
    void setCredsFile(QString credsFile) { mCredsFile = credsFile; }

    QHash<session_id_t, sessionTriple> & getActiveSessions(){return activeSessions;};
    
    
    void setIOStatTimeout(int timeout) { mIOStatTimeout = timeout; }
    void setIOStatStream(QSharedPointer<std::ofstream> statStream) { mIOStatStream = statStream; }

    void setBufferStrategy(int BufferStrategy) { mBufferStrategy = BufferStrategy; }
    void setBroadcast(int broadcast_queue) {mBroadcastQueue = broadcast_queue;}

};

#endif
