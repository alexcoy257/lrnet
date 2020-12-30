#ifndef ROSTERTEST_H
#define ROSTERTEST_H




#include <QObject>
#include <QtTest/QtTest>
#include "lrnet_roster.h"

class RosterTest: public QObject
{
    Q_OBJECT
    Roster roster;
    QVector<QMetaObject::Connection> * connections;
private slots:
    void initTestCase();
    void init();
    void addMember();
    void removeMember();
    void memberUdpPortReturn();
    void cleanup();
    void cleanupTestCase();



};


#endif // ROSTERTEST_H
