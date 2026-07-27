# Project details
# ---------------
project(DAMZERNIKE_400_mpi)

enable_language (C Fortran)

find_package(MPI COMPONENTS Fortran REQUIRED)

add_definitions(${MPI_Fortran_COMPILE_FLAGS})
include_directories(${MPI_Fortran_INCLUDE_PATH})
link_directories(${MPI_Fortran_LIBRARIES})

# Fortran compiler flags
# ======================
if (NOT CMAKE_Fortran_FLAGS_RELEASE)
        set (CMAKE_Fortran_FLAGS_RELEASE "-O3")
endif (NOT CMAKE_Fortran_FLAGS_RELEASE)
if (NOT CMAKE_Fortran_FLAGS_DEBUG)
        set (CMAKE_Fortran_FLAGS_DEBUG "-O0 -g")
endif (NOT CMAKE_Fortran_FLAGS_DEBUG)

# Defines auxiliary containers with program names and directory with ancillary files
# ==================================================================================
set (DAMZERNIKE_400_mpi_PROGRAMS "DAMZERNIKE-JACOBI_STO_mpi" "DAMZERNIKE-JACOBI_GTO_mpi" "DAMDENZJ_400_mpi" )

set (DAMZERNIKE_400_mpi_EXECUTABLES "" )

set (ancillarydir "../DAMZERNIKE_400" )

# Prepares ZERNIKE-JACOBI_ckplm.F90 to be compiled without optimization (-O0)
# =============================================================================

set (ckplm_file "${ancillarydir}/ZERNIKE-JACOBI_ckplm.F90")

SET_SOURCE_FILES_PROPERTIES(${ckplm_file} PROPERTIES COMPILE_FLAGS -O0)	# ZERNIKE-JACOBI_ckplm.F90 compiled without optimization to prevent long delay in compilation

# Ancillary files for libraries
# ===========================================================================

set (ancillaryfiles "${ancillarydir}/DAMZERNIKE_400_GLOBAL.F90" "${ancillarydir}/ZERNIKE-JACOBI_ckplm.F90"  "${ancillarydir}/ZERNIKE-JACOBI_paribkb.F90"
    "${ancillarydir}/ZERNIKE-JACOBI_quadrat.f"  "${ancillarydir}/ZERNIKE-JACOBI_stngexp.f")

# Creates a library with modules
# ==============================
add_library(DAMZernikeGLOBAL_mpi_mods DAMZERNIKE_400_GLOBAL_mpi.F90 ZERNIKE-JACOBI_subVABC_mpi.F90 ${ancillaryfiles})

# Create binaries
# ===============
foreach (p ${DAMZERNIKE_400_mpi_PROGRAMS})
    if (UNIX AND NOT CYGWIN)
        add_executable(${p}.exe "${p}.F90")
        list (APPEND DAMZERNIKE_400_MPI_EXECUTABLES ${p}.exe )
    else (UNIX AND NOT CYGWIN)  # To prevent double .exe extension
        add_executable(${p} "${p}.F90")
        list (APPEND DAMZERNIKE_400_MPI_EXECUTABLES ${p} )
    endif (UNIX AND NOT CYGWIN)
endforeach (p)

# Links binaries to modules
# =========================
foreach (p ${DAMZERNIKE_400_MPI_EXECUTABLES})
        target_link_libraries("${p}" DAMZernikeGLOBAL_mpi_mods ${MPE_LOG_LIBRARIES} ${MPI_Fortran_LIBRARIES})
        set_target_properties("${p}" PROPERTIES COMPILE_FLAGS "${MPI_Fortran_COMPILE_FLAGS} ${MPE_COMPILE_FLAGS}")
        set_target_properties("${p}" PROPERTIES LINK_FLAGS "${MPI_Fortran_LINK_FLAGS} ${MPE_LINK_FLAGS}")
        message("Adds ${p}")
endforeach (p)

# install executables and scripts
# ===============================
install (TARGETS ${DAMZERNIKE_400_MPI_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/

# install executables and scripts
# ===============================
install (TARGETS ${DAMDENZERNIKEMPI_400_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/
