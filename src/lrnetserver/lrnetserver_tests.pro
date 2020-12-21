include(lrnetserver.pro)

QT += testlib

TARGET -= lrnetserver
TARGET += lrnetserver_tests

SOURCES -= lrnetservermain.cpp


HEADERS += lrnet_rostertest.h
SOURCES += lrnet_rostertest.cpp
