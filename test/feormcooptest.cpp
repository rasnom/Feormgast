#include "feormcoop.h"
#include "gtest/gtest.h"

namespace {

    TEST(FeormCoop, TestingTest) {
        EXPECT_EQ(1, 1);
        EXPECT_NE(1, 0);
        EXPECT_GT(1, 0);
    }

    TEST(FeormCoop, AnotherTest) {
        EXPECT_EQ(1, (2 - 1));
        EXPECT_LT(-1, 0);
        EXPECT_EQ(2, 2);
    }

}

void setup() {
    Serial.begin(115200);
    while(!Serial) { delay(10); };

    ::testing::InitGoogleTest();    
}

void loop() {
    if (RUN_ALL_TESTS());

    delay(1000);
}

