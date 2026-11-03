# ps1-ram-tester - Copyright (C) 2026 spicyjpeg
# SPDX-License-Identifier: MIT

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
