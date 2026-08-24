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

cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain.cmake")

project(
	ps1-ram-tester
	LANGUAGES    C ASM
	VERSION      1.0.0
	DESCRIPTION  "PlayStation 1 RAM, VRAM and SPU RAM testing tool"
	HOMEPAGE_URL "https://github.com/spicyjpeg/ps1-ram-tester"
)

include(cmake/options.cmake)
include(cmake/setup.cmake)
include(cmake/tools.cmake)

## Standard library

add_library(
	common OBJECT
	src/libc/clz.s
	#src/libc/crt0.c
	src/libc/misc.c
	src/libc/string.c
	src/libc/string.s
	src/ps1/cache.s
	src/vendor/printf.c
)
target_include_directories(
	common PUBLIC
	src
	src/libc
)
link_libraries(common)

## Main executable

addPS1Executable(
	"${RELEASE_NAME}"
	"${RELEASE_INFO}"
	src/common/gpu.c
	src/common/reboot.c
	src/common/reboot.s
	src/common/sio0.c
	src/common/spu.c
	src/main/crt0.s
	src/main/font.c
	src/main/main.c
	src/main/mainmenu.c
	src/main/modals.c
	src/main/ramconfig.c
	src/main/renderer.c
	src/main/test.c
	src/main/ui.c
)
target_compile_options(
	"${RELEASE_NAME}" PRIVATE
	-Wall
	-Wextra
)
target_compile_definitions(
	"${RELEASE_NAME}" PRIVATE
	$<$<BOOL:${ENABLE_LOGGING}>:ENABLE_LOGGING=1>
	VERSION="${PROJECT_VERSION}"
)

convertImage(assets/background.png 4 assets/bgTexture.dat assets/bgPalette.dat)
addBinaryFile("${RELEASE_NAME}" bgTexture "${PROJECT_BINARY_DIR}/assets/bgTexture.dat")
addBinaryFile("${RELEASE_NAME}" bgPalette "${PROJECT_BINARY_DIR}/assets/bgPalette.dat")

convertImage(assets/font.png 4 assets/fontTexture.dat assets/fontPalette.dat)
addBinaryFile("${RELEASE_NAME}" fontTexture "${PROJECT_BINARY_DIR}/assets/fontTexture.dat")
addBinaryFile("${RELEASE_NAME}" fontPalette "${PROJECT_BINARY_DIR}/assets/fontPalette.dat")

## CD-ROM image

configure_file(cdrom.json          cdrom.json  ESCAPE_QUOTES)
configure_file(assets/cdreadme.txt readme.txt  NEWLINE_STYLE CRLF)
configure_file(LICENSE             license.txt NEWLINE_STYLE CRLF)

add_custom_command(
	COMMAND
		"${Python3_EXECUTABLE}"
		"${PROJECT_SOURCE_DIR}/tools/buildCDImage.py"
		cdrom.json
		"${RELEASE_NAME}.iso"
	OUTPUT  "${RELEASE_NAME}.iso"
	DEPENDS
		cdrom.json
		"${RELEASE_NAME}"
	COMMENT "Building CD-ROM image"
	VERBATIM
)

if(EXISTS "${CHDMAN_PATH}")
	add_custom_command(
		COMMAND
			"${CHDMAN_PATH}" createcd -f
			-i "${RELEASE_NAME}.iso"
			-o "${RELEASE_NAME}.chd"
		OUTPUT  "${RELEASE_NAME}.chd"
		DEPENDS "${RELEASE_NAME}.iso"
		COMMENT "Building MAME CHD image"
		VERBATIM
	)

	add_custom_target(cdrom ALL DEPENDS "${RELEASE_NAME}.chd")
else()
	add_custom_target(cdrom ALL DEPENDS "${RELEASE_NAME}.iso")
endif()
