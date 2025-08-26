cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(SpaceInvader VERSION 0.1)

# specify the C++ standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(ConsoleGL)
# add the executable
add_executable(SpaceInvader Win32SpaceInvader.c SpaceInvader.c)

link_directories(C:\\Program Files \(x86\)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64)

target_link_libraries(SpaceInvader PUBLIC ConsoleGL winMM.Lib)

target_include_directories(SpaceInvader PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/ConsoleGL")

target_compile_definitions(SpaceInvader PUBLIC SPACEINVADER_INTERNAL=1)
                          