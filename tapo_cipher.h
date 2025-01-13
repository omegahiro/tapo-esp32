#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>


class TapoCipher {
public:
    TapoCipher(const std::vector<uint8_t>& local_seed,
        const std::vector<uint8_t>& remote_seed,
        const std::vector<uint8_t>& auth_hash) {
        std::vector<uint8_t> local_hash = concat(local_seed, remote_seed, auth_hash);

        key = key_derive(local_hash);
        auto iv_seq_pair = iv_derive(local_hash);
        iv = std::get<0>(iv_seq_pair);
        seq = std::get<1>(iv_seq_pair);
        sig = sig_derive(local_hash);
    }

    std::pair<std::vector<uint8_t>, uint32_t> encrypt(const std::string& data) {
        seq += 1;
        std::vector<uint8_t> iv_seq_data = iv_seq(seq);

        mbedtls_aes_context aes_ctx;
        mbedtls_aes_init(&aes_ctx);
        mbedtls_aes_setkey_enc(&aes_ctx, key.data(), key.size() * 8);

        std::vector<uint8_t> padded_data = pad(data);
        std::vector<uint8_t> encrypted_data(padded_data.size());
        mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, padded_data.size(), iv_seq_data.data(), padded_data.data(), encrypted_data.data());
        mbedtls_aes_free(&aes_ctx);

        std::vector<uint8_t> seq_bytes = to_bytes(seq);
        std::vector<uint8_t> combined_data = concat(sig, seq_bytes, encrypted_data);
        std::vector<uint8_t> signature = sha256(combined_data);

        std::vector<uint8_t> result = concat(signature, encrypted_data);
        return { result, seq };
    }

    std::string decrypt(uint32_t seq, const std::vector<uint8_t>& cipher_data) {
        std::vector<uint8_t> signature(cipher_data.begin(), cipher_data.begin() + 32);
        std::vector<uint8_t> encrypted_data(cipher_data.begin() + 32, cipher_data.end());
        std::vector<uint8_t> iv_seq_data = iv_seq(seq);

        mbedtls_aes_context aes_ctx;
        mbedtls_aes_init(&aes_ctx);
        mbedtls_aes_setkey_dec(&aes_ctx, key.data(), key.size() * 8);

        std::vector<uint8_t> decrypted_data(encrypted_data.size());
        mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, encrypted_data.size(), iv_seq_data.data(), encrypted_data.data(), decrypted_data.data());
        mbedtls_aes_free(&aes_ctx);

        return unpad(decrypted_data);
    }

    static std::vector<uint8_t> sha1(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> hash(20);
        mbedtls_sha1(data.data(), data.size(), hash.data());
        return hash;
    }

    static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> hash(32);
        mbedtls_sha256(data.data(), data.size(), hash.data(), 0);
        return hash;
    }

    static std::string to_hex_string(const std::vector<uint8_t>& data) {
        std::ostringstream oss;
        for (uint8_t byte : data) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return oss.str();
    }

    static std::vector<uint8_t> concat(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
        std::vector<uint8_t> combined(a);
        combined.insert(combined.end(), b.begin(), b.end());
        return combined;
    }

    static std::vector<uint8_t> concat(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, const std::vector<uint8_t>& c) {
        std::vector<uint8_t> combined(a);
        combined.insert(combined.end(), b.begin(), b.end());
        combined.insert(combined.end(), c.begin(), c.end());
        return combined;
    }

private:
    std::vector<uint8_t> key;
    std::vector<uint8_t> iv;
    uint32_t seq;
    std::vector<uint8_t> sig;

    std::vector<uint8_t> key_derive(const std::vector<uint8_t>& local_hash) {
        std::vector<uint8_t> input = concat({ 'l', 's', 'k' }, local_hash);
        auto hash = sha256(input);
        return { hash.begin(), hash.begin() + 16 };
    }

    std::pair<std::vector<uint8_t>, uint32_t> iv_derive(const std::vector<uint8_t>& local_hash) {
        std::vector<uint8_t> input = concat({ 'i', 'v' }, local_hash);
        auto hash = sha256(input);
        std::vector<uint8_t> iv(hash.begin(), hash.begin() + 12);
        uint32_t seq = (hash[28] << 24) | (hash[29] << 16) | (hash[30] << 8) | hash[31];
        return { iv, seq };
    }

    std::vector<uint8_t> sig_derive(const std::vector<uint8_t>& local_hash) {
        std::vector<uint8_t> input = concat({ 'l', 'd', 'k' }, local_hash);
        auto hash = sha256(input);
        return { hash.begin(), hash.begin() + 28 };
    }

    std::vector<uint8_t> iv_seq(uint32_t seq) {
        std::vector<uint8_t> seq_bytes = to_bytes(seq);
        std::vector<uint8_t> iv_seq_combined = concat(iv, seq_bytes);
        return iv_seq_combined;
    }

    std::vector<uint8_t> pad(const std::string& data) {
        size_t padding_len = 16 - (data.size() % 16);
        std::vector<uint8_t> padded(data.begin(), data.end());
        padded.insert(padded.end(), padding_len, static_cast<uint8_t>(padding_len));
        return padded;
    }

    std::string unpad(const std::vector<uint8_t>& data) {
        uint8_t padding_len = data.back();
        return std::string(data.begin(), data.end() - padding_len);
    }

    std::vector<uint8_t> to_bytes(uint32_t value) {
        return { static_cast<uint8_t>(value >> 24),
                static_cast<uint8_t>(value >> 16),
                static_cast<uint8_t>(value >> 8),
                static_cast<uint8_t>(value) };
    }
};


int test_tapo_cipher() {
    // Generate seed data
    std::vector<uint8_t> local_seed = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
	std::cout << "Local seed: " << TapoCipher::to_hex_string(local_seed) << std::endl;

    std::vector<uint8_t> remote_seed = { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                         0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20 };
	std::cout << "Remote seed: " << TapoCipher::to_hex_string(remote_seed) << std::endl;

    std::vector<uint8_t> auth_hash = { 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                                       0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30 };
	std::cout << "User hash: " << TapoCipher::to_hex_string(auth_hash) << std::endl;

    // Initialize TapoCipher class
    TapoCipher cipher(local_seed, remote_seed, auth_hash);
    
    // Test data
	std::string plaintext = "Hello, World!";
    std::cout << "Original plaintext: " << plaintext << std::endl;

    // Encryption
    auto encrypted = cipher.encrypt(plaintext);
	std::cout << "Encrypted data (hex): " << TapoCipher::to_hex_string(std::get<0>(encrypted)) << std::endl;

    // Decryption
    std::string decrypted_text = cipher.decrypt(std::get<1>(encrypted), std::get<0>(encrypted));
    std::cout << "Decrypted text: " << decrypted_text << std::endl;

    // Test result
    if (plaintext == decrypted_text) {
        std::cout << "Test passed: decrypted text matches the original plaintext." << std::endl;
        return 0;
    }
    else {
        std::cout << "Test failed: decrypted text does not match the original plaintext." << std::endl;
        return 1;
    }
}
