DLLDESTDIR=$$PWD/lib
LIBINCDIR=$$PWD/include

INCLUDEPATH += $$LIBINCDIR
LIBS += -L$$DLLDESTDIR
HEADERS += $$PWD/liblrnet_globals.h


#message($$DLLDESTDIR)
macx{
CONFIG += sdk_no_version_check
LIBS += -L/Volumes/Alex_Coy_Projects_2/jack2/lib \
    -L/Volumes/Alex_Coy_Projects_2/openssl-bin/lib
#    -L/usr/local/lib

INCLUDEPATH += /usr/local/include \
    /Volumes/Alex_Coy_Projects_2/openssl-bin/include
DEFINES += __MAC_OSX__
}

win32{
LIBS += "C:/Qt/Tools/OpenSSL/Win_x64/lib/libcrypto.lib"

INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/include

OSCDIR = C:/Qt/Tools/osc/osc
INCLUDEPATH += C:/Qt/Tools/osc
SOURCES += $$OSCDIR/OscOutboundPacketStream.cpp \
    $$OSCDIR/OscReceivedElements.cpp \
    $$OSCDIR/OscTypes.cpp

DEFINES += __WIN32__
DEFINES += __WIN_32__
}

linux{
DEFINES += __LINUX__
}

DEFINES += WAIRTOHUB
