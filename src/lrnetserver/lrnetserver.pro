include(../lrnet.pri)


CONFIG += c++11 console
CONFIG += qt

QT += sql network widgets


#LIBS += -lssl -lcrypto
LIBS += -L$$DLLDESTDIR -llrdb_client -lssl -lcrypto -llrnetjackserver -ljack -ljackserver -ljacktrip

TARGET = lrnetserver
HEADERS += lrnetserver.h \
           auth_types.h \
           lrnet_member.h \
           lrnet_roster.h \
           portpool.h \
           sslserver.h \
           auth.h \
           JackTripWorker.h
SOURCES += lrnetserver.cpp \
            lrnet_member.cpp \
            lrnet_roster.cpp \
            portpool.cpp \
            sslserver.cpp \
            lrnetservermain.cpp \
            /usr/local/include/osc/*.cpp \
            auth.cpp \
            JackTripWorker.cpp
