include(../lrnet.pri)


CONFIG += c++11 console
CONFIG += qt link_prl

QT += sql network widgets

!isEmpty(lrnetdeps){
message(using lrnetdeps in $$lrnetdeps)
INCLUDEPATH += $$lrnetdeps/include
LIBS += -L$$lrnetdeps/lib
}else{
message(lrnetdeps not set!)
}

INSTALLS += target
target.path = $$PREFIX/bin/

#LIBS += -lssl -lcrypto
LIBS += -L$$DLLDESTDIR -llrdb_client -lssl -lcrypto -llrnetjackserver -ljack -ljackserver -ljacktrip

TARGET = lrnetserver
HEADERS += lrnetserver.h \
           auth_types.h \
           control_types.h \
           lrnet_member.h \
           lrnet_roster.h \
           lrnetserver_types.h \
           portpool.h \
           sslserver.h \
           auth.h \
           JackTripWorker.h \
           channelStrip.h \
           ../common/oscstreambuffer.h
SOURCES += lrnetserver.cpp \
            lrnet_member.cpp \
            lrnet_roster.cpp \
            portpool.cpp \
            sslserver.cpp \
            lrnetservermain.cpp \
            /usr/local/include/osc/*.cpp \
            auth.cpp \
            JackTripWorker.cpp
