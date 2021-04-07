#include "auth.h"

/*
using ossl::RSA;
using ossl::RSA_new;
using ossl::RSA_free;

using ossl::BIO;
using ossl::BIO_s_mem;
using ossl::BIO_new;
using ossl::BIO_free;

using ossl::RAND_bytes;
*/

Auth::Auth(LRdbClient * p_lrdb){

  readdb = p_lrdb;
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

temp_auth_type_t Auth::checkCredentials (AuthPacket & pck)
{
    //Need to rewrite to accomodate user id
//#if false //def AUTH_TEST_SHORTCUT
//    if (std::strcmp(pck.netid, "superchef") == 0)
//        return {1, SUPERCHEF};
//    if (std::strcmp(pck.netid, "chef") == 0)
//        return {1, SUPERCHEF};
//    if (std::strcmp(pck.netid, "memberchef") == 0)
//        return {1, SUPERCHEF};
//    return {0, NONE};
//#endif
    //

    //qDebug() <<"Checking credentials for " <<pck.netid;
    QScopedPointer<QVector<int>> ids(readdb->getIDsForNetid(pck.netid, pck.netid_length));

    if (ids->isEmpty()){
        return {0,0,NONE};
    }

    BIO * t_pub = BIO_new(BIO_s_mem());
    RSA * pubkey = RSA_new();

    AuthTypeE role = NONE;
    int user_id = -1;

    for (int id:*ids){
        qDebug() <<"Checking uniqueid " <<id;
        QByteArray * key = readdb->getKeyForID(id);

        qDebug() <<"Got key from db.";// <<*key <<"of length " <<key->length();

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
            QString * srole = readdb->getRoleForID(id);
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
            user_id = id;
            break;
        }
    }

    BIO_free(t_pub);
    RSA_free(pubkey);

    if (role != NONE){
      return {genSessionKey(),user_id,role};
    }

    qDebug() <<"Challenge failed";
    return {0,0,NONE};
}

bool Auth::addKey(const char * key, AuthPacket & pck){
    qDebug() <<QByteArray(key, 451);
    BIO * t_pub = BIO_new(BIO_s_mem());
    RSA * pubkey = NULL;
     bool err = false;
    int nw = BIO_write(t_pub, key, 451);
    if (nw < 1){
        qDebug() << "BIO write failed somehow.";
        err = true;

    }
    if(!err){
        qDebug() << "bio_read rsa key";
        if(!PEM_read_bio_RSA_PUBKEY(t_pub, &pubkey, NULL, NULL)){
            err = true;
            qDebug() << "PEM read failed somehow";
        }
        if (!err){
        int verified = RSA_verify(NID_sha256, pck.challenge, 214, pck.sig, 256, pubkey);
        if(verified){
            addKeyToDb(key, pck);
        }else{
            qDebug() <<"Not RSA Verified";
            err = true;
        }

        RSA_free(pubkey);
    }}

    BIO_free(t_pub);

    return err;
}

std::list<auth_roster_t> * Auth::getRoles(){
    return readdb->getRoles();
}

int Auth::getIDforKeyAndAuthPacket(const char * key, AuthPacket & pkt){
    QByteArray bakey(key, 451);
    QString netid = QString::fromLocal8Bit(pkt.netid, pkt.netid_length);
    return readdb->getIDforKeyAndNetID(bakey, netid);
}


void Auth::updatePermission(QString netid, AuthTypeE authType){
    readdb->setRoleForNetID(authType, netid);
}

void Auth::removeUser(QString netid){
    readdb->removeUser(netid);
}

void Auth::addKeyToDb(const char * key, AuthPacket & pkt){
    qDebug() <<"Verified key, adding to db if not present";
    QByteArray bakey(key, 451);
    QString netid = QString::fromLocal8Bit(pkt.netid, pkt.netid_length);
    readdb->addKeyToNetid(bakey, netid);
}
