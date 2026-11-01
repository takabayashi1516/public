#include "crypt.h"

Crypto::Crypto(ECCurve curve)
    :   aes_key_(16), aes_iv_(16), curve_(curve)
{
}

Crypto::Crypto(const std::vector<uint8_t>& aes_key, const std::vector<uint8_t>& aes_iv)
    :   aes_key_(aes_key), aes_iv_(aes_iv), curve_(ECCurve::P256)
{
    validateAesKey();
}

std::vector<uint8_t> Crypto::cmac(const uint8_t* data, size_t data_len)
{
    validateAesKey();

    EVP_MAC* mac = EVP_MAC_fetch(nullptr, "CMAC", nullptr);
    if (!mac) {
        throw std::runtime_error(__FUNCTION__);
    }

    EVP_MAC_CTX* ctx = EVP_MAC_CTX_new(mac);
    EVP_MAC_free(mac);
    if (!ctx) {
        throw std::runtime_error(__FUNCTION__);
    }

    const char* cipher_name = cipherName();
    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_CIPHER,
                const_cast<char*>(cipher_name), 0),
        OSSL_PARAM_construct_end()
    };

    if (EVP_MAC_init(ctx, aes_key_.data(), aes_key_.size(),
            params) != 1) {
        EVP_MAC_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_MAC_update(ctx, data, data_len) != 1) {
        EVP_MAC_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    size_t mac_len = 0;
    std::vector<uint8_t> result(16);

    if (EVP_MAC_final(ctx, result.data(), &mac_len,
            result.size()) != 1) {
        EVP_MAC_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    result.resize(mac_len);

    EVP_MAC_CTX_free(ctx);

    return result;
}

std::vector<uint8_t> Crypto::ctrCrypt(const uint8_t* data, size_t data_len)
{
    return cipherCrypt(ctrCipher(), data, data_len);
}

std::vector<uint8_t> Crypto::ecbCrypt(const uint8_t* data, size_t data_len)
{
    return cipherCrypt(ecbCipher(), data, data_len);
}

EVP_PKEY* Crypto::generateKeyPair()
{
    int nid;

    switch (curve_) {
    case ECCurve::P256:
        nid = NID_X9_62_prime256v1;
        break;
    case ECCurve::P384:
        nid = NID_secp384r1;
        break;
    case ECCurve::P521:
        nid = NID_secp521r1;
        break;
    default:
        throw std::runtime_error(__FUNCTION__);
    }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    if (!ctx) {
        throw std::runtime_error(__FUNCTION__);
    }
    EVP_PKEY* pkey = nullptr;

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    EVP_PKEY_CTX_free(ctx);

    return pkey;
}

std::vector<uint8_t> Crypto::sign(EVP_PKEY* privateKey,
        const uint8_t* data, size_t data_len)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(),
            nullptr, privateKey) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_DigestSignUpdate(ctx, data, data_len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    size_t sig_len = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sig_len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    std::vector<uint8_t> sig(sig_len);
    if (EVP_DigestSignFinal(ctx, sig.data(), &sig_len) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    sig.resize(sig_len);
    EVP_MD_CTX_free(ctx);

    return sig;
}

bool Crypto::verify(EVP_PKEY* publicKey, const uint8_t* data,
        size_t data_len, const std::vector<uint8_t>& sig)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        throw std::runtime_error(__FUNCTION__);
    }
    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, publicKey) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }
    EVP_DigestVerifyUpdate(ctx, data, data_len);

    int rc = EVP_DigestVerifyFinal(ctx, sig.data(), sig.size());
    EVP_MD_CTX_free(ctx);

    return rc == 1;
}

std::vector<uint8_t> Crypto::xorVector(const std::vector<uint8_t>& data1, const std::vector<uint8_t>& data2)
{
    if (data1.size() != data2.size()) {
        throw std::runtime_error(__FUNCTION__);
    }
    std::vector<uint8_t> result(data1.size());
    for (size_t i = 0; i < data1.size(); i++) {
        result[i] = data1[i] ^ data2[i];
    }
    return result;
}

void Crypto::validateAesKey() const
{
    switch (aes_key_.size()) {
    case 16:
    case 24:
    case 32:
        break;
    default:
        throw std::runtime_error(__FUNCTION__);
    }

    if (aes_iv_.size() != 16) {
        throw std::runtime_error(__FUNCTION__);
    }
}

const EVP_CIPHER* Crypto::ctrCipher() const
{
    switch (aes_key_.size()) {
    case 16:
        return EVP_aes_128_ctr();
    case 24:
        return EVP_aes_192_ctr();
    case 32:
        return EVP_aes_256_ctr();
    default:
        throw std::runtime_error(__FUNCTION__);
    }
}

const EVP_CIPHER* Crypto::ecbCipher() const
{
    switch (aes_key_.size()) {
    case 16:
        return EVP_aes_128_ecb();
    case 24:
        return EVP_aes_192_ecb();
    case 32:
        return EVP_aes_256_ecb();
    default:
        throw std::runtime_error(__FUNCTION__);
    }
}

const char* Crypto::cipherName() const
{
    switch (aes_key_.size()) {
    case 16:
        return "AES-128-CBC";
    case 24:
        return "AES-192-CBC";
    case 32:
        return "AES-256-CBC";
    default:
        throw std::runtime_error(__FUNCTION__);
    }
}

std::vector<uint8_t> Crypto::cipherCrypt(const EVP_CIPHER* cipher, const uint8_t* data, size_t data_len)
{
    validateAesKey();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx) {
        throw std::runtime_error(__FUNCTION__);
    }

    if (EVP_EncryptInit_ex(ctx, cipher, nullptr, aes_key_.data(),
            aes_iv_.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    std::vector<uint8_t> out(data_len + 16);

    int out_len1 = 0;
    int out_len2 = 0;

    if (EVP_EncryptUpdate(ctx, out.data(), &out_len1, data,
            static_cast<int>(data_len)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    if (EVP_EncryptFinal_ex(ctx, out.data() + out_len1,
            &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error(__FUNCTION__);
    }

    out.resize(out_len1 + out_len2);

    EVP_CIPHER_CTX_free(ctx);

    return out;
}
