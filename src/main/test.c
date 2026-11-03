/*
 * ps1-ram-tester - Copyright (C) 2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <stdbool.h>
#include <stdint.h>
#include "common/gpu.h"
#include "common/spu.h"
#include "main/defs.h"
#include "main/test.h"
#include "ps1/registers.h"

#define VRAM_WIDTH     1024
#define VRAM_LINE_SIZE (VRAM_WIDTH * 2)

// TEST_BUFFER_SIZE must be a power of 2 and greater than 2048.
#define TEST_BUFFER_SIZE 16384
#define TEST_DMA_HEIGHT  (TEST_BUFFER_SIZE / VRAM_LINE_SIZE)

/* Test pattern generator */

static uint32_t generatorState, verifierState;

static inline uint32_t __attribute__((always_inline)) xorshift32(
	uint32_t state
) {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state <<  5;
	return state;
}

static void resetState(void) {
	generatorState = 0x7e57c0de;
	verifierState  = 0x7e57c0de;
}

static void generatePattern(uint32_t *data) {
	uint32_t *dataEnd = &data[TEST_BUFFER_SIZE / 4];
	uint32_t state    = generatorState;

	for (; data < dataEnd; data++) {
		*data = state;
		state = xorshift32(state);
	}

	generatorState = state;
}

static bool verifyPattern(
	TestError      *output,
	int            pass,
	uintptr_t      address,
	const uint32_t *data
) {
	const uint32_t *dataStart = data;
	const uint32_t *dataEnd   = &data[TEST_BUFFER_SIZE / 4];
	uint32_t       state      = verifierState;

	for (; data < dataEnd; data++) {
		uint32_t read = *data;

		if (__builtin_expect(read != state, false)) {
			output->pass     = pass;
			output->address  = address
				+ ((uintptr_t) data - (uintptr_t) dataStart);
			output->expected = state;
			output->read     = read;

			// Narrow the address down to a specific byte (little-endian). This
			// is useful when testing main RAM on earlier console models and
			// development units, which stripe four different chips into a
			// 32-bit bus.
			for (uint32_t diff = read ^ state; !(diff & 0xff); diff >>= 8)
				output->address++;

			LOG("mismatch at 0x%08x (pass %d)", output->address, pass + 1);
			LOG("expected 0x%08x, read 0x%08x", state, read);
			return false;
		}

		state = xorshift32(state);
	}

	verifierState = state;
	return true;
}

/* RAM testers */

// In order to minimize the size of .data/.bss (and thus maximize the span of
// testable main RAM), the temporary double buffer for DMA transfers is not
// declared as a global but instead allocated at the beginning of testable main
// RAM during VRAM and SPU RAM testing.
extern char _bssEnd[];

static uint32_t (*const testBuffers)[TEST_BUFFER_SIZE / 4] = (void *) _bssEnd;

bool testMainRAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
) {
	start +=  (TEST_BUFFER_SIZE - 1);
	start &= ~(TEST_BUFFER_SIZE - 1);
	end   &= ~(TEST_BUFFER_SIZE - 1);

	int progress = 0;

	LOG("testing 0x%08x-0x%08x", start, end - 1);
	resetState();

	for (int i = 0; i < passes; i++) {
		if (callback)
			callback(arg, progress++, passes * 2);

		for (uintptr_t ptr = start; ptr < end; ptr += TEST_BUFFER_SIZE)
			generatePattern((uint32_t *) ptr);

		if (callback)
			callback(arg, progress++, passes * 2);

		for (uintptr_t ptr = start; ptr < end; ptr += TEST_BUFFER_SIZE) {
			if (!verifyPattern(output, i, ptr, (const uint32_t *) ptr))
				return false;
		}
	}

	LOG("test passed");
	return true;
}

bool testVRAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
) {
	start +=  (TEST_DMA_HEIGHT - 1);
	start &= ~(TEST_DMA_HEIGHT - 1);
	end   &= ~(TEST_DMA_HEIGHT - 1);

	int progress = 0;

	LOG("testing lines %d-%d", start, end - 1);
	resetState();

	for (int i = 0; i < passes; i++) {
		if (callback)
			callback(arg, progress++, passes * 2);

		// Pipeline the test by transferring each buffer while the next buffer
		// is being generated (or the previous buffer is being verified).
		bool usingSecondBuffer = false;

		for (uintptr_t y = start; y < end; y += TEST_DMA_HEIGHT) {
			uint32_t *buffer  = testBuffers[usingSecondBuffer];
			usingSecondBuffer = !usingSecondBuffer;

			generatePattern(buffer);
			sendVRAMData(buffer, 0, y, VRAM_WIDTH, TEST_DMA_HEIGHT);
		}

		waitForGPUDMADone();
		GPU_GP0 = gp0_flushCache();

		if (callback)
			callback(arg, progress++, passes * 2);

		usingSecondBuffer = false;
		receiveVRAMData(testBuffers[0], 0, start, VRAM_WIDTH, TEST_DMA_HEIGHT);

		for (uintptr_t y = start; y < end; y += TEST_DMA_HEIGHT) {
			uint32_t *buffer     = testBuffers[usingSecondBuffer];
			usingSecondBuffer    = !usingSecondBuffer;
			uint32_t *nextBuffer = testBuffers[usingSecondBuffer];

			receiveVRAMData(
				nextBuffer,
				0,
				y + TEST_DMA_HEIGHT,
				VRAM_WIDTH,
				TEST_DMA_HEIGHT
			);

			if (!verifyPattern(output, i, y * VRAM_LINE_SIZE, buffer))
				return false;
		}
	}

	LOG("test passed");
	return true;
}

bool testSPURAM(
	TestError    *output,
	TestCallback callback,
	void         *arg,
	uintptr_t    start,
	uintptr_t    end,
	int          passes
) {
	start +=  (TEST_BUFFER_SIZE - 1);
	start &= ~(TEST_BUFFER_SIZE - 1);
	end   &= ~(TEST_BUFFER_SIZE - 1);

	int progress = 0;

	LOG("testing 0x%05x-0x%05x", start, end - 1);
	resetState();

	for (int i = 0; i < passes; i++) {
		if (callback)
			callback(arg, progress++, passes * 2);

		bool usingSecondBuffer = false;

		for (uintptr_t ptr = start; ptr < end; ptr += TEST_BUFFER_SIZE) {
			uint32_t *buffer  = testBuffers[usingSecondBuffer];
			usingSecondBuffer = !usingSecondBuffer;

			generatePattern(buffer);
			sendSPURAMData(buffer, ptr, TEST_BUFFER_SIZE);
		}

		waitForSPUDMADone();

		if (callback)
			callback(arg, progress++, passes * 2);

		usingSecondBuffer = false;
		receiveSPURAMData(testBuffers[0], start, TEST_BUFFER_SIZE);

		for (uintptr_t ptr = start; ptr < end; ptr += TEST_BUFFER_SIZE) {
			uint32_t *buffer     = testBuffers[usingSecondBuffer];
			usingSecondBuffer    = !usingSecondBuffer;
			uint32_t *nextBuffer = testBuffers[usingSecondBuffer];

			receiveSPURAMData(
				nextBuffer,
				ptr + TEST_BUFFER_SIZE,
				TEST_BUFFER_SIZE
			);

			if (!verifyPattern(output, i, ptr, buffer))
				return false;
		}
	}

	LOG("test passed");
	return true;
}
