/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	int       pass;
	uintptr_t address;
	uint32_t  expected, read;
} TestError;

typedef void (*TestCallback)(void *arg, int progress, int total);

#ifdef __cplusplus
extern "C" {
#endif

bool testMainRAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
);
bool testVRAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
);
bool testSPURAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
);

#ifdef __cplusplus
}
#endif
