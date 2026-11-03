/*
 * ps1-bare-metal - Copyright (C) 2023-2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "common/spu.h"
#include "ps1/delay.h"
#include "ps1/registers.h"

#define DMA_MAX_CHUNK_SIZE 16

// This version of common/spu.c was modified from the one in ps1-bare-metal to
// make the SPU RAM addressing unit configurable at runtime, in order to support
// sizes larger than 512 KB. The dummy ADPCM block was also removed as it's not
// needed here and would be overwritten anyway during SPU RAM testing.
uint8_t spuRAMAddressShift = 3;

void initSPU(void) {
	BIU_DEV4_DELAY = 0
		| BIU_DEV_DELAY_WRITE_CYCLES(1)
		| BIU_DEV_DELAY_READ_CYCLES(14)
		| BIU_DEV_DELAY_RECOVERY
		| BIU_DEV_DELAY_WIDTH_16
		| BIU_DEV_DELAY_AUTO_INCR
		| BIU_DEV_DELAY_ADDR_BITS(9)
		| BIU_DEV_DELAY_DMA_CYCLES(0)
		| BIU_DEV_DELAY_DMA_CYCLES_ENABLE;
	BIU_COM_DELAY = 0
		| BIU_COM_DELAY_RECOVERY(5)
		| BIU_COM_DELAY_HOLD(2)
		| BIU_COM_DELAY_FLOAT(3)
		| BIU_COM_DELAY_PRESTROBE(1);

	SPU_ATTR     = 0
		| SPU_ATTR_XFER_NONE
		| SPU_ATTR_ENABLE;
	SPU_RAM_CTRL = 0
		| SPU_RAM_CTRL_BANKS_1
		| SPU_RAM_CTRL_SIZE_512KB;
	SPU_MVOLL    = 0x3fff & ~SPU_VOL_SLIDE_ENABLE;
	SPU_MVOLR    = 0x3fff & ~SPU_VOL_SLIDE_ENABLE;
	SPU_EVOLL    = 0;
	SPU_EVOLR    = 0;
	SPU_ESA      = 0xfffe;

	DMAC_PCR          |= DMAC_PCR_CH_ENABLE(DMA_SPU);
	DMAC_CHCR(DMA_SPU) = 0;

	resetAllSPUChannels();
	SPU_ATTR = 0
		| SPU_ATTR_XFER_NONE
		| SPU_ATTR_DAC_ENABLE
		| SPU_ATTR_ENABLE;

	spuRAMAddressShift = 3;
}

void waitForSPUDMADone(void) {
	while (DMAC_CHCR(DMA_SPU) & DMAC_CHCR_ENABLE)
		__asm__ volatile("");

	delayMicroseconds(2 * DMA_MAX_CHUNK_SIZE);
}

void sendSPURAMData(const void *data, unsigned int offset, size_t length) {
	waitForSPUDMADone();
	assert(!((uintptr_t) data % 4));
	assert(!(offset % (1 << spuRAMAddressShift)));

	length = (length + 3) / 4;
	size_t chunkSize, numChunks;

	if (length < DMA_MAX_CHUNK_SIZE) {
		chunkSize = length;
		numChunks = 1;
	} else {
		chunkSize = DMA_MAX_CHUNK_SIZE;
		numChunks = length / DMA_MAX_CHUNK_SIZE;

		assert(!(length % DMA_MAX_CHUNK_SIZE));
	}

	uint16_t attr = SPU_ATTR & ~SPU_ATTR_XFER_BITMASK;
	SPU_ATTR      = attr;

	while ((SPU_STATX & SPU_STATX_XFER_BITMASK) != SPU_STATX_XFER_NONE)
		__asm__ volatile("");

	SPU_TSA  = offset >> spuRAMAddressShift;
	SPU_ATTR = attr | SPU_ATTR_XFER_DMA_WRITE;

	while ((SPU_STATX & SPU_STATX_XFER_BITMASK) != SPU_STATX_XFER_DMA_WRITE)
		__asm__ volatile("");

	// SPU RAM writes can be performed with the default bus configuration, while
	// reads require slightly increasing DMA waitstates.
	uint32_t ctrl  = BIU_DEV4_DELAY & ~BIU_DEV_DELAY_DMA_CYCLES_BITMASK;
	BIU_DEV4_DELAY = ctrl           |  BIU_DEV_DELAY_DMA_CYCLES(0);

	DMAC_MADR(DMA_SPU) = (uintptr_t) data;
	DMAC_BCR (DMA_SPU) = chunkSize | (numChunks << 16);
	DMAC_CHCR(DMA_SPU) = 0
		| DMAC_CHCR_DIR_WRITE
		| DMAC_CHCR_MODE_SLICE
		| DMAC_CHCR_ENABLE;
}

void receiveSPURAMData(void *data, unsigned int offset, size_t length) {
	waitForSPUDMADone();
	assert(!((uintptr_t) data % 4));
	assert(!(offset % (1 << spuRAMAddressShift)));

	length = (length + 3) / 4;
	size_t chunkSize, numChunks;

	if (length < DMA_MAX_CHUNK_SIZE) {
		chunkSize = length;
		numChunks = 1;
	} else {
		chunkSize = DMA_MAX_CHUNK_SIZE;
		numChunks = length / DMA_MAX_CHUNK_SIZE;

		assert(!(length % DMA_MAX_CHUNK_SIZE));
	}

	uint16_t attr = SPU_ATTR & ~SPU_ATTR_XFER_BITMASK;
	SPU_ATTR      = attr;

	while ((SPU_STATX & SPU_STATX_XFER_BITMASK) != SPU_STATX_XFER_NONE)
		__asm__ volatile("");

	SPU_TSA  = offset >> spuRAMAddressShift;
	SPU_ATTR = attr | SPU_ATTR_XFER_DMA_READ;

	while ((SPU_STATX & SPU_STATX_XFER_BITMASK) != SPU_STATX_XFER_DMA_READ)
		__asm__ volatile("");

	uint32_t ctrl  = BIU_DEV4_DELAY & ~BIU_DEV_DELAY_DMA_CYCLES_BITMASK;
	BIU_DEV4_DELAY = ctrl           |  BIU_DEV_DELAY_DMA_CYCLES(2);

	DMAC_MADR(DMA_SPU) = (uintptr_t) data;
	DMAC_BCR (DMA_SPU) = chunkSize | (numChunks << 16);
	DMAC_CHCR(DMA_SPU) = 0
		| DMAC_CHCR_DIR_READ
		| DMAC_CHCR_MODE_SLICE
		| DMAC_CHCR_ENABLE;
}

void resetAllSPUChannels(void) {
	// Mute all channels and point them to the beginning of SPU RAM to avoid
	// needing a dummy sample. Doing this is technically invalid as the first 4
	// KB of SPU RAM contain PCM capture buffers rather than ADPCM samples, but
	// it does not matter as we're also "freezing" them by setting the pitch to
	// zero.
	for (int i = 0; i < SPU_NUM_CHANNELS; i++) {
		SPU_VOLL (i) = 0;
		SPU_VOLR (i) = 0;
		SPU_PITCH(i) = 0;
		SPU_SSA  (i) = 0;
	}

	SPU_PMON0 = 0;
	SPU_PMON1 = 0;
	SPU_NON0  = 0;
	SPU_NON1  = 0;
	SPU_EON0  = 0;
	SPU_EON1  = 0;

	SPU_KON0 = 0xffff;
	SPU_KON1 = 0x00ff;
}

int findFreeSPUChannel(void) {
	for (int i = 0; i < SPU_NUM_CHANNELS; i++) {
		if (!SPU_ENVX(i))
			return i;
	}

	return -1;
}

int playSample(unsigned int offset, unsigned int sampleRate, int16_t volume) {
	int ch = findFreeSPUChannel();

	if (ch >= 0) {
		SPU_VOLL (ch) = (volume / 2) & ~SPU_VOL_SLIDE_ENABLE;
		SPU_VOLR (ch) = (volume / 2) & ~SPU_VOL_SLIDE_ENABLE;
		SPU_PITCH(ch) = (sampleRate * SPU_PITCH_UNIT) / 44100;
		SPU_SSA  (ch) = offset >> spuRAMAddressShift;

		SPU_ADSR1(ch) = 0
			| SPU_ADSR1_SL(15)
			| SPU_ADSR1_DR(15)
			| SPU_ADSR1_AR(0);
		SPU_ADSR2(ch) = 0
			| SPU_ADSR2_RR(0)
			| SPU_ADSR2_SR(0);
		SPU_ENVX (ch) = volume;

		if (ch < 16)
			SPU_KON0 = 1 << ch;
		else
			SPU_KON1 = 1 << (ch - 16);
	}

	return ch;
}
