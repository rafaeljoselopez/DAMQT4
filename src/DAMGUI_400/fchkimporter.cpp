#include "fchkimporter.h"

#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>

FchkImporter::FchkImporter(QObject *parent)
    : CalculationImporter(parent)
{
    connect(
        &process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &FchkImporter::onProcessFinished
    );
}

bool FchkImporter::analyzeFirstPass()
{
    if (currentDensityIndex_ != 0) {
        return true;
    }

    const QString outputFilePath =
        currentRequest_.projectFolder
        + currentRequest_.projectName
        + QStringLiteral("-GAUSS_interface.out");

    QFile file(outputFilePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit errorOccurred(
            tr("File %1 cannot be read:\n%2.")
                .arg(outputFilePath, file.errorString())
        );
        return false;
    }

    QTextStream input(&file);

    availableDensities_.clear();

    while (!input.atEnd()) {
//        availableDensities_.append(input.readLine().trimmed());
        const QString line = input.readLine().trimmed();

        availableDensities_.append(line);

    }

//    const int densityCount = availableDensityCount();

    file.close();

    if (!selectDensity()) {
        return false;
    }

    return true;
}

bool FchkImporter::hasDensityMarker(const QString& marker) const
{
    return availableDensities_.contains(marker);
}

bool FchkImporter::selectDensity()
{
    if (availableDensityCount() == 0) {
        selectedDensity_ = DensityType::None;
        return true;
    }
    if (availableDensityCount() == 1 &&
        hasDensityMarker(QStringLiteral("existSCF"))) {

        selectedDensity_ = DensityType::SCF;

        return true;
    }
    return showDensitySelectionDialog();
}

bool FchkImporter::showDensitySelectionDialog()
{
    const QList<DensityType> densities = availableDensityTypes();

    if (densities.isEmpty())
        return false;

    QStringList items;

    for (DensityType density : densities) {
        items.append(densityDisplayName(density));
    }

    bool accepted = false;

    const QString selectedItem = QInputDialog::getItem(
        nullptr,
        tr("Density selection"),
        tr("Select the density to import:"),
        items,
        0,
        false,
        &accepted
    );

    if (!accepted)
        return false;

    const int selectedPosition = items.indexOf(selectedItem);

    if (selectedPosition < 0)
        return false;

    selectedDensity_ = densities.at(selectedPosition);

    return true;
}


bool FchkImporter::supportsFile(const QFileInfo& fileInfo) const
{
    return fileInfo.suffix().compare(
        QStringLiteral("fchk"),
        Qt::CaseInsensitive
    ) == 0;
}

int FchkImporter::availableDensityCount() const
{
    int count = 0;

    if (hasDensityMarker(QStringLiteral("existSCF")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existSpSCF")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existCI")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existSpCI")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existMP2")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existSpMP2")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existCC")))
        ++count;

    if (hasDensityMarker(QStringLiteral("existSpCC")))
        ++count;

    return count;
}


void FchkImporter::handleNormalProcessExit()
{
    const QString outputFilePath =
        currentRequest_.projectFolder
        + currentRequest_.projectName
        + QStringLiteral("-GAUSS_interface.out");

    QFile file(outputFilePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit outputFileError(
            outputFilePath,
            file.errorString()
        );
        return;
    }

    QTextStream in(&file);
    emit outputTextReady(in.readAll());
}

int FchkImporter::interfaceDensityIndex(DensityType density) const
{
    switch (density) {
    case DensityType::None:
        return 0;
    case DensityType::SCF:
        return 1;
    case DensityType::SpinSCF:
        return 2;
    case DensityType::CI:
        return 3;
    case DensityType::SpinCI:
        return 4;
    case DensityType::MP2:
        return 5;
    case DensityType::SpinMP2:
        return 6;
    case DensityType::CC:
        return 7;
    case DensityType::SpinCC:
        return 8;
    }

    return 0;
}

void FchkImporter::onProcessFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus
)
{
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit errorOccurred(
            tr("GAUSS_interface finished with an error.")
        );
        return;
    }

    if (!analyzeFirstPass()) {
        return;
    }

    if (currentDensityIndex_ == 0) {
        const int densityIndex =
            interfaceDensityIndex(selectedDensity_);

        runInterface(densityIndex);
        return;
    }
    handleNormalProcessExit();
    emit importFinished();

}

void FchkImporter::runInterface(int densityIndex)
{
    currentDensityIndex_ = densityIndex;

    QStringList parameters;

    const QFileInfo fileInfo(currentRequest_.importFilePath);

    parameters
        << fileInfo.fileName()
        << QString::number(densityIndex)
        << fileInfo.absolutePath()
        << currentRequest_.projectFolder
        << currentRequest_.projectName;

    process_.start(
        currentRequest_.executablePath,
        parameters
    );
}

void FchkImporter::start(const ImportRequest& request)
{
    QFileInfo fileInfo(request.importFilePath);

    if (!supportsFile(fileInfo)) {
        emit errorOccurred(
            tr("The selected file is not a Gaussian formatted checkpoint (*.fchk) file.")
        );
        return;
    }

    currentRequest_ = request;

    currentDensityIndex_ = 0;
    selectedDensity_ = DensityType::None;
    availableDensities_.clear();

    if (request.executablePath.isEmpty()) {
        emit errorOccurred(
            tr("The path to GAUSS_interface has not been specified.")
        );
        return;
    }

    const QString outputFilePath =
        currentRequest_.projectFolder
        + currentRequest_.projectName
        + QStringLiteral("-GAUSS_interface.out");

    QFile::remove(outputFilePath);

    runInterface(0);
}

FchkImporter::DensityType
FchkImporter::densityTypeFromMarker(const QString& marker) const
{
    if (marker == QStringLiteral("existSCF"))
        return DensityType::SCF;

    if (marker == QStringLiteral("existSpSCF"))
        return DensityType::SpinSCF;

    if (marker == QStringLiteral("existCI"))
        return DensityType::CI;

    if (marker == QStringLiteral("existSpCI"))
        return DensityType::SpinCI;

    if (marker == QStringLiteral("existMP2"))
        return DensityType::MP2;

    if (marker == QStringLiteral("existSpMP2"))
        return DensityType::SpinMP2;

    if (marker == QStringLiteral("existCC"))
        return DensityType::CC;

    if (marker == QStringLiteral("existSpCC"))
        return DensityType::SpinCC;

    return DensityType::None;
}

QList<FchkImporter::DensityType>
FchkImporter::availableDensityTypes() const
{
    QList<DensityType> result;

    if (hasDensityMarker(QStringLiteral("existSCF")))
        result.append(DensityType::SCF);

    if (hasDensityMarker(QStringLiteral("existSpSCF")))
        result.append(DensityType::SpinSCF);

    if (hasDensityMarker(QStringLiteral("existCI")))
        result.append(DensityType::CI);

    if (hasDensityMarker(QStringLiteral("existSpCI")))
        result.append(DensityType::SpinCI);

    if (hasDensityMarker(QStringLiteral("existMP2")))
        result.append(DensityType::MP2);

    if (hasDensityMarker(QStringLiteral("existSpMP2")))
        result.append(DensityType::SpinMP2);

    if (hasDensityMarker(QStringLiteral("existCC")))
        result.append(DensityType::CC);

    if (hasDensityMarker(QStringLiteral("existSpCC")))
        result.append(DensityType::SpinCC);

    return result;
}

QString FchkImporter::densityDisplayName(DensityType density) const
{
    switch (density) {
    case DensityType::SCF:
        return tr("SCF density");

    case DensityType::SpinSCF:
        return tr("SCF spin density");

    case DensityType::CI:
        return tr("CI density");

    case DensityType::SpinCI:
        return tr("CI spin density");

    case DensityType::MP2:
        return tr("MP2 density");

    case DensityType::SpinMP2:
        return tr("MP2 spin density");

    case DensityType::CC:
        return tr("CC density");

    case DensityType::SpinCC:
        return tr("CC spin density");

    case DensityType::None:
        return QString();
    }

    return QString();
}

QString FchkImporter::densitySuffix(DensityType density) const
{
    switch (density) {
    case DensityType::SCF:     return QStringLiteral("SCF");
    case DensityType::SpinSCF: return QStringLiteral("SP_SCF");
    case DensityType::CI:      return QStringLiteral("CI");
    case DensityType::SpinCI:  return QStringLiteral("SP_CI");
    case DensityType::MP2:     return QStringLiteral("MP2");
    case DensityType::SpinMP2: return QStringLiteral("SP_MP2");
    case DensityType::CC:      return QStringLiteral("CC");
    case DensityType::SpinCC:  return QStringLiteral("SP_CC");
    default:
        return QString();
    }
}
