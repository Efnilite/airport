cmake_minimum_required(VERSION 3.31)
project(airport C)

set(CMAKE_C_STANDARD 11)

# Main executable

add_executable(airport
        src/common.h
        src/common_handle_tub.c
        src/common_hardware.c
        src/common_hardware.h
        src/common_init.c
        src/common_loop.c
        src/common_loop.h
        src/libc_builtin.h
        src/defs.h
        src/pi.c
        src/pico_check-in.c
        src/pico_default.c
        src/pico_gate.c
        src/pico_security.c
        src/quercus_lib_pico.h
        src/tub_writer.c
)

# Test compilation

set(TEST_SOURCES test/querces_test_impl.c test/queue.c)

add_custom_target(setup
        COMMAND ${CMAKE_COMMAND} -E env rm /dev/mqueue/* -f
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

function(add_executable_with_def target file defs)
    add_executable(${target} ${file} ${TEST_SOURCES})
    target_compile_definitions(${target} PUBLIC Q_TEST ${defs})
    add_dependencies(${target} setup)
    target_link_libraries(${target} rt)
    set_target_properties(${target} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target"
            OUTPUT_NAME "${target}"
    )
endfunction()

# Executables for running on its own

add_executable_with_def(pi_single src/pi.c Q_DEBUG)
add_executable_with_def(pico_gate_single src/pico_gate.c Q_DEBUG)
add_executable_with_def(pico_check-in_single src/pico_check-in.c Q_DEBUG)
add_executable_with_def(pico_security_single src/pico_security.c Q_DEBUG)
add_executable_with_def(pico_default_single src/pico_default.c Q_DEBUG)

# Test executable

add_executable(test
        test/defs.h

        test/queue.c
        test/queue.h
        test/test.h

        test/main.c
        test/pico_test.c
        test/querces_test_impl.c
)

# Executables as part of test target

add_executable_with_def(pi src/pi.c D)
add_executable_with_def(pico_gate src/pico_gate.c D)
add_executable_with_def(pico_check-in src/pico_check-in.c D)
add_executable_with_def(pico_security src/pico_security.c D)
add_executable_with_def(pico_default src/pico_default.c D)

add_dependencies(test setup pi pico_gate pico_check-in pico_security pico_default display manager)

# Display

add_executable(display
        test/display/display.c
        test/display/display.h

        test/queue.h
        test/queue.c
)

add_dependencies(display setup)
set_target_properties(display PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target"
        OUTPUT_NAME "display"
)
target_link_libraries(display m)

# SDL

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(display ${SDL2_LIBRARIES})

# SDL_TTF

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
include_directories(${SDL2_TTF_INCLUDE_DIRS})
link_directories(${SDL2_TTF_LIBRARY_DIRS})
target_link_libraries(display ${SDL2_TTF_LIBRARIES})


# Manager

add_executable(manager
        test/manager/manager.c

        test/queue.h
        test/queue.c
        test/test.h
)

add_dependencies(manager setup)
set_target_properties(manager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/target"
        OUTPUT_NAME "manager"
)