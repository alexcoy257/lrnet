#ifndef AUTH_TYPES_H
#define AUTH_TYPES_H
#include <stdint.h>
#include <cstddef>
#include <string>
#include <cstring>
#include <cstdlib>


typedef struct __attribute__ ((packed)) {
    char netid[30];
    uint8_t netid_length;
    unsigned char challenge[214]; //256-42 = 214
    unsigned char sig[256];
} auth_packet_t;

class AuthPacket {

public:
    char netid[30];
    uint8_t netid_length;
    unsigned char challenge[214]; //256-42 = 214
    unsigned char sig[256];
    AuthPacket(char * qs_netid){
        netid_length = strlen(qs_netid)>29 ? 29:strlen(qs_netid);
        std::memcpy(netid, qs_netid, netid_length+1);
    }

    AuthPacket(auth_packet_t &pkt){
        std::memcpy(netid, pkt.netid, 30);
        std::memcpy(&netid_length, &pkt.netid_length, 1);
        std::memcpy(challenge, pkt.challenge, 214);
        std::memcpy(sig, pkt.sig, 256);
    }

    ~AuthPacket(){};
    /**
     * @brief pack(pkt) Packs instance's contents into dense struct pkt.
     * @param pkt
     */
    void pack(auth_packet_t & pkt){
        std::memcpy(pkt.netid, netid, 30);
        std::memcpy(&pkt.netid_length, &netid_length, 1);
        std::memcpy(pkt.challenge, challenge, 214);
        std::memcpy(pkt.sig, sig, 256);
    }
};

typedef uint64_t session_id_t;
typedef enum {
      NONE = 0,
      MEMBER = 1,
      CHEF = (1<<1) ,
      SUPERCHEF = (1<<2)
  } AuthTypeE;

typedef struct {
  session_id_t session_id;
  AuthTypeE authType;
} auth_type_t;

typedef struct {
    session_id_t session_id;
    int user_id;
    AuthTypeE authType;
} temp_auth_type_t;

typedef struct {
    std::string netid;
    AuthTypeE authType;
} auth_roster_t;


#endif // AUTH_TYPES_H
