# ps1-bare-metal - (C) 2023-2026 spicyjpeg
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

.macro KERNEL_FUNC entryPoint, id, name
	.section .text.\name, "ax", @progbits
	.global  \name
	.type    \name, @function

	\name:
		# Kernel APIs are invoked by calling one of the stubs the kernel places
		# at 0xa0, 0xb0 and 0xc0 in RAM, passing the function number in $t1 and
		# any arguments in $a0-$a3. The latter are already in place here per the
		# MIPS calling convention, so we can use a direct jump (tail call) to
		# the stub and set $t1 in its delay slot.
		j     \entryPoint
		li    $t1, \id
.endm

KERNEL_FUNC 0x800000b0, 0x34, kernel_read
KERNEL_FUNC 0x800000b0, 0x35, kernel_write
KERNEL_FUNC 0x800000b0, 0x37, kernel_ioctl
