#ifndef LRDBAPP_H
#define LRDBAPP_H

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QSharedPointer>

#include "lrdbclient.h"

class LRdbApp : public QCoreApplication
{
    QCommandLineParser parser;
    QSharedPointer<LRdbClient> client;
public:
    //using QCoreApplication::QCoreApplication;
    LRdbApp(int argc, char ** argv);
};

#endif // LRDBAPP_H
