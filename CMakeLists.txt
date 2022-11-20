cmake_minimum_required ( VERSION 3.15 )
project ( di )
set ( DI_VERSION 0.1.0 )
set ( DI_SOVERSION 1 )

# possible options
option ( BUILD_TESTS  "Enable building tests"                      OFF )
option ( ENABLE_TSAN  "Enables clang thread sanitizer for tests"   OFF )
option ( ENABLE_ASAN  "Enables clang address sanitizer for tests"  OFF )
option ( BUILD_DOCS   "Enable building documentation"              OFF )

MESSAGE ( STATUS "DI Options:" )
MESSAGE ( STATUS "----" )
MESSAGE ( STATUS "BUILD_TESTS:  " ${BUILD_TESTS} )
MESSAGE ( STATUS "ENABLE_TSAN:   " ${ENABLE_TSAN} )
MESSAGE ( STATUS "ENABLE_ASAN:   " ${ENABLE_ASAN} )
MESSAGE ( STATUS "BUILD_DOCS:   " ${BUILD_DOCS} )
MESSAGE ( STATUS "----" )

add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/src ) # builds the lib
add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/example ) # builds the app

if ( BUILD_TESTS )
  enable_testing ()
  include ( GoogleTest )
  add_subdirectory ( ${CMAKE_CURRENT_SOURCE_DIR}/tests )
endif ()

if ( BUILD_DOCS )
    find_package ( Doxygen 1.8 )
    if ( DOXYGEN_FOUND )
        add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/docs )
    endif ()
endif ()
