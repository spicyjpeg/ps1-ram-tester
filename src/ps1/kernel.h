/*
 * ps1-bare-metal - Copyright (C) 2023-2026 spicyjpeg
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	KERNEL_FD_STDIN  = 0,
	KERNEL_FD_STDOUT = 1
} KernelFileDescriptor;

typedef enum {
	KERNEL_IOCTL_NBLOCK = ('f' << 8) | 1,
	KERNEL_IOCTL_SCAN   = ('f' << 8) | 2
} KernelIoctl;

/* B0 functions */

int kernel_read (int fd, void        *data, int length);
int kernel_write(int fd, const void  *data, int length);
int kernel_ioctl(int fd, KernelIoctl ioctl, int param);
