#ifndef LRN_CLIENT_H
#define LRN_CLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QTimer>

#include <osc/OscOutboundPacketStream.h>
#include <osc/OscReceivedElements.h>
#include "../lrnetserver/auth_types.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define OUTPUT_BUFFER_SIZE 1024

class LRNetClient : public QObject
    {
        Q_OBJECT

        RSA * authKey;
    
    public:
        LRNetClient(RSA * k = NULL);
        ~LRNetClient();
    
    signals:
        void responseReceived();
        void connected();
        void timeout();
        void newMember(const QString& name, const QString& group, int id);
        void lostMember(int id);
    
    public slots:
        void tryConnect(const QString &host, int port);
        void requestRoster();
        void setRSAKey (RSA * key);

    private slots:
        void waitForGreeting();
        void readResponse();
        //void startTimer();
    
    private:
        QSslSocket *socket;
        char buffer[OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream oscOutStream;
        session_id_t session;
        QTimer m_timeoutTimer;

        void sendPacket();
        void connectionTimedOut();
        
    };

#endif
