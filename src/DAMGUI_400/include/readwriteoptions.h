//  Copyright 2008-2026, Jaime Fernandez Rico, Rafael Lopez, Ignacio Ema,
//  Guillermo Ramirez, David Zorrilla, Anmol Kumar, Sachin D. Yeole, Shridhar R. Gadre
// 
//  This file is part of DAMQT.
// 
//  DAMQT is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  DAMQT is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with DAMQT.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------
//
//  File:   READWRITEOPTIONS_H.h
//
//      Last version: March 2026
//

#ifndef READWRITEOPTIONS_H
#define READWRITEOPTIONS_H

#include <QString>
#include <QMessageBox>
#include <QVector3D>
#include <QTranslator>
#include <functional>
#include <string>
#include "IniFile.h"
#include "igridoptionspage.h"
#include "ixyztabulationpage.h"

class QCheckBox;
class QLineEdit;
class QRadioButton;
class QSpinBox;

struct PlaneResult {
    int planeCase = 1;
    QVector3D wu = QVector3D(1.0, 0.0, 0.0);
    QVector3D wv = QVector3D(0.0, 1.0, 0.0);
};

class ReadWriteOptions
{
public:
    using DeltaCalculator = std::function<double(const char*, double, double)>;

    static double computeDelta(const char* key,
                               double ini,
                               double fin,
                               const IGridOptionsPage* page);

    static PlaneResult  get_plane_case(double a, double b, double c);

    // ---- Generic readers with widget target ----
    static void readCheckBox(const char* key, const char* section,
                             QCheckBox* target, const std::string& file);

    static void readRadioButton(const char* key, const char* section,
                                QRadioButton* target, const std::string& file);

    static void readSpinBox(const char* key, const char* section,
                            QSpinBox* target, const std::string& file);

    static void readDouble(const char* key, const char* section,
                                     double* target, const std::string& file);

    static void readDoubleToLineEdit(const char* key, const char* section,
                                     QLineEdit* target, const std::string& file);

    static void readInt(const char* key, const char* section,
                                     int* target, const std::string& file);

    static void readTextToLineEdit(const char* key, const char* section,
                                   QLineEdit* target, const std::string& file);

    // ---- Generic readers returning value (compatibility with old pages) ----
    static bool readCheckBox(const char* key, const char* section,
                             const std::string& file);

    static bool readRadioButton(const char* key, const char* section,
                                const std::string& file);

    static int readSpinBox(const char* key, const char* section,
                           const std::string& file);

    static QString readDoubleToLineEdit(const char* key, const char* section,
                                        const std::string& file);

    static QString readTextToLineEdit(const char* key, const char* section,
                                      const std::string& file);

    // ---- Generic replacements for read_den_dimension / read_pot_dimension ----
    static void readDimensionOption(const char* key, const char* section,
                                    const std::string& file,
                                    IGridOptionsPage* page);

    // ---- Generic resolution query ----
    static int whatResolution2D(IGridOptionsPage* page);
    static int whatResolution3D(IGridOptionsPage* page);

    static int whatResolution(const char* dxKey, const char* dyKey, const char* dzKey,
                              const char* section, const std::string& file,
                              IGridOptionsPage* page);

    // ---- Generic replacement for read_density_resolution / read_potential_resolution ----
    static void readResolution(const char* dxKey, const char* dyKey, const char* dzKey,
                               const char* section, const std::string& file,
                               IGridOptionsPage* page);

    // ---- Generic replacement for the old read_plot_dimension ----
    static void readPlotDimension(const char* key, const char* section,
                                  const std::string& file,
                                  QRadioButton* grid2DButton,
                                  QRadioButton* lowButton,
                                  QRadioButton* mediumButton,
                                  QRadioButton* highButton);

    // ---- Writers ----

    static void writeIntervals(const char* minKey, const char* maxKey, const char* deltaKey,
                               const char* section, const std::string& file,
                               QLineEdit* minEdit, QLineEdit* maxEdit,
                               bool* errorFlag, QString* errorText,
                               const DeltaCalculator& deltaCalculator);

    static double writeIntervals(const char* minKey, const char* maxKey, const char* deltaKey,
                               const char* section, const std::string& file,
                               const QString& minValue, const QString& maxValue,
                               bool* errorFlag, QString* errorText,
                               const DeltaCalculator& deltaCalculator);

    static void writeOption(const char* key, const char* section,
                            const QString& value, const std::string& file,
                            bool* errorFlag, QString* errorText);

    static void writeQuotedOption(const char* key, const char* section,
                                  const QString& value, const std::string& file,
                                  bool* errorFlag, QString* errorText);

    static void writeRTable(const char* section,
                              const std::string& file,
                              const IXYZTabulationPage* page,
                              int maxRows,
                              bool* errorFlag,
                              QString* errorText);

    static void writeUVLinesTable(const char* section,
                              const std::string& file,
                              const ILinesTabulationPage* page,
                              int maxRows,
                              bool* errorFlag,
                              QString* errorText);

    static void writeXYZLinesTable(const char* section,
                              const std::string& file,
                              const ILinesTabulationPage* page,
                              int maxRows,
                              bool* errorFlag,
                              QString* errorText);

//    static void writeXYZTable(const char* section, const std::string& file,
//                              int rowCount,
//                              const std::function<QString(int,int)>& cellGetter,
//                              bool* errorFlag, QString* errorText);

    static void writeXYZTable(const char* section,
                              const std::string& file,
                              const IXYZTabulationPage* page,
                              int maxRows,
                              bool* errorFlag,
                              QString* errorText);

private:

    static QString readRawValue(const char* key, const char* section,
                                const std::string& file);

    static bool isTrueValue(const QString& value);
};

#endif

