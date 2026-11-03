/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "common/gpu.h"

typedef struct {
	GPUDMAChain dmaChains[2];
	TextureInfo background, font;

	uint32_t frameCounter;
	uint16_t screenWidth, screenHeight;
} RenderContext;

#ifdef __cplusplus
extern "C" {
#endif

static inline GPUDMAChain *__attribute__((always_inline)) getCurrentChain(
	RenderContext *ctx
) {
	return &ctx->dmaChains[ctx->frameCounter % 2];
}

void setupRenderer(RenderContext *ctx, int width, int height);
void reloadTextures(RenderContext *ctx);
void beginFrame(RenderContext *ctx);
void endFrame(RenderContext *ctx);

void drawBackground(RenderContext *ctx);
void drawRect(
	RenderContext *ctx,
	int           x,
	int           y,
	int           width,
	int           height,
	uint32_t      color,
	bool          blend
);
void drawGradientRectH(
	RenderContext *ctx,
	int           x,
	int           y,
	int           width,
	int           height,
	uint32_t      leftColor,
	uint32_t      rightColor,
	bool          blend
);

#ifdef __cplusplus
}
#endif
