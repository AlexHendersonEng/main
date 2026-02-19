# Variables
set(DOXYGEN_VERSION "1.16.1")
set(DOXYGEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen-${DOXYGEN_VERSION}")
set(DOCS_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
set(DOXYFILE_OUT "${DOCS_DIR}/Doxyfile")

# Operating system specific variables
if(WIN32)
    set(DOXYGEN_ARCHIVE "${DOXYGEN_DIR}.zip")
    set(DOXYGEN_EXECUTABLE "${DOXYGEN_DIR}/doxygen.exe")
    set(DOXYGEN_URL "https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.windows.x64.bin.zip")
elseif(APPLE)
    set(DOXYGEN_ARCHIVE "${DOXYGEN_DIR}.zip")
    set(DOXYGEN_EXECUTABLE "${DOXYGEN_DIR}/doxygen-${DOXYGEN_VERSION}/doxygen")
    set(DOXYGEN_URL "https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}-mac-arm.zip")
elseif(UNIX)
    set(DOXYGEN_ARCHIVE "${DOXYGEN_DIR}.tar.gz")
    set(DOXYGEN_EXECUTABLE "${DOXYGEN_DIR}/doxygen-${DOXYGEN_VERSION}/bin/doxygen")
    set(DOXYGEN_URL "https://www.doxygen.nl/files/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Download and extract doxygen
if(NOT EXISTS "${DOXYGEN_EXECUTABLE}")
    message(STATUS "Doxygen executable not found. Downloading doxygen from '${DOXYGEN_URL}' to '${DOXYGEN_ARCHIVE}'")
    file(
        DOWNLOAD
        "${DOXYGEN_URL}"
        "${DOXYGEN_ARCHIVE}"
        SHOW_PROGRESS
    )

    message(STATUS "Extracing '${DOXYGEN_ARCHIVE}' to '${DOXYGEN_DIR}'")
    file(
        ARCHIVE_EXTRACT
        INPUT "${DOXYGEN_ARCHIVE}"
        DESTINATION "${DOXYGEN_DIR}"
    )

    file(REMOVE "${DOXYGEN_ARCHIVE}")
endif()

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

# Install
install(DIRECTORY ${DOCS_DIR}/html DESTINATION docs)
