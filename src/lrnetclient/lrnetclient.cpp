#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "client.h"


Client::Client(const QString &host, int port)
{
    socket = new QSslSocket();
    connect(socket, SIGNAL(connected()), SLOT(waitForGreeting()));
    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    connect(socket, SIGNAL(disconnected()), qApp, SLOT(quit()));
    socket->connectToHost(host, port);
}
    

void Client::waitForGreeting()
{
    qDebug("Connected; now waiting for the greeting");
}


void Client::readResponse()
{
    if (socket->canReadLine()) {
        qDebug("Received: %s", socket->readLine());
        socket->close();
    }
}


int main(int argc, char *argv[])
    {
        QApplication app(argc, argv, false);
        Client client("localhost", 4463);
        return app.exec();
    }