/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "main/renderer.h"

#define FONT_SPACE_WIDTH  4
#define FONT_TAB_WIDTH   64
#define FONT_LINE_HEIGHT 10

#ifdef __cplusplus
extern "C" {
#endif

void printString(
	RenderContext *ctx,
	int           x,
	int           y,
	uint32_t      color,
	const char    *str
);
int getStringWidth(const char *str);

#ifdef __cplusplus
}
#endif
