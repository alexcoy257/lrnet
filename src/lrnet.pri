DLLDESTDIR=$$PWD/lib
LIBS += -L$$DLLDESTDIR
HEADERS += $$PWD/liblrnet_globals.h


#message($$DLLDESTDIR)
macx{
CONFIG += sdk_no_version_check
LIBS += -L/usr/local/lib/jack \
    -L/usr/local/opt/openssl/lib \

INCLUDEPATH += /usr/local/include \
    /usr/local/opt/openssl/include
}

win32{
LIBS += "C:/Qt/Tools/OpenSSL/Win_x64/lib/libcrypto.lib"

INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/include

OSCDIR = C:/Qt/Tools/osc/osc
INCLUDEPATH += C:/Qt/Tools/osc
SOURCES += $$OSCDIR/OscOutboundPacketStream.cpp \
    $$OSCDIR/OscReceivedElements.cpp \
    $$OSCDIR/OscTypes.cpp
}
