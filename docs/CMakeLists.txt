set ( DOXY_SOURCE_DIRECTORY ${CMAKE_SOURCE_DIR}/lib )
set ( DOXY_DOCS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
set ( DOXY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

set ( DOXYGEN_GENERATE_HTML YES )
set ( DOXYGEN_GENERATE_MAN NO )

doxygen_add_docs (
    docs
    ${PROJECT_SOURCE_DIR}/src
    COMMENT "Generate docs"
)

install (
    DIRECTORY ${DOXY_OUTPUT_DIRECTORY}/html
    DESTINATION share/${PROJECT_NAME}/
)
