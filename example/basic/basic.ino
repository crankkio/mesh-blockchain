#include <BlockchainHandler.h>

// Test keys - replace with actual test keys
const char* TEST_PUBLIC_KEY = "1234567890123456789012345678901234567890123456789012345678901234";
const char* TEST_PRIVATE_KEY = "1234567890123456789012345678901234567890123456789012345678901234";
const char* TEST_NODE_ID = "test_node_001";

static int32_t callBlockchain() {
    auto packetIdGen = []() -> uint32_t {
        static uint32_t id = 0;
        return ++id;
    };

    std::unique_ptr<BlockchainHandler> blockchainHandler(
        new BlockchainHandler(TEST_PUBLIC_KEY, TEST_PRIVATE_KEY, true, packetIdGen));
    return blockchainHandler->performNodeSync(TEST_NODE_ID);
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