#ifndef ATOMICDENSITIESPAGE_H
#define ATOMICDENSITIESPAGE_H

#include <QWidget>
#include "readwriteoptions.h"
#include "IniFile.h"
#include "externalprogramrunner.h"
#include "iexecutablepage.h"

class QCheckBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QSpinBox;

class AtomicDensitiesPage : public QWidget, public IExecutablePage
{
    Q_OBJECT

public:
    explicit AtomicDensitiesPage(bool mpiAvailable, QWidget* parent = nullptr);

    int  cutoffThreshold() const;
    int  fitThreshold() const;
    int  lmaxExpansion() const;
    int  lmaxDisplayed() const;
    int  mpiProcessors() const;
    int  read_natom(QString fileName);

    bool isInputOnly() const;
    bool isMpiChecked() const;
    bool isMpiEnabled() const;
    bool isOneCenterChecked() const;
    bool isTotalDensityChecked() const;
    bool isTwoCenterChecked() const;
    bool isWindows() const;

    ExternalProgramRunner *runner() const;

    void execDam();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setCutoffThreshold(int value);
    void setExecEnabled(bool enabled);
    void setFitThreshold(int value);
    void setIsWindows(bool windows);
    void setLmaxExpansion(int value);
    void setLmaxDisplayed(int value);
    void setlslater(bool value);
    void setlvalence(bool value);
    void setlzdo(bool value);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiVisible(bool visible);
    void setMpiSettings(const QString &mpiCommand,
                        const QString &mpiFlags);
    void set_natom(int);
    void setOneCenterChecked(bool checked);
    void setPageEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                            const QString &projectName);
    void setProjectFolder(const QString& value);
    void setStopEnabled(bool enabled);
    void setTopLmaxDisplayed(int value);
    void setTotalDensityChecked(bool checked);
    void setTwoCenterChecked(bool checked);
    void stopCurrentProcess();
    void writeToFile(const std::string& file,
                     bool isWindows,
                     bool* printwarns,
                     QString* warns);

signals:
    void execRequested();
    void lmaxExpansionChanged(int value);
    void openOutputRequested();
    void stopRequested(); 

    void errorOccurred(const QString& message);
    void execPagesEnabledChanged(bool enabled);
    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void outputTextReady(const QString &text);
    void statusMessageRequested(const QString &message);

    void showInputFileRequested(const QString &text);

private:
    enum class RunningOperation
    {
        None,
        Sgbs2Sxyz,
        Dam
    };

    bool finishSgbs2Sxyz();

    void buildUi();
    void connectSignals();
    void handleNormalProcessExit();
    void inputOnlyStateChanged(int state);

    void runSgbs2Sxyz(const QString& targetSxyzFile);
    void startSgbs2SxyzProcess();
    void startDam();

    void mpiStateChanged(int state);

    int natom_;

    bool iswindows_ = false;
    bool lslater_ = false;
    bool lvalence_ = false;
    bool lzdo_ = false;
    bool mpiAvailable_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    RunningOperation runningOperation_ = RunningOperation::None;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;


    QString sgbs2sxyzTargetFile_;
    QString sgbs2sxyzInputFile_;
    QString sgbs2sxyzErrorFile_;

    QGroupBox* lmaxExpansionGroup_ = nullptr;
    QSpinBox* lmaxExpansionSpin_ = nullptr;

    QGroupBox* lmaxDisplayedGroup_ = nullptr;
    QSpinBox* lmaxDisplayedSpin_ = nullptr;

    QGroupBox* fittingTypeGroup_ = nullptr;
    QRadioButton* totalDensityRadio_ = nullptr;
    QRadioButton* oneCenterRadio_ = nullptr;
    QRadioButton* twoCenterRadio_ = nullptr;

    QGroupBox* thresholdsGroup_ = nullptr;
    QLabel* fitThresholdLabel_ = nullptr;
    QLabel* cutoffThresholdLabel_ = nullptr;
    QSpinBox* fitThresholdSpin_ = nullptr;
    QSpinBox* cutoffThresholdSpin_ = nullptr;

    QGroupBox* inputOnlyGroup_ = nullptr;
    QCheckBox* inputOnlyCheck_ = nullptr;

    QGroupBox* mpiGroup_ = nullptr;
    QCheckBox* mpiCheck_ = nullptr;
    QLabel* mpiProcessorsLabel_ = nullptr;
    QSpinBox* mpiProcessorsSpin_ = nullptr;

    QPushButton* execButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* outputButton_ = nullptr;
};

#endif
