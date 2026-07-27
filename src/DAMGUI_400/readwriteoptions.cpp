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
//    Class defining the functions for reading/writing options from/to .damproj files. 
//
//  File:   readwriteoptions.cpp
//
//      Last version: March 2026
//
#include "readwriteoptions.h"

// Ajusta este include a la ruta real de tu proyecto
// #include "inifile.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QVector3D>

#include <cmath>
#include <functional>

double ReadWriteOptions::computeDelta(const char* key,
                                      double ini,
                                      double fin,
                                      const IGridOptionsPage* page)
{
    if (!page || fin == ini) {
        return 1.0;
    }

    const double range = fin - ini;

    if (page->isLowResolution()) {
        return page->is3DGrid() ? range / 64.0 : range / 128.0;
    }

    if (page->isMediumResolution()) {
        return page->is3DGrid() ? range / 128.0 : range / 256.0;
    }

    if (page->isHighResolution()) {
        return page->is3DGrid() ? range / 256.0 : range / 512.0;
    }

    if (page->isCustomResolution()) {
        const QString k = QString::fromLatin1(key);

        if (k == "dltx") return range / page->pointsX();
        if (k == "dlty") return range / page->pointsY();
        if (k == "dltz") return range / page->pointsZ();
        if (k == "dltu") return range / page->pointsUV();
        if (k == "dltv") return range / page->pointsUV();
    }

    return 1.0;
}

PlaneResult  ReadWriteOptions::get_plane_case(double a, double b, double c){
    // Return plane case and vectors wu and wv
    //      Case 1: A == 0, B == 0, C != 0
    //      Case 2: A == 0, B != 0, C == 0
    //      Case 3: A != 0, B == 0, C == 0
    //      Case 4: A != 0, B != 0, C == 0
    //      Case 5: A != 0, B == 0, C != 0
    //      Case 6: A == 0, B != 0, C != 0
    //      Case 7: A != 0, b != 0, C != 0
    //      Case -1: Error
    PlaneResult result;
    if (std::abs(a) < 1.e-7){
        if (std::abs(b) < 1.e-7){
            if (std::abs(c) < 1.e-7){
                QMessageBox::warning(nullptr, QObject::tr("DAMQT"),
                    QObject::tr("Parameters A=%1, B=%2, C=%3 do not define a plane")
                    .arg(a).arg(b).arg(c));
                result.planeCase = -1;
                return result;
            }
            else{       // A == 0, B == 0, C != 0
                result.wu = QVector3D(1.0, 0.0, 0.0);
                result.wv = QVector3D(0.0, 1.0, 0.0);
                result.planeCase = 1;
                return result;
            }
        }
        else{
            if (std::abs(c) < 1.e-7){   // A == 0, B != 0, C == 0
                result.wu = QVector3D(1.0, 0.0, 0.0);
                result.wv = QVector3D(0.0, 0.0, 1.0);
                result.planeCase = 2;
                return result;
            }
            else{   // A == 0, B != 0, C != 0
                result.wu = QVector3D(-1.0, 0.0, 0.0);
                result.wv = QVector3D(0.0, -c, b) / QVector3D(0.0,b,c).length();
                result.planeCase = 6;
                return result;
            }
        }
    }
    else{
        if (std::abs(b) < 1.e-7){
            if (std::abs(c) < 1.e-7){   // A != 0, B == 0, C == 0
                result.wu = QVector3D(0.0, 1.0, 0.0);
                result.wv = QVector3D(0.0, 0.0, 1.0);
                result.planeCase = 3;
                return result;
            }
            else{   // A != 0, B == 0, C != 0
                result.wu = QVector3D(0.0, 1.0, 0.0);
                result.wv = QVector3D(-c, 0.0, a) / QVector3D(a,0.0,c).length();
                result.planeCase = 5;
                return result;
            }
        }
        else{
            if (std::abs(c) < 1.e-7){   // A != 0, B != 0, C == 0
                result.wu = QVector3D(-b, a, 0.0) / QVector3D(a,b,0.0).length();
                result.wv = QVector3D(0.0, 0.0, 1.0);
                result.planeCase = 4;
                return result;
            }
            else{   // A != 0, b != 0, C != 0
                result.wu = QVector3D(-b, a, 0.0) / QVector3D(a,b,0.0).length();
                result.wv = QVector3D(-a*c, -b*c, a*a+b*b) / (QVector3D(a,b,0.0).length() * QVector3D(a,b,c).length());
                result.planeCase = 7;
                return result;
            }
        }
    }
}

QString ReadWriteOptions::readRawValue(const char* key, const char* section,
                                       const std::string& file)
{
    const std::string value = CIniFile::GetValue(key, section, file);
    return QString::fromStdString(value);
}

bool ReadWriteOptions::isTrueValue(const QString& value)
{
    return value.compare("T", Qt::CaseInsensitive) == 0;
}

// -----------------------------------------------------------------------------
// Generic readers with widget target
// -----------------------------------------------------------------------------

void ReadWriteOptions::readCheckBox(const char* key, const char* section,
                                    QCheckBox* target, const std::string& file)
{
    if (!target) return;
    target->setChecked(isTrueValue(readRawValue(key, section, file)));
}

void ReadWriteOptions::readInt(const char* key, const char* section,
                                  int* target, const std::string& file)
{
    if (!target) return;

    const QString value = readRawValue(key, section, file);
    bool ok = false;
    double result = value.toInt(&ok);

    if (ok) {
        *target = result;
    }
    else{
        *target = 1;
    }
}

void ReadWriteOptions::readRadioButton(const char* key, const char* section,
                                       QRadioButton* target, const std::string& file)
{
    if (!target) return;
    target->setChecked(isTrueValue(readRawValue(key, section, file)));
}

void ReadWriteOptions::readSpinBox(const char* key, const char* section,
                                   QSpinBox* target, const std::string& file)
{
    if (!target) return;

    bool ok = false;
    const QString value = readRawValue(key, section, file);
    const int intValue = value.toInt(&ok);
    if (ok) {
        target->setValue(intValue);
    }
}

void ReadWriteOptions::readDouble(const char* key, const char* section,
                                  double* target, const std::string& file)
{
    if (!target) return;

    const QString value = readRawValue(key, section, file);
    bool ok = false;
    double result = value.toDouble(&ok);

    if (ok) {
        *target = result;
    }
    else{
        *target = 1.;
    }
}

void ReadWriteOptions::readDoubleToLineEdit(const char* key, const char* section,
                                            QLineEdit* target, const std::string& file)
{
    if (!target) return;

    bool ok = false;
    const QString value = readRawValue(key, section, file);
    value.toDouble(&ok);
    if (!value.isEmpty() && ok) {
        target->setText(value);
    }
}

void ReadWriteOptions::readTextToLineEdit(const char* key, const char* section,
                                          QLineEdit* target, const std::string& file)
{
    if (!target) return;

    QString value = readRawValue(key, section, file);
    value.remove("\"");
    if (!value.isEmpty()) {
        target->setText(value);
    }
}

// -----------------------------------------------------------------------------
// Generic readers returning value
// -----------------------------------------------------------------------------

bool ReadWriteOptions::readCheckBox(const char* key, const char* section,
                                    const std::string& file)
{
    return isTrueValue(readRawValue(key, section, file));
}

bool ReadWriteOptions::readRadioButton(const char* key, const char* section,
                                       const std::string& file)
{
    return isTrueValue(readRawValue(key, section, file));
}

int ReadWriteOptions::readSpinBox(const char* key, const char* section,
                                  const std::string& file)
{
    bool ok = false;
    const QString value = readRawValue(key, section, file);
    const int intValue = value.toInt(&ok);
    return ok ? intValue : -1;
}

QString ReadWriteOptions::readDoubleToLineEdit(const char* key, const char* section,
                                               const std::string& file)
{
    bool ok = false;
    const QString value = readRawValue(key, section, file);
    value.toDouble(&ok);
    return (!value.isEmpty() && ok) ? value : QString();
}

QString ReadWriteOptions::readTextToLineEdit(const char* key, const char* section,
                                             const std::string& file)
{
    QString value = readRawValue(key, section, file);
    value.remove("\"");
    return value;
}

// -----------------------------------------------------------------------------
// Generic dimension reader
// -----------------------------------------------------------------------------

void ReadWriteOptions::readDimensionOption(const char* key, const char* section,
                                           const std::string& file,
                                           IGridOptionsPage* page)
{
    if (!page) return;

    const bool is2D = isTrueValue(readRawValue(key, section, file));
    page->setGrid2D(is2D);

    if (page->is2DGrid()) {
        page->setLowResolutionTip("129x129");
        page->setMediumResolutionTip("257x257");
        page->setHighResolutionTip("513x513");
    } else {
        page->setLowResolutionTip("65x65x65");
        page->setMediumResolutionTip("129x129x129");
        page->setHighResolutionTip("257x257x257");
    }
}

// -----------------------------------------------------------------------------
// Generic replacement for read_plot_dimension
// -----------------------------------------------------------------------------

void ReadWriteOptions::readPlotDimension(const char* key, const char* section,
                                         const std::string& file,
                                         QRadioButton* grid2DButton,
                                         QRadioButton* lowButton,
                                         QRadioButton* mediumButton,
                                         QRadioButton* highButton)
{
    if (!grid2DButton || !lowButton || !mediumButton || !highButton) return;

    const bool is2D = isTrueValue(readRawValue(key, section, file));
    grid2DButton->setChecked(is2D);

    if (grid2DButton->isChecked()) {
        lowButton->setToolTip("129x129");
        mediumButton->setToolTip("257x257");
        highButton->setToolTip("513x513");
    } else {
        lowButton->setToolTip("65x65x65");
        mediumButton->setToolTip("129x129x129");
        highButton->setToolTip("257x257x257");
    }
}

// -----------------------------------------------------------------------------
// Generic resolution query
// -----------------------------------------------------------------------------


int ReadWriteOptions::whatResolution3D(IGridOptionsPage* page)
{
    if (page->deltaX() <= 1.e-10 || page->deltaY() <= 1.e-10 || page->deltaZ() <= 1.e-10) {
        return 0;
    }

    const int depthX = static_cast<int>(
        std::round((page->xMaxValue().toDouble() - page->xMinValue().toDouble()) / page->deltaX()));
    const int depthY = static_cast<int>(
        std::round((page->yMaxValue().toDouble() - page->yMinValue().toDouble()) / page->deltaY()));
    const int depthZ = static_cast<int>(
        std::round((page->zMaxValue().toDouble() - page->zMinValue().toDouble()) / page->deltaZ()));
    if (depthX == depthY && depthX == depthZ) {
        if (depthX == 256) {
            return 2;
        } else if (depthX == 128) {
                return 1;
        } else if (depthX == 64) {
                return 0;
        } else {
            return 3;
        }
    } else {
        return 3;
    }
}

int ReadWriteOptions::whatResolution2D(IGridOptionsPage* page)
{
    if (page->deltaU() <= 1.e-10 || page->deltaV() <= 1.e-10) {
        return 0;
    }

    const int depthU = static_cast<int>(
        std::round((page->uMaxValue().toDouble() - page->uMinValue().toDouble()) / page->deltaU()));
    const int depthV = static_cast<int>(
        std::round((page->vMaxValue().toDouble() - page->vMinValue().toDouble()) / page->deltaV()));
    if (depthU == depthV) {
        if (depthU == 512) {
            return 2;
        } else if (depthU == 256) {
            return 1;
        } else if (depthU == 128) {
                return 0;
        } else {
            return 3;
        }
    } else {
        return 3;
    }
}

// -----------------------------------------------------------------------------
// Generic resolution reader
// -----------------------------------------------------------------------------

void ReadWriteOptions::readResolution(const char* dxKey, const char* dyKey, const char* dzKey,
                                      const char* section, const std::string& file,
                                      IGridOptionsPage* page)
{
    if (!page) return;

    bool okX = false;
    bool okY = false;
    bool okZ = false;

    const double dx = readRawValue(dxKey, section, file).toDouble(&okX);
    const double dy = readRawValue(dyKey, section, file).toDouble(&okY);
    const double dz = readRawValue(dzKey, section, file).toDouble(&okZ);

    if (page->is3DGrid()){
        page->setDeltaX(dx);
        page->setDeltaY(dy);
        page->setDeltaZ(dz);
    }
    else{
        page->setDeltaU(dx);
        page->setDeltaV(dy);
    }

    page->setLowResolution(true); // default

    if (!(okX && okY && okZ) || dx <= 0.0 || dy <= 0.0 || dz <= 0.0) {
        return;
    }

    const int depthX = static_cast<int>(
        std::round((page->xMaxValue().toDouble() - page->xMinValue().toDouble()) / dx));
    const int depthY = static_cast<int>(
        std::round((page->yMaxValue().toDouble() - page->yMinValue().toDouble()) / dy));
    const int depthZ = static_cast<int>(
        std::round((page->zMaxValue().toDouble() - page->zMinValue().toDouble()) / dz));

    if (depthX == depthY && depthX == depthZ) {
        if (depthX == 512) {
            page->setHighResolution(true);
        } else if (depthX == 256) {
            if (page->is3DGrid()) {
                page->setHighResolution(true);
            } else {
                page->setMediumResolution(true);
            }
        } else if (depthX == 128) {
            if (page->is3DGrid()) {
                page->setMediumResolution(true);
            } else {
                page->setLowResolution(true);
            }
        } else if (depthX == 64) {
            if (page->is3DGrid()) {
                page->setLowResolution(true);
            } else {
                page->setCustomResolution(true);
            }
        } else {
            page->setCustomResolution(true);
            if (page->is3DGrid()) {
                page->setPointsX(depthX);
                page->setPointsY(depthY);
                page->setPointsZ(depthZ);
            } else {
                page->setPointsUV(depthX);
            }
        }
    } else {
        page->setCustomResolution(true);
        if (page->is3DGrid()) {
            page->setPointsX(depthX);
            page->setPointsY(depthY);
            page->setPointsZ(depthZ);
        } else {
            page->setPointsUV(depthX);
        }
    }
}

// -----------------------------------------------------------------------------
// Writers
// -----------------------------------------------------------------------------


void ReadWriteOptions::writeIntervals(const char* minKey, const char* maxKey, const char* deltaKey,
                                      const char* section, const std::string& file,
                                      QLineEdit* minEdit, QLineEdit* maxEdit,
                                      bool* errorFlag, QString* errorText,
                                      const DeltaCalculator& deltaCalculator)
{
    if (!minEdit || !maxEdit) return;

    double minValue = minEdit->text().toDouble();
    double maxValue = maxEdit->text().toDouble();

    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
        minEdit->setText(QString::number(minValue));
        maxEdit->setText(QString::number(maxValue));
    }

    double delta = 1.0;
    if (deltaCalculator) {
        delta = (maxValue != minValue)
                    ? deltaCalculator(deltaKey, minValue, maxValue)
                    : 1.0;
    }

    writeOption(minKey, section, QString::number(minValue), file, errorFlag, errorText);
    writeOption(maxKey, section, QString::number(maxValue), file, errorFlag, errorText);

    if (maxValue != minValue) {
        writeOption(deltaKey, section, QString::number(delta), file, errorFlag, errorText);
    }
}

double ReadWriteOptions::writeIntervals(const char* minKey, const char* maxKey, const char* deltaKey,
                                      const char* section, const std::string& file,
                                      const QString& minValueText, const QString& maxValueText,
                                      bool* errorFlag, QString* errorText,
                                      const DeltaCalculator& deltaCalculator)
{
    double minValue = minValueText.toDouble();
    double maxValue = maxValueText.toDouble();

    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
    }

    double delta = 1.0;

    if (deltaCalculator && maxValue != minValue) {
        delta = deltaCalculator(deltaKey, minValue, maxValue);
    }

    writeOption(minKey, section, QString::number(minValue), file, errorFlag, errorText);
    writeOption(maxKey, section, QString::number(maxValue), file, errorFlag, errorText);

    if (maxValue != minValue) {
        writeOption(deltaKey, section, QString::number(delta), file, errorFlag, errorText);
    }
    return delta;
}

void ReadWriteOptions::writeOption(const char* key, const char* section,
                                   const QString& value, const std::string& file,
                                   bool* errorFlag, QString* errorText)
{
    if (!CIniFile::SetValue(key, value.toStdString(), section, file)) {
        if (errorText) {
            errorText->append(key).append(" in ").append(section).append("\n");
        }
        if (errorFlag) {
            *errorFlag = true;
        }
    }
}

void ReadWriteOptions::writeRTable(const char* section,
                                     const std::string& file,
                                     const IXYZTabulationPage* page,
                                     int maxRows,
                                     bool* errorFlag,
                                     QString* errorText)
{
    if (!page) return;

    const int rows = page->xyzTableRows();

    writeOption(page->numtabular().toUtf8().constData(), section, QString::number(rows),
                file, errorFlag, errorText);

    for (int i = 0; i < rows; ++i) {
            const QString key = page->tabularkey()+QString("(%1)").arg(i + 1);

            writeOption(key.toStdString().c_str(),
                        section,
                        page->getCellValue(i, 0),
                        file,
                        errorFlag,
                        errorText);
    }

    for (int i = rows; i < maxRows; ++i) {
        const QString key = page->tabularkey()+QString("(%1)").arg(i + 1);
        CIniFile::DeleteRecord(key.toStdString(), section, file);
    }
}

void ReadWriteOptions::writeUVLinesTable(const char* section,
                                     const std::string& file,
                                     const ILinesTabulationPage* page,
                                     int maxRows,
                                     bool* errorFlag,
                                     QString* errorText)
{
    if (!page) return;

    const int rows = page->linesUVTableRows();
    const int cols = 3;

    writeOption(page->numtabular().toUtf8().constData(), section, QString::number(rows),
                file, errorFlag, errorText);

    for (int i = 0; i < rows; ++i) {
        const QString key = QString("icntlines(%1)").arg(i+1);

        QString value = page->getUVCellValue(i, 0);
        if (value.isEmpty()) value = QString("0");
        writeOption(key.toStdString().c_str(),
                    section,
                    value,
                    file,
                    errorFlag,
                    errorText);
        for (int j = 1; j < cols; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j).arg(i + 1);

            QString value = page->getUVCellValue(i, j);
            if (value.isEmpty()) value = QString("0");
            writeOption(key.toStdString().c_str(),
                        section,
                        value,
                        file,
                        errorFlag,
                        errorText);
        }
    }

    for (int i = rows; i < maxRows; ++i) {
        CIniFile::DeleteRecord(QString("icntlines(%1)").arg(i + 1).toStdString(), section, file);
        for (int j = 1; j <= cols; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j).arg(i + 1);
            CIniFile::DeleteRecord(key.toStdString(), section, file);
        }
    }
}


void ReadWriteOptions::writeXYZLinesTable(const char* section,
                                     const std::string& file,
                                     const ILinesTabulationPage* page,
                                     int maxRows,
                                     bool* errorFlag,
                                     QString* errorText)
{
    if (!page) return;

    const int rows = page->linesXYZTableRows();
    const int cols = 4;

    writeOption(page->numtabular().toUtf8().constData(), section, QString::number(rows),
                file, errorFlag, errorText);

    for (int i = 0; i < rows; ++i) {
        const QString key = QString("icntlines(%1)").arg(i+1);

        QString value = page->getXYZCellValue(i, 0);
        if (value.isEmpty()) value = QString("0");
        writeOption(key.toStdString().c_str(),
                    section,
                    value,
                    file,
                    errorFlag,
                    errorText);

        for (int j = 1; j < cols; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j).arg(i + 1);

            QString value = page->getXYZCellValue(i, j);
            if (value.isEmpty()) value = QString("0");
            writeOption(key.toStdString().c_str(),
                        section,
                        value,
                        file,
                        errorFlag,
                        errorText);
        }
    }

    for (int i = rows; i < maxRows; ++i) {
        CIniFile::DeleteRecord(QString("icntlines(%1)").arg(i + 1).toStdString(), section, file);
        for (int j = 1; j <= cols; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j).arg(i + 1);
            CIniFile::DeleteRecord(key.toStdString(), section, file);
        }
    }
}

void ReadWriteOptions::writeXYZTable(const char* section,
                                     const std::string& file,
                                     const IXYZTabulationPage* page,
                                     int maxRows,
                                     bool* errorFlag,
                                     QString* errorText)
{
    if (!page) return;

    const int rows = page->xyzTableRows();

    writeOption(page->numtabular().toUtf8().constData(), section, QString::number(rows),
                file, errorFlag, errorText);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < 3; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j + 1).arg(i + 1);

            writeOption(key.toStdString().c_str(),
                        section,
                        page->getCellValue(i, j),
                        file,
                        errorFlag,
                        errorText);
        }
    }

    for (int i = rows; i < maxRows; ++i) {
        for (int j = 1; j <= 3; ++j) {
            const QString key = page->tabularkey()+QString("(%1,%2)").arg(j).arg(i + 1);
            CIniFile::DeleteRecord(key.toStdString(), section, file);
        }
    }
}


void ReadWriteOptions::writeQuotedOption(const char* key, const char* section,
                                         const QString& value, const std::string& file,
                                         bool* errorFlag, QString* errorText)
{
    QString quoted = value;
    quoted.remove("\"");
    quoted.prepend("\"").append("\"");

    writeOption(key, section, quoted, file, errorFlag, errorText);
}

