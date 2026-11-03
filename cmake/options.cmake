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

cmake_minimum_required(VERSION 4.0)

## External tools

find_program(
	CHDMAN_PATH chdman
	PATHS
		"C:/Program Files/MAME"
		"C:/Program Files (x86)/MAME"
		"/opt/mame"
	DOC "Path to MAME chdman tool for generating CHD image (optional)"
)

## Release information

set(
	RELEASE_INFO "${PROJECT_NAME} ${PROJECT_VERSION} - (C) 2026 spicyjpeg"
	CACHE STRING "Executable description and version string, placed in the \
executable header (optional)"
)
set(
	RELEASE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}"
	CACHE STRING "CD-ROM image and release package file name"
)

string(TOUPPER "${RELEASE_NAME}" _cdVolumeName)
string(REGEX REPLACE "[^0-9A-Z_]" "_" _cdVolumeName "${_cdVolumeName}")

set(
	CD_VOLUME_NAME "${_cdVolumeName}"
	CACHE STRING "CD-ROM image volume label"
)

## Compile-time options

set(
	ENABLE_KERNEL_LOGGING OFF
	CACHE BOOL "Enable debug logging to BIOS kernel TTY"
)
set(
	ENABLE_SIO1_LOGGING OFF
	CACHE BOOL "Enable debug logging to serial port (SIO1)"
)
