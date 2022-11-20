add_library ( di INTERFACE )
add_library ( schwifty::di ALIAS di )

target_compile_features ( di 
    INTERFACE  cxx_std_20 )

target_link_libraries ( di 
    INTERFACE  "-lstdc++" )

target_include_directories( di
    INTERFACE  "${CMAKE_CURRENT_DIR}" )
