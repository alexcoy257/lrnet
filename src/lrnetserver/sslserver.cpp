#include "sslserver.h"

SslServer::SslServer(QObject* parent) :
    QTcpServer(parent)
{}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    //Override of the QTcpServer incomingConnection function to create
    //an SslSocket rather than a regular socket.
    QSslSocket *sslSocket = new QSslSocket(this);
    sslSocket->setSocketDescriptor(socketDescriptor);
    sslSocket->setLocalCertificate(m_certificate);
    sslSocket->setPrivateKey(m_privateKey);
    this->addPendingConnection(sslSocket);
}

void SslServer::setCertificate (const QSslCertificate &certificate)
{
    m_certificate = certificate;
}

void SslServer::setPrivateKey (const QSslKey &key)
{
    m_privateKey = key;
}

SslServer::~SslServer() = default;