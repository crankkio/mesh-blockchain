#include <unity.h>
#include "EncryptionHandler.h"


void test_binary_hash_generation(void) {
    EncryptionHandler handler;
    HashVector test_vector = {
        "test",
        "test_data"
    };
    uint8_t* bin_hash = handler.Binhash(&test_vector);
    TEST_ASSERT_NOT_NULL(bin_hash);
}

void test_kda_hash_generation(void) {
    EncryptionHandler handler;
    HashVector test_vector = {
        "test",
        "test_data"
    };
    String kda_hash = handler.KDAhash(&test_vector);
    TEST_ASSERT_TRUE(kda_hash.length() > 0);
}

void test_hex_conversion(void) {
    EncryptionHandler handler;
    std::string test_hex = "48656c6c6f"; // "Hello" in hex
    char output[6] = {0};  // Increased buffer size and null-terminated

    handler.HexToBytes(test_hex, output);
    TEST_ASSERT_EQUAL_STRING("Hello", output);
}

void test_payload_encryption(void) {
    EncryptionHandler handler;
    // Test RSA public key in PEM format (base64 encoded)
    // Generated using:
    // 1. Generate 1024-bit RSA private key:
    //    openssl genrsa -out private.pem 1024
    // 2. Extract PKCS#1 public key:
    //    openssl rsa -in private.pem -pubout -RSAPublicKey_out -out public.pem
    // 3. Encode the public key in base64:
    // WARNING: 1024-bit keys are considered weak - used here for testing only
    std::string base64_public_key =
        "LS0tLS1CRUdJTiBSU0EgUFVCTElDIEtFWS0tLS0tCk1JR0pBb0dCQU5YODZUNmRuVStsZ2phVHNIMjdhTVlx"
        "OXlEOCtCVHV1WEczMlRpb2Z0QURkdUJ1SWlXbEpKVXAKaFpPcDdGM2ZBTmtoczNXOHJuNy9tKzhESWZ3bWhZ"
        "eVZYaS9EK3gyWDBTcXAwOVBnVU9mdlo0dVlTSFlheFhqSApxaTZRSlRKY2NnQTEwRzBRUm9hZTk5MXV4VVVX"
        "WEN2dU9sR2RxL2NwOFRXUFMvOTkrZ1RiQWdNQkFBRT0KLS0tLS1FTkQgUlNBIFBVQkxJQyBLRVktLS0tLQ==";
    // Test payload
    std::string test_payload = "test_data_123";
    String encrypted = handler.encrypt(base64_public_key, test_payload);

    // Split and verify structure
    std::string encrypted_str(encrypted.c_str());
    size_t delim_pos = encrypted_str.find(";;;;;");
    TEST_ASSERT_TRUE(delim_pos != std::string::npos);

    // Get encrypted data and encrypted AES key
    std::string encrypted_data = encrypted_str.substr(0, delim_pos);
    std::string encrypted_key = encrypted_str.substr(delim_pos + 5);
    // Verify base64 format
    TEST_ASSERT_TRUE(encrypted_data.find_first_not_of(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="
    ) == std::string::npos);

    // Decode and verify structure
    std::vector<uint8_t> decoded_data(base64::decodeLength(encrypted_data.c_str()));
    base64::decode(encrypted_data.c_str(), decoded_data.data());
    // Verify "Salted__" prefix
    TEST_ASSERT_EQUAL_STRING_LEN("Salted__", (char*)decoded_data.data(), 8);

    // Extract components
    std::vector<uint8_t> salt(decoded_data.begin() + 8, decoded_data.begin() + 16);
    std::vector<uint8_t> encrypted_content(decoded_data.begin() + 16, decoded_data.end());
    // Verify component sizes
    TEST_ASSERT_EQUAL(8, salt.size());  // Salt should be 8 bytes
    TEST_ASSERT_GREATER_THAN(0, encrypted_content.size());  // Should have encrypted content
    TEST_ASSERT_EQUAL(0, encrypted_content.size() % 16);  // Should be multiple of AES block size
}
