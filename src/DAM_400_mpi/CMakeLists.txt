# Project details
# ---------------
project(DAM_400_mpi)

enable_language (C Fortran)

find_package(MPI COMPONENTS Fortran REQUIRED)
# === AÑADIDO: Buscar LAPACK y BLAS ===
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

add_definitions(${MPI_Fortran_COMPILE_FLAGS})
include_directories(${MPI_Fortran_INCLUDE_PATH})
link_directories(${MPI_Fortran_LIBRARIES})

# Fortran compiler flags
# ======================
if (NOT CMAKE_Fortran_FLAGS_RELEASE)
        # === MODIFICADO: Quitamos -llapack -lblas de aquí (NO van en compilación) ===
        set (CMAKE_Fortran_FLAGS_RELEASE "-O3 -march=native")
endif (NOT CMAKE_Fortran_FLAGS_RELEASE)
if (NOT CMAKE_Fortran_FLAGS_DEBUG)
        # === MODIFICADO: Quitamos -llapack -lblas de aquí ===
        set (CMAKE_Fortran_FLAGS_DEBUG "-O0 -g -march=native")
endif (NOT CMAKE_Fortran_FLAGS_DEBUG)

# Defines auxiliary containers with program names
# ===============================================
set (DAM_400_mpi_PROGRAMS "DAMSTO_400_mpi" "DAMGTO_400_mpi" "DAMDEN_400_mpi" "DAMDENGRAD_400_mpi" "DAMFIELD_400_mpi"
"DAMISODEN_400_mpi" "DAMISOPOT_400_mpi" "DAMORB_400_mpi" "DAMPOT_400_mpi" "DAMPOT_400_mpi_new" "DAMSGHOLE_400_mpi")

set (DAM_400_mpi_EXECUTABLES "" )

# Creates a library with modules
# ==============================
add_library(DAMGLOBAL_mpi_mods DAM_400_GLOBAL_mpi.F90)

# Create binaries
# ===============
foreach (p ${DAM_400_mpi_PROGRAMS})
    if (UNIX AND NOT CYGWIN)
        add_executable(${p}.exe "${p}.F90")
        list (APPEND DAM_400_mpi_EXECUTABLES ${p}.exe )
    else (UNIX AND NOT CYGWIN)  # To prevent double .exe extension
        add_executable(${p} "${p}.F90")
        list (APPEND DAM_400_mpi_EXECUTABLES ${p} )
    endif (UNIX AND NOT CYGWIN)
endforeach (p)

# Links binaries to modules
# =========================
foreach (p ${DAM_400_mpi_EXECUTABLES})
        # === MODIFICADO: Añadimos LAPACK y BLAS al linkado ===
        target_link_libraries("${p}"
        DAMGLOBAL_mpi_mods
        ${MPE_LOG_LIBRARIES}
        ${MPI_Fortran_LIBRARIES}
        ${BLAS_LIBRARIES}      # AÑADIDO: Librerías BLAS
        ${LAPACK_LIBRARIES}     # AÑADIDO: Librerías LAPACK
    )
        set_target_properties("${p}" PROPERTIES COMPILE_FLAGS "${MPI_Fortran_COMPILE_FLAGS} ${MPE_COMPILE_FLAGS}")
        set_target_properties("${p}" PROPERTIES LINK_FLAGS "${MPI_Fortran_LINK_FLAGS} ${MPE_LINK_FLAGS}")
        message("Adds ${p}")
endforeach (p)

# install executables and scripts
# ===============================
install (TARGETS ${DAM_400_mpi_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/

