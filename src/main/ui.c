/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "main/defs.h"
#include "main/font.h"
#include "main/renderer.h"
#include "main/ui.h"
#include "ps1/ctpdef.h"

#define MARGIN_LEFT   16
#define MARGIN_RIGHT  16
#define MARGIN_TOP    20
#define MARGIN_BOTTOM  8

#define ITEM_HEIGHT        (FONT_LINE_HEIGHT + 4)
#define STATIC_ITEM_HEIGHT (FONT_LINE_HEIGHT + 1)
#define SEPARATOR_HEIGHT   (ITEM_HEIGHT / 2)
#define HIGHLIGHT_PADDING  2

#define PROGRESS_BAR_HEIGHT  10
#define PROGRESS_BAR_SPACING  8

#define BUTTON_REPEAT_DELAY 30

enum Color {
	COLOR_WINDOW1     = 0x505050,
	COLOR_WINDOW2     = 0x242424,
	COLOR_WINDOW3     = 0x080808,
	COLOR_HIGHLIGHT1  = 0x40b0c8,
	COLOR_HIGHLIGHT2  = 0x3088a0,
	COLOR_PROGRESS1   = 0x10c048,
	COLOR_PROGRESS2   = 0x007820,
	COLOR_TEXT1       = 0x808080,
	COLOR_TEXT2       = 0x505050,
	COLOR_TEXT3       = 0x383838,
	COLOR_TEXT_ACTIVE = 0x4078a0
};

/* Utilities */

static size_t getMenuLength(const MenuItem *menu) {
	size_t length = 0;

	for (; menu->type != ITEM_END; menu++)
		length++;

	return length;
}

static void moveMenuCursor(UIState *state, int step) {
	const MenuItem *item = &state->currentMenu[state->menuCursor];

	// This is very crude and will break if step is not 1 or -1.
	do {
		state->menuCursor += step;
		item              += step;

		if (state->menuCursor < 0) {
			state->menuCursor = getMenuLength(state->currentMenu) - 1;
			item              = &state->currentMenu[state->menuCursor];
		} else if (item->type == ITEM_END) {
			state->menuCursor = 0;
			item              = state->currentMenu;
		}
	} while (item->type < ITEM_ACTION);
}

static void drawButtonPrompt(RenderContext *ctx, const char *prompt) {
	const char *version = "ps1-ram-tester v" VERSION_STRING;
	int        promptY  =
		ctx->screenHeight - (MARGIN_BOTTOM + FONT_LINE_HEIGHT);

	printString(ctx, MARGIN_LEFT, promptY, COLOR_TEXT3, prompt);
	printString(
		ctx,
		ctx->screenWidth - (MARGIN_RIGHT + getStringWidth(version)),
		promptY,
		COLOR_TEXT3,
		version
	);
}

static void updateUIState(UIState *state, uint16_t buttons) {
	uint16_t changed        = buttons ^ state->lastButtons;
	state->buttonsPressed   = buttons & changed;
	state->buttonsRepeating = buttons & changed;

	if (buttons && !changed) {
		if (state->repeatTimer >= BUTTON_REPEAT_DELAY)
			state->buttonsRepeating |= buttons;
		else
			state->repeatTimer++;
	} else {
		state->repeatTimer = 0;
	}

	state->lastButtons = buttons;
}

void setupUIState(UIState *state) {
	state->currentMenu      = 0;
	state->buttons          = 0;
	state->lastButtons      = 0;
	state->buttonsPressed   = 0;
	state->buttonsRepeating = 0;
	state->repeatTimer      = 0;
	state->menuCursor       = 0;
}

/* Menus */

static const char *menuButtonPrompts[] = {
	[ITEM_ACTION] =
		CH_PS1_DPAD_Y " Move   "
		CH_PS1_CIRCLE_BUTTON CH_PS1_CROSS_BUTTON " Select",
	[ITEM_INT]    = CH_PS1_DPAD_Y " Move   " CH_PS1_DPAD_X " Adjust",
	[ITEM_BINARY] = CH_PS1_DPAD_Y " Move   " CH_PS1_DPAD_X " Adjust",
	[ITEM_ENUM]   = CH_PS1_DPAD_Y " Move   " CH_PS1_DPAD_X " Adjust"
};

void renderMenu(RenderContext *ctx, const UIState *state) {
	drawBackground(ctx);

	const MenuItem *item = state->currentMenu;

	int cursorOffset = state->menuCursor;
	int currentY     = MARGIN_TOP;

	while (item->type != ITEM_END) {
		// Determine the item's state.
		uint32_t color  = COLOR_TEXT1;
		int      height = ITEM_HEIGHT;

		const char *value = 0;
		char       buffer[8];

		switch (item->type) {
			case ITEM_SEPARATOR:
				height = SEPARATOR_HEIGHT;
				goto _nextItem;

			case ITEM_TITLE:
				height = STATIC_ITEM_HEIGHT;
				break;

			case ITEM_STATIC:
				color  = COLOR_TEXT2;
				height = STATIC_ITEM_HEIGHT;
				break;

			case ITEM_ACTION:
				if (item->action.tag)
					value = item->action.tag;
				break;

			case ITEM_INT:
				snprintf(buffer, sizeof(buffer), "%d", *item->int_.value);
				value = buffer;
				break;

			case ITEM_BINARY:
				snprintf(
					buffer,
					sizeof(buffer),
					"%0*b",
					item->bitLength,
					*item->int_.value
				);
				value = buffer;
				break;

			case ITEM_ENUM:
				value = item->enum_.items[*item->enum_.value];
				break;

			default:
				assert(false);
		}

		// Draw the item.
		int valueWidth = getStringWidth(value);

		if (!cursorOffset) {
			drawRect(
				ctx,
				MARGIN_LEFT - HIGHLIGHT_PADDING,
				currentY - HIGHLIGHT_PADDING,
				ctx->screenWidth
					- (MARGIN_LEFT + MARGIN_RIGHT - HIGHLIGHT_PADDING * 2),
				height,
				COLOR_HIGHLIGHT1,
				false
			);

			if (valueWidth)
				drawRect(
					ctx,
					ctx->screenWidth
						- (MARGIN_RIGHT + HIGHLIGHT_PADDING * 3 + valueWidth),
					currentY - HIGHLIGHT_PADDING,
					valueWidth + HIGHLIGHT_PADDING * 4,
					height,
					COLOR_HIGHLIGHT2,
					false
				);
		}

		printString(ctx, MARGIN_LEFT, currentY, color, item->name);
		printString(
			ctx,
			ctx->screenWidth - (MARGIN_RIGHT + valueWidth),
			currentY,
			cursorOffset ? COLOR_TEXT2 : COLOR_TEXT1,
			value
		);

_nextItem:
		item++;
		cursorOffset--;
		currentY += height;
	}

	item = &state->currentMenu[state->menuCursor];

	drawButtonPrompt(ctx, menuButtonPrompts[item->type]);
}

void updateMenu(RenderContext *ctx, UIState *state, uint16_t buttons) {
	updateUIState(state, buttons);

	uint16_t pressed   = state->buttonsPressed;
	uint16_t repeating = state->buttonsRepeating;

	// Handle the currently highlighted item.
	const MenuItem *item = &state->currentMenu[state->menuCursor];
	int            value;

	switch (item->type) {
		case ITEM_ACTION:
			if (pressed & (BTN_PAD_START | BTN_PAD_CIRCLE | BTN_PAD_CROSS))
				item->action.callback(ctx, state, item);
			break;

		case ITEM_INT:
		case ITEM_BINARY:
		case ITEM_ENUM:
			// Note that this assumes item->int_.value aliases to
			// item->enum_.value.
			value = *item->int_.value;

			if (value > item->minValue) {
				if (repeating & BTN_PAD_LEFT)
					*item->int_.value = value - 1;
			} else {
				if (pressed & BTN_PAD_LEFT)
					*item->int_.value = item->maxValue;
			}

			if (value < item->maxValue) {
				if (repeating & BTN_PAD_RIGHT)
					*item->int_.value = value + 1;
			} else {
				if (pressed & BTN_PAD_RIGHT)
					*item->int_.value = item->minValue;
			}
			break;

		default:
			assert(false);
	}

	// Handle menu navigation.
	uint16_t upMask   = !state->menuCursor         ? pressed : repeating;
	uint16_t downMask = (item[1].type == ITEM_END) ? pressed : repeating;

	if (upMask   & BTN_PAD_UP)
		moveMenuCursor(state, -1);
	if (downMask & BTN_PAD_DOWN)
		moveMenuCursor(state,  1);
}

/* Progress bar screen */

void renderProgressScreen(
	RenderContext *ctx,
	int           progress,
	int           total,
	const char    *message
) {
	drawBackground(ctx);

	int textY      = (ctx->screenHeight - PROGRESS_BAR_SPACING) / 2;
	int barY       = (ctx->screenHeight + PROGRESS_BAR_SPACING) / 2;
	int totalWidth = ctx->screenWidth - (MARGIN_LEFT + MARGIN_RIGHT);

	printString(
		ctx,
		MARGIN_LEFT,
		textY - FONT_LINE_HEIGHT,
		COLOR_TEXT1,
		message
	);
	drawRect(
		ctx,
		MARGIN_LEFT,
		barY,
		totalWidth,
		PROGRESS_BAR_HEIGHT,
		COLOR_WINDOW3,
		true
	);
	drawGradientRectH(
		ctx,
		MARGIN_LEFT,
		barY,
		(totalWidth * progress) / total,
		PROGRESS_BAR_HEIGHT,
		COLOR_PROGRESS2,
		COLOR_PROGRESS1,
		false
	);

	drawButtonPrompt(ctx, 0);
}
