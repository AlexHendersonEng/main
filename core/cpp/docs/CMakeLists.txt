# Find doxygen
find_package(Doxygen REQUIRED)

# Variables
set(DOCS_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${DOCS_DIR}/Doxyfile)

# Configure 'Doxyfile'
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Add documentation target
add_custom_command(
    OUTPUT ${DOCS_DIR}/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    WORKING_DIRECTORY ${DOCS_DIR}
    DEPENDS ${DOXYFILE_OUT}
    COMMENT "Generating documentation with Doxygen"
    VERBATIM
)
add_custom_target(
    docs
    ALL
    DEPENDS ${DOCS_DIR}/html/index.html
)
