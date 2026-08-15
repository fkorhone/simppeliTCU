#include <gtest/gtest.h>
#include "../simppeliTCU/canLeaf.h"
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
extern float lastHVACSetpoint_value;
extern float lastFanSpeed_value;
extern bool lastHeating_value;
extern bool lastCooling_value;

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
        lastHVACSetpoint_value = 0.0f;
        lastFanSpeed_value = 0.0f;
        lastHeating_value = false;
        lastCooling_value = false;
    }
}

// Test ZE1 parsing first (since is_ze1 defaults to true)
TEST(CanLeafZE1Parsing, ZE1_Parsing) {
    resetVehicleDetection();
    resetMocks();
    
    // ZE1 SOC (confirms is_ze1 = true)
    uint8_t ze1SocData[8] = {0};
    ze1SocData[7] = 27; // 27 * 0.5 = 13.5%
    handleReceivedMessage(0x59E, ze1SocData, sizeof(ze1SocData));
    EXPECT_NEAR(lastSOC_value, 13.5f, 0.01f);
    
    resetMocks();
    ze1SocData[7] = 16; // 16 * 0.5 = 8.0%
    handleReceivedMessage(0x59E, ze1SocData, sizeof(ze1SocData));
    EXPECT_NEAR(lastSOC_value, 8.0f, 0.01f);
    
    // ZE1 Temp: raw * 0.5 - 40
    resetMocks();
    uint8_t ze1TempData1[] = {0x78}; // 120 * 0.5 - 40 = 20.0f
    handleReceivedMessage(0x54F, ze1TempData1, sizeof(ze1TempData1));
    EXPECT_NEAR(lastTemp_value, 20.0f, 0.01f);
    
    // ZE1 Temp Sentinel
    resetMocks();
    uint8_t ze1TempSentinel[] = {0x50}; // Sentinel (was previously 0.0f)
    handleReceivedMessage(0x54F, ze1TempSentinel, sizeof(ze1TempSentinel));
    EXPECT_EQ(lastTemp_value, -100.0f); // Unchanged
    
    resetMocks();
    uint8_t ze1TempData3[] = {0x3C}; // 60 * 0.5 - 40 = -10.0f
    handleReceivedMessage(0x54F, ze1TempData3, sizeof(ze1TempData3));
    EXPECT_NEAR(lastTemp_value, -10.0f, 0.01f);
    
    resetMocks();
    uint8_t ze1TempData4[] = {0xA0}; // 160 * 0.5 - 40 = 40.0f
    handleReceivedMessage(0x54F, ze1TempData4, sizeof(ze1TempData4));
    EXPECT_NEAR(lastTemp_value, 40.0f, 0.01f);
}

// Test AZE0 parsing (this will set is_ze1 to false)
TEST(CanLeafZE1Parsing, AZE0_Parsing) {
    resetVehicleDetection();
    resetMocks();
    
    // AZE0 SOC
    uint8_t aze0SocData[8] = {0};
    aze0SocData[4] = 13; // 13% -> Triggers is_ze1 = false
    handleReceivedMessage(0x1DB, aze0SocData, sizeof(aze0SocData));
    EXPECT_NEAR(lastSOC_value, 13.0f, 0.01f);

    resetMocks();
    aze0SocData[4] = 85; // 85%
    handleReceivedMessage(0x1DB, aze0SocData, sizeof(aze0SocData));
    EXPECT_NEAR(lastSOC_value, 85.0f, 0.01f);

    // AZE0 Temp: (raw - 32) * 5/9
    resetMocks();
    uint8_t aze0TempData[] = {68}; // (68 - 32) * 5/9 = 20.0f
    handleReceivedMessage(0x54F, aze0TempData, sizeof(aze0TempData));
    EXPECT_NEAR(lastTemp_value, 20.0f, 0.01f);

    // AZE0 Temp Sentinel
    resetMocks();
    uint8_t aze0TempSentinel[] = {20}; // Sentinel
    handleReceivedMessage(0x54F, aze0TempSentinel, sizeof(aze0TempSentinel));
    EXPECT_EQ(lastTemp_value, -100.0f); // Unchanged
}

// Test car awake parsing
TEST(CanLeafZE1Parsing, CarAwake_Parsing) {
    resetVehicleDetection();
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
    uint8_t hvacData3[] = {0x00, 0x08};
    handleReceivedMessage(0x54B, hvacData3, sizeof(hvacData3));
    EXPECT_FALSE(lastHVAC_value);
    
    resetMocks();
    uint8_t hvacData4[] = {0x00, 0x31};
    handleReceivedMessage(0x54B, hvacData4, sizeof(hvacData4));
    EXPECT_TRUE(lastHVAC_value);
}

// Test HVAC Setpoint parsing
TEST(CanLeafZE1Parsing, HVACSetpoint_Parsing) {
    resetMocks();
    
    // Setpoint is byte 4, divided by 2
    uint8_t hvacData1[] = {0x00, 0x00, 0x00, 0x00, 40, 0x00, 0x00, 0x00}; // 20.0 C
    handleReceivedMessage(0x54A, hvacData1, sizeof(hvacData1));
    EXPECT_NEAR(lastHVACSetpoint_value, 20.0f, 0.01f);
    
    resetMocks();
    uint8_t hvacData2[] = {0x00, 0x00, 0x00, 0x00, 33, 0x00, 0x00, 0x00}; // 16.5 C
    handleReceivedMessage(0x54A, hvacData2, sizeof(hvacData2));
    EXPECT_NEAR(lastHVACSetpoint_value, 16.5f, 0.01f);
}

// Test HVAC Fan Speed and Heating/Cooling Mode parsing
TEST(CanLeafZE1Parsing, HVACAdvancedMetrics_Parsing) {
    resetMocks();
    
    // Heating ON (bit 0), Fan Speed 7 (bits 5:3 = 111 -> 0x38)
    uint8_t hvacData1[] = {0x00, 0x01, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00};
    handleReceivedMessage(0x54B, hvacData1, sizeof(hvacData1));
    EXPECT_TRUE(lastHeating_value);
    EXPECT_FALSE(lastCooling_value);
    EXPECT_NEAR(lastFanSpeed_value, 100.0f, 0.01f);
    
    resetMocks();
    // Cooling ON (bit 4 -> 0x10), Heating OFF, Fan Speed 3 (bits 5:3 = 011 -> 0x18)
    uint8_t hvacData2[] = {0x00, 0x10, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00};
    handleReceivedMessage(0x54B, hvacData2, sizeof(hvacData2));
    EXPECT_FALSE(lastHeating_value);
    EXPECT_TRUE(lastCooling_value);
    EXPECT_NEAR(lastFanSpeed_value, 42.857f, 0.01f);
    
    resetMocks();
    // Heating ON and Cooling ON (0x11), Fan Speed 0 (0x00)
    uint8_t hvacData3[] = {0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    handleReceivedMessage(0x54B, hvacData3, sizeof(hvacData3));
    EXPECT_TRUE(lastHeating_value);
    EXPECT_TRUE(lastCooling_value);
    EXPECT_NEAR(lastFanSpeed_value, 0.0f, 0.01f);
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
