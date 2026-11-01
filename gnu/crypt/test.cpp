
#include <stdio.h>
#include "crypt.h"

static void dumpHex(const char* name, const std::vector<uint8_t>& data);

int main(int argc, char* argv[])
{
    std::vector<uint8_t> key = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10,
        0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x18,
        0x19, 0x1A, 0x1B, 0x1C,
        0x1D, 0x1E, 0x1F, 0x20
    };
    std::vector<uint8_t> iv = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };
    Crypto aes(key, iv);

    std::string text = "Hello OpenSSL";

    auto encrypted = aes.ctrCrypt(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    dumpHex("encrypted", encrypted);

    auto decrypted = aes.ctrCrypt(reinterpret_cast<const uint8_t*>(encrypted.data()), encrypted.size());
    dumpHex("decrypted", decrypted);

    auto mac = aes.cmac(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    dumpHex("mac", mac);

    std::vector<uint8_t> plain(reinterpret_cast<const uint8_t*>(text.data()),
            reinterpret_cast<const uint8_t*>(text.data()) + text.size());
    std::vector<uint8_t> counter = iv;
    auto key_stream = aes.ecbCrypt(counter.data(), counter.size());
    key_stream.resize(text.size());
    auto encrypted_ecb_xor = aes.xorVector(plain, key_stream);
    dumpHex("encrypted(ecb + xor)", encrypted_ecb_xor);

    Crypto ecc(Crypto::ECCurve::P256);
    EVP_PKEY* pkey = ecc.generateKeyPair();
    auto sig = ecc.sign(pkey, reinterpret_cast<const uint8_t*>(text.data()),
            text.size());
    dumpHex("sig", sig);

    bool rc = ecc.verify(pkey, reinterpret_cast<const uint8_t*>(text.data()),
            text.size(), sig);
    printf("verify: %s\n", rc ? "OK" : "NG");
}

static void dumpHex(const char* name, const std::vector<uint8_t>& data)
{
    printf("%s (%zu bytes): ", name, data.size());
    for (uint8_t b : data) {
        printf("%02X", b);
    }
    printf("\n");
}
