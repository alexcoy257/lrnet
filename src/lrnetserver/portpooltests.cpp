#include "portpooltests.h"
#include <QMetaObject>

void PortPoolTests::initTestCase(){
    pool = new PortPool(61002, 61200);
    QVERIFY2(pool != NULL, "Pool is null");
    QVERIFY2(pool->getCurrentBoolAddress() != NULL, "Pool array is null");
}

void PortPoolTests::init(){

}


void PortPoolTests::cleanup(){

}


void PortPoolTests::addMember()
{
    int aPort = pool->getPort();
    QVERIFY2(pool->getCurrentBoolAddress() != NULL, "Pool array is null");
    QVERIFY2(pool->getCurrentCBoolAddress() != NULL, "Pool current is null");
    QCOMPARE(aPort, 61002);


}

void PortPoolTests::removeMember(){

    pool->returnPort(61002);

    int aPort = pool->getPort();

    QCOMPARE(aPort, 61002);


}

void PortPoolTests::addSeveralMembers(){
    {int aPort = pool->getPort();
        QCOMPARE(aPort, 61003);
    }
    {int aPort = pool->getPort();
        QCOMPARE(aPort, 61004);
    }
    {pool->returnPort(61003);
    }
    {int aPort = pool->getPort();
        QCOMPARE(aPort, 61003);
    }
    {int aPort = pool->getPort();
        QCOMPARE(aPort, 61005);
    }
}

void PortPoolTests::cleanupTestCase(){

}

QTEST_MAIN(PortPoolTests)


