#include <iostream>
#include <unistd.h>
#include <jack/jack.h>
#include <Patcher.h>
#include <QCoreApplication>
#include <QScopedPointer>

using std::cout;

std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

int main(int argc, char *argv[]){
  qRegisterMetaType<jack_port_id_t>("jack_port_id_t");

  QCoreApplication app(argc, argv);
  QScopedPointer<Patcher> thePatcher(new Patcher());

  cout << "Hello World \n";

  Q_UNUSED(thePatcher);
  

  return app.exec();
}
