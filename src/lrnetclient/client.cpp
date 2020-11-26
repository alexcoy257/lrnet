#include "client.h"
Client::Client(const QString &host, int port)
{
    socket = new QSslSocket();
    connect(socket, SIGNAL(connected()), SLOT(waitForGreeting()));
    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    connect(socket, SIGNAL(connectionClosed()), qApp, SLOT(quit()));
    connect(socket, SIGNAL(delayedCloseFinished()), qApp, SLOT(quit()));
    socket->connectToHost(host, port);
}
    

void Client::waitForGreeting()
{
    qDebug("Connected; now waiting for the greeting");
}


void Client::readResponse()
{
    if (socket->canReadLine()) {
        qDebug("Received: %s", socket->readLine().latin1());
        socket->close();
    }
}
    