# Project details
# ---------------
project(DAMZERNIKE_400)

enable_language (C Fortran)

# Fortran compiler flags
# ======================
if (NOT CMAKE_Fortran_FLAGS_RELEASE)
	set (CMAKE_Fortran_FLAGS_RELEASE "-O3")
endif (NOT CMAKE_Fortran_FLAGS_RELEASE)
if (NOT CMAKE_Fortran_FLAGS_DEBUG)
	set (CMAKE_Fortran_FLAGS_DEBUG "-O0 -g")
endif (NOT CMAKE_Fortran_FLAGS_DEBUG)

# Defines auxiliary containers with program names
# ===============================================
set (DAMZERNIKE_400_PROGRAMS "DAMZERNIKE-JACOBI_GTO" "DAMZERNIKE-JACOBI_STO" "DAMDENZJ_400")

set (DAMZERNIKE_400_EXECUTABLES "" )

set (ckplm_file "ZERNIKE-JACOBI_ckplm.F90")

SET_SOURCE_FILES_PROPERTIES(${ckplm_file} PROPERTIES COMPILE_FLAGS -O0)	# ZERNIKE-JACOBI_ckplm.F90 compiled without optimization to prevent long delay in compilation


# Creates a library with modules
# ==============================
add_library(DAMZERNIKE_mods   DAMZERNIKE_400_GLOBAL.F90   ZERNIKE-JACOBI_ckplm.F90   ZERNIKE-JACOBI_paribkb.F90  
	ZERNIKE-JACOBI_quadrat.f  ZERNIKE-JACOBI_stngexp.f  ZERNIKE-JACOBI_subVABC.F90)

# Create binaries
# ===============
foreach (p ${DAMZERNIKE_400_PROGRAMS})
    if (UNIX AND NOT CYGWIN)
        add_executable(${p}.exe "${p}.F90")
        list (APPEND DAMZERNIKE_400_EXECUTABLES ${p}.exe )
    else (UNIX AND NOT CYGWIN)  # To prevent double .exe extension
        add_executable(${p} "${p}.F90")
        list (APPEND DAMZERNIKE_400_EXECUTABLES ${p} )
    endif (UNIX AND NOT CYGWIN)
endforeach (p)


# Links binaries to modules
# =========================
foreach (p ${DAMZERNIKE_400_EXECUTABLES})
   target_link_libraries("${p}" DAMZERNIKE_mods)
   message("Adds ${p}")
endforeach (p)

# install executables and scripts
# ===============================
install (TARGETS ${DAMZERNIKE_400_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/
