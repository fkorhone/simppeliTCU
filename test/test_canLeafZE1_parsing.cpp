#include <gtest/gtest.h>
#include <cstring>
#include <vector>
#include "../simppeliTCU/canLeafZE1.h"
#include "../simppeliTCU/vehicleTypes.h"

struct SentCANMessage {
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
};

// Mock callback functions to capture parsed values - these are defined in test_canInterface_stubs.cpp
extern std::vector<SentCANMessage> sentCANMessages;
extern bool carIsAwake;
extern int nagScreenPopupCount;
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
        nagScreenPopupCount = 0;
        sentCANMessages.clear();
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

// Test nag screen popup detection and one-shot per boot cycle
TEST(CanLeafZE1Parsing, NagScreenPopup_DetectionAndRearm) {
    resetMocks();

    // Cluster boot init frame (seq 0) arms popup detection and resets sequence counter to 1
    uint8_t clusterBoot0[] = {0x07, 0x00, 0x00, 0x20, 0x00, 0x01, 0x09, 0x00};
    uint8_t clusterBoot1[] = {0x07, 0x10, 0x00, 0x2C, 0x00, 0x01, 0x09, 0x00};
    handleReceivedMessage(0x683, clusterBoot0, sizeof(clusterBoot0));
    handleReceivedMessage(0x683, clusterBoot1, sizeof(clusterBoot1));
    EXPECT_EQ(nagScreenPopupCount, 0);

    // Earlier 0x628 frames (seq 0x20 and seq 0x30) must not trigger popup callback
    uint8_t displayInit[]  = {0x04, 0x00, 0x00, 0x21, 0xD0, 0xFF, 0xFF, 0xFF};
    uint8_t displayEarly[] = {0x10, 0x09, 0x20, 0x2C, 0x0D, 0x88, 0x02, 0x00};
    uint8_t displaySeq30[] = {0x10, 0x24, 0x30, 0x20, 0x0D, 0x00, 0x00, 0x01};
    handleReceivedMessage(0x628, displayInit, sizeof(displayInit));
    handleReceivedMessage(0x628, displayEarly, sizeof(displayEarly));
    handleReceivedMessage(0x628, displaySeq30, sizeof(displaySeq30));
    EXPECT_EQ(nagScreenPopupCount, 0);

    // 0x628 seq 0x40 triggers popup callback once
    uint8_t displaySeq40[] = {0x10, 0x2A, 0x40, 0x20, 0x0D, 0x00, 0x00, 0x01};
    handleReceivedMessage(0x628, displaySeq40, sizeof(displaySeq40));
    EXPECT_EQ(nagScreenPopupCount, 1);

    // Subsequent 0x62C popup status messages in the same boot cycle do not re-trigger
    uint8_t popup62C[] = {0x10, 0x18, 0x40, 0x20, 0x0D, 0x87, 0x81, 0xC6};
    handleReceivedMessage(0x62C, popup62C, sizeof(popup62C));
    EXPECT_EQ(nagScreenPopupCount, 1);

    // Re-arm via 0x628 boot init frame (seq 0) and trigger via 0x62C
    handleReceivedMessage(0x628, displayInit, sizeof(displayInit));
    handleReceivedMessage(0x62C, popup62C, sizeof(popup62C));
    EXPECT_EQ(nagScreenPopupCount, 2);
}

// Test DISMISS_NAG sequence timing and rolling sequence counter on 0x683
TEST(CanLeafZE1Parsing, DismissNag_SequenceExecution) {
    resetMocks();

    // Simulate cluster boot init (seq 0 -> 1) and ensure 0xE0/0xF0 frames on 0x683 are ignored
    uint8_t clusterBoot0[] = {0x07, 0x00, 0x00, 0x20, 0x00, 0x01, 0x09, 0x00};
    uint8_t clusterBoot1[] = {0x07, 0x10, 0x00, 0x2C, 0x00, 0x01, 0x09, 0x00};
    uint8_t keepAlive[]    = {0xF0, 0x64, 0x0A, 0x01, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t ackFrame[]     = {0xE0, 0x30, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    handleReceivedMessage(0x683, clusterBoot0, sizeof(clusterBoot0));
    handleReceivedMessage(0x683, clusterBoot1, sizeof(clusterBoot1));
    handleReceivedMessage(0x683, keepAlive, sizeof(keepAlive));
    handleReceivedMessage(0x683, ackFrame, sizeof(ackFrame));

    startSequence(CanSequence::DISMISS_NAG, 1000);
    EXPECT_EQ(activeSequence, CanSequence::DISMISS_NAG);

    // Step 0: 1500 ms settle wait (no frames sent during wait)
    EXPECT_EQ(manageCANSequence(1000), CanSeqResult::PROCESSING);
    EXPECT_TRUE(sentCANMessages.empty());
    EXPECT_EQ(manageCANSequence(2499), CanSeqResult::PROCESSING);
    EXPECT_TRUE(sentCANMessages.empty());

    // t = 2500 ms (T + 0 ms): Frame 1 (press, seq 0x20)
    EXPECT_EQ(manageCANSequence(2500), CanSeqResult::PROCESSING);
    ASSERT_EQ(sentCANMessages.size(), 1u);
    const uint8_t expected1[8] = {0x04, 0x20, 0x20, 0x0D, 0x88, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(sentCANMessages[0].id, 0x683u);
    EXPECT_EQ(0, std::memcmp(sentCANMessages[0].data, expected1, 8));

    // t = 2600 ms (T + 100 ms): Frame 2 (hold, seq 0x30)
    EXPECT_EQ(manageCANSequence(2599), CanSeqResult::PROCESSING);
    EXPECT_EQ(sentCANMessages.size(), 1u);
    EXPECT_EQ(manageCANSequence(2600), CanSeqResult::PROCESSING);
    ASSERT_EQ(sentCANMessages.size(), 2u);
    const uint8_t expected2[8] = {0x04, 0x30, 0x20, 0x0D, 0x88, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(sentCANMessages[1].id, 0x683u);
    EXPECT_EQ(0, std::memcmp(sentCANMessages[1].data, expected2, 8));

    // t = 2650 ms (T + 150 ms): Frame 3 (release, seq 0x40)
    EXPECT_EQ(manageCANSequence(2649), CanSeqResult::PROCESSING);
    EXPECT_EQ(sentCANMessages.size(), 2u);
    EXPECT_EQ(manageCANSequence(2650), CanSeqResult::PROCESSING);
    ASSERT_EQ(sentCANMessages.size(), 3u);
    const uint8_t expected3[8] = {0x04, 0x40, 0x20, 0x0D, 0x08, 0xFF, 0xFF, 0xFF};
    EXPECT_EQ(sentCANMessages[2].id, 0x683u);
    EXPECT_EQ(0, std::memcmp(sentCANMessages[2].data, expected3, 8));

    // t = 2700 ms: sequence finishes
    EXPECT_EQ(manageCANSequence(2700), CanSeqResult::SEQUENCE_FINISHED);
    EXPECT_EQ(activeSequence, CanSequence::NONE);
}
