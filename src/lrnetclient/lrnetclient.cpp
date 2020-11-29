#include <iostream>
#include <QObject>
#include <QSslSocket>
#include <QApplication>
#include "client.h"


Client::Client(const QString &host, int port):
oscOutStream( buffer, OUTPUT_BUFFER_SIZE )
{
    socket = new QSslSocket();
    connect(socket, SIGNAL(connected()), SLOT(waitForGreeting()));
    connect(socket, SIGNAL(readyRead()), SLOT(readResponse()));
    connect(socket, SIGNAL(disconnected()), qApp, SLOT(quit()));
    socket->connectToHost(host, port);
}

Client::~Client(){
    socket->disconnect();
}
    

void Client::waitForGreeting()
{   
    QString temp = "Hi";
    oscOutStream << osc::BeginBundleImmediate
        << osc::BeginMessage( "/test1" ) 
            << true << 23 << (float)3.1415 << "hello" << osc::EndMessage
        << osc::BeginMessage( "/test2" ) 
            << true << 24 << (float)10.8 << "world" << osc::EndMessage
        << osc::EndBundle;
    socket->write(oscOutStream.Data(), oscOutStream.Size());
    qDebug("Connected; now waiting for the greeting");
}


void Client::readResponse()
{
    if (socket->canReadLine()) {
        QString line = socket->readLine();
        qDebug() <<"Received: " <<line ;
        std::cout <<line.toStdString();
        //socket->close();
    }
}


int main(int argc, char *argv[])
    {
        QApplication app(argc, argv, false);
        Client client("localhost", 4463);
        return app.exec();
    }