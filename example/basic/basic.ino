#include <BlockchainHandler.h>

// Test keys - replace with actual test keys
const char* TEST_PUBLIC_KEY = "1234567890123456789012345678901234567890123456789012345678901234";
const char* TEST_PRIVATE_KEY = "1234567890123456789012345678901234567890123456789012345678901234";
const char* TEST_NODE_ID = "test_node_001";
const char* TEST_SERVER_URL = "http://kda.crankk.org/chainweb/0.0/mainnet01/chain/19/pact/api/v1/";

static int32_t callBlockchain() {
    // Packet ID generator callback
    auto packetIdGen = []() -> uint32_t {
        static uint32_t id = 0;
        return ++id;
    };

    // Secret generation callback
    auto onSecretGen = [](uint32_t packetId) {
        Serial.printf("Generated secret for packet: %u\n", packetId);
    };

    // Create blockchain handler instance
    BlockchainHandler blockchainHandler(
        TEST_PUBLIC_KEY,
        TEST_PRIVATE_KEY,
        true,  // Enable wallet
        TEST_SERVER_URL
    );

    // Perform node sync
    return blockchainHandler.performNodeSync(
        TEST_NODE_ID,
        packetIdGen,
        onSecretGen
    );
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nBlockchain Handler Test - Setup");
}

void loop() {
    Serial.println("Starting blockchain call...");
    int32_t nextSync = callBlockchain();
    Serial.printf("Next sync in %d ms\n", nextSync);
    Serial.println("Waiting...");
    delay(nextSync);
}