CONFIG += c++11 console
CONFIG += qt
QT += gui widgets network

LIBS += -lssl -lcrypto

TARGET = lrnetserver
HEADERS += lrnetserver.h \
           sslserver.h \
           auth.h
SOURCES += lrnetserver.cpp \
            sslserver.cpp \
            lrnetservermain.cpp \
            /usr/local/include/osc/*.cpp \
            auth.cpp