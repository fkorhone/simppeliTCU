#ifndef HARDWARE_H
#define HARDWARE_H

// ==============================================================================
// Board Selection
// ==============================================================================
// Select the target board by defining BOARD, either here or via a
// compiler/build flag, e.g.:
//   -DBOARD=WAVESHARE_ESP32_S3_RS485_CAN
// Default is the LilyGO T-2Can.
// Note: the ESP32 Arduino core itself only defines ARDUINO_BOARD / ARDUINO_<board>,
// not a plain BOARD macro, so this does not clash with the core or the libraries
// used here (PubSubClient, autowp-mcp2515).

#define LILYGO_T2CAN                 1
#define WAVESHARE_ESP32_S3_RS485_CAN 2

#ifndef BOARD
#define BOARD LILYGO_T2CAN
#endif

// ==============================================================================
// Hardware Pin Configuration
// ==============================================================================

#if BOARD == LILYGO_T2CAN

// Primary CAN interface (ESP32 built-in TWAI, "Port B")
#define CAN_TX_PIN 7
#define CAN_RX_PIN 6

// Second CAN interface (external MCP2515 over SPI) - LilyGO T-2Can
#define MCP2515_CS   10
#define MCP2515_SCLK 12
#define MCP2515_MOSI 11
#define MCP2515_MISO 13
#define MCP2515_RST  9
#define MCP2515_INT  8

// SPI clock for the MCP2515
#define MCP2515_SPI_CLOCK 10000000

// This board has an MCP2515, so the second CAN interface is available
#define HAS_SECOND_CAN 1

#elif BOARD == WAVESHARE_ESP32_S3_RS485_CAN

// Primary CAN interface (ESP32 built-in TWAI) - the only CAN interface on
// this board; it has no MCP2515, so the second CAN is not available.
#define CAN_TX_PIN 15
#define CAN_RX_PIN 16

#define HAS_SECOND_CAN 0

#else
#error "Unknown BOARD - see board list at the top of hardware.h"
#endif

#endif // HARDWARE_H
