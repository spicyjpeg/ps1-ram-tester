/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "main/renderer.h"

typedef enum {
	// Static items
	ITEM_END       = 0,
	ITEM_SEPARATOR = 1,
	ITEM_TITLE     = 2,
	ITEM_STATIC    = 3,

	// Selectable items
	ITEM_ACTION = 4,
	ITEM_INT    = 5,
	ITEM_BINARY = 6,
	ITEM_ENUM   = 7
} MenuItemType;

struct UIState;
struct MenuItem;

typedef void (*MenuCallback)(
	RenderContext *,
	struct UIState *,
	const struct MenuItem *
);

typedef struct MenuItem {
	const char *name;
	uint8_t    type, minValue, maxValue, bitLength;

	union {
		struct {
			const char   *tag;
			MenuCallback callback;
		} action;
		struct {
			uint8_t *value;
		} int_;
		struct {
			uint8_t           *value;
			const char *const *items;
		} enum_;
	};
} MenuItem;

typedef struct UIState {
	const MenuItem *currentMenu;

	uint16_t buttons, lastButtons;
	uint16_t buttonsPressed, buttonsRepeating;
	uint8_t  repeatTimer;
	int8_t   menuCursor;
} UIState;

#ifdef __cplusplus
extern "C" {
#endif

void setupUIState(UIState *state);

void renderMenu(RenderContext *ctx, const UIState *state);
void updateMenu(RenderContext *ctx, UIState *state, uint16_t buttons);

void renderProgressScreen(
	RenderContext *ctx,
	int           progress,
	int           total,
	const char    *message
);

#ifdef __cplusplus
}
#endif
