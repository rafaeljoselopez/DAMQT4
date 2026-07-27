#ifndef MOLPROIMPORTER_H
#define MOLPROIMPORTER_H

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>

struct MolproImportRequest
{
    QString importFilePath;
    QString projectFolder;
    QString projectName;
    QString executablePath;
};

class MolproImporter : public QObject
{
    Q_OBJECT

public:
    explicit MolproImporter(QObject* parent = nullptr);

    void start(const MolproImportRequest& request);

signals:
    void importFinished();

    void outputFileError(
            const QString& outputFilePath,
            const QString& errorString);

    void outputTextReady(const QString &text);

private slots:
    void handleNormalProcessExit();
    void onProcessFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus
    );

private:
    void runInterface(int entryIndex);
    bool analyzeFirstPass();
    bool selectEntry();

    MolproImportRequest currentRequest_;

    QProcess process_;

    QStringList availableEntries_;

    int currentEntryIndex_ = 0;
    int selectedEntryIndex_ = 0;
};

#endif // MOLPROIMPORTER_H
