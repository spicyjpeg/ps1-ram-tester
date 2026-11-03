# ps1-ram-tester - Copyright (C) 2026 spicyjpeg
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.30)

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
	RELEASE_INFO
		"${PROJECT_NAME} ${PROJECT_VERSION} - Copyright (C) 2026 spicyjpeg"
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
