
add_executable(main 
    main.c
    py32f002b_it.c
    uart_dict.c
    BrakeController.c
)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(main PRIVATE Drivers)
# target_link_libraries(main PRIVATE SEGGER_RTT)

target_link_options(main PRIVATE
    -T${CMAKE_SOURCE_DIR}/py32f002bx5.ld
)

target_compile_definitions(main PRIVATE
    PY32F002Bx5
    # RTT_ENABLE
)