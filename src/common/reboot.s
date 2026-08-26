# ps1-bare-metal - (C) 2023-2025 spicyjpeg
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

.set noreorder
.set noat

.set DRAM_CTRL,    0xbf801060
.set GPU_GP1,      0xbf801814
.set SPU_RAM_CTRL, 0xbf801dac

.set GP1_CMD_VRAM_SIZE, 9 << 24

.set COP0_DCIC, $7

.set ptr,          $a0
.set ramConfig,    $a1
.set vramSize,     $a2
.set spuRAMConfig, $a3

.section .text._fastRebootBreakVector, "ax", @progbits
.global _fastRebootBreakVector
.type _fastRebootBreakVector, @function

_fastRebootBreakVector:
	# When performing a fast reboot (i.e. forcing the BIOS to skip running the
	# shell and move immediately onto booting the CD-ROM), this 16-byte stub is
	# going to be placed at the address the CPU jumps to when a COP0 breakpoint
	# is hit (0x80000040). The breakpoint will be configured to trip when the
	# BIOS relocates the first byte of the shell to RAM; once that stage is
	# reached the code below will proceed to remove the breakpoint, undo the
	# write, then hijack the control flow and force the copying function to
	# return before anything else is copied.
	mtc0  $0, COP0_DCIC
	sw    $0, -1($a0)

	jr    $ra
	rfe

.section .text._fastRebootDummyShell, "ax", @progbits
.global _fastRebootDummyShell
.type _fastRebootDummyShell, @function

_fastRebootDummyShell:
	# Since the COP0 breakpoint will not prevent the BIOS from attempting to run
	# the shell it "loaded", this dummy function will be copied in its place.
	# This is the simplest possible implementation of a shell: once it returns,
	# the kernel will proceed to launch the boot executable on the CD-ROM. Note
	# that the first instruction here will be overwritten with a nop by
	# _fastRebootBreakVector().
	nop
	jr    $ra
	nop
	nop

.section .text._fastRebootWithConfigShell, "ax", @progbits
.global _fastRebootWithConfigShell
.type _fastRebootWithConfigShell, @function

_fastRebootWithConfigShell:
	# This is a slightly more complex dummy shell that applies a custom main RAM
	# and VRAM configuration before returning. The first instruction will still
	# be overwritten by _fastRebootBreakVector(), while the 0x1234 immediates
	# will be patched by softFastRebootWithConfig() with the appropriate
	# configuration values.
	nop
	lui   vramSize, GP1_CMD_VRAM_SIZE >> 16
	ori   ramConfig,    $0, 0x1234
	ori   vramSize,         0x1234
	ori   spuRAMConfig, $0, 0x1234

	lui   ptr,          %hi(DRAM_CTRL)
	sw    ramConfig,    %lo(DRAM_CTRL)   (ptr)
	sw    vramSize,     %lo(GPU_GP1)     (ptr)
	sh    spuRAMConfig, %lo(SPU_RAM_CTRL)(ptr)

	jr    $ra
	nop
	nop
