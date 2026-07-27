# NOTE:
# This CMakeLists.txt appears to be legacy code from previous DAMGUI versions.
# DAMQT400 is currently built from ../CMakeLists.txt, where sources and MOC
# headers are explicitly listed in DAMGUI400_SRCS and DAMGUI400_MOC_HDRS.
# Project details
# ---------------
project(DAM400_include)

# another list, this time it includes all header files that should be treated with moc (Qt's meta-object compiler)
set( DAMGUI400_MOC_HDRS
    ./ColorButton.h
    ./configdialog.h
    ./criticalpoints.h
    ./dialog.h
    ./externalprogramrunner.h
    ./fieldlines.h
    ./forces.h
    ./glWidget.h
    ./glWindow.h
    ./grid.h
    ./mainwindow.h
    ./measures.h
    ./molecule.h
    ./plotter.h
    ./Sheet.h
    ./isosurface.h
    ./surface.h
    ./viewer2D.h
    ./viewerdialog.h
    ./widgetsubclasses.h
  )

# remaining headers
set( DAMGUI400_HDRS
    ./CIsoSurface.h
    ./contours.h
    ./elements.h
    ./geometryprocessor.h
    ./glmateriales.h
    ./glmath.h
    ./GlobalInfo.h
    ./IniFile.h
    ./VVBuffer.h
    ./VertexNormalData.h
   )
