# Project details
# ---------------
project(DAM_400)

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
set (DAM_400_PROGRAMS "DAMSTO_400" "DAMGTO_400" "DAMDEN_400" "DAMDENGRAD_400" "DAMFIELD_400" "DAMFIELDPNT_400" 
"DAMFORCES_400" "DAMFRAD_400" "DAMISODEN_400" "DAMISOPOT_400" "DAMMULTROT_400" "DAMORB_400" "DAMPOT_400" "DAMSGHOLE_400")

set (DAM_400_EXECUTABLES "" )

set (utilities_PROGRAMS "compareplt_400" "dmat_GAUSSIAN_to_DAM" "readcnt" "readdamqt_400" 
"readplt_400" "sgbs2sxyz" "subtractplt_400" )

set (utilities_EXECUTABLES "" )

# Creates a library with modules
# ==============================
add_library(DAMGLOBAL_mods DAM_400_GLOBAL.F90)

# Create binaries
# ===============
foreach (p ${DAM_400_PROGRAMS})
    if (UNIX AND NOT CYGWIN)
        add_executable(${p}.exe "${p}.F90")
        list (APPEND DAM_400_EXECUTABLES ${p}.exe )
    else (UNIX AND NOT CYGWIN)  # To prevent double .exe extension
        add_executable(${p} "${p}.F90")
        list (APPEND DAM_400_EXECUTABLES ${p} )
    endif (UNIX AND NOT CYGWIN)
endforeach (p)

foreach (p ${utilities_PROGRAMS})
    if (UNIX AND NOT CYGWIN)
        add_executable(${p}.exe "${p}.F90")
        list (APPEND utilities_EXECUTABLES ${p}.exe )
    else (UNIX AND NOT CYGWIN)  # To prevent double .exe extension
        add_executable(${p} "${p}.F90")
        list (APPEND utilities_EXECUTABLES ${p} )
    endif (UNIX AND NOT CYGWIN)
endforeach (p)

# Links binaries to modules
# =========================
foreach (p ${DAM_400_EXECUTABLES})
    target_link_libraries("${p}" DAMGLOBAL_mods)
    message("Adds ${p}")
endforeach (p)

foreach (p ${utilities_EXECUTABLES})
    target_link_libraries("${p}" DAMGLOBAL_mods)
    message("Adds ${p}")
endforeach (p)

# install executables and scripts
# ===============================
install (TARGETS ${DAM_400_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/
install (TARGETS ${utilities_EXECUTABLES} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/
