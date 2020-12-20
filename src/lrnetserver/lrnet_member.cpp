#include "lrnet_roster.h"

Member::Member(QString & netid, session_id_t s_id, QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,netid(netid)
    {

}
