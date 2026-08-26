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

#include <stddef.h>
#include <stdint.h>
#include "common/spu.h"
#include "main/defs.h"
#include "main/mainmenu.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/registers.h"

/* Utilities */

typedef struct {
	uint8_t bankSize, banks;
	uint8_t unknown0, unknown4, unknown8, unknown12;
} SPURAMConfig;

static void setSPURAMConfig(const SPURAMConfig *config) {
	uint16_t value = 0
		| (config->unknown0       <<  0)
		| (config->banks          <<  1)
		| ((config->bankSize + 1) <<  2)
		| (config->unknown4       <<  4)
		| (config->unknown8       <<  8)
		| (config->unknown12      << 12);

	SPU_RAM_CTRL = value;
	LOG("new value: 0x%04x", value);

	spuRAMAddressShift = 3 + config->bankSize * 2 + config->banks;
}

static void getSPURAMConfig(SPURAMConfig *config) {
	uint16_t value = SPU_RAM_CTRL;
	LOG("current value: 0x%04x", value);

	config->unknown0  = (value  >>  0) &  1;
	config->banks     = (value  >>  1) &  1;
	config->bankSize  = ((value >>  2) &  3) - 1;
	config->unknown4  = (value  >>  4) & 15;
	config->unknown8  = (value  >>  8) & 15;
	config->unknown12 = (value  >> 12) & 15;
}

size_t getSPURAMSize(void) {
	uint16_t value = SPU_RAM_CTRL;

	int banks    = (value  >> 1) & 1;
	int bankSize = ((value >> 2) & 3) - 1;

	return 0x80000 << (bankSize * 2 + banks);
}

/* Menu callbacks */

static SPURAMConfig currentConfig;

static void applyConfig(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	setSPURAMConfig(&currentConfig);
	enterMainMenu(ctx, state, item);
}

static void resetConfig(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) ctx;
	(void) item;

	__builtin_memset(&currentConfig, 0, sizeof(currentConfig));
	state->menuCursor = 0;
}

/* RAM configuration menu */

static const MenuItem spuRAMConfigMenu[] = {
	{
		.name     = "Bank size",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 1,
		.enum_    = {
			.value = &currentConfig.bankSize,
			.items = (const char *const[]) {
				"512 KB (stock)",
				"2 MB"
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
				"/OE0 only (stock)",
				"/OE0 + /OE1"
			}
		}
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name      = "Unknown SPU_RAM_CTRL[0]",
		.type      = ITEM_BINARY,
		.minValue  = 0,
		.maxValue  = 1,
		.bitLength = 1,
		.int_      = { .value = &currentConfig.unknown0 }
	}, {
		.name      = "Unknown SPU_RAM_CTRL[7:4]",
		.type      = ITEM_BINARY,
		.minValue  =  0,
		.maxValue  = 15,
		.bitLength =  4,
		.int_      = { .value = &currentConfig.unknown4 }
	}, {
		.name      = "Unknown SPU_RAM_CTRL[11:8]",
		.type      = ITEM_BINARY,
		.minValue  =  0,
		.maxValue  = 15,
		.bitLength =  4,
		.int_      = { .value = &currentConfig.unknown8 }
	}, {
		.name      = "Unknown SPU_RAM_CTRL[15:12]",
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

void enterSPURAMConfigMenu(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	(void) ctx;
	(void) item;

	getSPURAMConfig(&currentConfig);
	state->currentMenu = spuRAMConfigMenu;
	state->menuCursor  = 0;
}
