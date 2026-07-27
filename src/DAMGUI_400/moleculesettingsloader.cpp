#include "moleculesettingsloader.h"

#include "settingsreader.h"

#include "grid.h"
#include "isosurface.h"
#include "molecule.h"
#include "surface.h"

#include <QFileInfo>
#include <QObject>


MoleculeSettingsLoader::MoleculeSettingsLoader(
        const SettingsReader &reader)
    : reader_(reader)
{
}


void MoleculeSettingsLoader::applyMoleculeSettings(
        molecule *mol,
        const QString &section,
        bool restorePosition,
        bool restoreRotation) const
{
    if (!mol)
        return;

    QFont font;

    if (font.fromString(reader_.value(section,"font")))
        mol->setfont(font);

    mol->setdrawatomsymbols(
        reader_.boolValue(section,"showsymbols")
    );

    mol->setdrawatomindices(
        reader_.boolValue(section,"showindices")
    );

    const bool showSelected =
        reader_.boolValue(section,"showselect");

    mol->setonlyatomactive(showSelected);

    if (showSelected) {
        const QStringList activeAtoms =
            reader_.listValue(section,"atomactive");

        const int atomCount =
            qMin(activeAtoms.size(),mol->getnumatoms());

        for (int i = 0; i < atomCount; i++) {
            mol->setatomactive(
                i,
                activeAtoms.at(i).trimmed().toInt() != 0
            );
        }
    }

    mol->setlabelsvshift(
        reader_.intValue(section,"vshift")
    );

    if (restorePosition) {
        QVector3D translation;

        if (reader_.vectorValue(
                section,"position",translation)) {
            mol->settranslation(translation);
            mol->settranslationButtons();
        }
    }

    if (restoreRotation) {
        QQuaternion rotation;

        if (reader_.quaternionValue(
                section,"rotation",rotation)) {
            mol->setrotation(rotation.normalized());
            mol->setrotationButtons();
        }
    }

    QColor fontColor;

    if (reader_.colorValue(
            section,"fontcolor",fontColor)) {
        mol->setfontcolor(fontColor);
    }
}


void MoleculeSettingsLoader::loadSurfaces(
        molecule *mol,
        const QString &section) const
{
    if (!mol)
        return;

    const int surfaceCount =
        reader_.intValue(section,"nsurf");

    for (int i = 0; i < surfaceCount; i++) {
        const QString fileName = reader_.value(
            section,
            QString("surfname_%1").arg(i)
        );

        if (fileName.isEmpty())
            continue;

        if (!mol->retrievesurf(fileName))
            continue;

        surface *currentSurface = mol->surfaces->last();

        QObject::connect(
            currentSurface,&surface::updatedisplay,
            mol,&molecule::updatedisplay
        );

        applySurfaceSettings(
            currentSurface,section,i
        );

        if (mol->QDLeditMolecule_isVisible())
            mol->updateeditMoleculeDialog();
    }

    mol->emitupdatedisplay();
}


void MoleculeSettingsLoader::applySurfaceSettings(
        surface *surf,
        const QString &section,
        int surfaceIndex) const
{
    if (!surf)
        return;

    surf->setsolidsurf(
        reader_.boolValue(
            section,
            QString("solid_%1").arg(surfaceIndex)
        )
    );

    surf->settranslucence(
        reader_.boolValue(
            section,
            QString("trasluc_%1").arg(surfaceIndex)
        )
    );

    surf->setopacity(
        reader_.floatValue(
            section,
            QString("opacity_%1").arg(surfaceIndex),
            1.0f
        )
    );

    surf->settopcolor(
        reader_.floatValue(
            section,
            QString("topcolor_%1").arg(surfaceIndex)
        )
    );

    surf->resetsurface();
}


void MoleculeSettingsLoader::loadGrids(
        molecule *mol,
        const QString &section) const
{
    if (!mol)
        return;

    const int gridCount =
        reader_.intValue(section,"ngrids");

    for (int i = 0; i < gridCount; i++) {
        const QString gridName = reader_.value(
            section,
            QString("gridname_%1").arg(i)
        );

        if (gridName.isEmpty())
            continue;

        if (!mol->retrievegrid(gridName))
            continue;

        /*
         * retrievegrid() appends the successfully loaded grid.
         * Using last() is safer than using at(i), because a previous
         * grid may have failed to load.
         */
        grid *currentGrid = mol->grids->last();

        applyGridSettings(
            currentGrid,section,i
        );

        const int isosurfaceCount =
            reader_.intValue(
                section,
                QString("nisosurf_%1").arg(i)
            );

        for (int j = 0; j < isosurfaceCount; j++) {
            currentGrid->addisosurf();

            if (currentGrid->surfaces->isEmpty())
                continue;

            isosurface *currentIsosurface =
                currentGrid->surfaces->last();

            applyIsosurfaceSettings(
                currentIsosurface,
                section,
                i,
                j
            );

            /*
             * The index passed to generatesurf() must be the actual
             * position of the newly created isosurface.
             */
            const int generatedIndex =
                currentGrid->surfaces->size() - 1;

            currentGrid->generatesurf(generatedIndex);
        }
    }
}


void MoleculeSettingsLoader::applyGridSettings(
        grid *currentGrid,
        const QString &section,
        int gridIndex) const
{
    if (!currentGrid)
        return;

    currentGrid->setmaxcontourvalue(
        reader_.floatValue(
            section,
            QString("maxcontourval_%1").arg(gridIndex)
        )
    );

    currentGrid->setmincontourvalue(
        reader_.floatValue(
            section,
            QString("mincontourval_%1").arg(gridIndex)
        )
    );

    const QString gridFile = reader_.value(
        section,
        QString("gridname_%1").arg(gridIndex)
    );

    if (gridFile.isEmpty())
        return;

    const QFileInfo gridInfo(gridFile);

    currentGrid->set_ProjectFolder(
        gridInfo.absolutePath()
    );

    QString projectName = gridInfo.fileName();

    projectName.remove("-deform");
    projectName.remove("-d.plt");
    projectName.remove("v-plt");

    currentGrid->set_ProjectName(projectName);
}


void MoleculeSettingsLoader::applyIsosurfaceSettings(
        isosurface *iso,
        const QString &section,
        int gridIndex,
        int isoIndex) const
{
    if (!iso)
        return;

    iso->setcontourvalue(
        reader_.floatValue(
            section,
            QString("contourval_%1_%2")
                .arg(gridIndex)
                .arg(isoIndex)
        )
    );

    iso->setsolidsurf(
        reader_.boolValue(
            section,
            QString("isosurfsolid_%1_%2")
                .arg(gridIndex)
                .arg(isoIndex)
        )
    );

    iso->settranslucence(
        reader_.boolValue(
            section,
            QString("isosurftrasluc_%1_%2")
                .arg(gridIndex)
                .arg(isoIndex)
        )
    );

    iso->setopacity(
        reader_.floatValue(
            section,
            QString("isosurfopacity_%1_%2")
                .arg(gridIndex)
                .arg(isoIndex),
            1.0f
        )
    );

    QColor surfaceColor;

    if (reader_.colorValue(
            section,
            QString("isosurfcolor_%1_%2")
                .arg(gridIndex)
                .arg(isoIndex),
            surfaceColor)) {
        iso->setsurfcolor(surfaceColor);
    }
}
