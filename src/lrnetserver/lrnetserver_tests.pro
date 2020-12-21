include(lrnetserver.pro)

QT += testlib

TARGET -= lrnetserver
TARGET += lrnetserver_tests

DEFINES += AUTH_TEST_SHORTCUT

SOURCES -= lrnetservermain.cpp


HEADERS += lrnet_rostertest.h
#    lrnet_commands_test.h
SOURCES += lrnet_rostertest.cpp
#    lrnet_commands_test.cpp
