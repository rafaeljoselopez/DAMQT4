#ifndef TOPOGRAPHYPAGE_H
#define TOPOGRAPHYPAGE_H

#define SIZE4   400
#define SIZE20 2000

#include <QWidget>
#include <QButtonGroup>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QLocale>

#include "Sheet.h"
#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "ixyztabulationpage.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class Sheet;

class TopographyPage : public QWidget, public IXYZTabulationPage, public IExecutablePage
{
    Q_OBJECT

public:
    explicit TopographyPage(bool mpiAvailable, QWidget* parent = nullptr);
    
    // ---- basic API used from MainWindow ----
    int lMax() const;
    int lMaxTop() const;
    int mpiProcessors() const;
    int xyzTableRows() const;
    
    bool isBuildBasin() const;
    bool isBuildGraph() const;
    bool isExtraConnections() const;
    bool isInputOnly() const;
    bool isGuessPoints() const;
    bool isMapCritical() const;
    bool isMED() const;
    bool isMESP() const;
    bool isMpiChecked() const;
    bool isTopoDensity() const;
    bool isXYZTabulation() const;
    
    void clearSheet();
    void execDamTopography();
    void insertRow(int i);
    void loadDefault();
    void readFromFile(const std::string& file);
    void readXYZTable(const std::string& file, const char* section);
    void resizeSheet(int i);
    void setBoxBasinSize(const QString& value);
    void setCellValue(const QString& value, int i, int j);
    void setAddPointsToTable(bool checked);
    void setAddPointsToTableVisible(bool visible);
    void setBuildBasinCheck(bool checked);
    void setBuildGraphCheck(bool checked);
    void setConnectionsThreshold(const QString& value);
    void setDensityRadioChecked(bool checked);
    void setExecEnabled(bool enabled);
    void setExtraConnectionsCheck(bool checked);
    void setGuessPoints(bool checked);
    void setInitialStep(const QString& value);
    void setIsWindows(bool windows);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setMapCriticalCheck(bool checked);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiVisible(bool visible);
    void setMpiSettings(const QString &mpiCommand,
                            const QString &mpiFlags);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setPotentialRadioChecked(bool checked);
    void setProjectData(const QString &projectFolder,
                                const QString &projectName);
    void setProjectFolder(const QString& value);
    void setStopEnabled(bool enabled);
    void setXYZEnabled(bool visible);
    void setXYZVisible(bool visible);
    void stopCurrentProcess();
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);

    QString getCellValue(int i, int j) const;
    QString guessFile() const;
    QString outputPrefix() const;
    QString numtabular() const;
    QString tabularkey() const;
    
signals:
    void execRequested();
    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void openOutputRequested();
    void outputTextReady(const QString &text);
    void statusMessageRequested(const QString &message);
    void stopRequested();
    void showInputFileRequested(const QString &text);
    
private:
    void buildUi();
    void connectSignals();
    void extraConnectionsChanged(int state);
    void handleNormalProcessExit();
    void importFile();
    void inputOnlyStateChanged(int state);
    void mpiStateChanged(int state);

    void topoAddGuessChanged(int state);
    void topoBasinChanged(int state);
    void topoMolGraphChanged(int state);
    void topoTypeChanged();
    void topoXYZChanged(int state);

    QDoubleValidator* myDoubleValidator_;

    bool mpiAvailable_ = false;
    bool iswindows_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;
    
    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;
    
    // ---- Topography type ----
    QGroupBox* topoTypeGroup_ = nullptr;
    
    QButtonGroup* topoButtonsGroup_ = nullptr;
    QRadioButton* densityRadio_;
    QRadioButton* potentialRadio_;
    
    QLabel* lMaxLabel_ = nullptr;
    QSpinBox* lMaxSpin_ = nullptr;
    
    // ---- Topography mapping ----
    QGroupBox* mappingGroup_ = nullptr;
    
    QCheckBox* mapCriticalCheck_ = nullptr;
    QLabel* boxMarginsSizeLabel_ = nullptr;
    QLineEdit* boxMarginsSizeEdit_ = nullptr;
    QLabel* convergenceThresholdLabel_ = nullptr;
    QLineEdit* convergenceThresholdEdit_ = nullptr;
    
    // ---- Guess points ----
    QGroupBox* guessPointsGroup_ = nullptr;
    QCheckBox* guessPointsCheck_ = nullptr;
    QGroupBox* guessPointsToTableGroup_ = nullptr;
    QCheckBox* addPointsToTableCheck_ = nullptr;
    
    // ---- XYZ tabulation ----
    QGroupBox* xyzGroup_ = nullptr;
    QCheckBox* xyzCheck_ = nullptr;
    QWidget* xyzTable_ = nullptr;
    Sheet* xyzSheet_ = nullptr;
    
    // ---- Guess points from file ----
    QGroupBox* guessFromFileGroup_ = nullptr;
    QLineEdit* guessFileEdit_ = nullptr;
    QToolButton* guessFileButton_ = nullptr;
    
    QLabel* boxSizeLabel_ = nullptr;
    QLineEdit* boxSizeEdit_ = nullptr;
    QLabel* stepSizeLabel_ = nullptr;
    QLineEdit* stepSizeEdit_ = nullptr;
    
    // ---- Molecular graph ----
    QGroupBox* buildGraphGroup_ = nullptr;
    QCheckBox* buildGraphCheck_ = nullptr;
    QLabel* boxGraphSizeLabel_ = nullptr;
    QLineEdit* boxGraphSizeEdit_ = nullptr;
    QLabel* convergenceGradLabel_ = nullptr;
    QLineEdit* convergenceGradEdit_ = nullptr;
    
    // ---- Atomic basins ----
    QGroupBox* buildBasinGroup_ = nullptr;
    QCheckBox* buildBasinCheck_ = nullptr;
    QLabel* boxBasinSizeLabel_ = nullptr;
    QLineEdit* boxBasinSizeEdit_ = nullptr;
    QCheckBox* extraConnectionsCheck_ = nullptr;
    QGroupBox* extraConnectionsGroup_ = nullptr;
    QLabel* connectionsThresholdLabel_ = nullptr;
    QLineEdit* connectionsThresholdEdit_ = nullptr;
    QLabel* initialStepLabel_ = nullptr;
    QLineEdit* initialStepEdit_ = nullptr;
    
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
