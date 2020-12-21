#include "lrnet_rostertest.h"


void RosterTest::initTestCase(){

}

void RosterTest::addMember()
{
    QString netid = "ac2456";
    bool gotSignal = false;
    connect(&roster, &Roster::memberAdded, this, [&](Member * mem){
        if (mem->getNetID().compare("ac2456") == 0){
            gotSignal=true;
        }});

    roster.addMember(netid, 1);
    QVERIFY2(gotSignal, "Didn't get memberAdded signal with member ac2456.");

    for (Member * m:roster.getMembers()){
        QCOMPARE(m->getNetID(), "ac2456");
        QCOMPARE(m->getSerialID(), 0);
    }

    netid = "aa1111";
    roster.addMember(netid, 3);
    QCOMPARE(roster.getMembers()[1]->getSerialID(), 1);
}

void RosterTest::removeMember(){

}

void RosterTest::cleanupTestCase(){

}

QTEST_MAIN(RosterTest)
//#include "lrnet_rostertest.moc"
