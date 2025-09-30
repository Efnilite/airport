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

set(TEST_SOURCES test/querces_test_impl.c test/queue.c test/test.c)

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
        test/test.c
        test/test.h

        test/main.c
        test/pico_test.c
        test/querces_test_impl.c
        test/test_example.c
)


# Executables as part of test target

add_executable_with_def(pi src/pi.c D)
add_executable_with_def(pico_gate src/pico_gate.c D)
add_executable_with_def(pico_check-in src/pico_check-in.c D)
add_executable_with_def(pico_security src/pico_security.c D)
add_executable_with_def(pico_default src/pico_default.c D)

add_dependencies(test setup pi pico_gate pico_check-in pico_security pico_default)
