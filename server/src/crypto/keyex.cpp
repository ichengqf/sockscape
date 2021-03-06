#include "keyex.hpp"

sosc::BigUInt sosc::cgc::KeyExchange::secret;

sosc::cgc::KeyExchange::KeyExchange() {
    if(this->secret.IsZero())
        this->secret = FastRandomPrime();
            
    this->modulus = FastRandomPrime();
}

sosc::Packet sosc::cgc::KeyExchange::GenerateRequest() const {
    return Packet(1, {
        this->generator.ToString(),
        this->modulus.ToString(),
        BigUInt::ModPow(this->generator, this->secret, this->modulus)
    });
}

bool sosc::cgc::KeyExchange::ParseRequest
    (const Packet& request, Packet* response, uint8_t id)
{
    if(request.GetId() != 1 || request.RegionCount() != 3)
        return false;
    
    BigUInt generator, modulus, public_key;
    bool check = generator.Parse(request[0]);
    check = check || modulus.Parse(request[1]);
    check = check || public_key.Parse(request[2]);
    
    if(!check)
        return false;
    
    this->private_key = BigUInt::ModPow(public_key, this->secret, modulus);
    public_key = BigUInt::ModPow(generator, this->secret, modulus);
    *response = Packet(id, { public_key.ToString() });
    
    return true;
}

bool sosc::cgc::KeyExchange::ParseResponse(const Packet& response) {
    if(response.GetId() != 1 || response.RegionCount() != 1)
        return false;
    
    BigUInt public_key;
    if(!public_key.Parse(response[0]))
        return false;
    
    this->private_key = 
        BigUInt::ModPow(public_key, this->secret, this->modulus);
        
    return true;
}

sosc::BigUInt sosc::cgc::KeyExchange::FastRandomPrime() {
    BigUInt prime;
    for(int i = 0; i < this->key_size_bytes; i += 16)
        prime += BigUInt::GenerateRandomPrime(16) << (i * 8);
    
    return prime;
}
    
