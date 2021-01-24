#ifndef LRS_AUTH_H
#define LRS_AUTH_H

//namespace ossl{
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
//}

#include <QObject>
#include <QDebug>
#include <QVector>
#include "../lrdb_client/lrdbclient.h"

#include "auth_types.h"



class Auth : public QObject{

  Q_OBJECT;
  
  LRdbClient readdb;
  void addKeyToDb(const char * key, AuthPacket & pkt);

  public:
  
  Auth();
  ~Auth();

  auth_type_t checkCredentials (AuthPacket &pkt);
  bool addKey (const char * key, AuthPacket &pkt);
  std::list<auth_roster_t> * getRoles();
  void updatePermission(QString netid, AuthTypeE authType);
  void removeUser(QString netid, AuthTypeE authType);


  signals:

public:
  session_id_t genSessionKey();
};

#endif
