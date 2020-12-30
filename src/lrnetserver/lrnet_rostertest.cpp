#include "lrnet_rostertest.h"
#include <QMetaObject>

void RosterTest::initTestCase(){

}

void RosterTest::init(){
    connections = new QVector<QMetaObject::Connection>();
}


void RosterTest::cleanup(){
    for (QMetaObject::Connection c:*connections)
        disconnect(c);
    delete connections;
}


void RosterTest::addMember()
{
    QString netid = "ac2456";
    bool gotSignal = false;

    connections->append(connect(&roster, &Roster::sigMemberUpdate, this, [&](Member * mem){
        if (mem->getNetID().compare("ac2456") == 0){
            gotSignal=true;
        }}));

    roster.addMember(netid, 1);
    QVERIFY2(gotSignal, "Didn't get memberAdded signal with member ac2456.");

    gotSignal = false;
    roster.addMember(netid, 1);
    QVERIFY2(!gotSignal, "Got signal for member with duplicate sessionI");



    for (Member * m:roster.getMembers()){
        QCOMPARE(m->getNetID(), "ac2456");
        QCOMPARE(m->getSerialID(), 0UL);
    }

    netid = "aa1111";
    roster.addMember(netid, 3);
    QCOMPARE(roster.getMembers()[1]->getSerialID(), 1UL);
}

void RosterTest::removeMember(){

    bool gotSignal = false;
    connections->append(connect(&roster, &Roster::memberRemoved, this, [&](Member::serial_t id){
        if (id == 1){
            gotSignal=true;
        }}));

    roster.removeMemberBySerialID((Member::serial_t)1);
    QVERIFY2(gotSignal, "Didn't get memberRemoved signal with member 1.");

    roster.removeMemberBySerialID((Member::serial_t)0);
}

void RosterTest::memberUdpPortReturn(){

    QString netid = "ac2456";
    int i=0;
    int ports[4];
    connections->append(connect(&roster, &Roster::sigMemberUpdate, this, [&](Member * mem){
        ports[i++]=mem->getPort();
                        }));

    roster.addMember(netid, 1);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);
    qDebug() <<"Add member ac2456,1";
    roster.removeMemberBySessionID((session_id_t)1);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);
    qDebug() <<"Remvoe member ac2456, 1";

    roster.addMember(netid, 2);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);

    roster.addMember(netid, 3);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);

    roster.removeMemberBySessionID((session_id_t)2);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);
    roster.addMember(netid, 4);
    QAbstractEventDispatcher::instance()->processEvents(QEventLoop::AllEvents);

    QCOMPARE(ports[0], 61002);
    QCOMPARE(ports[1], 61002);
    QCOMPARE(ports[2], 61003);
    QCOMPARE(ports[3], 61002);
}

void RosterTest::cleanupTestCase(){

}

QTEST_MAIN(RosterTest)
//#include "lrnet_rostertest.moc"
