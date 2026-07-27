#include "molproimporter.h"
#include "dialog.h"

#include <QFileInfo>

MolproImporter::MolproImporter(QObject* parent)
    : QObject(parent)
{
    connect(&process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MolproImporter::onProcessFinished);
}

bool MolproImporter::analyzeFirstPass()
{
    if (currentEntryIndex_ != 0)
        return true;

    const QString outputFilePath =
        currentRequest_.projectFolder
        + currentRequest_.projectName
        + "-MOLPRO_out_interface.out";

    QFile file(outputFilePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);

    const QString firstLine = in.readLine();
    const int numberOfEntries = firstLine.left(2).toInt();

    availableEntries_.clear();

    for (int i = 0; i < numberOfEntries; ++i) {
        const QString line = in.readLine();
        availableEntries_.append(line.mid(2));
    }

    return !availableEntries_.isEmpty();
}

bool MolproImporter::selectEntry()
{
    const int numberOfEntries = availableEntries_.size();

    if (numberOfEntries == 0)
        return false;

    if (numberOfEntries == 1) {
        selectedEntryIndex_ = 1;
        return true;
    }

    Dialog dialog(numberOfEntries);

    for (int i = 0; i < numberOfEntries; ++i) {
        dialog.buttons[i] = true;
        dialog.RBToption[i]->setText(availableEntries_.at(i));
    }

    dialog.RBToption[0]->setChecked(true);
    dialog.val = 1;

    dialog.Implement();
    dialog.RadioChange();

    if (dialog.exec() != QDialog::Accepted)
        return false;

    selectedEntryIndex_ = dialog.val;

    return selectedEntryIndex_ > 0;
}


void MolproImporter::handleNormalProcessExit()
{
    QString outputFilePath =
        currentRequest_.projectFolder
        + currentRequest_.projectName
        + "-MOLPRO_out_interface.out";

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

void MolproImporter::onProcessFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0)
        return;

    if (currentEntryIndex_ == 0) {

        if (!analyzeFirstPass())
            return;

        if (!selectEntry())
            return;

        runInterface(selectedEntryIndex_);
        return;
    }
    handleNormalProcessExit();
    emit importFinished();
}

void MolproImporter::start(const MolproImportRequest& request)
{
    currentRequest_ = request;

    currentEntryIndex_ = 0;
    selectedEntryIndex_ = 0;
    availableEntries_.clear();

    runInterface(0);
}

void MolproImporter::runInterface(int entryIndex)
{
    currentEntryIndex_ = entryIndex;

        QFileInfo importFile(currentRequest_.importFilePath);

        QString importFolder = importFile.absolutePath();
        if (!importFolder.endsWith('/'))
            importFolder.append('/');

        QStringList parameters;
        parameters
            << importFile.fileName()
            << QString::number(entryIndex)
            << importFolder
            << currentRequest_.projectFolder
            << currentRequest_.projectName;

qDebug() << "parameters = " << parameters;

        QString outputFile =
            currentRequest_.projectFolder
            + currentRequest_.projectName
            + "-MOLPRO_out_interface.out";

        process_.setStandardOutputFile(
            outputFile,
            QIODevice::Truncate);

        process_.start(
            currentRequest_.executablePath,
            parameters);
}

