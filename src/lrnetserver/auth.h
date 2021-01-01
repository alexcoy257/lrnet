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

  public:
  
  Auth();
  ~Auth();

  auth_type_t checkCredentials (AuthPacket &pkt);


  signals:

public:
  session_id_t genSessionKey();
};

#endif
