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
#define INPUT_BUFFER_SIZE 1024

class LRNetClient : public QObject
    {
        Q_OBJECT

        RSA * authKey;
        class Buffer{
            char _base[INPUT_BUFFER_SIZE];
            char * _head = _base;
            size_t _remaining = INPUT_BUFFER_SIZE;
        public:
            Buffer(){};
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
        };

        Buffer buffer;
    
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
        void startHandshake();
        void readResponse();
        //void startTimer();
    
    private:
        QSslSocket *socket;
        char oBuffer[OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream oscOutStream;
        session_id_t session;
        QTimer m_timeoutTimer;

        void sendPacket();
        void connectionTimedOut();
        void sendAuthPacket(auth_packet_t & pck);
        void handleMessage(osc::ReceivedMessage * inMsg);
        void sendPing();
        
    };

#endif
