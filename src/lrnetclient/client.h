#include <QObject>
#include <QSslSocket>

class Client : public QObject
    {
        Q_OBJECT
    
    public:
        Client(const QString &host, int port);
    
    signals:
        void responseReceived();
    
    private slots:
        void waitForGreeting();
        void readResponse();
    
    private:
        QSslSocket *socket;
    };