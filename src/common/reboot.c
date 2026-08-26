/*
 * ps1-bare-metal - (C) 2023-2025 spicyjpeg
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

#include <stdbool.h>
#include <stdint.h>
#include "ps1/cache.h"
#include "ps1/cop0.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"

#define ROM_ENTRY_POINT     ((void (*)(void)) (DEV2_BASE | 0x00000))
#define ROM_SIGNATURE       ((const char *)   (DEV2_BASE | 0x00108))
#define ROM_ALT_ENTRY_POINT ((void (*)(void)) (DEV2_BASE | 0x00398))

#define RAM_BREAK_VECTOR    ((uint32_t *)     (RAM_BASE | 0x00040))
#define RAM_EXC_VECTOR      ((uint32_t *)     (RAM_BASE | 0x00080))
#define RAM_SHELL_LOAD_ADDR ((void (*)(void)) (RAM_BASE | 0x30000))

/* Full soft reboot */

static void prepareForReboot(void) {
	// Clear the current COP0 state, ensuring interrupts are disabled and the
	// GTE is enabled.
	cop0_setReg(COP0_STATUS, COP0_STATUS_CU0 | COP0_STATUS_CU2);

	// Abort all ongoing DMA transfers, clear all pending IRQ flags and prevent
	// the interrupt controller from generating further IRQs.
	for (int i = DMA_MDEC_IN; i <= DMA_OTC; i++)
		DMA_CHCR(i) = 0;

	IRQ_MASK = 0;
	IRQ_STAT = 0;
	DMA_DPCR = 0;
	DMA_DICR = DMA_DICR_CH_STAT_BITMASK;

	// Clear any leftover GPU and SPU state. Since we are not performing a full
	// GPU reset the last frame rendered will remain on screen, which is useful
	// for displaying a message indicating that a reboot is in progress.
	GPU_GP1 = gp1_resetFIFO();
	GPU_GP1 = gp1_acknowledge();
	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_NONE);

	SPU_ATTR  = 0;
	SPU_MVOLL = 0;
	SPU_MVOLR = 0;
}

void softReset(void) {
	// Jump back to the entry point of the BIOS ROM. While not technically
	// equivalent to a full system reset, this will still result in the BIOS
	// reinitializing most of the hardware and running again.
	prepareForReboot();
	ROM_ENTRY_POINT();
	__builtin_unreachable();
}

/* "Fast" reboot (skipping BIOS shell) */

void _fastRebootBreakVector(void);
void _fastRebootDummyShell(void);
void _fastRebootWithConfigShell(void);

static void performFastReboot(void) {
	// Configure COP0 to run _fastRebootBreakVector() when the first write to
	// the 0x80030000-0x8003ffff range occurs.
	cop0_setReg(COP0_DCIC, 0);
	cop0_setReg(COP0_BDA,  (uint32_t) RAM_SHELL_LOAD_ADDR);
	cop0_setReg(COP0_BDAM, 0xffff0000);
	cop0_setReg(
		COP0_DCIC,
		0
			| COP0_DCIC_DE
			| COP0_DCIC_DAE
			| COP0_DCIC_DW
			| COP0_DCIC_KD
			| COP0_DCIC_UD
			| COP0_DCIC_TR
	);

	// Reconfigure the bus interface to expect a 16-bit ROM on the parallel
	// port. As all parallel port cartridges use 8-bit ROMs, this will prevent
	// the BIOS from successfully reading the magic string from one if present,
	// thus disabling any cartridge hooks that may interfere with ours.
	BIU_DEV0_CTRL = 0
		| (15 << 0) // Write delay
		| ( 3 << 4) // Read delay
		| BIU_CTRL_FLOAT
		| BIU_CTRL_WIDTH_16
		| BIU_CTRL_AUTO_INCR
		| (19 << 16); // Number of address lines

	// Once the breakpoint is configured, jump to the middle of the BIOS entry
	// point in order to skip the aforementioned initialization code.
	ROM_ALT_ENTRY_POINT();
	__builtin_unreachable();
}

bool isFastRebootCompatible(void) {
	// Ensure the BIOS ROM contains the retail version of Sony's the kernel
	// rather than a development/arcade version or a custom kernel, neither of
	// which would not be compatible with the fast reboot hack.
	// TODO: make sure it actually is a retail BIOS
	return !__builtin_memcmp(
		ROM_SIGNATURE,
		"Sony Computer Entertainment Inc.",
		32
	);
}

void softFastReboot(void) {
	if (!isFastRebootCompatible())
		return;

	// Place a dummy shell (a function that returns immediately) at the location
	// the BIOS will try to load the actual shell binary at and set up a COP0
	// breakpoint to protect it from being overwritten.
	prepareForReboot();
	__builtin_memcpy(RAM_BREAK_VECTOR,    &_fastRebootBreakVector, 16);
	__builtin_memcpy(RAM_SHELL_LOAD_ADDR, &_fastRebootDummyShell,  16);
	flushCache();
	performFastReboot();
}

void softFastRebootWithConfig(
	uint16_t ramConfig,
	uint16_t vramSize,
	uint16_t spuRAMConfig
) {
	if (!isFastRebootCompatible())
		return;

	prepareForReboot();
	__builtin_memcpy(RAM_BREAK_VECTOR,    &_fastRebootBreakVector,     16);
	__builtin_memcpy(RAM_SHELL_LOAD_ADDR, &_fastRebootWithConfigShell, 48);

	// Patch the provided main RAM and VRAM configuration values into the third
	// and fourth instructions respectively of the relocated copy of
	// _fastRebootWithConfigShell(). Both instructions hold an immediate value
	// in the bottommost 16 bits.
	uint32_t *ptr = (uint32_t *) RAM_SHELL_LOAD_ADDR;
	ptr[2]        = (ptr[2] & 0xffff0000) | ramConfig;
	ptr[3]        = (ptr[3] & 0xffff0000) | vramSize;
	ptr[4]        = (ptr[4] & 0xffff0000) | spuRAMConfig;

	flushCache();
	performFastReboot();
}
