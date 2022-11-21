set ( TEST_BIN ${CMAKE_PROJECT_NAME}_test )
file ( GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.hpp *.cpp )
set ( SOURCES ${TEST_SOURCES} )
add_executable ( ${TEST_BIN} ${TEST_SOURCES} )
add_test ( NAME ${TEST_BIN} COMMAND ${TEST_BIN} )
target_link_libraries ( ${TEST_BIN} PUBLIC di gtest gmock )

if ( ENABLE_TSAN )
    target_compile_options ( 
        ${TEST_BIN} PRIVATE -g
            -fsanitize=thread 
    )
    target_link_options ( 
        ${TEST_BIN} PRIVATE -g
            -fsanitize=thread 
    )
endif ()

if ( ENABLE_ASAN )
    target_compile_options ( 
        ${TEST_BIN} PRIVATE -g
            -fsanitize=address 
            -fno-omit-frame-pointer 
    )
    target_link_options ( 
        ${TEST_BIN} PRIVATE -g
            -fsanitize=address 
            -fno-omit-frame-pointer
    )
endif ()
