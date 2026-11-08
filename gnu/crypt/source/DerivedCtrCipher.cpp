
#include "DerivedCtrCipher.h"

DerivedCtrCipher::DerivedCtrCipher(
    const std::vector<uint8_t>& masterKey,
    const std::vector<uint8_t>& context,
    const std::vector<uint8_t>& nonce)
    :   master_key_(masterKey), context_(context), nonce_(nonce)
{
}

std::vector<uint8_t> DerivedCtrCipher::encrypt(
    const std::vector<uint8_t>& data,
    uint32_t counter)
{
    auto derivedKey = deriveKey(counter);
    auto keystream = generateKeystream(
        derivedKey, counter, data.size());

    Crypto aes(derivedKey, std::vector<uint8_t>(16, 0));
    return aes.xorVector(data, keystream);
}

std::vector<uint8_t> DerivedCtrCipher::deriveKey(
    uint32_t counter)
{
    std::vector<uint8_t> msg;

    msg.insert(msg.end(), context_.begin(), context_.end());
    msg.insert(msg.end(), nonce_.begin(), nonce_.end());
    msg.push_back((counter >> 24) & 0xff);
    msg.push_back((counter >> 16) & 0xff);
    msg.push_back((counter >>  8) & 0xff);
    msg.push_back(counter & 0xff);

    Crypto cmac(master_key_, std::vector<uint8_t>(16, 0));

    return cmac.cmac(msg.data(), msg.size());
}

std::vector<uint8_t> DerivedCtrCipher::buildCounterBlock(
    uint32_t counter)
{
    std::vector<uint8_t> block;

    block.insert(block.end(), nonce_.begin(), nonce_.end());
    block.push_back((counter >> 24) & 0xff);
    block.push_back((counter >> 16) & 0xff);
    block.push_back((counter >>  8) & 0xff);
    block.push_back(counter & 0xff);

    block.resize(16, 0x00);
    return block;
}

std::vector<uint8_t> DerivedCtrCipher::generateKeystream(
        const std::vector<uint8_t>& derivedKey,
        uint32_t counter, size_t length)
{
    std::vector<uint8_t> stream;
    Crypto aes(derivedKey, std::vector<uint8_t>(16, 0));

    while (stream.size() < length) {
        auto block = buildCounterBlock(counter++);
        auto encrypted = aes.ecbCrypt(block.data(), block.size());
        stream.insert(stream.end(), encrypted.begin(), encrypted.end());
    }

    stream.resize(length);
    return stream;
}
