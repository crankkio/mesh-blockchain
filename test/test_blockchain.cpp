#include <unity.h>
#include "BlockchainHandler.h"


void test_invalid_wallet_config(void) {
    BlockchainHandler handler("", "", false, "http://test.url");
    TEST_ASSERT_FALSE(handler.isWalletConfigValid());
}

void test_valid_wallet_config(void) {
    std::string valid_pub_key(64, 'a');
    std::string valid_priv_key(64, 'b');
    BlockchainHandler handler(valid_pub_key, valid_priv_key, true, "http://test.url");
    TEST_ASSERT_TRUE(handler.isWalletConfigValid());
}
