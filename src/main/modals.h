/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include "main/renderer.h"
#include "main/ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void enterFastRebootMenu(
	RenderContext  *ctx,
	UIState        *state,
	const MenuItem *item
);
void doFullReboot(RenderContext *ctx, UIState *state, const MenuItem *item);

void enterAboutMenu(RenderContext *ctx, UIState *state, const MenuItem *item);

#ifdef __cplusplus
}
#endif
