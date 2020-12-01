#include <iostream>
#include <QApplication>

#include <faust/dsp/dsp.h>
#include <faust/gui/meta.h>
#include <faust/gui/QTUI.h>
#include <faust/audio/jack-dsp.h>


#include "channelStrip.h"
//#include "jackChannelStrip.h"

std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

int main(int argc, char *argv[]){
  QApplication MixerApp(argc, argv);
  ChannelStrip * cs = new ChannelStrip();
  QTGUI * u_ChannelStrip = new QTGUI();
  cs->buildUserInterface((UI *) u_ChannelStrip);
  jackaudio audio;
  audio.init("JCS", cs);
  audio.start();

  u_ChannelStrip->QWidget::show();
  u_ChannelStrip->run();
  //JackChannelStrip * jcs = new JackChannelStrip(cs);


  MixerApp.exec();

  audio.stop();
  delete cs;
  delete u_ChannelStrip;
  return 0;
}