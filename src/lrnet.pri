CONFIG(debug, debug|release){
DLLDESTDIR=$$PWD/lib/debug
}else{
DLLDESTDIR=$$PWD/lib/release
}
LIBINCDIR=$$PWD/include

!isEmpty(lrnetdeps){
message(using lrnetdeps in $$lrnetdeps)
INCLUDEPATH += $$lrnetdeps/include
LIBS += -L$$lrnetdeps/lib
}else{
message(lrnetdeps not set!)
}

INCLUDEPATH += $$LIBINCDIR
LIBS += -L$$DLLDESTDIR
HEADERS += $$PWD/liblrnet_globals.h

# isEmpty(PREFIX) will allow path to be changed during the command line
# call to qmake, e.g. qmake PREFIX=/usr
isEmpty(PREFIX) {
 PREFIX = /usr/local
}

message(Installing to $$PREFIX)

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
#CONFIG += static
#DEFINES += STATIC_LRNET
#LIBS += "C:/Qt/Tools/OpenSSL/Win_x64/lib/libcrypto.lib"
#INCLUDEPATH += C:/Qt/Tools/OpenSSL/Win_x64/include
INCLUDEPATH += C:/msys64/mingw64/include
LIBS += -LC:/msys64/mingw64/lib
LIBS += -lcrypto
#LIBS += "C:/msys64/mingw64/lib/libcrypto.a"

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

FORMS +=

SOURCES +=

RESOURCES += $$PWD/lrnet.qrc
