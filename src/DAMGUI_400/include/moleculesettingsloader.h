#ifndef MOLECULESETTINGSLOADER_H
#define MOLECULESETTINGSLOADER_H

#include <QString>

class SettingsReader;
class molecule;
class grid;
class surface;
class isosurface;

class MoleculeSettingsLoader
{
public:
    explicit MoleculeSettingsLoader(const SettingsReader &reader);

    void applyMoleculeSettings(molecule *mol,
                               const QString &section,
                               bool restorePosition,
                               bool restoreRotation) const;

    void loadSurfaces(molecule *mol,
                      const QString &section) const;

    void loadGrids(molecule *mol,
                   const QString &section) const;

private:
    void applySurfaceSettings(surface *surf,
                              const QString &section,
                              int surfaceIndex) const;

    void applyGridSettings(grid *currentGrid,
                           const QString &section,
                           int gridIndex) const;

    void applyIsosurfaceSettings(isosurface *iso,
                                 const QString &section,
                                 int gridIndex,
                                 int isoIndex) const;

    const SettingsReader &reader_;
};

#endif
