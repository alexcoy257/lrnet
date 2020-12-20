include(../lrnet.pri)

DESTDIR=$$DLLDESTDIR

CONFIG += c++11 console
CONFIG += qt
QT += gui widgets network sql


TEMPLATE = lib
LIBS += -lssl -lcrypto

TARGET = lrnetclient

HEADERS += lrnetclient.h \
          ../lrnetserver/auth_types.h
SOURCES += lrnetclient.cpp \
           /usr/local/include/osc/*.cpp \
          #../lrnetserver/auth.cpp \
          lrnetclientmain.cpp
