#ifndef FCHKIMPORTER_H
#define FCHKIMPORTER_H

#include "calculationimporter.h"

#include <QStringList>

enum class FchkDensity
{
    ScfTotal = 1,
    ScfSpin,
    CiTotal,
    CiSpin,
    Mp2Total,
    Mp2Spin,
    CcTotal,
    CcSpin
};

struct FchkDensityOptions
{
    bool scfTotal = false;
    bool scfSpin = false;
    bool ciTotal = false;
    bool ciSpin = false;
    bool mp2Total = false;
    bool mp2Spin = false;
    bool ccTotal = false;
    bool ccSpin = false;

    int count() const
    {
        return static_cast<int>(scfTotal)
             + static_cast<int>(scfSpin)
             + static_cast<int>(ciTotal)
             + static_cast<int>(ciSpin)
             + static_cast<int>(mp2Total)
             + static_cast<int>(mp2Spin)
             + static_cast<int>(ccTotal)
             + static_cast<int>(ccSpin);
    }
};

class FchkImporter : public CalculationImporter
{
    Q_OBJECT

public:
    explicit FchkImporter(QObject* parent = nullptr);

    bool supportsFile(const QFileInfo& fileInfo) const override;
    void start(const ImportRequest& request) override;

signals:
    void importFinished();

    void outputFileError(
            const QString& outputFilePath,
            const QString& errorString);

    void outputTextReady(const QString &text);

private slots:
    void handleNormalProcessExit();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:

    enum class DensityType {
        None,
        SCF,
        SpinSCF,
        CI,
        SpinCI,
        MP2,
        SpinMP2,
        CC,
        SpinCC
    };

    bool analyzeFirstPass();
    bool hasDensityMarker(const QString& marker) const;
    bool selectDensity();
    bool showDensitySelectionDialog();

    int availableDensityCount() const;
    int interfaceDensityIndex(DensityType density) const;

    void runInterface(int densityIndex);

    QList<DensityType> availableDensityTypes() const;

    QString densityDisplayName(DensityType density) const;
    QString densitySuffix(DensityType density) const;

    DensityType densityTypeFromMarker(const QString& marker) const;

    ImportRequest currentRequest_;

    int currentDensityIndex_ = 0;

    DensityType selectedDensity_ = DensityType::None;


    QStringList availableDensities_;


};

#endif
