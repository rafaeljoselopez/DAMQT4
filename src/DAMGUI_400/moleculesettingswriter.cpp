#include "moleculesettingswriter.h"

#include "settingswriter.h"

#include "grid.h"
#include "molecule.h"
#include "surface.h"
#include "isosurface.h"

#include <QColor>
#include <QFont>
#include <QStringList>

MoleculeSettingsWriter::MoleculeSettingsWriter(SettingsWriter& writer)
    : writer_(writer)
{
}

void MoleculeSettingsWriter::writeMolecule(
    molecule* mol,
    const QString& section,
    bool savePosition,
    bool saveRotation,
    bool saveSurfaces,
    bool saveGrids,
    bool saveIsosurfaces)
{
    if (!mol)
        return;

    writer_.beginSection(section);

    writer_.writeText("name", mol->getfullname());
    writer_.writeText("font", mol->getfont().toString());

    writer_.writeBool(
        "showsymbols",
        mol->getdrawatomsymbols()
    );

    writer_.writeBool(
        "showindices",
        mol->getdrawatomindices()
    );

    writer_.writeBool(
        "showselect",
        mol->getonlyatomactive()
    );

    if (mol->getonlyatomactive()) {
        const int numberOfAtoms = mol->getnumatoms();

        writer_.writeInt(
            "atomactlen",
            numberOfAtoms
        );

        QStringList activeAtoms;
        activeAtoms.reserve(numberOfAtoms);

        for (int i = 0; i < numberOfAtoms; ++i) {
            activeAtoms.append(
                QString::number(mol->isatomactive(i))
            );
        }

        writer_.writeList(
            "atomactive",
            activeAtoms
        );
    }

    writer_.writeFloat(
        "vshift",
        mol->getlabelsvshift()
    );

    if (savePosition) {
        writer_.writeVector(
            "position",
            mol->gettranslation()
        );
    }

    if (saveRotation) {
        writer_.writeQuaternion(
            "rotation",
            mol->getrotation()
        );
    }

    writer_.writeColor(
        "fontcolor",
        mol->getfontcolor()
    );

    if (saveSurfaces) {
        const int numberOfSurfaces = mol->surfaces.count();

        writer_.writeInt(
            "nsurf",
            numberOfSurfaces
        );

        for (int i = 0; i < numberOfSurfaces; ++i) {
            writeSurface(
                mol->surfaces.at(i),
                i
            );
        }
    }

    if (saveGrids) {
        const int numberOfGrids = mol->grids.count();

        writer_.writeInt(
            "ngrids",
            numberOfGrids
        );

        for (int i = 0; i < numberOfGrids; ++i) {
            writeGrid(
                mol->grids.at(i),
                i,
                saveIsosurfaces
            );
        }
    }
}

void MoleculeSettingsWriter::writeSurface(
    surface* surf,
    int index)
{
    if (!surf)
        return;

    writer_.writeText(
        QString("surfname_%1").arg(index),
        surf->getfullname()
    );

    writer_.writeBool(
        QString("solid_%1").arg(index),
        surf->getsolidsurf()
    );

    writer_.writeFloat(
        QString("opacity_%1").arg(index),
        surf->getopacity()
    );

    writer_.writeBool(
        QString("trasluc_%1").arg(index),
        surf->gettranslucence()
    );

    writer_.writeFloat(
        QString("topcolor_%1").arg(index),
        surf->gettopcolor()
    );

}

void MoleculeSettingsWriter::writeGrid(
    grid* grd,
    int index,
    bool saveIsosurfaces)
{
    if (!grd)
        return;

    writer_.writeText(
        QString("gridname_%1").arg(index),
        grd->getfullname()
    );

    if (!saveIsosurfaces)
        return;

    const int numberOfIsosurfaces = grd->surfaces.count();

    writer_.writeInt(
        QString("nisosurf_%1").arg(index),
        numberOfIsosurfaces
    );

    writer_.writeFloat(
        QString("maxcontourval_%1").arg(index),
        grd->getmaxcontourvalue()
    );

    writer_.writeFloat(
        QString("mincontourval_%1").arg(index),
        grd->getmincontourvalue()
    );

    for (int i = 0; i < numberOfIsosurfaces; ++i) {
        writeIsosurface(
            grd->surfaces.at(i),
            index,
            i
        );
    }
}

void MoleculeSettingsWriter::writeIsosurface(
    isosurface* iso,
    int gridIndex,
    int isoIndex)
{
    if (!iso)
        return;

    const QString suffix =
        QString("%1_%2").arg(gridIndex).arg(isoIndex);

    writer_.writeFloat(
        "contourval_" + suffix,
        iso->getcontourvalue()
    );

    writer_.writeBool(
        "isosurfsolid_" + suffix,
        iso->getsolidsurf()
    );

    writer_.writeFloat(
        "isosurfopacity_" + suffix,
        iso->getopacity()
    );

    writer_.writeBool(
        "isosurftrasluc_" + suffix,
        iso->gettranslucence()
    );

    writer_.writeColor(
        "isosurfcolor_" + suffix,
        iso->getsurfcolor()
    );
}
