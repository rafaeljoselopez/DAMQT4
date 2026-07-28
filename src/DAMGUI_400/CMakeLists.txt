# Project details
# ---------------
project(DAMGUI400)

#add_subdirectory(include)

enable_language (C CXX)

# Instruct CMake to run moc automatically when needed.
# ===================================================

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DAMGUI400_INCLUDE_DIR ./include)
include_directories(${DAMGUI400_INCLUDE_DIR})
message ("INCLUDE DIR FOR DAMGUI400 = " ${DAMGUI400_INCLUDE_DIR})

# here we define DAMGUI400_SRCS variable that contains a list of all .cpp files
# note that we don't need \ at the end of line
set( DAMGUI400_SRCS
   ./atomicdensitiespage.cpp
   ./axes.cpp
   ./ballscyls.cpp
   ./calculationimporter.cpp
   ./CIsoSurface.cpp 
   ./ColorButton.cpp
   ./configdialog.cpp
   ./contours.cpp 
   ./criticalpoints.cpp
   ./densitypage.cpp
   ./densitygradientpage.cpp
   ./dialog.cpp  
   ./elements.cpp
   ./externals.cpp
   ./externalprogramrunner.cpp
   ./fchkimporter.cpp
   ./fieldlines.cpp
   ./fieldlinespage.cpp
   ./filedialogutils.cpp
   ./forces.cpp
   ./geometryprocessor.cpp
   ./glmath.cpp 
   ./glWidget.cpp
   ./glWindow.cpp
   ./GlobalInfo.cpp
   ./grid.cpp
   ./hfforcespage.cpp
   ./IniFile.cpp
   ./isosurface.cpp
   ./lights.cpp
   ./main.cpp
   ./mainwindow.cpp 
   ./measures.cpp
   ./mespimizer.cpp
   ./molecule.cpp
   ./moleculesettingsloader.cpp
   ./moleculesettingswriter.cpp
   ./molproimporter.cpp
   ./orbitalspage.cpp
   ./orientedmultipolespage.cpp
   ./plotter.cpp
   ./potentialpage.cpp
   ./projectpage.cpp
   ./radialfactorspage.cpp
   ./readwriteoptions.cpp
   ./recorder.cpp
   ./rotations.cpp
   ./screenshot.cpp
   ./settingsreader.cpp
   ./settingswriter.cpp
   ./sigmaholepage.cpp
   ./singlepassimporter.cpp
   ./Sheet.cpp 
   ./surface.cpp
   ./topographypage.cpp
   ./translations.cpp
   ./VertexUtils.cpp
   ./viewer2D.cpp
   ./viewerdialog.cpp
   ./viewport.cpp
   ./VVBuffer.cpp 
   ./widgetsubclasses.cpp
   ./zjdensitypage.cpp
   ./zjexpansionpage.cpp
   ./run_mpi.sh
  )

# another list, this time it includes all header files that should be treated with moc (Qt's meta-object compiler)
set( DAMGUI400_MOC_HDRS
   ${DAMGUI400_INCLUDE_DIR}/atomicdensitiespage.h
   ${DAMGUI400_INCLUDE_DIR}/axes.h
   ${DAMGUI400_INCLUDE_DIR}/ballscyls.h
   ${DAMGUI400_INCLUDE_DIR}/calculationimporter.h
   ${DAMGUI400_INCLUDE_DIR}/ColorButton.h
   ${DAMGUI400_INCLUDE_DIR}/configdialog.h
   ${DAMGUI400_INCLUDE_DIR}/criticalpoints.h
   ${DAMGUI400_INCLUDE_DIR}/densitypage.h
   ${DAMGUI400_INCLUDE_DIR}/densitygradientpage.h
   ${DAMGUI400_INCLUDE_DIR}/dialog.h
   ${DAMGUI400_INCLUDE_DIR}/externals.h
   ${DAMGUI400_INCLUDE_DIR}/externalprogramrunner.h
   ${DAMGUI400_INCLUDE_DIR}/fchkimporter.h
   ${DAMGUI400_INCLUDE_DIR}/fieldlines.h
   ${DAMGUI400_INCLUDE_DIR}/forces.h
   ${DAMGUI400_INCLUDE_DIR}/glWidget.h
   ${DAMGUI400_INCLUDE_DIR}/glWindow.h
   ${DAMGUI400_INCLUDE_DIR}/grid.h
   ${DAMGUI400_INCLUDE_DIR}/iexecutablepage.h
   ${DAMGUI400_INCLUDE_DIR}/isosurface.h
   ${DAMGUI400_INCLUDE_DIR}/fieldlinespage.h
   ${DAMGUI400_INCLUDE_DIR}/hfforcespage.h
   ${DAMGUI400_INCLUDE_DIR}/lights.h
   ${DAMGUI400_INCLUDE_DIR}/mainwindow.h
   ${DAMGUI400_INCLUDE_DIR}/measures.h
   ${DAMGUI400_INCLUDE_DIR}/mespimizer.h
   ${DAMGUI400_INCLUDE_DIR}/molecule.h
   ${DAMGUI400_INCLUDE_DIR}/molproimporter.h
   ${DAMGUI400_INCLUDE_DIR}/orbitalspage.h
   ${DAMGUI400_INCLUDE_DIR}/orientedmultipolespage.h
   ${DAMGUI400_INCLUDE_DIR}/plotter.h
   ${DAMGUI400_INCLUDE_DIR}/potentialpage.h
   ${DAMGUI400_INCLUDE_DIR}/projectpage.h
   ${DAMGUI400_INCLUDE_DIR}/radialfactorspage.h
   ${DAMGUI400_INCLUDE_DIR}/readwriteoptions.h
   ${DAMGUI400_INCLUDE_DIR}/recorder.h
   ${DAMGUI400_INCLUDE_DIR}/rotations.h
   ${DAMGUI400_INCLUDE_DIR}/screenshot.h
   ${DAMGUI400_INCLUDE_DIR}/sigmaholepage.h
   ${DAMGUI400_INCLUDE_DIR}/singlepassimporter.h
   ${DAMGUI400_INCLUDE_DIR}/Sheet.h
   ${DAMGUI400_INCLUDE_DIR}/surface.h
   ${DAMGUI400_INCLUDE_DIR}/topographypage.h
   ${DAMGUI400_INCLUDE_DIR}/translations.h
   ${DAMGUI400_INCLUDE_DIR}/viewer2D.h
   ${DAMGUI400_INCLUDE_DIR}/viewerdialog.h
   ${DAMGUI400_INCLUDE_DIR}/viewport.h
   ${DAMGUI400_INCLUDE_DIR}/widgetsubclasses.h
   ${DAMGUI400_INCLUDE_DIR}/zjdensitypage.h
   ${DAMGUI400_INCLUDE_DIR}/zjexpansionpage.h
  )
  
# remaining headers
set( DAMGUI400_HDRS
   ${DAMGUI400_INCLUDE_DIR}/CIsoSurface.h
   ${DAMGUI400_INCLUDE_DIR}/contours.h
   ${DAMGUI400_INCLUDE_DIR}/elements.h
   ${DAMGUI400_INCLUDE_DIR}/filedialogutils.h
   ${DAMGUI400_INCLUDE_DIR}/geometryprocessor.h
   ${DAMGUI400_INCLUDE_DIR}/glmath.h
   ${DAMGUI400_INCLUDE_DIR}/GlobalInfo.h
   ${DAMGUI400_INCLUDE_DIR}/igridoptionspage.h
   ${DAMGUI400_INCLUDE_DIR}/ixyztabulationpage.h
   ${DAMGUI400_INCLUDE_DIR}/IniFile.h
   ${DAMGUI400_INCLUDE_DIR}/moleculesettingsloader.h
   ${DAMGUI400_INCLUDE_DIR}/moleculesettingswriter.h
   ${DAMGUI400_INCLUDE_DIR}/settingsreader.h
   ${DAMGUI400_INCLUDE_DIR}/settingswriter.h
   ${DAMGUI400_INCLUDE_DIR}/VertexNormalData.h
   ${DAMGUI400_INCLUDE_DIR}/VertexUtils.h
   ${DAMGUI400_INCLUDE_DIR}/VVBuffer.h
   )

# and finally a resource file
set( DAMGUI400_RCS
   ./damqt.qrc
   ./shaders.qrc
  )
  
# enable warnings
  add_definitions( -Wall )

# This GUI requires OpenGL library available
# ==========================================
find_package(OpenGL REQUIRED)

# This GUI requires Qt5
# =====================
# Seeks Qt5 libraries and sets all required variables

#find_package(Qt5 COMPONENTS Widgets Core Gui OpenGL PrintSupport REQUIRED)

find_package(Qt5Widgets)
# BLock for Qt5
# =============
if (Qt5Widgets_FOUND)
    set(QT_USE_QTOPENGL TRUE)
    message ("Builds GUI with Qt5")
    find_package(Qt5Core)
    find_package(Qt5Gui)
    find_package(Qt5OpenGL)
    find_package(Qt5PrintSupport)

    include_directories( ${OPENGL_INCLUDE_DIRS} )

    QT5_ADD_RESOURCES( DAMGUI400_RC_SRCS ${DAMGUI400_RCS} )
    QT5_WRAP_CPP( DAMGUI400_MOC_SRCS ${DAMGUI400_MOC_HDRS} )

    set(DAMGUI400_EXECUTABLE DAMQT400)

# Add ejecutable (constant target name)
    add_executable(
        ${DAMGUI400_EXECUTABLE}
        ${DAMGUI400_SRCS}
        ${DAMGUI400_HDRS}
        ${DAMGUI400_MOC_HDRS}
        ${DAMGUI400_MOC_SRCS}
        ${DAMGUI400_RC_SRCS}
    )

# Pass the interfaces directory name to the C++ code
get_property(
    DAMQT_INTERFACES_DIR_NAME
    GLOBAL
    PROPERTY
    DAMQT_INTERFACES_DIR_NAME
)

target_compile_definitions(
    ${DAMGUI400_EXECUTABLE}
    PRIVATE
    DAMQT_INTERFACES_DIR="${DAMQT_INTERFACES_DIR_NAME}"
)

message(
    STATUS
    "DAMQT interfaces directory: ${DAMQT_INTERFACES_DIR_NAME}"
)

# Change the name of the binary file in case of UNIX
    if (UNIX AND NOT CYGWIN)
        set_target_properties(${DAMGUI400_EXECUTABLE} PROPERTIES OUTPUT_NAME "DAMQT400.exe")
        message(NOTICE "
            To run in QtCreator, it is necessary to select DAMQT (without .exe) in the Run configuration combo of the Kit
            Also add the path to the Qt5 library in the top of LD_LIBRARY_PATH if necessary
            ")
    endif()

# Link libraries
    target_link_libraries(${DAMGUI400_EXECUTABLE} ${QT_LIBRARIES} ${OPENGL_LIBRARIES}
            Qt5::Widgets Qt5::OpenGL Qt5::PrintSupport)



else (Qt5Widgets_FOUND) # If Qt5 not found
        message (FATAL_ERROR "Qt5 not available")
endif (Qt5Widgets_FOUND)  # End of Block for Qt5Widgets

# install executables and scripts
# ===============================
install (TARGETS ${DAMGUI400_EXECUTABLE} RUNTIME DESTINATION ".")  # Default installation directory: /usr/local/bin/
