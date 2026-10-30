#ifndef FEATUREFLAGS_H
#define FEATUREFLAGS_H

#include "hardware.h"

// ==============================================================================
// Feature Flags & Compile-Time Configuration
// ==============================================================================

// Second CAN interface (MCP2515 via SPI, e.g. for LilyGO T-2Can board)
// Only possible on boards with an MCP2515 (HAS_SECOND_CAN in
// hardware.h); forced off on boards without one. Can also be disabled
// explicitly via compiler flag on supported boards: e.g. -DUSE_SECOND_CAN=0
#if HAS_SECOND_CAN
#ifndef USE_SECOND_CAN
#define USE_SECOND_CAN 1
#endif
#else
#undef USE_SECOND_CAN
#define USE_SECOND_CAN 0
#endif

#endif // FEATUREFLAGS_H
