/*
 * ps1-bare-metal - Copyright (C) 2023-2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void softReset(void);

bool isFastRebootCompatible(void);
void softFastReboot(void);
void softFastRebootWithConfig(
	uint16_t ramConfig,
	uint16_t vramSize,
	uint16_t spuRAMConfig
);

#ifdef __cplusplus
}
#endif
