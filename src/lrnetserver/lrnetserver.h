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

#define OUTPUT_BUFFER_SIZE 1024

extern int gVerboseFlag;

typedef struct {
    QString address;
    int16_t port;
    QString clientName;
} addressPortNameTriple;

class LRNetServer : public QObject
{
    Q_OBJECT;

    typedef struct {
        Auth::session_id_t id;
        QSslSocket * lastSeenConnection;
        bool ShasCheckedIn;
    }sessionTriple;

    typedef struct {
        QMutex * mutex;
        Auth::session_id_t assocSession;
        bool ChasCheckedIn;
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
    void sendRoster(QSslSocket * socket);

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
    

    //QUdpSocket mUdpHubSocket; ///< The UDP socket
    //QHostAddress mPeerAddress; ///< The Peer Address

    QThreadPool mThreadPool; ///< The Thread Pool

    SslServer mTcpServer;
    int mServerPort; //< Server known port number
    int mServerUdpPort; //< Server udp base port number
    int mBasePort;
    static const int gMaxThreads = 2;
    addressPortNameTriple mActiveAddress[gMaxThreads]; ///< Active address pool addresses
    //QHash<QString, uint16_t> mActiveAddressPortPair;
    
    bool mRequireAuth;
    QString mCertFile;
    QString mKeyFile;
    QString mCredsFile;

    /// Boolean stop the execution of the thread
    volatile bool mStopped;
    static bool sSigInt;
    QTimer mStopCheckTimer;
    int mTotalRunningThreads; ///< Number of Threads running in the pool
    QMutex mMutex;
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
    QHash<Auth::session_id_t, sessionTriple>activeSessions;
    QHash<QSslSocket *, connectionPair>activeConnections;
    Auth authorizer;
    
public :
    void setRequireAuth(bool requireAuth) { mRequireAuth = requireAuth; }
    void setCertFile(QString certFile) { mCertFile = certFile; }
    void setKeyFile(QString keyFile) { mKeyFile = keyFile; }
    void setCredsFile(QString credsFile) { mCredsFile = credsFile; }
    
    
    void setIOStatTimeout(int timeout) { mIOStatTimeout = timeout; }
    void setIOStatStream(QSharedPointer<std::ofstream> statStream) { mIOStatStream = statStream; }

    void setBufferStrategy(int BufferStrategy) { mBufferStrategy = BufferStrategy; }
    void setBroadcast(int broadcast_queue) {mBroadcastQueue = broadcast_queue;}

};

#endif
