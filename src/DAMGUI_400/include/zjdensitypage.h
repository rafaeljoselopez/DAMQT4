#ifndef ZERNIKEJACOBIDENSITYPAGE_H
#define ZERNIKEJACOBIDENSITYPAGE_H

#include <QWidget>
#include <QValidator>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QSignalBlocker>
#include <QButtonGroup>
#include <QToolButton>
#include <QFileDialog>

#include "Sheet.h"
#include <functional>
#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "igridoptionspage.h"
#include "ixyztabulationpage.h"
#include "readwriteoptions.h"

#define MAX_LEXP 25
#define MAX_LEXPZJ 22
#define MAX_KEXPZJ 40

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class Sheet;

class ZJDensityPage : public QWidget, public IGridOptionsPage, public IXYZTabulationPage, public IExecutablePage
{
    Q_OBJECT

public:
    using DeltaCalculator = std::function<double(const char*, double, double)>;
    explicit ZJDensityPage(bool mpiAvailable, QWidget* parent = nullptr);

    // ---- basic API used from MainWindow ----

    int kMax() const;
    int lMax() const;
    int lMaxTop() const;
    int lMin() const;
    int mpiProcessors() const;
    int planecase() const;
    int pointsX() const;
    int pointsY() const;
    int pointsZ() const;
    int pointsUV() const;
    int xyzTableRows() const;

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
    bool isGrid() const;
    bool isHighResolution() const;
    bool isInputOnly() const;
    bool isJacobi() const;
    bool isLowResolution() const;
    bool isMediumResolution() const;
    bool isMpiChecked() const;
    bool isMpiEnabled() const;
    bool isPlane2D() const;
    bool isPlaneOther() const;
    bool isPlaneXY() const;
    bool isPlaneXZ() const;
    bool isPlaneYZ() const;
    bool isXYZTabulation() const;
    bool isZernike() const;

    ExternalProgramRunner *runner() const;

    void clearSheet();
    void execDamDenZJ();
    void insertRow(int i);
    void loadDefault();
    void readFromFile(const std::string& file);
    void readXYZTable(const std::string& file, const char* section);
    void resizeSheet(int i);
    void setCellValue(const QString& value, int i, int j);
    void setChooseAllIndices(bool checked);
    void setChooseKIndices(bool checked);
    void setChooseLIndices(bool checked);
    void setChooseLKIndices(bool checked);
    void setChooseLKMIndices(bool checked);
    void setCustomResolution(bool checked);
    void setCustomResolutionEnabled(bool enabled);
    void setDeltaU(const double value);
    void setDeltaV(const double value);
    void setDeltaX(const double value);
    void setDeltaY(const double value);
    void setDeltaZ(const double value);
    void setDensityExpansionEnabled(bool enabled);
    void setDerivativesEnabled(bool enabled);
    void setEquationX(const QString& value);
    void setEquationY(const QString& value);
    void setEquationZ(const QString& value);
    void setExecEnabled(bool enabled);
    void setGradient(bool checked);
    void setGrid(bool checked);
    void setGrid2D(bool checked);
    void setGrid2DVisible(bool visible);
    void setGrid3D(bool checked);
    void setGrid3DVisible(bool visible);
    void setGridButtonsEnabled(bool enabled);
    void setGridEnabled(bool enabled);
    void setGridTypeVisible(bool visible);
    void setHighResolution(bool checked);
    void setHighResolutionEnabled(bool enabled);
    void setHighResolutionTip(const QString& value);
    void setImport(const QString& value);
    void setIsWindows(bool windows);
    void setKmax(int kmax);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setLmin(int lmin);
    void setLminEnabled(bool enabled);
    void setLminTop(int ltop);
    void setLRangeDensity(bool checked);
    void setLowResolution(bool checked);
    void setLowResolutionEnabled(bool enabled);
    void setLowResolutionTip(const QString& value);
    void setMediumResolution(bool checked);
    void setMediumResolutionEnabled(bool enabled);
    void setMediumResolutionTip(const QString& value);
    void setMolecularFragments(bool checked);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiVisible(bool visible);
    void setMpiSettings(const QString &mpiCommand,
                        const QString &mpiFlags);
    void setOutputGroupEnabled(bool enabled);
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
    void setResolutionVisible(bool visible);
    void setResolution2DVisible(bool visible);
    void setResolution3DVisible(bool visible);
    void setStopEnabled(bool enabled);
    void setUmax(const QString& value);
    void setUmin(const QString& value);
    void setVmax(const QString& value);
    void setVmin(const QString& value);
    void setXmax(const QString& value);
    void setXmin(const QString& value);
    void setXYZTabulation(bool checked);
    void setXYZEnabled(bool visible);
    void setXYZVisible(bool visible);
    void setYmax(const QString& value);
    void setYmin(const QString& value);
    void setZmax(const QString& value);
    void setZmin(const QString& value);
    void stopCurrentProcess();
    void updatePlaneSelectionFromCoefficients();
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool* printwarns,
//                     QString* warns,
//                     const DeltaCalculator& deltaCalculator);
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);

    QString equationX() const;
    QString equationY() const;
    QString equationZ() const;
    QString fragmentAtomsText() const;
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
    QString numtabular() const;
    QString tabularkey() const;

    QVector3D getPlaneWu() const;
    QVector3D getPlaneWv() const;


signals: 
    void execRequested();
    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void openOutputRequested();
    void outputTextReady(const QString &text);
    void statusMessageRequested(const QString &message);

    void showInputFileRequested(const QString &text);

private:
    void buildUi();
    void connectSignals();
    void dataOriginChanged();
    void gradientStateChanged(int state);
    void gridDimensionChanged(int id);
    void gridStateChanged(int state);
    void handleNormalProcessExit();
    void importZJDensity();
    void importZJDensityChanged();
    void indicesChanged(int id);
    void inputOnlyStateChanged(int state);
    void hideIndicesEditors();
    void lMaxChanged();
    void lMinChanged();
    void mpiStateChanged(int state);
    void plane2DChanged(int id);
    void planeModeChanged(int id);
    void resolutionModeChanged(int id);

    void xyzTabulationStateChanged(int state);
    
    enum Indices {
        ALLINDICES = 0,
        LINDICES = 1,
        KINDICES = 2,
        LKINDICES = 3,
        LKMINDICES = 4
    };

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
        
    // ---- Import data ----
    QGroupBox* importDataGroup_ = nullptr;
    QLineEdit* importDataEdit_ = nullptr;
    QToolButton* importDataButton_ = nullptr;

    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;

    // ---- Density expansion terms ----
    QGroupBox* densityExpansionGroup_ = nullptr;
    
    QGroupBox* lkRangesGroup_ = nullptr;
    QLabel* kMaxLabel_ = nullptr;
    QSpinBox* kMaxSpin_ = nullptr;
    QLabel* lMaxLabel_ = nullptr;
    QSpinBox* lMaxSpin_ = nullptr;
    QLabel* lMinLabel_ = nullptr;
    QSpinBox* lMinSpin_ = nullptr;
    
    QRadioButton* allFunctionsRadio_ = nullptr;
    
    // ---- K indices selection ----
    QRadioButton* chooseKIndicesRadio_ = nullptr;
    QLabel* chooseKIndicesLabel_ = nullptr;
    QRegExpValidator* chooseKIndicesValidator_ = nullptr;
    
    // ---- L indices selection ----
    QRadioButton* chooseLIndicesRadio_ = nullptr;
    QLabel* chooseLIndicesLabel_ = nullptr;
    QRegExpValidator* chooseLIndicesValidator_ = nullptr;
    
    // ---- (L,K) indices selection ----
    QRadioButton* chooseLKIndicesRadio_ = nullptr;
    QLabel* chooseLKIndicesLabel_ = nullptr;
    QRegExpValidator* chooseLKIndicesValidator_ = nullptr;
    
    // ---- (L,K,M) indices selection ----
    QRadioButton* chooseLKMIndicesRadio_ = nullptr;
    QLabel* chooseLKMIndicesLabel_ = nullptr;
    QRegExpValidator* chooseLKMIndicesValidator_ = nullptr;
    
    QLineEdit* chooseIndicesEdit_ = nullptr;
    QStringList* chooseIndicesList_ = nullptr;

    QButtonGroup* chooseIndicesGroup_ = nullptr;

    // ---- Derivatives ----
    QGroupBox* derivativesGroup_ = nullptr;
    QCheckBox* gradientCheck_ = nullptr;

    // ---- Grid ----
    QGroupBox* gridGroup_ = nullptr;
    QCheckBox* gridCheck_ = nullptr;

    QGroupBox* gridTypeGroup_ = nullptr;
    QRadioButton* grid2DRadio_ = nullptr;
    QRadioButton* grid3DRadio_ = nullptr;

    QButtonGroup* gridDimensionGroup_ = nullptr;

    // ---- 3D limits ----
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

    // ---- 2D plane / box ----
    QGroupBox* grid2DGroup_ = nullptr;
    QLabel* inf2DLabel_ = nullptr;
    QLabel* sup2DLabel_ = nullptr;
    QRadioButton* plane2DRadio_ = nullptr;
    QRadioButton* parametricSurfaceRadio_ = nullptr;

    QButtonGroup* surface2DGroup_ = nullptr;

    // ---- Surface Type ----
    QGroupBox* surfaceTypeGroup_ = nullptr;

    // ---- 2D Planes ----
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

    // ---- Parametric equations ----
    QGroupBox* parametricEquationsGroup_ = nullptr;
    QLabel* equationsLabel_ = nullptr;
    QLabel* equationXLabel_ = nullptr;
    QLabel* equationYLabel_ = nullptr;
    QLabel* equationZLabel_ = nullptr;
    QLineEdit* equationXEdit_ = nullptr;
    QLineEdit* equationYEdit_ = nullptr;
    QLineEdit* equationZEdit_ = nullptr;

    // ---- Resolution ----
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

    // ---- XYZ tabulation ----
    QGroupBox* xyzGroup_ = nullptr;
    QCheckBox* xyzCheck_ = nullptr;
    QWidget* xyzTable_ = nullptr;
    Sheet* xyzSheet_ = nullptr;

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
