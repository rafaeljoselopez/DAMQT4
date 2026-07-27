#ifndef CALCULATIONIMPORTER_H
#define CALCULATIONIMPORTER_H

#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

struct ImportRequest
{
    QString importFilePath;
    QString projectFolder;
    QString projectName;
    QString executablePath;

    bool isWindows = false;
};

struct ImportResult
{
    bool success = false;
    QString errorMessage;

    QStringList generatedFiles;

    bool atomicDensitiesAvailable = false;
    bool postDamFilesAvailable = false;
    bool orbitalsAvailable = false;
};

class CalculationImporter : public QObject
{
    Q_OBJECT

public:
    explicit CalculationImporter(QObject* parent = nullptr);
    ~CalculationImporter() override;

    virtual bool supportsFile(const QFileInfo& fileInfo) const = 0;
    virtual void start(const ImportRequest& request) = 0;

    void stop();
    bool isRunning() const;

signals:
    void started();
    void messageChanged(const QString& message);
    void finished(const ImportResult& result);
    void errorOccurred(const QString& message);

protected:
    QProcess process_;
};

#endif
