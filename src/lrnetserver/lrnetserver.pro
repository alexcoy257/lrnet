include(../lrnet.pri)


CONFIG += c++11 console
CONFIG += qt

QT += sql network widgets


#LIBS += -lssl -lcrypto
LIBS += -L$$DLLDESTDIR -llrdb_client -lssl -lcrypto -llrnetjackservertest -ljack -ljackserver -ljacktrip

TARGET = lrnetserver
HEADERS += lrnetserver.h \
           auth_types.h \
           lrnet_roster.h \
           sslserver.h \
           auth.h \
           JackTripWorker.h
SOURCES += lrnetserver.cpp \
            lrnet_member.cpp \
            lrnet_roster.cpp \
            sslserver.cpp \
            lrnetservermain.cpp \
            /usr/local/include/osc/*.cpp \
            auth.cpp \
            JackTripWorker.cpp
