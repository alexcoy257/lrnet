#ifndef ROSTERTEST_H
#define ROSTERTEST_H




#include <QObject>
#include <QtTest/QtTest>
#include "lrnet_roster.h"

class RosterTest: public QObject
{
    Q_OBJECT
private slots:
    void addMember();



};


#endif // ROSTERTEST_H
