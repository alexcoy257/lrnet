CONFIG += c++11 console
CONFIG += qt
QT += gui widgets network


TEMPLATE = lib
LIBS += -lssl -lcrypto

TARGET = lrnetclient

HEADERS += lrnetclient.h \
          ../lrnetserver/auth.h
SOURCES += lrnetclient.cpp \
           /usr/local/include/osc/*.cpp \
          ../lrnetserver/auth.cpp \
          lrnetclientmain.cpp
