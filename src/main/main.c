/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "common/gpu.h"
#include "common/sio0.h"
#include "common/spu.h"
#include "main/defs.h"
#include "main/mainmenu.h"
#include "main/ramconfig.h"
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/gpucmd.h"

#define SCREEN_HRES   GP1_HRES_320
#define SCREEN_VRES   GP1_VRES_256
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

int main(int argc, const char **argv) {
	(void) argc;
	(void) argv;

#ifdef ENABLE_KERNEL_LOGGING
	initKernelTTY();
#endif
#ifdef ENABLE_SIO1_LOGGING
	initSIO1TTY(115200);
#endif

	LOG("ps1-ram-tester " VERSION_STRING " (" __DATE__ " " __TIME__ ")");
	LOG("(C) 2026 spicyjpeg");

	GP1VideoMode mode = getCurrentVideoMode();

	LOG("using %s mode", (mode == GP1_MODE_PAL) ? "PAL" : "NTSC");

	setupGPU(mode, SCREEN_HRES, SCREEN_VRES, SCREEN_WIDTH, SCREEN_HEIGHT);
	initSPU();
	initSIO0();
	fixRetailRAMConfig();

	static RenderContext ctx;
	static UIState       state;

	setupRenderer(&ctx, SCREEN_WIDTH, SCREEN_HEIGHT);
	setupUIState(&state);
	enterMainMenu(&ctx, &state, 0);

	for (;;) {
		beginFrame(&ctx);
		renderMenu(&ctx, &state);
		updateMenu(&ctx, &state, pollController(0) | pollController(1));
		endFrame(&ctx);
	}

	return 0;
}
