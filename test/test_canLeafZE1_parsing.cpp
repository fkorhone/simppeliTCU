#include <gtest/gtest.h>
#include "../simppeliTCU/canLeafZE1.h"
#include "../simppeliTCU/vehicleTypes.h"

// Mock callback functions to capture parsed values - these are defined in test_canInterface_stubs.cpp
extern bool carIsAwake;
extern float lastSOC_value;
extern float lastTemp_value;
extern bool lastCharging_value;
extern ChargerState lastChargerState_value;
extern bool lastHVAC_value;
extern bool lastDoor_fl;
extern bool lastDoor_fr;
extern bool lastDoor_rl;
extern bool lastDoor_rr;
extern bool lastDoor_trunk;
extern bool lastLockStatus;

// Accessor functions for the test
namespace {
    void resetMocks() {
        lastSOC_value = -1.0f;
        lastTemp_value = -100.0f;
        carIsAwake = false;
        lastCharging_value = false;
        lastChargerState_value = ChargerState::IDLE;
        lastHVAC_value = false;
        lastDoor_fl = false;
        lastDoor_fr = false;
        lastDoor_rl = false;
        lastDoor_rr = false;
        lastDoor_trunk = false;
        lastLockStatus = false;
    }
}

// Test SOC parsing
TEST(CanLeafZE1Parsing, SOC_Parsing) {
    resetMocks();
    
    uint8_t socData[] = {0x21, 0x98};
    handleReceivedMessage(0x55B, socData, sizeof(socData));
    EXPECT_NEAR(lastSOC_value, 13.4f, 0.01f);
    
    resetMocks();
    uint8_t socData2[] = {0x14, 0x00};
    handleReceivedMessage(0x55B, socData2, sizeof(socData2));
    EXPECT_NEAR(lastSOC_value, 8.0f, 0.01f);
    
    resetMocks();
    uint8_t socData3[] = {0x27, 0x00};
    handleReceivedMessage(0x55B, socData3, sizeof(socData3));
    EXPECT_NEAR(lastSOC_value, 15.6f, 0.01f);
}

// Test cabin temperature parsing
TEST(CanLeafZE1Parsing, CabinTemp_Parsing) {
    resetMocks();
    
    uint8_t tempData1[] = {0x78};
    handleReceivedMessage(0x54F, tempData1, sizeof(tempData1));
    EXPECT_NEAR(lastTemp_value, 20.0f, 0.01f);
    
    resetMocks();
    uint8_t tempData2[] = {0x50};
    handleReceivedMessage(0x54F, tempData2, sizeof(tempData2));
    EXPECT_NEAR(lastTemp_value, 0.0f, 0.01f);
    
    resetMocks();
    uint8_t tempData3[] = {0x3C};
    handleReceivedMessage(0x54F, tempData3, sizeof(tempData3));
    EXPECT_NEAR(lastTemp_value, -10.0f, 0.01f);
    
    resetMocks();
    uint8_t tempData4[] = {0xA0};
    handleReceivedMessage(0x54F, tempData4, sizeof(tempData4));
    EXPECT_NEAR(lastTemp_value, 40.0f, 0.01f);
}

// Test car awake parsing
TEST(CanLeafZE1Parsing, CarAwake_Parsing) {
    resetMocks();
    EXPECT_FALSE(carIsAwake);
    
    handleReceivedMessage(0x601, nullptr, 0);
    EXPECT_TRUE(carIsAwake);
}

// Test charger status parsing
TEST(CanLeafZE1Parsing, ChargerStatus_Parsing) {
    resetMocks();
    uint8_t chargeData1[] = {0x00, 0x00, 0x00, 0x00, 0x40, 0x02};
    handleReceivedMessage(0x390, chargeData1, sizeof(chargeData1));
    EXPECT_TRUE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::CHARGING);
    
    resetMocks();
    uint8_t chargeData2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
    handleReceivedMessage(0x390, chargeData2, sizeof(chargeData2));
    EXPECT_FALSE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::IDLE);
    
    resetMocks();
    uint8_t chargeData3[] = {0x00, 0x00, 0x00, 0x08, 0x00, 0x08};
    handleReceivedMessage(0x390, chargeData3, sizeof(chargeData3));
    EXPECT_TRUE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::CHARGING);
    
    resetMocks();
    uint8_t chargeData4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x08};
    handleReceivedMessage(0x390, chargeData4, sizeof(chargeData4));
    EXPECT_FALSE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::INTERRUPTED);
    
    resetMocks();
    uint8_t chargeData5[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04};
    handleReceivedMessage(0x390, chargeData5, sizeof(chargeData5));
    EXPECT_FALSE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::FINISHED);
    
    resetMocks();
    uint8_t chargeData6[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18};
    handleReceivedMessage(0x390, chargeData6, sizeof(chargeData6));
    EXPECT_FALSE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::WAITING);
}

// Test HVAC status parsing
TEST(CanLeafZE1Parsing, HVACStatus_Parsing) {
    resetMocks();
    uint8_t hvacData1[] = {0x00, 0x01};
    handleReceivedMessage(0x54B, hvacData1, sizeof(hvacData1));
    EXPECT_TRUE(lastHVAC_value);
    
    resetMocks();
    uint8_t hvacData2[] = {0x00, 0x30};
    handleReceivedMessage(0x54B, hvacData2, sizeof(hvacData2));
    EXPECT_TRUE(lastHVAC_value);
    
    resetMocks();
    uint8_t hvacData3[] = {0x00, 0x00};
    handleReceivedMessage(0x54B, hvacData3, sizeof(hvacData3));
    EXPECT_FALSE(lastHVAC_value);
    
    resetMocks();
    uint8_t hvacData4[] = {0x00, 0x31};
    handleReceivedMessage(0x54B, hvacData4, sizeof(hvacData4));
    EXPECT_TRUE(lastHVAC_value);
}

// Test Doors and Locks parsing
TEST(CanLeafZE1Parsing, DoorsAndLocks_Parsing) {
    resetMocks();
    
    // Everything closed and unlocked
    uint8_t closedUnlockedData[] = {0x00, 0x00, 0x00};
    handleReceivedMessage(0x60D, closedUnlockedData, sizeof(closedUnlockedData));
    EXPECT_FALSE(lastDoor_trunk);
    EXPECT_FALSE(lastDoor_rr);
    EXPECT_FALSE(lastDoor_rl);
    EXPECT_FALSE(lastDoor_fr);
    EXPECT_FALSE(lastDoor_fl);
    EXPECT_FALSE(lastLockStatus);
    
    resetMocks();

    // All doors open and locked
    // Trunk=1(0x80), RL=1(0x40), RR=1(0x20), FR=1(0x10), FL=1(0x08) -> 0xF8
    // Locked d[2] byte field bit 3 = 0x10 -> byte[2]=0x10 
    uint8_t allOpenLockedData[] = {0xF8, 0x00, 0x10};
    handleReceivedMessage(0x60D, allOpenLockedData, sizeof(allOpenLockedData));
    EXPECT_TRUE(lastDoor_trunk);
    EXPECT_TRUE(lastDoor_rr);
    EXPECT_TRUE(lastDoor_rl);
    EXPECT_TRUE(lastDoor_fr);
    EXPECT_TRUE(lastDoor_fl);
    EXPECT_TRUE(lastLockStatus);

    resetMocks();

    // Just Trunk Open + Unlocked
    uint8_t trunkOpenUnlockedData[] = {0x80, 0x00, 0x00};
    handleReceivedMessage(0x60D, trunkOpenUnlockedData, sizeof(trunkOpenUnlockedData));
    EXPECT_TRUE(lastDoor_trunk);
    EXPECT_FALSE(lastDoor_rr);
    EXPECT_FALSE(lastDoor_rl);
    EXPECT_FALSE(lastDoor_fr);
    EXPECT_FALSE(lastDoor_fl);
    EXPECT_FALSE(lastLockStatus);

    resetMocks();

    // Rear Left Open + Unlocked
    uint8_t rlOpenUnlockedData[] = {0x40, 0x00, 0x00};
    handleReceivedMessage(0x60D, rlOpenUnlockedData, sizeof(rlOpenUnlockedData));
    EXPECT_FALSE(lastDoor_trunk);
    EXPECT_FALSE(lastDoor_rr);
    EXPECT_TRUE(lastDoor_rl);
    EXPECT_FALSE(lastDoor_fr);
    EXPECT_FALSE(lastDoor_fl);
    EXPECT_FALSE(lastLockStatus);

    resetMocks();

    // Rear Right Open + Unlocked
    uint8_t rrOpenUnlockedData[] = {0x20, 0x00, 0x00};
    handleReceivedMessage(0x60D, rrOpenUnlockedData, sizeof(rrOpenUnlockedData));
    EXPECT_FALSE(lastDoor_trunk);
    EXPECT_TRUE(lastDoor_rr);
    EXPECT_FALSE(lastDoor_rl);
    EXPECT_FALSE(lastDoor_fr);
    EXPECT_FALSE(lastDoor_fl);
    EXPECT_FALSE(lastLockStatus);

    resetMocks();

    // Front Left Open + Locked
    uint8_t flOpenLockedData[] = {0x08, 0x00, 0x10};
    handleReceivedMessage(0x60D, flOpenLockedData, sizeof(flOpenLockedData));
    EXPECT_FALSE(lastDoor_trunk);
    EXPECT_FALSE(lastDoor_rr);
    EXPECT_FALSE(lastDoor_rl);
    EXPECT_FALSE(lastDoor_fr);
    EXPECT_TRUE(lastDoor_fl);
    EXPECT_TRUE(lastLockStatus);
}

// Test that unknown message IDs are ignored
TEST(CanLeafZE1Parsing, UnknownMessageID) {
    resetMocks();
    
    uint8_t unknownData[] = {0x01, 0x02, 0x03, 0x04};
    handleReceivedMessage(0x999, unknownData, sizeof(unknownData));
    
    EXPECT_EQ(lastSOC_value, -1.0f);
    EXPECT_EQ(lastTemp_value, -100.0f);
    EXPECT_FALSE(carIsAwake);
    EXPECT_FALSE(lastCharging_value);
    EXPECT_EQ(lastChargerState_value, ChargerState::IDLE);
    EXPECT_FALSE(lastHVAC_value);
}
