set ( BENCH_BIN ${CMAKE_PROJECT_NAME}_benchmark )
file ( GLOB_RECURSE BENCH_SOURCES LIST_DIRECTORIES false *.hpp *.cpp )
set ( SOURCES ${BENCH_SOURCES} )
add_executable ( ${BENCH_BIN} ${BENCH_SOURCES} )
target_link_libraries ( ${BENCH_BIN} PUBLIC di benchmark::benchmark )

if ( ENABLE_TSAN )
    target_compile_options ( 
        ${BENCH_BIN} PRIVATE -g
            -fsanitize=thread 
    )
    target_link_options ( 
        ${BENCH_BIN} PRIVATE -g
            -fsanitize=thread 
    )
endif ()

if ( ENABLE_ASAN )
    target_compile_options ( 
        ${BENCH_BIN} PRIVATE -g
            -fsanitize=address 
            -fno-omit-frame-pointer 
    )
    target_link_options ( 
        ${BENCH_BIN} PRIVATE -g
            -fsanitize=address 
            -fno-omit-frame-pointer
    )
endif ()
