cmake_minimum_required ( VERSION 3.15 )
project ( di )

# possible options
option ( BUILD_TESTS  "Enable building tests"                      OFF )
option ( BUILD_BENCH  "Enable building benchmarks"                 OFF )
option ( ENABLE_TSAN  "Enables clang thread sanitizer for tests"   OFF )
option ( ENABLE_ASAN  "Enables clang address sanitizer for tests"  OFF )
option ( BUILD_DOCS   "Enable building documentation"              OFF )

MESSAGE ( STATUS "DI Options:" )
MESSAGE ( STATUS "----" )
MESSAGE ( STATUS "BUILD_TESTS:  " ${BUILD_TESTS} )
MESSAGE ( STATUS "BUILD_BENCH:  " ${BUILD_BENCH} )
MESSAGE ( STATUS "ENABLE_TSAN:   " ${ENABLE_TSAN} )
MESSAGE ( STATUS "ENABLE_ASAN:   " ${ENABLE_ASAN} )
MESSAGE ( STATUS "BUILD_DOCS:   " ${BUILD_DOCS} )
MESSAGE ( STATUS "----" )

add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/src ) # builds the lib
add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/example ) # builds the app

if ( BUILD_TESTS )
  enable_testing ()
  include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoogleTest.cmake )
  add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif ()

if ( BUILD_BENCH )
  include( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GoogleBenchmark.cmake )
  add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks )
endif ()

if ( BUILD_DOCS )
    find_package ( Doxygen 1.8 )
    if ( DOXYGEN_FOUND )
        add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/docs )
    endif ()
endif ()
