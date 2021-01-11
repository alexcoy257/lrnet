include(lrdb_client.pri)
include(../lrnet.pri)

DESTDIR=$$DLLDESTDIR
#message($$DESTDIR)

TEMPLATE = lib

INSTALLS += target
target.path = $$PREFIX/lib/

