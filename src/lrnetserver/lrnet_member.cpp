#include "lrnet_roster.h"

Member::serial_t Member::currentSerial=0;

Member::Member(QString & netid, session_id_t s_id, QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,serial(currentSerial++)
  ,netid(netid)
    {

}

Member::Member(QObject * parent): QObject(parent)
  ,s_id(0)
  ,serial(0)
  ,netid("")
    {

}
