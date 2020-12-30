#ifndef LRN_CLIENT_H
#define LRN_CLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QTimer>


#ifdef __WIN32__
#define NDEBUG
#include "osc/OscOutboundPacketStream.h"
#include "osc/OscReceivedElements.h"
#else
#include <osc/OscOutboundPacketStream.h>
#include <osc/OscReceivedElements.h>
#endif
#include "../lrnetserver/auth_types.h"

#ifdef LIBLRNET_LIBRARY
#warning "LIBLRNET_LIBRARY defined"
#else
#warning "LIBLRNET_LIBRARY not defined"
#endif


#include "../liblrnet_globals.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define OUTPUT_BUFFER_SIZE 1024
#define INPUT_BUFFER_SIZE 1024



class  LIBLRNET_EXPORT LRNetClient : public QObject
    {
        Q_OBJECT

public:
    typedef enum{
        CODE,
        KEY
    }AuthMethodE;

private:
        RSA * authKey;
        AuthTypeE authType;
        QString tempCode = "";
        AuthMethodE authMethod = KEY;
        char netid[30] = "";

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

        enum MemberInfoTypeE{
            MEMBER_ADD,
            MEMBER_UPDATE
        };
    
    public:
        LRNetClient(RSA * k = NULL);
        ~LRNetClient();


    
    signals:
        void responseReceived();
        void connected();
        void timeout();
        void newMember(const QString& name, const QString& group, int id);
        void updateMember(const QString& name, const QString& group, int id);
        void lostMember(int id);
        void authenticated(AuthTypeE type);
        void authFailed();
        void chatReceived(const QString& name, const QString& chatMsg);
        void gotUdpPort(int port);
    
    public slots:
        void tryConnect(const QString &host, int port);
        void requestRoster();
        void setNetid(const QString & nnetid);
        void setRSAKey (RSA * key);
        void setTempCode (const QString &_tempCode){tempCode = _tempCode;}
        void setAuthMethod (AuthMethodE method){authMethod = method;}
        void setKeyAuthMethod(){setAuthMethod(KEY);}
        void setCodeAuthMethod(){setAuthMethod(CODE);}
        void updateName(const QString & nname);
        void updateSection(const QString & nsection);
        void tryToAuthenticate();
        void subSuperchef();
        void subChef();
        void subMember();
        void sendChat(const QString &chatMsg);

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
        void sendKeyAuthPacket(auth_packet_t & pck);
        void sendCodeAuthPacket();
        void sendSmallMessage(QString & handle);
        void handleMessage(osc::ReceivedMessage * inMsg);
        void sendPing();
        void handleMemberGroup(osc::ReceivedMessageArgumentStream & args, MemberInfoTypeE type);
        void handleRemoveMember(osc::ReceivedMessageArgumentStream & args);
        void handleNewUdpPort(osc::ReceivedMessageArgumentStream & args);
        
    };

#endif
