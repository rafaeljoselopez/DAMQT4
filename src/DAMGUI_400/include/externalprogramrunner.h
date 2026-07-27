#ifndef EXTERNALPROGRAMRUNNER_H
#define EXTERNALPROGRAMRUNNER_H

#include <QObject>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>


class ExternalProgramRunner : public QObject
{
    Q_OBJECT

public:
    struct RunRequest {
        bool isWindows = false;
        bool runMpi = false;
        QString outputPrefix;
        QString rootName;
        QString stdInput;
        QString stdOutput;
        QString suffix;
        QString subdir;
        int nprocs = 1;
    };

    explicit ExternalProgramRunner(QObject *parent = nullptr);

    void inputdatafile(const QString &suffix,
                       const QString &section,
                       const QString &fullFileName,
                       const QString &projectDir,
                       const QString &projectName);

    void setProjectFolder(const QString &projectFolder);
    void setMpiCommand(const QString &mpiCommand);
    void setMpiFlags(const QString &mpiFlags);

    bool start(const RunRequest &request);
    void stop();

    bool isRunning() const;

    QByteArray ReadSectionOptions(const QString &sectionName, QFile *fileName);

signals:
    void started();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void errorOccurred(QProcess::ProcessError error);
    void messageChanged(const QString &message);

private:
    QString buildOutputFileName(const RunRequest &request) const;
    QString getExecutableName(const QString &processName,
                              const QString &subdir) const;

    void killMpiProcessesIfNeeded();

private:
    QProcess *process_ = nullptr;

    QString projectFolder_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString currentProgramName_;
    
    bool processKilled_ = false;
};

#endif
