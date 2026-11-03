/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "main/defs.h"
#include "main/mainmenu.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/registers.h"

/* Utilities */

typedef struct {
	uint8_t valid;
	uint8_t bankSize, banks;
	uint8_t byteCAS, refreshPeriod, refreshTimeout, accessDelay;
	uint8_t unknown6, unknown12;
} MainRAMConfig;

static void setMainRAMConfig(const MainRAMConfig *config) {
	assert(config->valid);

	// TODO: add support for different /RAS0 and /RAS1 bank sizes
	uint32_t value = 0
		| (config->refreshTimeout <<  0)
		| (config->byteCAS        <<  3)
		| (config->refreshPeriod  <<  4)
		| (config->unknown6       <<  6)
		| (config->accessDelay    <<  7)
		| ((config->bankSize & 2) <<  7)
		| ((config->bankSize & 2) <<  8)
		| (config->banks          << 10)
		| ((config->bankSize & 1) << 11)
		| (config->unknown12      << 12);

	DRAM_CTRL = value;
	LOG("new value: 0x%04x", value);
}

static void getMainRAMConfig(MainRAMConfig *config) {
	uint32_t value = DRAM_CTRL;
	LOG("current value: 0x%04x", value & 0xffff);

	config->refreshTimeout = (value >>  0) &  7;
	config->byteCAS        = (value >>  3) &  1;
	config->refreshPeriod  = (value >>  4) &  3;
	config->unknown6       = (value >>  6) &  1;
	config->accessDelay    = (value >>  7) &  1;
	config->bankSize       = (value >>  8) &  2;
	config->banks          = (value >> 10) &  1;
	config->bankSize      |= (value >> 11) &  1;
	config->unknown12      = (value >> 12) & 15;
	config->valid          = true;
}

size_t getMainRAMSize(void) {
	uint32_t value = DRAM_CTRL;

	int bankSize = (value >>  8) & 2;
	int banks    = (value >> 10) & 1;
	bankSize    |= (value >> 11) & 1;

	return 0x100000 << (bankSize + banks);
}

void fixRetailRAMConfig(void) {
	// The retail BIOS accidentally configures main RAM as 8 MB. To avoid
	// confusing the user with incorrect defaults, we'll check for this
	// configuration and shrink it to 2 MB. This will of course result in the
	// tester defaulting to 2 MB on development kits as well, but that is an
	// acceptable tradeoff.
	uint32_t value = DRAM_CTRL;

	if (
		((value & DRAM_CTRL_SIZE_BITMASK)  == DRAM_CTRL_SIZE_8MB) &&
		((value & DRAM_CTRL_BANKS_BITMASK) == DRAM_CTRL_BANKS_1)
	)
		DRAM_CTRL = (value & ~DRAM_CTRL_SIZE_BITMASK) | DRAM_CTRL_SIZE_2MB;
}

/* Menu callbacks */

static MainRAMConfig initialConfig = { .valid = false };
static MainRAMConfig currentConfig = { .valid = false };

static void applyConfig(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	setMainRAMConfig(&currentConfig);
	enterMainMenu(ctx, state, item);
}

static void resetConfig(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) ctx;
	(void) item;

	currentConfig     = initialConfig;
	state->menuCursor = 0;
}

/* RAM configuration menu */

static const MenuItem ramConfigMenu[] = {
	{
		.name     = "Bank size",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 3,
		.enum_    = {
			.value = &currentConfig.bankSize,
			.items = (const char *const[]) {
				"1 MB",
				"2 MB (retail/arcade)",
				"4 MB",
				"8 MB (dev/arcade)"
			}
		}
	}, {
		.name     = "Active banks",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 1,
		.enum_    = {
			.value = &currentConfig.banks,
			.items = (const char *const[]) {
				"/RAS0 only (retail/dev)",
				"/RAS0 + /RAS1 (arcade)"
			}
		}
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name      = "/CAS and /WE wiring",
		.type      = ITEM_ENUM,
		.minValue  = 0,
		.maxValue  = 1,
		.enum_     = {
			.value = &currentConfig.byteCAS,
			.items = (const char *const[]) {
				"Common /CAS, per-byte /WE",
				"Per-byte /CAS, common /WE"
			}
		}
	}, {
		.name     = "Refresh period",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 3,
		.enum_    = {
			.value = &currentConfig.refreshPeriod,
			.items = (const char *const[]) {
				"256 cycles",
				"320 cycles",
				"384 cycles",
				"448 cycles"
			}
		}
	}, {
		.name     = "Pending refresh timeout",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 7,
		.enum_    = {
			.value = &currentConfig.refreshTimeout,
			.items = (const char *const[]) {
				"Refresh immediately",
				"4 cycles",
				"8 cycles",
				"16 cycles",
				"32 cycles",
				"64 cycles",
				"128 cycles",
				"Unlimited"
			}
		}
	}, {
		.name     = "Consecutive access delay",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 1,
		.enum_    = {
			.value = &currentConfig.accessDelay,
			.items = (const char *const[]) {
				"None",
				"1 cycle"
			}
		}
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name      = "Unknown DRAM_CTRL[6]",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 1,
		.bitLength = 1,
		.int_      = { .value = &currentConfig.unknown6 }
	}, {
		.name      = "Unknown DRAM_CTRL[15:12]",
		.type      = ITEM_BINARY,
		.minValue  =  0,
		.maxValue  = 15,
		.bitLength =  4,
		.int_      = { .value = &currentConfig.unknown12 }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Apply settings and exit",
		.type   = ITEM_ACTION,
		.action = { .callback = applyConfig }
	}, {
		.name   = "Discard settings and exit",
		.type   = ITEM_ACTION,
		.action = { .callback = enterMainMenu }
	}, {
		.name   = "Reset settings to initial values",
		.type   = ITEM_ACTION,
		.action = { .callback = resetConfig }
	}, {
		.type = ITEM_END
	}
};

void enterMainRAMConfigMenu(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) ctx;
	(void) item;

	getMainRAMConfig(&currentConfig);

	if (!initialConfig.valid)
		initialConfig = currentConfig;

	state->currentMenu = ramConfigMenu;
	state->menuCursor  = 0;
}
