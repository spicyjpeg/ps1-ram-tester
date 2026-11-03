/*
 * ps1-ram-tester - (C) 2026 spicyjpeg
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

#pragma once

#include <stdio.h>

#ifndef VERSION
#define VERSION "<unknown build>"
#endif

#ifdef NDEBUG
#define VERSION_STRING VERSION
#else
#define VERSION_STRING VERSION "-debug"
#endif

#if defined(ENABLE_KERNEL_LOGGING) || defined(ENABLE_SIO1_LOGGING)
#define LOG(fmt, ...) \
	printf("%s(%d): " fmt "\n", __func__, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif

#define CH_PS1_DPAD            "\x80"
#define CH_PS1_DPAD_X          "\x81"
#define CH_PS1_DPAD_Y          "\x82"
#define CH_PS1_CIRCLE_BUTTON   "\x83"
#define CH_PS1_CROSS_BUTTON    "\x84"
#define CH_PS1_TRIANGLE_BUTTON "\x85"
#define CH_PS1_SQUARE_BUTTON   "\x86"
#define CH_PS1_SELECT_BUTTON   "\x87"
#define CH_PS1_START_BUTTON    "\x88"
