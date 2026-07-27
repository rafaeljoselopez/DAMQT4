#include "calculationimporter.h"

CalculationImporter::CalculationImporter(QObject* parent)
    : QObject(parent)
{
    connect(
        &process_,
        &QProcess::started,
        this,
        &CalculationImporter::started
    );

    connect(
        &process_,
        &QProcess::errorOccurred,
        this,
        [this](QProcess::ProcessError error) {
            QString message;

            switch (error) {
            case QProcess::FailedToStart:
                message = tr(
                    "The interface program could not be started."
                );
                break;

            case QProcess::Crashed:
                message = tr(
                    "The interface program crashed."
                );
                break;

            case QProcess::Timedout:
                message = tr(
                    "The interface program timed out."
                );
                break;

            case QProcess::WriteError:
                message = tr(
                    "An error occurred while writing to the interface program."
                );
                break;

            case QProcess::ReadError:
                message = tr(
                    "An error occurred while reading from the interface program."
                );
                break;

            case QProcess::UnknownError:
            default:
                message = tr(
                    "An unknown error occurred in the interface program."
                );
                break;
            }

            if (!process_.errorString().isEmpty()) {
                message += QStringLiteral("\n");
                message += process_.errorString();
            }

            emit errorOccurred(message);
        }
    );
}

CalculationImporter::~CalculationImporter()
{
    stop();
}

void CalculationImporter::stop()
{
    if (!isRunning())
        return;

    process_.terminate();

    if (!process_.waitForFinished(3000)) {
        process_.kill();
        process_.waitForFinished();
    }
}

bool CalculationImporter::isRunning() const
{
    return process_.state() != QProcess::NotRunning;
}
