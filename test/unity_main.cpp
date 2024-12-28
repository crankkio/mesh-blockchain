#include <unity.h>


void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

// Declare test functions from other files
void test_invalid_wallet_config(void);
void test_valid_wallet_config(void);
void test_binary_hash_generation(void);
void test_kda_hash_generation(void);
void test_hex_conversion(void);
void test_payload_encryption(void);
void test_wifi_connection(void);

int main(void) {
    UNITY_BEGIN();

    // Blockchain tests
    RUN_TEST(test_invalid_wallet_config);
    RUN_TEST(test_valid_wallet_config);
    RUN_TEST(test_wifi_connection);

    // Encryption tests
    RUN_TEST(test_binary_hash_generation);
    RUN_TEST(test_kda_hash_generation);
    RUN_TEST(test_hex_conversion);
    RUN_TEST(test_payload_encryption);

    return UNITY_END();
}
