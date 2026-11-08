#pragma once

#include "crypt.h"

class SRMCRYPT_API DerivedCtrCipher {
public:
    DerivedCtrCipher(
        const std::vector<uint8_t>& masterKey,
        const std::vector<uint8_t>& context,
        const std::vector<uint8_t>& nonce);

    std::vector<uint8_t> encrypt(
        const std::vector<uint8_t>& data,
        uint32_t counter);

private:

    std::vector<uint8_t> deriveKey(
        uint32_t counter);

    std::vector<uint8_t> buildCounterBlock(
        uint32_t counter);

    std::vector<uint8_t> generateKeystream(
        const std::vector<uint8_t>& derivedKey,
        uint32_t counter, size_t length);

private:
    std::vector<uint8_t> master_key_;
    std::vector<uint8_t> context_;
    std::vector<uint8_t> nonce_;
};
