CONFIG += c++11 console
CONFIG += qt
QT += gui widgets network

TARGET = lrnetserver
HEADERS += lrnetserver.h \
           sslserver.h
SOURCES += lrnetserver.cpp \
            sslserver.cpp \
            lrnetservermain.cpp \
            /usr/local/include/osc/*.cpp \