#ifndef SIGMAHOLEPAGE_H
#define SIGMAHOLEPAGE_H

#include <QWidget>
#include <QValidator>
#include <QFileDialog>
#include "externalprogramrunner.h"
#include "iexecutablepage.h"

class QButtonGroup;
class QToolButton;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class SigmaHolePage : public QWidget, public IExecutablePage
{
    Q_OBJECT

public:
    explicit SigmaHolePage(bool mpiAvailable, QWidget* parent = nullptr);

    int geometryThreshold() const;
    int localMaxThreshold() const;
    int localMinThreshold() const;
    int longRangeThreshold() const;
    int lMax() const;
    int lMaxTop() const;
    int mpiProcessors() const;
    
    bool isInputOnly() const;
    bool isExactPotential() const;
    bool isMpiChecked() const;
    bool isMpiEnabled() const;
    
    void execDamSGhole();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setDensityValue(const QString& value);
    void setExactPotential(bool checked);
    void setExactPotentialEnabled(bool enabled);
    void setExecEnabled(bool enabled);
    void setExtremaSeparation(const QString& value);
    void setGeometryThreshold(int value);
    void setImport(const QString& value);
    void setIsValence(bool valence);
    void setIsWindows(bool windows);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setLocalMaxThreshold(int value);
    void setLocalMinThreshold(int value);
    void setLongRangeThreshold(int value);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiSettings(const QString &mpiCommand,
                            const QString &mpiFlags);
    void setMpiVisible(bool visible);
    void setOptionsEnabled(bool enabled);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setPotentialExpansionEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                        const QString &projectName);
    void setProjectFolder(const QString& value);
    void setStopEnabled(bool enabled);
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool lvalence,
//                     bool* printwarns,
//                     QString* warns);
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);
    
    QString extremaSeparation() const;
    QString importFile() const;
    QString outputPrefix() const;

signals:
    void execRequested();
    void openOutputRequested();
//    void stopRequested();

    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void outputTextReady(const QString &text);
    void showInputFileRequested(const QString &text);
    void statusMessageRequested(const QString &message);
    
private:
    void buildUi();
    void connectSignals();
    void handleNormalProcessExit();
    void importSGdensity();
    void importSGholeDensityChanged();
    void inputOnlyStateChanged(int state);
    void mpiStateChanged(int state);
    
    bool mpiAvailable_ = false;
    bool lvalence_ = false;
    bool iswindows_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QDoubleValidator* myDoubleValidator_;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;
    
    // ---- Import density grid ----
    QGroupBox* importDensityGroup_ = nullptr;
    QLineEdit* importDensityEdit_ = nullptr;
    QToolButton* importDensityButton_ = nullptr;
    
    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;
    
    // ---- Thresholds and other options ----
    QGroupBox* optionsGroup_ = nullptr;
    
    QLabel* densityValueLabel_ = nullptr;
    QLineEdit* densityValueEdit_ = nullptr;
    
    QLabel* geometryThresholdLabel_ = nullptr;
    QSpinBox* geometryThresholdSpin_ = nullptr;
    
    QLabel* longRangeThresholdLabel_ = nullptr;
    QSpinBox* longRangeThresholdSpin_ = nullptr;
    
    QLabel* localMaxThresholdLabel_ = nullptr;
    QSpinBox* localMaxThresholdSpin_ = nullptr;
    
    QLabel* localMinThresholdLabel_ = nullptr;
    QSpinBox* localMinThresholdSpin_ = nullptr;
    
    QLabel* extremaSeparationLabel_ = nullptr;
    QLineEdit* extremaSeparationEdit_ = nullptr;
    
    // ---- Potential ----
    QCheckBox* exactPotentialCheck_ = nullptr;
    
    QGroupBox* potentialExpansionGroup_ = nullptr;
    QLabel* potentialExpansionLabel_ = nullptr;
    QSpinBox* potentialExpansionSpin_ = nullptr;
    
    // ---- Input only ----
    QGroupBox* inputOnlyGroup_ = nullptr;
    QCheckBox* inputOnlyCheck_ = nullptr;

    // ---- MPI ----
    QGroupBox* mpiGroup_ = nullptr;
    QCheckBox* mpiCheck_ = nullptr;
    QLabel* mpiProcessorsLabel_ = nullptr;
    QSpinBox* mpiProcessorsSpin_ = nullptr;

    // ---- Buttons ----
    QPushButton* execButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* outputButton_ = nullptr;
};


#endif
