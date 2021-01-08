include(../lrnet.pri)

DESTDIR=$$DLLDESTDIR

CONFIG += c++11 console
CONFIG += qt #staticlib
QT += gui widgets network sql


DEFINES += LIBLRNET_LIBRARY
TEMPLATE = lib
!win32{
LIBS += -lssl -lcrypto
}

TARGET = lrnetclient

INSTALLS += target
target.path = $$PREFIX/lib/

HEADERS += lrnetclient.h \
          ../lrnetserver/auth_types.h
SOURCES += lrnetclient.cpp \
          #../lrnetserver/auth.cpp
          lrnetclientmain.cpp

!win32{
SOURCES += /usr/local/include/osc/*.cpp
}


