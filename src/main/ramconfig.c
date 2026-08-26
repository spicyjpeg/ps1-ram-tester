/*
 * ps1-ram-tester - (C) 2026 spicyjpeg
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
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
	uint8_t refreshPeriod, fetchDelay;
	uint8_t unknown0, unknown3, unknown6, unknown8, unknown12;
} MainRAMConfig;

static void setMainRAMConfig(const MainRAMConfig *config) {
	assert(config->valid);

	uint32_t value = 0
		| (config->unknown0       <<  0)
		| (config->unknown3       <<  3)
		| (config->refreshPeriod  <<  4)
		| (config->unknown6       <<  6)
		| (config->fetchDelay     <<  7)
		| (config->unknown8       <<  8)
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

	config->unknown0      = (value >>  0) &  7;
	config->unknown3      = (value >>  3) &  1;
	config->refreshPeriod = (value >>  4) &  3;
	config->unknown6      = (value >>  6) &  1;
	config->fetchDelay    = (value >>  7) &  1;
	config->unknown8      = (value >>  8) &  1;
	config->bankSize      = (value >>  8) &  2;
	config->banks         = (value >> 10) &  1;
	config->bankSize     |= (value >> 11) &  1;
	config->unknown12     = (value >> 12) & 15;
	config->valid         = true;
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
		.name     = "Fetch conflict delay",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 1,
		.enum_    = {
			.value = &currentConfig.fetchDelay,
			.items = (const char *const[]) {
				"Disabled",
				"Enabled"
			}
		}
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name      = "Unknown DRAM_CTRL[2:0]",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 7,
		.bitLength = 3,
		.int_      = { .value = &currentConfig.unknown0 }
	}, {
		.name      = "Unknown DRAM_CTRL[3] (timing?)",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 1,
		.bitLength = 1,
		.int_      = { .value = &currentConfig.unknown3 }
	}, {
		.name      = "Unknown DRAM_CTRL[6]",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 1,
		.bitLength = 1,
		.int_      = { .value = &currentConfig.unknown6 }
	}, {
		.name      = "Unknown DRAM_CTRL[8] (size?)",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 1,
		.bitLength = 1,
		.int_      = { .value = &currentConfig.unknown8 }
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
