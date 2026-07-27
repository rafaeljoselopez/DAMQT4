#ifndef MOLECULESETTINGSWRITER_H
#define MOLECULESETTINGSWRITER_H

#include <QString>

class molecule;
class surface;
class grid;
class isosurface;
class SettingsWriter;

class MoleculeSettingsWriter
{
public:
    explicit MoleculeSettingsWriter(SettingsWriter& writer);

    void writeMolecule(molecule* mol,
                       const QString& section,
                       bool savePosition,
                       bool saveRotation,
                       bool saveSurfaces,
                       bool saveGrids,
                       bool saveIsosurfaces);

private:
    void writeSurface(surface* surf,
                      int index);

    void writeGrid(grid* grd,
                   int index,
                   bool saveIsosurfaces);

    void writeIsosurface(isosurface* iso,
                         int gridIndex,
                         int isoIndex);

    SettingsWriter& writer_;
};

#endif // MOLECULESETTINGSWRITER_H
