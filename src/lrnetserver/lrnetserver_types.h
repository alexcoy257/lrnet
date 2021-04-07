#ifndef LRNETSERVER_TYPES_H
#define LRNETSERVER_TYPES_H
#include <QSslSocket>
#include "auth_types.h"
#include "control_types.h"

namespace LRServer_types{
/**
  * This type represents a session and which connection
  * that session was last seen on. This type is useful for
  * for publishers to publish messages to subscribers.
  */
typedef struct {
    session_id_t session_id;
    int user_id;
    QSslSocket * lastSeenConnection;
    bool ShasCheckedIn;
    AuthTypeE role;
    AuthTypeE subcribedRole;
    char netid[31];
}sessionTriple;
}

#endif // LRNETSERVER_TYPES_H
