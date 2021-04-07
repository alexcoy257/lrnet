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
  
  void addKeyToDb(const char * key, AuthPacket & pkt);

  public:
  
  Auth(LRdbClient * p_lrdb);
  ~Auth();

  temp_auth_type_t checkCredentials (AuthPacket &pkt);
  bool addKey (const char * key, AuthPacket &pkt);
  std::list<auth_roster_t> * getRoles();
  int getIDforKeyAndAuthPacket(const char * key, AuthPacket & pkt);
  void updatePermission(QString netid, AuthTypeE authType);


  signals:

private:
  LRdbClient * readdb;

public:
  session_id_t genSessionKey();
};

#endif
