#ifndef SOSC_CRYPTO_KEYEX_H
#define SOSC_CRYPTO_KEYEX_H

#include "../utils/bigint.hpp"
#include "../sock/packet.hpp"

namespace sosc {
namespace cgc {
class KeyExchange {
public:
    KeyExchange();
    
    Packet GenerateRequest() const;
    bool ParseRequest(const Packet& request, Packet* response, uint8_t id);
    bool ParseResponse(const Packet& response);
    
    inline bool Succeeded() const {
        return !this->private_key.IsZero();
    }
    
    inline const BigUInt& GetPrivateKey() const {
        return this->private_key;
    }
    
    const int key_size = 512;
    const int key_size_bytes = key_size / 8;
private:
    BigUInt FastRandomPrime();
    
    const BigUInt generator = BigUInt(2u);
    static BigUInt secret;
    BigUInt modulus;
    
    BigUInt private_key;
};
}}

#endif
