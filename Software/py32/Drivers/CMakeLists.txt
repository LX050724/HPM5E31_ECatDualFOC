
add_library(Drivers INTERFACE)

target_compile_definitions(Drivers INTERFACE
    USE_FULL_LL_DRIVER
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(Drivers INTERFACE
    CMSIS/Include
    CMSIS/Device/PY32F0xx/Include
    PY32F002B_HAL_Driver/Inc
)

target_sources(Drivers INTERFACE
    PY32F002B_HAL_Driver/Src/py32f002b_ll_adc.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_comp.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_crc.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_exti.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_gpio.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_i2c.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_lptim.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_pwr.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_rcc.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_spi.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_tim.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_usart.c
    PY32F002B_HAL_Driver/Src/py32f002b_ll_utils.c
    startup_py32f002bxx.s
    CMSIS/Device/PY32F0xx/Source/system_py32f0xx.c
)
