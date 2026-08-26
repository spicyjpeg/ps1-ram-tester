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
#include <stdio.h>
#include "common/reboot.h"
#include "common/spu.h"
#include "main/mainmenu.h"
#include "main/modals.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/spuconfig.h"
#include "main/test.h"
#include "main/ui.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"

/* Menu callbacks */

uint8_t vramSize   =  0;
uint8_t testPasses = 10;

static char mainRAMConfig[32] = "";
static char spuRAMConfig [32] = "";
static char finalResult  [32] = "";
static char mainRAMResult[32] = "";
static char vramResult   [32] = "";
static char spuRAMResult [32] = "";

static const char *testMessage = 0;

extern char _bssEnd[];

static void testCallback(void *arg, int progress, int total) {
	RenderContext *ctx = (RenderContext *) arg;
	char          message[64];

	snprintf(message, sizeof(message), testMessage, progress / 2 + 1);

	beginFrame(ctx);
	renderProgressScreen(ctx, progress, total, message);
	endFrame(ctx);
}

static void runMainRAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running main RAM test... (pass %d)";

	size_t    size = getMainRAMSize();
	TestError error;

	if (testMainRAM(
		&error,
		testCallback,
		ctx,
		(uintptr_t) _bssEnd,
		RAM_BASE | size,
		testPasses
	))
		snprintf(
			mainRAMResult,
			sizeof(mainRAMResult),
			"Passed (%d MB)",
			size / 0x100000
		);
	else
		snprintf(
			mainRAMResult,
			sizeof(mainRAMResult),
			"Error at 0x%08X (pass %d)",
			error.address,
			error.pass + 1
		);

	enterMainMenu(ctx, state, item);
}

static void runVRAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running VRAM test... (pass %d)";

	GPU_GP1 = gp1_vramSize(vramSize);

	TestError error;

	if (testVRAM(
		&error,
		0,
		0,
		0,
		512 << vramSize,
		testPasses
	))
		snprintf(
			vramResult,
			sizeof(vramResult),
			"Passed (%d MB)",
			1 << vramSize
		);
	else
		snprintf(
			vramResult,
			sizeof(vramResult),
			"Error at 0x%06X (pass %d)",
			error.address,
			error.pass + 1
		);

	reloadTextures(ctx);
	enterMainMenu(ctx, state, item);
}

static void runSPURAMTest(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	testMessage = "Running SPU RAM test... (pass %d)";

	size_t    size = getSPURAMSize();
	TestError error;

	if (testSPURAM(
		&error,
		testCallback,
		ctx,
		SPU_RAM_ALLOC_OFFSET,
		size,
		testPasses
	)) {
		int  shift = (size >= 0x100000) ? 20  : 10;
		char unit  = (size >= 0x100000) ? 'M' : 'K';

		snprintf(
			spuRAMResult,
			sizeof(spuRAMResult),
			"Passed (%d %cB)",
			size >> shift,
			unit
		);
	} else {
		snprintf(
			spuRAMResult,
			sizeof(spuRAMResult),
			"Error at 0x%06X (pass %d)",
			error.address,
			error.pass + 1
		);
	}

	enterMainMenu(ctx, state, item);
}

static void runAllTests(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
) {
	runMainRAMTest(ctx, state, item);
	runVRAMTest(ctx, state, item);
	runSPURAMTest(ctx, state, item);
}

/* Main menu */

static const MenuItem mainMenu[] = {
	{
		.name   = "Configure main RAM...",
		.type   = ITEM_ACTION,
		.action = {
			.tag      = mainRAMConfig,
			.callback = enterMainRAMConfigMenu
		}
	}, {
		.name     = "VRAM size",
		.type     = ITEM_ENUM,
		.minValue = 0,
		.maxValue = 1,
		.enum_    = {
			.value = &vramSize,
			.items = (const char *const[]) {
				"1 MB (retail/dev)",
				"2 MB (arcade)"
			}
		}
	}, {
		.name   = "Configure SPU RAM...",
		.type   = ITEM_ACTION,
		.action = {
			.tag      = spuRAMConfig,
			.callback = enterSPURAMConfigMenu
		}
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name     = "Number of test passes",
		.type     = ITEM_INT,
		.minValue =   1,
		.maxValue = 250,
		.int_     = { .value = &testPasses }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Test main RAM, VRAM and SPU RAM",
		.type   = ITEM_ACTION,
		.action = {
			.tag      = finalResult,
			.callback = runAllTests
		}
	}, {
		.name     = "Test main RAM",
		.type     = ITEM_ACTION,
		.action = {
			.tag      = mainRAMResult,
			.callback = runMainRAMTest
		}
	}, {
		.name     = "Test VRAM",
		.type     = ITEM_ACTION,
		.action = {
			.tag      = vramResult,
			.callback = runVRAMTest
		}
	}, {
		.name     = "Test SPU RAM",
		.type     = ITEM_ACTION,
		.action = {
			.tag      = spuRAMResult,
			.callback = runSPURAMTest
		}
	}, {
		.name = "Warning: the screen will flicker while testing VRAM.",
		.type = ITEM_STATIC
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "Boot CD-ROM with current configuration...",
		.type   = ITEM_ACTION,
		.action = { .callback = enterFastRebootMenu }
	}, {
		.name   = "Reboot system (discards configuration)",
		.type   = ITEM_ACTION,
		.action = { .callback = doFullReboot }
	}, {
		.type = ITEM_SEPARATOR
	}, {
		.name   = "About this tool...",
		.type   = ITEM_ACTION,
		.action = { .callback = enterAboutMenu }
	}, {
		.type = ITEM_END
	}
};

void enterMainMenu(RenderContext *ctx, UIState *state, const MenuItem *item) {
	(void) ctx;
	(void) item;

	size_t size  = getSPURAMSize();
	int    shift = (size >= 0x100000) ? 20  : 10;
	char   unit  = (size >= 0x100000) ? 'M' : 'K';

	snprintf(
		mainRAMConfig,
		sizeof(mainRAMConfig),
		"%d MB [0x%04X]",
		getMainRAMSize() / 0x100000,
		DRAM_CTRL & 0xffff
	);
	snprintf(
		spuRAMConfig,
		sizeof(spuRAMConfig),
		"%d %cB [0x%04X]",
		size >> shift,
		unit,
		SPU_RAM_CTRL
	);

	// Somewhat ugly hack, but it works.
	if (
		(mainRAMResult[0] == 'E') ||
		(vramResult[0]    == 'E') ||
		(spuRAMResult[0]  == 'E')
	)
		__builtin_strncpy(finalResult, "Failed", sizeof(finalResult));
	else if (
		(mainRAMResult[0] == 'P') &&
		(vramResult[0]    == 'P') &&
		(spuRAMResult[0]  == 'P')
	)
		__builtin_strncpy(finalResult, "Passed", sizeof(finalResult));
	else
		finalResult[0] = 0;

	state->currentMenu = mainMenu;
	state->menuCursor  = 0;
}
