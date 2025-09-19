# Copyright (c) 2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

set(APP_NAME HPM5E31_DualFOC_Demo)
set(SES_TOOLCHAIN_VARIANT "Andes")
set(CONFIG_HPM_ADC 1)
set(CONFIG_EEPROM_EMULATION 1)


find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(foc_demo)
sdk_compile_options(-Wno-unused-function -Wno-strict-prototypes -Wno-unused-parameter -Wno-attributes)
sdk_compile_definitions(HPM_SPI_DRV_DEFAULT_RETRY_COUNT=0xfffffffe)
sdk_ses_compile_options(-mabi=ilp32d)
sdk_ses_compile_options(-march=rv32gc)
sdk_ses_opt_debug_connection(J-Link)

sdk_inc(src)

file(GLOB_RECURSE SRC_FILES src/*.c)
sdk_app_src(${SRC_FILES})

generate_ses_project()
