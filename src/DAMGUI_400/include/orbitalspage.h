#ifndef ORBITALSPAGE_H
#define ORBITALSPAGE_H

#include <QWidget>
#include <QValidator>
#include <QFileDialog>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QSignalBlocker>
#include <QButtonGroup>
#include <QToolButton>

#include <functional>
#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "igridoptionspage.h"
#include "readwriteoptions.h"
#include <cmath>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class OrbitalsPage : public QWidget, public IGridOptionsPage, public IExecutablePage
{
    Q_OBJECT

public:
#include <functional>
    using DeltaCalculator = std::function<double(const char*, double, double)>;

    explicit OrbitalsPage(bool mpiAvailable, QWidget* parent = nullptr);

    // ---- basic API used from MainWindow ----

    int getPlaneCase() const;
    int mpiProcessors() const;
    int pointsX() const;
    int pointsY() const;
    int pointsZ() const;
    int pointsUV() const;

    double deltaU() const;
    double deltaV() const;
    double deltaX() const;
    double deltaY() const;
    double deltaZ() const;

    double computeDelta(const char* key, double ini, double fin) const;

    bool is2DGrid() const;
    bool is3DGrid() const;
    bool isCustomResolution() const;
    bool isGradient() const;
    bool isHighResolution() const;
    bool isInputOnly() const;
    bool isLowResolution() const;
    bool isMediumResolution() const;
    bool isMpiChecked() const;
    bool isMpiEnabled() const;
    bool isPlane2D() const;
    bool isPlaneOther() const;
    bool isPlaneXY() const;
    bool isPlaneXZ() const;
    bool isPlaneYZ() const;

    ExternalProgramRunner *runner() const;

    void execDamOrb();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setCustomResolution(bool checked);
    void setCustomResolutionEnabled(bool enabled);
    void setDerivativesOptionsVisible(bool visible);
    void setDeltaU(const double value);
    void setDeltaV(const double value);
    void setDeltaX(const double value);
    void setDeltaY(const double value);
    void setDeltaZ(const double value);
    void setEquationX(const QString& value);
    void setEquationY(const QString& value);
    void setEquationZ(const QString& value);
    void setExecEnabled(bool enabled);
    void setGradient(bool checked);
    void setGrid2D(bool checked);
    void setGrid2DVisible(bool visible);
    void setGrid3D(bool checked);
    void setGrid3DVisible(bool visible);
    void setHighResolution(bool checked);
    void setHighResolutionEnabled(bool enabled);
    void setHighResolutionTip(const QString& value);
    void setImport(const QString& value);
    void setIsWindows(bool windows);
    void setLowResolution(bool checked);
    void setLowResolutionEnabled(bool enabled);
    void setLowResolutionTip(const QString& value);
    void setMediumResolution(bool checked);
    void setMediumResolutionEnabled(bool enabled);
    void setMediumResolutionTip(const QString& value);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiVisible(bool visible);
    void setMpiSettings(const QString &mpiCommand,
                            const QString &mpiFlags);
    void setOutputButtonEnabled(bool enabled);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setParametricEquationsVisible(bool visible);
    void setPlanes2DVisible(bool visible);
    void setPlane2D(bool checked);
    void setPlaneA(const QString& value);
    void setPlaneB(const QString& value);
    void setPlaneC(const QString& value);
    void setPlaneCoefficientsVisible(bool visible);
    void setPlaneOther(bool checked);
    void setPlaneXY(bool checked);
    void setPlaneXZ(bool checked);
    void setPlaneYZ(bool checked);
    void setPointsUV(int number);
    void setPointsX(int number);
    void setPointsY(int number);
    void setPointsZ(int number);
    void setProjectData(const QString &projectFolder,
                                const QString &projectName);
    void setProjectFolder(const QString& value);
    void setResolution2DVisible(bool visible);
    void setResolution3DVisible(bool visible);
    void setSecondDerivatives(bool checked);
    void setStopEnabled(bool enabled);
    void setUmax(const QString& value);
    void setUmin(const QString& value);
//    void setUVRange(bool checked);
    void setVmax(const QString& value);
    void setVmin(const QString& value);
    void setXmax(const QString& value);
    void setXmin(const QString& value);
//    void setXYZRange(bool checked);
    void setYmax(const QString& value);
    void setYmin(const QString& value);
    void setZmax(const QString& value);
    void setZmin(const QString& value);
    void stopCurrentProcess();
    void updatePlaneSelectionFromCoefficients();
    void writeToFile(const std::string& file,
                         bool* printwarns,
                         QString* warns);
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool* printwarns,
//                     QString* warns,
//                     const DeltaCalculator& deltaCalculator);

    QString equationX() const;
    QString equationY() const;
    QString equationZ() const;
    QString getCellValue(int i, int j) const;
    QString importFile() const;
    QString outputPrefix() const;
    QString planeA() const;
    QString planeB() const;
    QString planeC() const;
    QString uMaxValue() const;
    QString uMinValue() const;
    QString vMaxValue() const;
    QString vMinValue() const;
    QString xMaxValue() const;
    QString xMinValue() const;
    QString yMaxValue() const;
    QString yMinValue() const;
    QString zMaxValue() const;
    QString zMinValue() const;

    QVector3D getPlaneWu() const;
    QVector3D getPlaneWv() const;

signals:

    void execRequested();
    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void openOutputRequested();
    void outputTextReady(const QString &text);
    void showInputFileRequested(const QString &text);
    void statusMessageRequested(const QString &message);
    void stopRequested();

private:   
    void buildUi();
    void connectSignals();
    void dataOriginChanged();
    void gradientStateChanged(int state);
    void gridDimensionChanged(int id);
    void handleNormalProcessExit();
    void importFileNameMO();
    void inputOnlyStateChanged(int state);
    void molecularOrbitalsChanged();
    void mpiStateChanged(int state);
    void plane2DChanged(int id);
    void planeModeChanged(int id);
    void resolutionModeChanged(int id);

    enum GridDimension {
        DIM_2D = 0,
        DIM_3D = 1
    };

    enum planes {
        planeXY = 0,
        planeXZ = 1,
        planeYZ = 2,
        planeOther = 3
    };

    enum Gridresolution {
        LOW_RES = 0,
        MEDIUM_RES = 1,
        HIGH_RES = 2,
        CUSTOM_RES = 3
    };

    bool activebeware_ = true;
    bool mpiAvailable_ = false;
    bool iswindows_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;

    double deltaU_;
    double deltaV_;
    double deltaX_;
    double deltaY_;
    double deltaZ_;

    PlaneResult planeResult;

    // ---- Import data file ----
    QGroupBox* importGroup_ = nullptr;
    QLabel* importLabel_ = nullptr;
    QLineEdit* importEdit_ = nullptr;
    QToolButton* importBrowseButton_ = nullptr;

    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;

    // ---- Molecular orbitals selection ----
    QGroupBox* molecularOrbitalsGroup_ = nullptr;
    QLabel* molecularOrbitalsLabel_ = nullptr;
    QLineEdit* molecularOrbitalsEdit_ = nullptr;
    QRegExpValidator* molecularOrbitalsValidator_ = nullptr;
    QStringList* molecularOrbitalsList_ = nullptr;

    // ---- Derivatives ----
    QGroupBox* derivativesGroup_ = nullptr;
    QCheckBox* gradientCheck_ = nullptr;
    QCheckBox* secondDerivativesCheck_ = nullptr;

    // ---- Grid ----
//    QGroupBox* gridGroup_ = nullptr;
//    QCheckBox* gridCheck_ = nullptr;

    QGroupBox* gridTypeGroup_ = nullptr;
    QRadioButton* grid2DRadio_ = nullptr;
    QRadioButton* grid3DRadio_ = nullptr;

    QButtonGroup* gridDimensionGroup_ = nullptr;

    // 3D limits
    QGroupBox* grid3DGroup_ = nullptr;

    QGroupBox* box3DGroup_ = nullptr;
    QLabel* inf3DLabel_ = nullptr;
    QLabel* sup3DLabel_ = nullptr;
    QLabel* xLabel_ = nullptr;
    QLabel* yLabel_ = nullptr;
    QLabel* zLabel_ = nullptr;
    QLineEdit* xMinEdit_ = nullptr;
    QLineEdit* xMaxEdit_ = nullptr;
    QLineEdit* yMinEdit_ = nullptr;
    QLineEdit* yMaxEdit_ = nullptr;
    QLineEdit* zMinEdit_ = nullptr;
    QLineEdit* zMaxEdit_ = nullptr;

    QDoubleValidator* myDoubleValidator_;

    // 2D plane / box
    QGroupBox* grid2DGroup_ = nullptr;
    QLabel* inf2DLabel_ = nullptr;
    QLabel* sup2DLabel_ = nullptr;
    QRadioButton* plane2DRadio_ = nullptr;
    QRadioButton* parametricSurfaceRadio_ = nullptr;

    QButtonGroup* surface2DGroup_ = nullptr;

    // Surface Type
    QGroupBox* surfaceTypeGroup_ = nullptr;

    // 2D Planes
    QGroupBox* planes2DGroup_ = nullptr;
    QRadioButton* planeXYRadio_ = nullptr;
    QRadioButton* planeXZRadio_ = nullptr;
    QRadioButton* planeYZRadio_ = nullptr;
    QRadioButton* planeOtherRadio_ = nullptr;

    QButtonGroup* planesButtonsGroup_ = nullptr;

    QGroupBox* planeCoefficientsGroup_ = nullptr;
    QLabel* planeCoefficients_ = nullptr;
    QLineEdit* planeAEdit_ = nullptr;
    QLineEdit* planeBEdit_ = nullptr;
    QLineEdit* planeCEdit_ = nullptr;

    QGroupBox* box2DGroup_ = nullptr;
    QLabel* uLabel_ = nullptr;
    QLabel* vLabel_ = nullptr;
    QLineEdit* uMinEdit_ = nullptr;
    QLineEdit* uMaxEdit_ = nullptr;
    QLineEdit* vMinEdit_ = nullptr;
    QLineEdit* vMaxEdit_ = nullptr;

    // Parametric equations
    QGroupBox* parametricEquationsGroup_ = nullptr;
    QLabel* equationsLabel_ = nullptr;
    QLabel* equationXLabel_ = nullptr;
    QLabel* equationYLabel_ = nullptr;
    QLabel* equationZLabel_ = nullptr;
    QLineEdit* equationXEdit_ = nullptr;
    QLineEdit* equationYEdit_ = nullptr;
    QLineEdit* equationZEdit_ = nullptr;

    // Resolution
    QGroupBox* resolutionGroup_ = nullptr;
    QRadioButton* lowResolutionRadio_ = nullptr;
    QRadioButton* mediumResolutionRadio_ = nullptr;
    QRadioButton* highResolutionRadio_ = nullptr;
    QRadioButton* customResolutionRadio_ = nullptr;

    QButtonGroup* resolutionButtonsGroup_ = nullptr;

    QGroupBox* resolution2DGroup_ = nullptr;
    QLabel* pointsUVLabel_ = nullptr;
    QSpinBox* pointsUVSpin_ = nullptr;

    QGroupBox* resolution3DGroup_ = nullptr;
    QLabel* pointsXLabel_ = nullptr;
    QLabel* pointsYLabel_ = nullptr;
    QLabel* pointsZLabel_ = nullptr;
    QSpinBox* pointsXSpin_ = nullptr;
    QSpinBox* pointsYSpin_ = nullptr;
    QSpinBox* pointsZSpin_ = nullptr;


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
