#include "lrnet_commands_test.h"
#include <QMetaObject>

void CommandsTest::initTestCase(){

}

void CommandsTest::init(){
    connections = new QVector<QMetaObject::Connection>();
}


void CommandsTest::cleanup(){
    for (QMetaObject::Connection c:*connections)
        disconnect(c);
    delete connections;
}

void CommandsTest::cleanupTestCase(){

}

QTEST_MAIN(CommandsTest)
