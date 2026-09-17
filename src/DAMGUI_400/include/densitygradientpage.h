#ifndef DENSITYGRADIENTPAGE_H
#define DENSITYGRADIENTPAGE_H

#include <QWidget>
#include <QValidator>
#include <QIntValidator>
#include <QButtonGroup>
#include <QFileDialog>

#include "Sheet.h"
#include <functional>
#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "igridoptionspage.h"
#include "ixyztabulationpage.h"
#include "readwriteoptions.h"
#include <cmath>

#define MAX_LEXP 25

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class Sheet;

class DensityGradientPage : public QWidget, public ILinesTabulationPage, public IExecutablePage
{
    Q_OBJECT

public:
#include <functional>

    explicit DensityGradientPage(bool mpiAvailable, QWidget* parent = nullptr);

    // ---- basic API used from MainWindow ----
    int linesUVTableRows() const;
    int linesXYZTableRows() const;
    int lMax() const;
    int lMaxTop() const;
    int longRangeThreshold() const;
    int mpiProcessors() const;
    int planecase() const;

    bool isExtraLines() const;
    bool isInputOnly() const;
    bool is2DPlot() const;
    bool is3DPlot() const;
    bool isPlane2D() const;
    bool isPlaneOther() const;
    bool isPlaneXY() const;
    bool isPlaneXZ() const;
    bool isPlaneYZ() const;
    bool isTabulation() const;
    bool isMpiChecked() const;
    bool isMpiEnabled() const;

    void clearUVSheet();
    void clearXYZSheet();
    void execDamDenGrad();
    void insertUVRow(int i);
    void insertXYZRow(int i);
    void loadDefault();
    void readFromFile(const std::string& file);
    void readUVLinesTable(const std::string& file, const char* section);
    void resizeUVSheet(int i);
    void readXYZLinesTable(const std::string& file, const char* section);
    void resizeXYZSheet(int i);
    void setCustomResolutionEnabled(bool enabled);
    void setExecEnabled(bool enabled);
    void setExtraLines(bool checked);
    void setImportLinesFile(const QString& value);
    void setIsValence(bool valence);
    void setIsWindows(bool windows);
    void setLinesPerNucleus(const QString& value);
    void setLinesPoints(const QString& value);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setLmaxVisible(bool visible);
    void setLongRangeMinThreshold(int number);
    void setLongRangeThreshold(int number);
    void setLongRangeTopThreshold(int number);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiSettings(const QString &mpiCommand,
                        const QString &mpiFlags);
    void setMpiVisible(bool visible);
    void setOutputButtonEnabled(bool enabled);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
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
    void setPlot(bool checked);
    void setPlot2D(bool checked);
    void setPlot2DVisible(bool visible);
    void setPlot3D(bool checked);
    void setPlot3DVisible(bool visible);
    void setPlotTypeVisible(bool visible);
    void setProjectData(const QString &projectFolder,
                            const QString &projectName);
    // void setProjectFolder(const QString& value);
    void setStopEnabled(bool enabled);
    void setStride(const QString& value);
    void setTabulation(bool checked);
    void setUmax(const QString& value);
    void setUmin(const QString& value);
    void setUVCellValue(const QString& value, int i, int j);
    void setUVEnabled(bool visible);
    void setUVTableEnabled(bool visible);
    void setUVTableVisible(bool visible);
    void setVmax(const QString& value);
    void setVmin(const QString& value);
    void setXYZCellValue(const QString& value, int i, int j);
    void setXmax(const QString& value);
    void setXmin(const QString& value);   
    void setXYZTableEnabled(bool visible);
    void setXYZTableVisible(bool visible);
    void setYmax(const QString& value);
    void setYmin(const QString& value);
    void setZmax(const QString& value);
    void setZmin(const QString& value);
    void stopCurrentProcess();
    void updatePlaneSelectionFromCoefficients();
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);

    QString getUVCellValue(int i, int j) const;
    QString getXYZCellValue(int i, int j) const;
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
    void dataOriginChanged();
    void extraLinesChanged(int state);
    void handleNormalProcessExit();
    void importGradientLines();
    void plotDimensionChanged(int id);
    void plotStateChanged(int state);
    void inputOnlyStateChanged(int state);
    void lMaxChanged(int value);
    void mpiStateChanged(int state);
    void plane2DChanged(int id);
    void planeModeChanged(int id);
    void resolutionModeChanged(int id);
    void tabulationStateChanged(int state);

    enum PlotDimension {
        DIM_2D = 0,
        DIM_3D = 1
    };

    enum planes {
        planeXY = 0,
        planeXZ = 1,
        planeYZ = 2,
        planeOther = 3
    };

    bool activebeware_ = true;
    bool mpiAvailable_ = false;
    bool iswindows_ = false;
    bool lvalence_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;

    PlaneResult planeResult;

    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;

    // ---- Long range ----
    QGroupBox* longRangeGroup_ = nullptr;

    QGroupBox* lmaxGroup_ = nullptr;
    QSpinBox* lMaxSpin_ = nullptr;
    QLabel* longRangeLabel_ = nullptr;
    QSpinBox* longRangeSpin_ = nullptr;
    
    // ---- Lines ----
    QGroupBox* linesGroup_ = nullptr;
    QLabel* linesPointsLabel_ = nullptr;
    QLineEdit* linesPointsEdit_ = nullptr;
    QLabel* strideLabel_ = nullptr;
    QLineEdit* strideEdit_ = nullptr;

    // ---- Plot ----

    QGroupBox* plotTypeGroup_ = nullptr;
    QRadioButton* plot2DRadio_ = nullptr;
    QRadioButton* plot3DRadio_ = nullptr;

    QButtonGroup* plotDimensionGroup_ = nullptr;

    QLabel* startDirectionsLabel_ = nullptr;
    QComboBox* startDirectionsCombo_ = nullptr;

    // 3D limits
    QGroupBox* plot3DGroup_ = nullptr;

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

    // 2D plot
    QGroupBox* plot2DGroup_ = nullptr;
    QLabel* inf2DLabel_ = nullptr;
    QLabel* sup2DLabel_ = nullptr;
    QRadioButton* plane2DRadio_ = nullptr;

    QLabel* linesPerNucleusLabel_ = nullptr;
    QLineEdit* linesPerNucleusEdit_ = nullptr;

    // 2D box
    QGroupBox* box2DGroup_ = nullptr;
    QLabel* uLabel_ = nullptr;
    QLabel* vLabel_ = nullptr;
    QLineEdit* uMinEdit_ = nullptr;
    QLineEdit* uMaxEdit_ = nullptr;
    QLineEdit* vMinEdit_ = nullptr;
    QLineEdit* vMaxEdit_ = nullptr;

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


    // Lines tabulation
    QCheckBox* extraLinesCheck_ = nullptr;
    QGroupBox* tableLinesGroup_ = nullptr;
    QCheckBox* addPointsCheck_ = nullptr;
    QWidget* linesUVTable_ = nullptr;
    Sheet* linesUVSheet_ = nullptr;
    QWidget* linesXYZTable_ = nullptr;
    Sheet* linesXYZSheet_ = nullptr;
    
    // Starting slopes ratio
    QLabel* slopesRatioLabel_ = nullptr;
    QLabel* uvRatioLabel_ = nullptr;
    QLineEdit* uvRatioEdit_ = nullptr;
    
    // ---- Import gradient lines ----
    QGroupBox* importLinesGroup_ = nullptr;
    QLineEdit* importLinesEdit_ = nullptr;
    QToolButton* importLinesButton_ = nullptr;

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
