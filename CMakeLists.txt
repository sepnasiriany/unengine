cmake_minimum_required(VERSION 3.14)

project(unengine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(SDL2 REQUIRED)
include_directories(src ${SDL2_INCLUDE_DIRS})

# Add executable
add_executable(${PROJECT_NAME}
    src/unengine.cpp
    src/instruction.cpp
    src/emulator.cpp
    src/rom.cpp
    src/gpu.cpp
)

target_link_libraries(unengine ${SDL2_LIBRARIES})

add_executable(disassemble
    src/disassemble.cpp
    src/rom.cpp
    src/instruction.cpp
)


enable_testing()

add_executable(tests
  src/tests.cpp
  src/emulator.cpp
  src/gpu.cpp
  src/instruction.cpp
  src/rom.cpp
)

target_link_libraries(
  tests
  GTest::gtest_main
  ${SDL2_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(tests)
