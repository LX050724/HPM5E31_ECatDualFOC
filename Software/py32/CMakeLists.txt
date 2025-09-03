cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME HPM5E31_DualFoc_py32core)


project(${CMAKE_PROJECT_NAME} C ASM)


add_subdirectory(Drivers)
add_subdirectory(Third_Party/SEGGER_RTT_V812)
add_subdirectory(app)


