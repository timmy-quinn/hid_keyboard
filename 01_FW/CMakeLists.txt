#
# Copyright (c) 2019 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND DTS_ROOT_BINDINGS ${CMAKE_CURRENT_SOURCE_DIR}/dts/bindings)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_hids_keyboard)

FILE(GLOB app_sources src/*.c)
# NORDIC SDK APP START
target_sources(app PRIVATE ${app_sources})
# NORDIC SDK APP END

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

zephyr_library_include_directories(${ZEPHYR_BASE}/samples/bluetooth)

add_subdirectory(src/kb_ble)