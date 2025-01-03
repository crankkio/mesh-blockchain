#include "BlockchainHandler.h"
#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk.h"
#include "mbedtls/pkcs5.h"
#include "utils.h"
#include <ArduinoJson.h>
#include <cstdio>
#include <ctime>
#include <sstream>

// Redefine strptime to avoid IRAM issue when using the HTTPClient functions
char *strptime(const char *str, const char *format, struct tm *tm)
{
    if (sscanf(str, format, &tm->tm_year, &tm->tm_mon, &tm->tm_mday, &tm->tm_hour, &tm->tm_min, &tm->tm_sec) == 6) {
        tm->tm_year -= 1900; // Adjust year to be relative to 1900
        tm->tm_mon -= 1;     // Adjust month to be 0-based
        return (char *)(str + strlen(str));
    }
    return NULL;
}

BlockchainHandler::BlockchainHandler(const std::string& public_key, 
                                   const std::string& private_key,
                                   bool is_wallet_enabled,
                                   const String& server_url)
    : public_key_(public_key)
    , private_key_(private_key)
    , is_wallet_enabled_(is_wallet_enabled)
{
    kda_server_ = server_url;
    encryptionHandler_ = std::unique_ptr<EncryptionHandler>(new EncryptionHandler());
}

bool BlockchainHandler::isWalletConfigValid()
{
    return is_wallet_enabled_ && public_key_.length() == 64 && private_key_.length() == 64;
}

int32_t BlockchainHandler::performNodeSync(const std::string& node_id,
                                           PacketIdGenerator packetIdGen,
                                           SecretCallback onSecretGen) {
    Serial.printf("\nWallet public key: %s\n", public_key_.data());

    if (!isWalletConfigValid() || !isWifiAvailable()) {
        return 300000; // Every 5 minutes.
    }

    BlockchainStatus status = executeBlockchainCommand("local", "(free.mesh03.get-my-node)");
    Serial.printf("Response: %s\n", blockchainStatusToString(status).c_str());

    // node exists, due for sending
    if (status == BlockchainStatus::READY) {
        uint32_t packetId = 0;
        if (packetIdGen) {
            packetId = packetIdGen();
        }
        String secret_hex = String(packetId, HEX);
        String secret = encryptPayload(secret_hex.c_str());
        status = executeBlockchainCommand("send", "(free.mesh03.update-sent \"" + secret + "\")");
        if (status == BlockchainStatus::SUCCESS) {
            // Only send the radio beacon if the update-sent command is successful
            if (onSecretGen) {
                onSecretGen(packetId);
                Serial.printf("Update sent successfully with packet id: %d\n", packetId);
            }
        } else {
            Serial.printf("Update sent failed: %s\n", blockchainStatusToString(status).c_str());
        }
    } else if (status == BlockchainStatus::NODE_NOT_FOUND) { // node doesn't exist, insert it
        status = executeBlockchainCommand("send", "(free.mesh03.insert-my-node \"" + String(node_id.c_str()) + "\")");
        Serial.printf("Node insert local response: %s\n", blockchainStatusToString(status).c_str());
    } else if (status == BlockchainStatus::NOT_DUE) { // node exists, not due for sending
        Serial.printf("DON'T SEND beacon\n");
    } else {
        Serial.printf("Error occurred: %s\n", blockchainStatusToString(status).c_str());
    }
    return 300000; // Every 5 minutes. That should be enough for previous txn to be complete
}

JsonDocument BlockchainHandler::createCommandObject(const String &command)
{
    JsonDocument cmdObject;

    // Create signers array
    JsonArray signers = cmdObject["signers"].to<JsonArray>();
    JsonObject signer = signers.add<JsonObject>();
    signer["scheme"] = "ED25519";
    signer["pubKey"] = public_key_;
    signer["addr"] = public_key_;

    // Create meta object
    JsonObject meta = cmdObject["meta"].to<JsonObject>();
    meta["creationTime"] = getCurrentUnixTime();
    meta["ttl"] = 28800;
    meta["chainId"] = "19";
    meta["gasPrice"] = 1e-7;
    meta["gasLimit"] = 8000;
    meta["sender"] = "k:" + public_key_;

    cmdObject["nonce"] = getCurrentTimestamp();
    cmdObject["networkId"] = "mainnet01";

    // Create payload object
    JsonObject payload = cmdObject["payload"].to<JsonObject>();
    JsonObject exec = payload["exec"].to<JsonObject>();
    exec["code"] = command;
    exec["data"].to<JsonObject>(); // Empty data object

    return cmdObject;
}

JsonDocument BlockchainHandler::preparePostObject(const JsonDocument &cmdObject, const String &commandType)
{
    JsonDocument postObject;

    // Serialize cmdObject to a string first
    String cmdString;
    serializeJson(cmdObject, cmdString);
    postObject["cmd"] = cmdString;

    HashVector vector{"Test1", cmdString.c_str()};

    uint8_t *hashBin = encryptionHandler_->Binhash(&vector);
    String hash = encryptionHandler_->KDAhash(&vector);
    String signHex = encryptionHandler_->generateSignature(public_key_, private_key_, hashBin);

    postObject["hash"] = hash;
    JsonArray sigs = postObject["sigs"].to<JsonArray>();
    JsonObject sigObject = sigs.add<JsonObject>();
    sigObject["sig"] = signHex;

    return postObject;
}

BlockchainStatus BlockchainHandler::parseBlockchainResponse(const String &response, const String &command)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        Serial.printf("JSON parsing failed: %s\n", error.c_str());
        return BlockchainStatus::PARSING_ERROR;
    }

    JsonObject resultObject = doc["result"];
    JsonObject dataObject = resultObject["data"];
    const char* status = resultObject["status"];

    BlockchainStatus returnStatus = 
        command.indexOf("get-my-node") > 0 ? BlockchainStatus::NODE_NOT_FOUND : BlockchainStatus::FAILURE;

    if (String(status).startsWith("s")) {
        if (dataObject["pubkeyd"].is<const char*>()) {
            director_pubkeyd_ = dataObject["pubkeyd"].as<std::string>();
        }

        if (command.indexOf("get-my-node") > 0) {
            bool sendValue = dataObject["send"];
            returnStatus = sendValue ? BlockchainStatus::READY : BlockchainStatus::NOT_DUE;
        } else if (command.indexOf("get-sender-details") > 0) {
            returnStatus = BlockchainStatus::SUCCESS;
        }
    }
    return returnStatus;
}

BlockchainStatus BlockchainHandler::executeBlockchainCommand(const String &commandType, const String &command)
{
    if (!isWifiAvailable()) {
        return BlockchainStatus::NO_WIFI;
    }

    HTTPClient http;
    http.begin(kda_server_ + commandType);
    http.addHeader("Content-Type", "application/json");

    JsonDocument cmdObject = createCommandObject(command);
    JsonDocument postObject = preparePostObject(cmdObject, commandType);

    String postRaw;
    if (commandType == "local") {
        serializeJson(postObject, postRaw);
    } else {
        JsonDocument finalDoc;
        JsonArray cmds = finalDoc["cmds"].to<JsonArray>();
        cmds.add(postObject.as<JsonObject>());
        serializeJson(finalDoc, postRaw);
    }

    logLongString(postRaw);

    http.setTimeout(15000);
    int httpResponseCode = http.POST(postRaw);
    String response = http.getString();
    logLongString(response);

    http.end();
    // Handle HTTP response codes
    if (httpResponseCode < 0 || (httpResponseCode >= 400 && httpResponseCode <= 599)) {
        return BlockchainStatus::HTTP_ERROR;
    }
    if (httpResponseCode == HTTP_CODE_NO_CONTENT) {
        return BlockchainStatus::EMPTY_RESPONSE;
    }

    return commandType == "local" ? parseBlockchainResponse(response, command) : BlockchainStatus::SUCCESS;
}

String BlockchainHandler::encryptPayload(const std::string &payload)
{
    if (!encryptionHandler_) {
        //LOG_ERROR("Encryption handler is not initialized. Encryption failed.\n");
        return "";
    }
    return encryptionHandler_->encrypt(director_pubkeyd_, payload);
}

// Function to convert enum to string
std::string BlockchainHandler::blockchainStatusToString(BlockchainStatus status)
{
    switch (status) {
    case BlockchainStatus::SUCCESS:
        return "SUCCESS";
    case BlockchainStatus::FAILURE:
        return "FAILURE";
    case BlockchainStatus::NO_WIFI:
        return "NO_WIFI";
    case BlockchainStatus::HTTP_ERROR:
        return "HTTP_ERROR";
    case BlockchainStatus::EMPTY_RESPONSE:
        return "EMPTY_RESPONSE";
    case BlockchainStatus::PARSING_ERROR:
        return "PARSING_ERROR";
    case BlockchainStatus::NODE_NOT_FOUND:
        return "NODE_NOT_FOUND";
    case BlockchainStatus::READY:
        return "READY";
    case BlockchainStatus::NOT_DUE:
        return "NOT_DUE";
    default:
        return "UNKNOWN_STATUS";
    }
}