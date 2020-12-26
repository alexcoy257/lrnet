QT += sql

HEADERS += \
    $$PWD/lrdb_app.h \
    $$PWD/lrdbclient.h \
    $$PWD/lrdbsettings.h

SOURCES += \
    $$PWD/lrdb_app.cpp \
    $$PWD/lrdbclient.cpp \
    $$PWD/lrdbsettings.cpp

DISTFILES += \
    $$PWD/settings.json
