include(lrnetserver_tests.pri)
HEADERS += lrnet_rostertest.h
SOURCES += lrnet_rostertest.cpp
TARGET += lrnetserver_roster_tests
DEFINES += ROSTER_TEST_NO_SERVER
