#ifndef ROSTERTEST_H
#define ROSTERTEST_H




#include <QObject>
#include <QtTest/QtTest>
#include "lrnet_roster.h"

class RosterTest: public QObject
{
    Q_OBJECT
    Roster roster;

private slots:
    void initTestCase();
    void addMember();
    void removeMember();
    void cleanupTestCase();



};


#endif // ROSTERTEST_H
