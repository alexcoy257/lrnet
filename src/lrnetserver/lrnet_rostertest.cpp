#include "lrnet_rostertest.h"


void RosterTest::addMember()
{
    Roster roster;
    QString netid = "ac2456";
    roster.addMember(netid, 1);
    for (Member * m:roster.getMembers()){
        QCOMPARE(m->getNetID(), "ac2456");
    }
}

QTEST_MAIN(RosterTest)
//#include "lrnet_rostertest.moc"
