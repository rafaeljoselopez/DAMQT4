#ifndef SINGLEPASSIMPORTER_H
#define SINGLEPASSIMPORTER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

struct SinglePassImportRequest
{
    QString executablePath;
    QStringList arguments;
    QString outputFilePath;

    std::function<void()> postProcess;
};

class SinglePassImporter : public QObject
{
    Q_OBJECT

public:
    explicit SinglePassImporter(QObject* parent = nullptr);

    void start(const SinglePassImportRequest& request);

signals:
    void processStarted();

    void importFinished(const QString& outputFilePath);

    void importFailed(
        int exitCode,
        QProcess::ExitStatus exitStatus);

    void outputFileError(
            const QString& outputFilePath,
            const QString& errorString);

    void outputTextReady(const QString &text);

private slots:
    void handleNormalProcessExit();
    void onProcessFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus);
private:
    QProcess process_;

    QString currentOutputFilePath_;

    std::function<void()> currentPostProcess_;
};

#endif
