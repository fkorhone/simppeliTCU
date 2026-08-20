#ifndef HARDWARE_H
#define HARDWARE_H

// ==============================================================================
// Hardware Pin Configuration
// ==============================================================================

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

#endif // HARDWARE_H
