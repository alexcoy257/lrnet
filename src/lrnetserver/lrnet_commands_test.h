#ifndef COMMANDSTEST_H
#define COMMANDSTEST_H




#include <QObject>
#include <QtTest/QtTest>
#include "lrnetserver.h"

class CommandsTest: public QObject
{
    Q_OBJECT
    QVector<QMetaObject::Connection> * connections;
private slots:
    void initTestCase();
    void init();
    void emptyTest(){QVERIFY(true);};
    void cleanup();
    void cleanupTestCase();



};


#endif // ROSTERTEST_H
