DLLDESTDIR=$$PWD/lib
LIBS += -L$$DLLDESTDIR
#message($$DLLDESTDIR)
macx{
CONFIG += sdk_no_version_check
LIBS += -L/usr/local/lib/jack \
    -L/usr/local/opt/openssl/lib \

INCLUDEPATH += /usr/local/include \
    /usr/local/opt/openssl/include
}
