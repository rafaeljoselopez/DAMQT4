# Project details
# ---------------
project(interfaces)

# Defines auxiliary containers with program names
# ===============================================
set(CPP_INTERFACES
    "GAUSS_interface"
    "MOLEKEL_interface"
    "MOLPRO_out_interface"
    "TURBOMOLE_interface"
    "NWChem_interface"
    "Mopac_aux_interface"
)

set(PYTHON_SCRIPTS
    "MOLPRO_xml_interface.py"
    "MOPAC_aux_interface.py"
)

set(CPP_INTERFACES_EXECUTABLES "")

# Create C++ binaries
# ===================
if(UNIX AND NOT CYGWIN)

    foreach(p ${CPP_INTERFACES})
        add_executable(${p}.exe "${p}.cpp")

        list(
            APPEND
            CPP_INTERFACES_EXECUTABLES
            ${p}.exe
        )

        message(STATUS "Adds ${p}.exe")
    endforeach()

else()

    # Prevent double .exe extension on Windows/Cygwin
    foreach(p ${CPP_INTERFACES})
        add_executable(${p} "${p}.cpp")

        list(
            APPEND
            CPP_INTERFACES_EXECUTABLES
            ${p}
        )

        message(STATUS "Adds ${p}")
    endforeach()

endif()

# Install C++ executables
# =======================
install(
    TARGETS ${CPP_INTERFACES_EXECUTABLES}
    RUNTIME DESTINATION "."
)

# Python interfaces
# =================
set(
    PYTHON_OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/src/interfaces_400"
)

add_custom_target(
    python_interfaces ALL

    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "${PYTHON_OUTPUT_DIR}"

    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${PYTHON_SCRIPTS}
        "${PYTHON_OUTPUT_DIR}"

    WORKING_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}"

    SOURCES
        ${PYTHON_SCRIPTS}

    COMMENT
        "Copying Python interfaces to ${PYTHON_OUTPUT_DIR}"
)

# Install Python interfaces
# =========================
install(
    PROGRAMS
        ${PYTHON_SCRIPTS}
    DESTINATION
        "."
)

# Export the name of this directory (interfaces_400, ...)
# =======================================================================
get_filename_component(
    INTERFACES_DIR_NAME
    "${CMAKE_CURRENT_SOURCE_DIR}"
    NAME
)

set_property(
    GLOBAL
    PROPERTY
    DAMQT_INTERFACES_DIR_NAME
    "${INTERFACES_DIR_NAME}"
)
