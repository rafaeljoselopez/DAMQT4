#include "singlepassimporter.h"

#include <QFile>
#include <QTextStream>

SinglePassImporter::SinglePassImporter(QObject* parent)
    : QObject(parent)
{
    connect(&process_, &QProcess::started,
        this, &SinglePassImporter::processStarted);

    connect(&process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this, &SinglePassImporter::onProcessFinished);
}

void SinglePassImporter::handleNormalProcessExit()
{
    QFile file(currentOutputFilePath_);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        emit outputFileError(
            currentOutputFilePath_,
            file.errorString()
        );
        return;
    }

    QTextStream in(&file);
    emit outputTextReady(in.readAll());
}

void SinglePassImporter::start(
    const SinglePassImportRequest& request)
{
    currentOutputFilePath_ = request.outputFilePath;
    currentPostProcess_ = request.postProcess;

    process_.start(
        request.executablePath,
        request.arguments);
}

void SinglePassImporter::onProcessFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
//    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
//        handleNormalProcessExit();
//        emit importFinished(currentOutputFilePath_);
//    } else {
//        emit importFailed(exitCode, exitStatus);
//    }
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        if (currentPostProcess_)
            currentPostProcess_();

        handleNormalProcessExit();
        emit importFinished(currentOutputFilePath_);
        return;
    }

    emit importFailed(exitCode, exitStatus);
}
