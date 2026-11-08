#pragma once

#if defined(_WIN32)
#include <cstddef>
#endif
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/core_names.h>
#include <vector>
#include <stdexcept>
#include <cstdint>

#if defined(_WIN32)
  #if defined(SRMCRYPT_EXPORTS)
    #define SRMCRYPT_API __declspec(dllexport)
  #else
    #define SRMCRYPT_API __declspec(dllimport)
  #endif
#else
  #define SRMCRYPT_API
#endif

#define STR2(x) #x
#define STR(x) STR2(x)

class SRMCRYPT_API Crypto {
public:
    enum class ECCurve {
        P256,
        P384,
        P521
    };

public:
    Crypto(ECCurve curve);
    Crypto(const std::vector<uint8_t>& aes_key, const std::vector<uint8_t>& aes_iv);

    // AES-CMAC
    std::vector<uint8_t> cmac(const uint8_t* data, size_t data_len);
    // AES-CTR
    std::vector<uint8_t> ctrCrypt(const uint8_t* data, size_t data_len);
    // AES-ECB
    std::vector<uint8_t> ecbCrypt(const uint8_t* data, size_t data_len);

    // Create ECDSA key
    EVP_PKEY* generateKeyPair();
    // ECDSA signature
    std::vector<uint8_t> sign(EVP_PKEY* privateKey,
        const uint8_t* data, size_t data_len);
    // verify ECDSA
    bool verify(EVP_PKEY* publicKey,
        const uint8_t* data, size_t data_len,
        const std::vector<uint8_t>& signature);

    // XOR
    std::vector<uint8_t> xorVector(const std::vector<uint8_t>& data1, const std::vector<uint8_t>& data2);

private:
    void validateAesKey() const;
    const EVP_CIPHER* ctrCipher() const;
    const EVP_CIPHER* ecbCipher() const;
    const char* cipherName() const;
    std::vector<uint8_t> cipherCrypt(const EVP_CIPHER* cipher, const uint8_t* data, size_t data_len);

private:
    std::vector<uint8_t> aes_key_;
    std::vector<uint8_t> aes_iv_;

    ECCurve curve_;
};
