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
#include "../common/oscstreambuffer.h"

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


struct AuthRoster{
    QString name;
    AuthTypeE authType;
};

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

        /*
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
        };*/

        OSCStreamingBuffer buffer;

        enum MemberInfoTypeE{
            MEMBER_ADD,
            MEMBER_UPDATE
        };
        bool mEncryptionEnabled = false;
    
    public:
        LRNetClient(RSA * k = NULL);
        ~LRNetClient();


    
    signals:
        void responseReceived();
        void connected();
        void disconnected();
        void timeout();
        void newMember(const QString& name, const QString& group, const QVector<float> controls, int id);
        void updateMember(const QString& name, const QString& group, int id);
        void updateMemberControls(QVector<float> &controls, int id);
        void lostMember(int id);
        void authenticated(AuthTypeE type);
        void authFailed();
        void authCodeIncorrect();
        void authCodeDisabled();
        void handleSoloResponse(int id, bool isSolo);
        void handleJoinMutedResponse(bool joinMuted);
        void updateAuthCodeStatus(bool enabled, const QString & authCode);
        void serverUpdatedAuthCodeEnabled(bool enabled);
        void serverUpdatedAuthCode(const QString & authCode);
        void storeKeyResultReceived(bool success);
        void gotUdpPort(int port);
        void serverJTReady();
        void chatReceived(const QString& name, const QString& chatMsg);
        void rolesReceived(QList<AuthRoster> *authRoster);
        void gotEncryptionKey(char * key);
    
    public slots:
        void tryConnect(const QString &host, int port);
        void disconnectFromHost();
        void requestRoster();
        void requestRoles();
        void setNetid(const QString & nnetid);
        void setRSAKey (RSA * key);
        void setTempCode (const QString &_tempCode){tempCode = _tempCode;}
        void setAuthMethod (AuthMethodE method){authMethod = method;}
        void setKeyAuthMethod(){setAuthMethod(KEY);}
        void setCodeAuthMethod(){setAuthMethod(CODE);}
        void updateName(const QString & nname);
        void updateSection(const QString & nsection);
        void updatePermissions(QList<QString> *netidsSelected, AuthTypeE authType);
        void removeUsers(QList<QString> *netidsSelected);
        void tryToAuthenticate();
        void sendChat(const QString &chatMsg);
        void sendAuthCode(const QString &chatMsg);
        void sendPublicKey();
        void updateAuthCodeEnabled(bool enabled);

        void subSuperChef();


        void subChef();
        void sendControlUpdate(int64_t id, QVector<float> & controls);
        void sendSoloUpdate(int64_t id, bool isSolo);
        void sendJoinMutedUpdate(bool joinMuted);

        void subMember();
        void setRedundancy(int n);
        void setNumChannels(int n);
        void setEncryption(bool e) {mEncryptionEnabled = e;}
        void setjtSelfLoopback(bool e);
        void startJackTrip(AuthTypeE role = MEMBER);
        void stopJackTrip(AuthTypeE role = MEMBER);

        void unsubscribe();

    private slots:
        void startHandshake();
        void readResponse();
        void writeStreamToSocket();
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
        void handleControlUpdate(osc::ReceivedMessageArgumentStream & args);
        void handleNewChat(osc::ReceivedMessageArgumentStream & args);
        void handleRoles(osc::ReceivedMessageArgumentStream & args);
        void handleSoloUpdate(osc::ReceivedMessageArgumentStream & args);
        void handleJoinMutedUpdated(osc::ReceivedMessageArgumentStream & args);
        void handleStoreKeyResult(osc::ReceivedMessageArgumentStream & args);
        void handleAuthCodeStatus(osc::ReceivedMessageArgumentStream & args);
        void handleAuthCodeUpdated(osc::ReceivedMessageArgumentStream & args);
        void handleAuthCodeEnabled(osc::ReceivedMessageArgumentStream & args);
        void handleNewEncryptionKey(osc::ReceivedMessageArgumentStream & args);
        
    };

#endif
