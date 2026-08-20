#ifndef FEATUREFLAGS_H
#define FEATUREFLAGS_H

// ==============================================================================
// Feature Flags & Compile-Time Configuration
// ==============================================================================

// Second CAN interface (MCP2515 via SPI, e.g. for LilyGO T-2Can board)
// Enabled by default (1). Can be disabled here or overridden via compiler flag:
// e.g. -DUSE_SECOND_CAN=0
#ifndef USE_SECOND_CAN
#define USE_SECOND_CAN 1
#endif

#endif // FEATUREFLAGS_H
