#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include "tapo_cipher.h"

#ifdef TAPO_DEBUG_MODE
#define TAPO_PROTOCOL_DEBUG(x) Serial.println(String("TAPO_PROTOCOL: ") + x)
#else
#define TAPO_PROTOCOL_DEBUG(x)
#endif

class TapoProtocol {
public:
    void handshake(const String& ip_address, const String& username, const String& password) {        
        this->url = "http://" + ip_address + "/app";
        this->username = username;
        this->password = password;
        
        std::vector<uint8_t> username_hash = TapoCipher::sha1(to_bytes(username));
        std::vector<uint8_t> password_hash = TapoCipher::sha1(to_bytes(password));
        std::vector<uint8_t> auth_hash = TapoCipher::sha256(TapoCipher::concat(username_hash, password_hash));
        TAPO_PROTOCOL_DEBUG("Auth hash: " + String(TapoCipher::to_hex_string(auth_hash).c_str()));
        std::vector<uint8_t> local_seed = random_bytes(16);
        TAPO_PROTOCOL_DEBUG("Local seed: " + String(TapoCipher::to_hex_string(local_seed).c_str()));

        std::vector<uint8_t> remote_seed = handshake1(local_seed, auth_hash);
        TAPO_PROTOCOL_DEBUG("Remote seed: " + String(TapoCipher::to_hex_string(remote_seed).c_str()));
        handshake2(local_seed, remote_seed, auth_hash);

        cipher = new TapoCipher(local_seed, remote_seed, auth_hash);
    }
    
    void rehandshake() {
        delete cipher;
        cookie = "";
        handshake(url, username, password);
    }
    
    String send_message(const String& message) {
        if (!cipher) {
            Serial.println("TAPO_PROTOCOL: Cipher is not initialized");
            return "";
        }

        TAPO_PROTOCOL_DEBUG("Sending message: " + message);
        auto encrypte_message_and_seq = cipher->encrypt(std::string(message.c_str()));
        auto& encrypte_message = encrypte_message_and_seq.first;
        auto& seq = encrypte_message_and_seq.second;
        
        std::vector<uint8_t> response;
        post("/request?seq=" + String(seq), encrypte_message, [&response](HTTPClient& http) {
            response.resize(http.getSize());
            http.getStream().readBytes(reinterpret_cast<char*>(response.data()), response.size());
        });

        String decrypted_response = String(cipher->decrypt(seq, response).c_str());
        TAPO_PROTOCOL_DEBUG("Response: " + decrypted_response);
        
        return decrypted_response;
    }
    
    static std::vector<uint8_t> to_bytes(const String& value) {
        return std::vector<uint8_t>(value.begin(), value.end());
    }

    static String to_string(const std::vector<uint8_t>& value) {
        return String(reinterpret_cast<const char*>(value.data()), value.size());
    }

    static std::vector<uint8_t> random_bytes(size_t size) {
        std::vector<uint8_t> data(size);
        std::generate(data.begin(), data.end(), [&]() { return random(256); });

        return data;
    }
    
    ~TapoProtocol() {
        delete cipher;
    }


private:
    TapoCipher* cipher = nullptr;
    String cookie;
    String url;
    String username;
    String password;

    int post(const String& endpoint, const std::vector<uint8_t>& payload, std::function<void(HTTPClient&)> response_handler, bool collect_headers = false) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("TAPO_PROTOCOL: Please connect to WiFi first");
            return -1;
        }
        
        HTTPClient http;
        http.begin(url + endpoint);

        if (collect_headers) {
            const char* header_keys[] = {"Set-Cookie"};
            size_t header_key_count = sizeof(header_keys) / sizeof(char*);
            http.collectHeaders(header_keys, header_key_count);
        }

        if (cookie.length() > 0) {
            http.addHeader("Cookie", "TP_SESSIONID=" + cookie);
        }

        int response_code = http.POST(to_string(payload));
        if (response_code != 200) {
            http.end();
            return response_code;
        }
        
        if (collect_headers) {
            String cookie_str = http.header("Set-Cookie");
            int start = cookie_str.indexOf("TP_SESSIONID=") + 13;
            int end = cookie_str.indexOf(";", start);
            cookie = cookie_str.substring(start, end);
        }

        response_handler(http);

        http.end();
        return response_code;
    }

    std::vector<uint8_t> handshake1(const std::vector<uint8_t>& local_seed, const std::vector<uint8_t>& auth_hash) {
        String response_str;
        post("/handshake1", local_seed, [&response_str](HTTPClient& http) {
            response_str = http.getString();
        }, true);

        std::vector<uint8_t> remote_seed(response_str.begin(), response_str.begin() + 16);
        std::vector<uint8_t> server_hash(response_str.begin() + 16, response_str.end());
        std::vector<uint8_t> local_hash = TapoCipher::sha256(TapoCipher::concat(local_seed, remote_seed, auth_hash));
        if (local_hash != server_hash) {
            Serial.println("TAPO_PROTOCOL: Invalid credentials during handshake1");
            return {};
        }
        TAPO_PROTOCOL_DEBUG("Handshake1 successful");

        return remote_seed;
    }

    void handshake2(const std::vector<uint8_t>& local_seed, const std::vector<uint8_t>& remote_seed, const std::vector<uint8_t>& auth_hash) {
        std::vector<uint8_t> payload = TapoCipher::sha256(TapoCipher::concat(remote_seed, local_seed, auth_hash));
        String response_str;
        int response_code = post("/handshake2", payload, [&response_str](HTTPClient& http) {
            response_str = http.getString();
        });
        if (response_code != 200) {
            Serial.println("TAPO_PROTOCOL: Handshake2 failed with response code " + String(response_code));
            return;
        }
        TAPO_PROTOCOL_DEBUG("Handshake2 successful");
    }
};
