#include "auth.h"

Auth::Auth():
readdb("lrnetread", "lrnetreadpw", "lrnetdb", "localhost", nullptr)
{


  unsigned char testRand;
  if(!RAND_bytes(&testRand, 1)){
    qDebug() <<"Machine doesn't support rand_bytes";
  }
  qDebug() <<"Using openssl RAND_bytes";
}

Auth::~Auth(){

}


/**
 * @brief Auth::genSessionKey
 * @return
 * nonzero if the generation was succesful. Zero if something failed.
 */
session_id_t Auth::genSessionKey(){
  session_id_t id;
    if (RAND_bytes(reinterpret_cast<unsigned char *>(&id), sizeof(session_id_t)))
      return id;
    else
      return 0;
}

auth_type_t Auth::checkCredentials (AuthPacket & pck)
{
#ifdef AUTH_TEST_SHORTCUT
    if (std::strcmp(pck.netid, "superchef") == 0)
        return {1, SUPERCHEF};
    if (std::strcmp(pck.netid, "chef") == 0)
        return {1, SUPERCHEF};
    if (std::strcmp(pck.netid, "memberchef") == 0)
        return {1, SUPERCHEF};
    return {0, NONE};
#endif

    QScopedPointer<QVector<int>> ids(readdb.getIDsForNetid(pck.netid, pck.netid_length));

    if (ids->isEmpty()){
        return {0,NONE};
    }

    BIO * t_pub = BIO_new(BIO_s_mem());
    RSA * pubkey = RSA_new();

    AuthTypeE role = NONE;

    for (int id:*ids){
        qDebug() <<"Checking uniqueid " <<id;
        QByteArray * key = readdb.getKeyForID(id);

        qDebug() <<"Got key from db:" <<*key <<"of length " <<key->length();

        int nw = BIO_write(t_pub, key->data(), key->length());
        if (nw < 1){
            qDebug() << "BIO write failed somehow.";
        }

        RSA * res = PEM_read_bio_RSA_PUBKEY(t_pub, &pubkey, NULL, NULL);

        if (!res){
            qDebug() << "RSA read failed somehow.";
        }
        //qDebug() <<"RSA Size: " <<RSA_size(pubkey);

        int verified = RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, 256, pubkey);
        if (verified) {
            QString * srole = readdb.getRoleForID(id);
            if (srole){
                qDebug() <<"Got a role: " <<*srole <<" to superchef: " <<srole->compare("superchef");
            }
            if (QString::compare(*srole, "chef") == 0){
                role = CHEF;
            }
            if (srole->compare("superchef") == 0){
                qDebug() <<"Set to superchef.";
                role = SUPERCHEF;
            }
            if (QString::compare(*srole, "user") == 0){
                role = MEMBER;
            }
             break;
        }
    }

    BIO_free(t_pub);
    RSA_free(pubkey);

    if (role != NONE){
      return {genSessionKey(), role};
    }

    qDebug() <<"Challenge failed";
    return {0, NONE};
}
