#ifndef PORTPOOLTESTS_H
#define PORTPOOLTESTS_H

#include <QObject>
#include <QtTest/QtTest>
#include "portpool.h"

class PortPoolTests: public QObject
{
    Q_OBJECT
    PortPool * pool;

private slots:
    void initTestCase();
    void init();
    void addMember();
    void removeMember();
    void addSeveralMembers();
    void cleanup();
    void cleanupTestCase();
};

#endif // PORTPOOLTESTS_H
