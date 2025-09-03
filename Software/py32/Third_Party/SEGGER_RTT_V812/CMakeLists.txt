add_library(SEGGER_RTT
    RTT/SEGGER_RTT.c
    RTT/SEGGER_RTT_printf.c
    RTT/SEGGER_RTT_ASM_ARMv7M.S
    Syscalls/SEGGER_RTT_Syscalls_GCC.c
)

target_include_directories(SEGGER_RTT PUBLIC RTT Config)

target_compile_definitions(SEGGER_RTT PUBLIC
    BUFFER_SIZE_UP=128
    BUFFER_SIZE_DOWN=16
    SEGGER_RTT_PRINTF_BUFFER_SIZE=64
)
