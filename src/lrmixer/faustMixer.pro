#message(Building channel strip)
#system(faust $$PWD/faustTest.dsp -o channelStrip.h -cn ChannelStrip)

CONFIG += c++11 console

CONFIG += qt

QT += gui widgets



FAUSTINC = /usr/local/include/faust/


INCLUDEPATH += $$FAUSTINC
message($$FAUSTINC)

LIBS += -ljack


lrmixer{
    TARGET = lrmixer
    SOURCES += lrmixer.cpp
    HEADERS += $$FAUSTINC/gui/QTUI.h
    HEADERS += $$FAUSTINC/gui/GUI.h

    HEADERS += channelStrip.h
    #HEADERS += jackChannelStrip.h

    #SOURCES += jackChannelStrip.cpp
}

patchtest{
  TARGET = patchtest
  SOURCES += patchTest.cpp
  HEADERS += Patcher.h \
#             $$FAUSTINC/gui/QTUI.h \
             $$FAUSTINC/gui/GUI.h \
             channelStrip.h
  SOURCES += Patcher.cpp \
             EnsMember.cpp
}




RESOURCES += $$FAUSTINC/gui/Styles/Grey.qrc