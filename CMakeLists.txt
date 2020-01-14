# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(watch)

target_sources(app PRIVATE src/qrcodegen.c src/graphics.c src/main.c src/hl_bluetooth.c src/touch.c)
