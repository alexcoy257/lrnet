DLLDESTDIR=$$PWD/lib
#message($$DLLDESTDIR)
macx{
CONFIG += sdk_no_version_check
LIBS += -L/usr/local/lib/jack \
    -L/usr/local/opt/openssl/lib \
    -L$$DLLDESTDIR
INCLUDEPATH += /usr/local/include \
    /usr/local/opt/openssl/include
}
