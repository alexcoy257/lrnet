#ifndef LRS_AUTH_H
#define LRS_AUTH_H

#include <openssl/rand.h>
#include <QObject>
#include <QDebug>
#include <QVector>


class Auth : public QObject{
  Q_OBJECT;
  
  
  public:
  typedef uint64_t session_id_t;
  typedef enum {
        NONE = 1 << 16,
        CHEF = 2 << 16,
        MEMBER = 3 << 16
    } AuthTypeE;
  
  typedef struct {
    session_id_t session_id;
    AuthTypeE authType;
  } auth_type_t;
  
  Auth();
  ~Auth();
  auth_type_t checkCredentials (QByteArray& key);

  private:
  session_id_t genSessionKey();
};

#endif