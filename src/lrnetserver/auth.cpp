#include "auth.h"

Auth::Auth(){
  unsigned char testRand;
  if(!RAND_bytes(&testRand, 1)){
    qDebug() <<"Machine doesn't support rand_bytes";
  }
  qDebug() <<"Using openssl RAND_bytes";
}

Auth::~Auth(){

}



Auth::session_id_t Auth::genSessionKey(){
  session_id_t id;
    if (RAND_bytes(reinterpret_cast<unsigned char *>(&id), sizeof(session_id_t)))
      return id;
    else
      return 0;
}

Auth::auth_type_t Auth::checkCredentials (QByteArray& key)
{
    if (key.isEmpty()) {
        return {0, NONE};
    }

    if (key.length() < 32){
      return {0, NONE};
    }

    if (QString::compare(QString(key), "cornell1000000000000000000000000")==0){
      return {genSessionKey(), CHEF};
    }

    qDebug() <<"Wrong credential" <<QString(key) <<" needed " <<QString("cornell1000000000000000000000000");
    return {0, NONE};
    
}
