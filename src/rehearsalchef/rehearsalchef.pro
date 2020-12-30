include(../lrnet.pri)

QT       += core gui testlib network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

win32{
INCLUDEPATH += C:\Users\alexc\Documents\libjacktrip_libs\include
LIBS += -LC:\Users\alexc\Documents\libjacktrip_libs\lib
INCLUDEPATH += "C:\Program Files\JACK2\include"
}

LIBS += -L../lrnetclient -llrnetclient -llrnetjackservertest -ljacktrip

!win32{
LIBS += -lcrypto
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    channelstrip.cpp \
    channeltester.cpp \
    chatform.cpp \
    chefform.cpp \
    compressor.cpp \
    launcher.cpp \
    lrchef_connectform.cpp \
    main.cpp \
    mainwindow.cpp \
    memberform.cpp \
    rc_tests_1.cpp \
    rcjtworker.cpp

HEADERS += \
    RehearsalChefabout.h \
    channelstrip.h \
    channeltester.h \
    chatform.h \
    chefform.h \
    compressor.h \
    launcher.h \
    lrchef_connectform.h \
    mainwindow.h \
    memberform.h \
    rcjtworker.h \
    testworker.h


FORMS += \
    channelStrip.ui \
    channeltester.ui \
    chatform.ui \
    chefform.ui \
    compressor.ui \
    mainwindow.ui \
    memberform.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
