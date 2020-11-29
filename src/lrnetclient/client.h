#include <QObject>
#include <QSslSocket>
#include <osc/OscOutboundPacketStream.h>

#define OUTPUT_BUFFER_SIZE 1024

class Client : public QObject
    {
        Q_OBJECT
    
    public:
        Client(const QString &host, int port);
        ~Client();
    
    signals:
        void responseReceived();
    
    private slots:
        void waitForGreeting();
        void readResponse();
    
    private:
        QSslSocket *socket;
        char buffer[OUTPUT_BUFFER_SIZE];
        osc::OutboundPacketStream oscOutStream;
    };