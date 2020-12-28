#include "lrnet_roster.h"
#include <QDebug>

Member::serial_t Member::currentSerial=0;

Member::Member(QString & netid, session_id_t s_id, Roster * roster, QObject * parent): QObject(parent)
  ,s_id(s_id)
  ,serial(currentSerial++)
  ,netid(netid)
    {
    qDebug() <<"Member constructor " <<netid;

}

Member::Member(QObject * parent): QObject(parent)
  ,s_id(0)
  ,serial(0)
  ,netid("")
  ,mRoster(NULL)
    {

}

void Member::setName(QString & nname){
    qDebug() <<"Set name " <<nname;
    name = nname;
}

void Member::setSection(QString & nsection){
    if(mRoster->sections.isEmpty() || mRoster->sections.contains(nsection))
        section = nsection;
}
