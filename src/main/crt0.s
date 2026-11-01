# ps1-ram-tester - (C) 2026 spicyjpeg
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

.set STACK_SIZE, 2048

# We're going to override ps1-bare-metal's _start() with a minimal version that
# moves the stack to a statically allocated buffer, allowing for the end of main
# RAM to be safely tested. Note that this implementation will not invoke global
# constructors and destructors.
.section .text._start, "ax", @progbits
.global  _start
.type    _start, @function

_start:
	la    $gp, _gp
	addiu $sp, $gp, %gprel(_stackBuffer) + STACK_SIZE - 16

	addiu $a0, $gp, %gprel(_bssStart)
	addiu $a2, $gp, %gprel(_bssEnd)
	subu  $a2, $a0
	jal   memset
	li    $a1, 0

	j     main
	nop

.section .sbss._stackBuffer, "aw"
.type    _stackBuffer, @object

_stackBuffer:
	.space STACK_SIZE
