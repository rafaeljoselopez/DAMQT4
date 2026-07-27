#include "potentialpage.h"

#include <QCheckBox>
#include <QDir>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDebug>

#include "elements.h"
#include "externalprogramrunner.h"
#include "readwriteoptions.h"
#include "IniFile.h"

#ifndef ANGSTROMTOBOHR
#define ANGSTROMTOBOHR 1.88971616463
#endif

#ifndef MAX_NUM_PROCESSORS
#define MAX_NUM_PROCESSORS 128
#endif

static QString quoteString(QString s)
{
    s.remove("\"");
    return "\"" + s + "\"";
}

PotentialPage::PotentialPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void PotentialPage::buildUi()
{
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Potential settings ----
    potentialSettingsGroup_ = new QGroupBox(this);

    exactPotentialCheck_ = new QCheckBox(tr("Exact potential"), potentialSettingsGroup_);

    lmaxGroup_ = new QGroupBox(tr("Highest l in expansion"), potentialSettingsGroup_);

    lMaxSpin_ = new QSpinBox(lmaxGroup_);
    lMaxSpin_->setRange(0, 25);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);

    auto* lmaxLayout = new QHBoxLayout(lmaxGroup_);
    lmaxLayout->addWidget(lMaxSpin_);

    longRangeGroup_ = new QGroupBox(tr("Long-range"), potentialSettingsGroup_);
    longRangeCheck_ = new QCheckBox(tr("Long-range only"), longRangeGroup_);
    longRangeLabel_ = new QLabel(tr("Long-range threshold: 10^"), longRangeGroup_);
    longRangeSpin_  = new QSpinBox(longRangeGroup_);
    longRangeSpin_->setRange(-10,0);
    longRangeSpin_->setValue(-7);
    longRangeSpin_->setMaximumWidth(60);
    setLongRangeEnabled(true);



    auto* longRangeLayout = new QGridLayout(longRangeGroup_);
    longRangeLayout->addWidget(longRangeCheck_,0,0,1,2);
    longRangeLayout->addWidget(longRangeLabel_,1,0);
    longRangeLayout->addWidget(longRangeSpin_,1,1);

    auto* potentialSettingsLayout = new QVBoxLayout(potentialSettingsGroup_);
    potentialSettingsLayout->addWidget(exactPotentialCheck_);
    potentialSettingsLayout->addWidget(lmaxGroup_);
    potentialSettingsLayout->addWidget(longRangeGroup_);


    // ---- Derivatives ----
    derivativesGroup_ = new QGroupBox(tr("Derivatives"), this);
    gradientCheck_ = new QCheckBox(tr("Gradient"), derivativesGroup_);
    secondDerivativesCheck_ = new QCheckBox(tr("Second derivatives"), derivativesGroup_);

    auto* derivativesLayout = new QVBoxLayout(derivativesGroup_);
    derivativesLayout->addWidget(gradientCheck_);
    derivativesLayout->addWidget(secondDerivativesCheck_);


    // ---- Grid ----
    gridGroup_ = new QGroupBox(tr("Grid"), this);
    gridCheck_ = new QCheckBox(tr("Generate grid"), gridGroup_);

    auto* gridGroupLayout = new QVBoxLayout(gridGroup_);
    gridGroupLayout->addWidget(gridCheck_);

    gridTypeGroup_ = new QGroupBox(tr("Grid type"), this);
    grid2DRadio_ = new QRadioButton(tr("2D"), gridTypeGroup_);
    grid3DRadio_ = new QRadioButton(tr("3D"), gridTypeGroup_);
    gridTypeGroup_->setVisible(true);
    grid2DRadio_->setChecked(false);
    grid3DRadio_->setChecked(true);

    gridDimensionGroup_ = new QButtonGroup(this);
    gridDimensionGroup_->addButton(grid2DRadio_, DIM_2D);
    gridDimensionGroup_->addButton(grid3DRadio_, DIM_3D);


    auto* gridTypeLayout = new QHBoxLayout(gridTypeGroup_);
    gridTypeLayout->addWidget(grid2DRadio_);
    gridTypeLayout->addWidget(grid3DRadio_);

    // 3D
    grid3DGroup_ = new QGroupBox(tr("3D grid"), this);
    setGrid3DVisible(true);

    box3DGroup_ = new QGroupBox(tr("3D boundaries"), grid3DGroup_);
    box3DGroup_->setEnabled(true);
    inf3DLabel_ = new QLabel(tr("Min"), box3DGroup_);
    sup3DLabel_ = new QLabel(tr("Max"), box3DGroup_);
    xLabel_ = new QLabel(tr("x"), box3DGroup_);
    yLabel_ = new QLabel(tr("y"), box3DGroup_);
    zLabel_ = new QLabel(tr("z"), box3DGroup_);

    xMinEdit_ = new QLineEdit(box3DGroup_);
    xMaxEdit_ = new QLineEdit(box3DGroup_);
    yMinEdit_ = new QLineEdit(box3DGroup_);
    yMaxEdit_ = new QLineEdit(box3DGroup_);
    zMinEdit_ = new QLineEdit(box3DGroup_);
    zMaxEdit_ = new QLineEdit(box3DGroup_);

    myDoubleValidator_ = new QDoubleValidator(nullptr);
    myDoubleValidator_->setLocale(QLocale::English);

    xMinEdit_->setText("-4.0");
    xMinEdit_->setAlignment(Qt::AlignRight);
    xMinEdit_->setValidator(myDoubleValidator_);

    xMaxEdit_->setText("4.0");
    xMaxEdit_->setAlignment(Qt::AlignRight);
    xMaxEdit_->setValidator(myDoubleValidator_);

    yMinEdit_->setText("-4.0");
    yMinEdit_->setAlignment(Qt::AlignRight);
    yMinEdit_->setValidator(myDoubleValidator_);

    yMaxEdit_->setText("4.0");
    yMaxEdit_->setAlignment(Qt::AlignRight);
    yMaxEdit_->setValidator(myDoubleValidator_);

    zMinEdit_->setText("-4.0");
    zMinEdit_->setAlignment(Qt::AlignRight);
    zMinEdit_->setValidator(myDoubleValidator_);

    zMaxEdit_->setText("4.0");
    zMaxEdit_->setAlignment(Qt::AlignRight);
    zMaxEdit_->setValidator(myDoubleValidator_);


    auto* box3DLayout = new QGridLayout(box3DGroup_);
    box3DLayout->addWidget(inf3DLabel_, 0, 1, Qt::AlignCenter);
    box3DLayout->addWidget(sup3DLabel_, 0, 2, Qt::AlignCenter);
    box3DLayout->addWidget(xLabel_, 1, 0);
    box3DLayout->addWidget(xMinEdit_, 1, 1);
    box3DLayout->addWidget(xMaxEdit_, 1, 2);
    box3DLayout->addWidget(yLabel_, 2, 0);
    box3DLayout->addWidget(yMinEdit_, 2, 1);
    box3DLayout->addWidget(yMaxEdit_, 2, 2);
    box3DLayout->addWidget(zLabel_, 3, 0);
    box3DLayout->addWidget(zMinEdit_, 3, 1);
    box3DLayout->addWidget(zMaxEdit_, 3, 2);

    auto* grid3DLayout = new QGridLayout(grid3DGroup_);
    grid3DLayout->addWidget(box3DGroup_);

    // 2D
    grid2DGroup_ = new QGroupBox(tr("2D grid"), this);
    setGrid2DVisible(false);

    box2DGroup_ = new QGroupBox(tr("2D boundaries"), grid2DGroup_);
    box2DGroup_->setEnabled(true);
    inf2DLabel_ = new QLabel(tr("Min"), box3DGroup_);
    sup2DLabel_ = new QLabel(tr("Max"), box3DGroup_);

    uLabel_ = new QLabel(tr("u"), box2DGroup_);
    vLabel_ = new QLabel(tr("v"), box2DGroup_);

    uMinEdit_ = new QLineEdit(box2DGroup_);
    uMinEdit_->setText("-4.0");
    uMinEdit_->setAlignment(Qt::AlignRight);
    uMinEdit_->setValidator(myDoubleValidator_);

    uMaxEdit_ = new QLineEdit(box2DGroup_);
    uMaxEdit_->setText("4.0");
    uMaxEdit_->setAlignment(Qt::AlignRight);
    uMaxEdit_->setValidator(myDoubleValidator_);

    vMinEdit_ = new QLineEdit(box2DGroup_);
    vMinEdit_->setText("-4.0");
    vMinEdit_->setAlignment(Qt::AlignRight);
    vMinEdit_->setValidator(myDoubleValidator_);

    vMaxEdit_ = new QLineEdit(box2DGroup_);
    vMaxEdit_->setText("4.0");
    vMaxEdit_->setAlignment(Qt::AlignRight);
    vMaxEdit_->setValidator(myDoubleValidator_);

    auto* box2DLayout = new QGridLayout(box2DGroup_);
    box2DLayout->addWidget(inf2DLabel_, 0, 1, Qt::AlignCenter);
    box2DLayout->addWidget(sup2DLabel_, 0, 2, Qt::AlignCenter);
    box2DLayout->addWidget(uLabel_, 1, 0);
    box2DLayout->addWidget(uMinEdit_, 1, 1);
    box2DLayout->addWidget(uMaxEdit_, 1, 2);
    box2DLayout->addWidget(vLabel_, 2, 0);
    box2DLayout->addWidget(vMinEdit_, 2, 1);
    box2DLayout->addWidget(vMaxEdit_, 2, 2);

    // Surface type
    surfaceTypeGroup_ = new QGroupBox(tr("Surface type"), grid2DGroup_);

    plane2DRadio_ = new QRadioButton(tr("Plane"), surfaceTypeGroup_);
    parametricSurfaceRadio_ = new QRadioButton(tr("Parametric surface"), surfaceTypeGroup_);
    parametricSurfaceRadio_->setToolTip(tr("Surface equation supplied in parametric form: x=x(u,v), y=y(u,v), z=z(u,v)"));

    auto* grid2DModeLayout = new QHBoxLayout(surfaceTypeGroup_);
    grid2DModeLayout->addWidget(plane2DRadio_);
    grid2DModeLayout->addWidget(parametricSurfaceRadio_);

    plane2DRadio_->setChecked(true);

    surface2DGroup_ = new QButtonGroup(this);
    surface2DGroup_->addButton(plane2DRadio_, 0);
    surface2DGroup_->addButton(parametricSurfaceRadio_, 1);

    planes2DGroup_ = new QGroupBox(tr("2D Planes"), grid2DGroup_);

    planeXYRadio_ = new QRadioButton(tr("XY"), grid2DGroup_);
    planeXZRadio_ = new QRadioButton(tr("XZ"), grid2DGroup_);
    planeYZRadio_ = new QRadioButton(tr("YZ"), grid2DGroup_);
    planeOtherRadio_ = new QRadioButton(tr("Other"), grid2DGroup_);

    planesButtonsGroup_ = new QButtonGroup(this);
    planesButtonsGroup_->addButton(planeXYRadio_, planeXY);
    planesButtonsGroup_->addButton(planeXZRadio_, planeXZ);
    planesButtonsGroup_->addButton(planeYZRadio_, planeYZ);
    planesButtonsGroup_->addButton(planeOtherRadio_, planeOther);

    planeCoefficientsGroup_ = new QGroupBox(tr("Plane coefficients"), grid2DGroup_);
    planeCoefficients_ = new QLabel("(A x + B y + C z = 0)", planeCoefficientsGroup_);
    planeAEdit_ = new QLineEdit(planeCoefficientsGroup_);
    planeBEdit_ = new QLineEdit(planeCoefficientsGroup_);
    planeCEdit_ = new QLineEdit(planeCoefficientsGroup_);
    planeAEdit_->setValidator(myDoubleValidator_);
    planeBEdit_->setValidator(myDoubleValidator_);
    planeCEdit_->setValidator(myDoubleValidator_);
    planeAEdit_->setText("0.");
    planeBEdit_->setText("0.");
    planeCEdit_->setText("1.");
    planeCoefficientsGroup_->setVisible(false);

    auto* planeCoefficientsLayout = new QGridLayout(planeCoefficientsGroup_);
    planeCoefficientsLayout->addWidget(planeCoefficients_, 0, 0, 1, 2);
    planeCoefficientsLayout->addWidget(new QLabel(tr("A")), 1, 0);
    planeCoefficientsLayout->addWidget(planeAEdit_, 1, 1);
    planeCoefficientsLayout->addWidget(new QLabel(tr("B")), 2, 0);
    planeCoefficientsLayout->addWidget(planeBEdit_, 2, 1);
    planeCoefficientsLayout->addWidget(new QLabel(tr("C")), 3, 0);
    planeCoefficientsLayout->addWidget(planeCEdit_, 3, 1);

    auto* planesLayout = new QGridLayout(planes2DGroup_);
    planesLayout->addWidget(planeXYRadio_, 0, 0);
    planesLayout->addWidget(planeXZRadio_, 0, 1);
    planesLayout->addWidget(planeYZRadio_, 1, 0);
    planesLayout->addWidget(planeOtherRadio_, 1, 1);
    planesLayout->addWidget(planeCoefficientsGroup_, 2,0,1,2);

    // Parametric equations
    parametricEquationsGroup_ = new QGroupBox(tr("Parametric equations"));
    equationXLabel_ = new QLabel(tr("x(u,v) = "), parametricEquationsGroup_);
    equationYLabel_ = new QLabel(tr("y(u,v) = "), parametricEquationsGroup_);
    equationZLabel_ = new QLabel(tr("z(u,v) = "), parametricEquationsGroup_);
    equationXEdit_ = new QLineEdit(parametricEquationsGroup_);
    equationXEdit_->setText("u");
    equationYEdit_ = new QLineEdit(parametricEquationsGroup_);
    equationYEdit_->setText("v");
    equationZEdit_ = new QLineEdit(parametricEquationsGroup_);
    equationZEdit_->setText("0");

    parametricEquationsGroup_->setVisible(false);

    auto* parametricEquationsLayout = new QGridLayout(parametricEquationsGroup_);
    parametricEquationsLayout->addWidget(equationXLabel_,0,0);
    parametricEquationsLayout->addWidget(equationXEdit_,0,1);
    parametricEquationsLayout->addWidget(equationYLabel_,1,0);
    parametricEquationsLayout->addWidget(equationYEdit_,1,1);
    parametricEquationsLayout->addWidget(equationZLabel_,2,0);
    parametricEquationsLayout->addWidget(equationZEdit_,2,1);

    // XYZ tabulation
    xyzGroup_ = new QGroupBox(tr("XYZ tabulation"), this);
    xyzCheck_ = new QCheckBox(tr("Tabulate values on xyz points"), xyzGroup_);
    xyzTable_ = new QWidget();
    xyzSheet_ = new Sheet(0, 3, 0,true, xyzTable_);
    QStringList xyzList_;
    xyzList_ << "x" << "y" << "z";
    xyzSheet_->setHeader(xyzList_);
    xyzTable_->setVisible(false);
    xyzTable_->setEnabled(false);

    auto* xyzLayout = new QVBoxLayout(xyzGroup_);
    xyzLayout->addWidget(xyzCheck_);
    xyzLayout->addWidget(xyzTable_);

    auto* grid2DLayout = new QVBoxLayout(grid2DGroup_);
    grid2DLayout->addWidget(box2DGroup_);
    grid2DLayout->addWidget(surfaceTypeGroup_);
    grid2DLayout->addWidget(planes2DGroup_);
    grid2DLayout->addWidget(parametricEquationsGroup_);


    // Resolution
    resolutionGroup_ = new QGroupBox(tr("Resolution"), this);
    lowResolutionRadio_ = new QRadioButton(tr("Low"), resolutionGroup_);
    mediumResolutionRadio_ = new QRadioButton(tr("Medium"), resolutionGroup_);
    highResolutionRadio_ = new QRadioButton(tr("High"), resolutionGroup_);
    customResolutionRadio_ = new QRadioButton(tr("Custom"), resolutionGroup_);
    mediumResolutionRadio_->setChecked(true);
    lowResolutionRadio_->setToolTip("65x65x65");
    mediumResolutionRadio_->setToolTip("129x129x129");
    highResolutionRadio_->setToolTip("257x257x257");

    resolutionButtonsGroup_ = new QButtonGroup(this);
    resolutionButtonsGroup_->addButton(lowResolutionRadio_, LOW_RES);
    resolutionButtonsGroup_->addButton(mediumResolutionRadio_, MEDIUM_RES);
    resolutionButtonsGroup_->addButton(highResolutionRadio_, HIGH_RES);
    resolutionButtonsGroup_->addButton(customResolutionRadio_, CUSTOM_RES);

    auto* resolutionModeLayout = new QGridLayout();
    resolutionModeLayout->addWidget(lowResolutionRadio_,0,0);
    resolutionModeLayout->addWidget(mediumResolutionRadio_,0,1);
    resolutionModeLayout->addWidget(highResolutionRadio_,1,0);
    resolutionModeLayout->addWidget(customResolutionRadio_,1,1);
    resolutionGroup_->setVisible(true);


    resolution2DGroup_ = new QGroupBox(tr("2D custom resolution"), resolutionGroup_);
    pointsUVLabel_ = new QLabel(tr("Points in each dimension"), resolution2DGroup_);
    pointsUVSpin_ = new QSpinBox(resolution2DGroup_);
    pointsUVSpin_->setRange(2, 5000);
    pointsUVSpin_->setValue(257);
    resolution2DGroup_->setVisible(true);


    auto* resolution2DLayout = new QHBoxLayout(resolution2DGroup_);
    resolution2DLayout->addWidget(pointsUVLabel_);
    resolution2DLayout->addWidget(pointsUVSpin_);


    resolution3DGroup_ = new QGroupBox(tr("3D custom resolution"), resolutionGroup_);
    pointsXLabel_ = new QLabel(tr("Points in x"), resolution3DGroup_);
    pointsYLabel_ = new QLabel(tr("Points in y"), resolution3DGroup_);
    pointsZLabel_ = new QLabel(tr("Points in z"), resolution3DGroup_);
    pointsXSpin_ = new QSpinBox(resolution3DGroup_);
    pointsYSpin_ = new QSpinBox(resolution3DGroup_);
    pointsZSpin_ = new QSpinBox(resolution3DGroup_);
    pointsXSpin_->setRange(2, 2000);
    pointsYSpin_->setRange(2, 2000);
    pointsZSpin_->setRange(2, 2000);
    pointsXSpin_->setValue(129);
    pointsYSpin_->setValue(129);
    pointsZSpin_->setValue(129);
    resolution3DGroup_->setVisible(false);

    auto* resolution3DLayout = new QGridLayout(resolution3DGroup_);
    resolution3DLayout->addWidget(pointsXLabel_, 0, 0);
    resolution3DLayout->addWidget(pointsXSpin_, 0, 1);
    resolution3DLayout->addWidget(pointsYLabel_, 1, 0);
    resolution3DLayout->addWidget(pointsYSpin_, 1, 1);
    resolution3DLayout->addWidget(pointsZLabel_, 2, 0);
    resolution3DLayout->addWidget(pointsZSpin_, 2, 1);

    auto* resolutionLayout = new QVBoxLayout(resolutionGroup_);
    resolutionLayout->addLayout(resolutionModeLayout);
    resolutionLayout->addWidget(resolution2DGroup_);
    resolutionLayout->addWidget(resolution3DGroup_);


    // Input only
    inputOnlyGroup_ = new QGroupBox(tr("Input only"), this);
    inputOnlyCheck_ = new QCheckBox(tr("Generate input file only"), inputOnlyGroup_);
    inputOnlyCheck_->setChecked(false);

    auto* inputOnlyLayout = new QVBoxLayout(inputOnlyGroup_);
    inputOnlyLayout->addWidget(inputOnlyCheck_);

    // MPI
    mpiGroup_ = new QGroupBox(tr("Parallel computing"), this);
    mpiCheck_ = new QCheckBox(tr("MPI"), mpiGroup_);
    mpiProcessorsLabel_ = new QLabel(tr("Number of processors"), mpiGroup_);
    mpiProcessorsSpin_ = new QSpinBox(mpiGroup_);
    mpiProcessorsSpin_->setRange(1, MAX_NUM_PROCESSORS);
    mpiProcessorsSpin_->setValue(1);
    mpiProcessorsSpin_->setMaximumWidth(60);

    auto* mpiLayout = new QHBoxLayout(mpiGroup_);
    mpiLayout->addWidget(mpiCheck_);
    mpiLayout->addWidget(mpiProcessorsLabel_);
    mpiLayout->addWidget(mpiProcessorsSpin_);

    if (mpiAvailable_) {
        mpiGroup_->setVisible(true);
        mpiCheck_->setChecked(true);
        mpiProcessorsSpin_->setEnabled(true);
    } else {
        mpiGroup_->setHidden(true);
        mpiCheck_->setChecked(false);
        mpiProcessorsSpin_->setEnabled(false);
    }

    // Buttons
    execButton_ = new QPushButton(QIcon(":/images/exec.png"), tr("Exec"), this);
    stopButton_ = new QPushButton(QIcon(":/images/stop.png"), tr("Stop"), this);
    stopButton_->setToolTip(tr("Kill the process"));
    stopButton_->setEnabled(false);
    outputButton_ = new QPushButton(QIcon(":/images/document_text.png"), "", this);

    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(outputButton_, 0, Qt::AlignLeft);
    buttonsLayout->addWidget(stopButton_, 0, Qt::AlignRight);
    buttonsLayout->addWidget(execButton_, 0, Qt::AlignRight);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(outputGroup_);
    mainLayout->addWidget(potentialSettingsGroup_);
    mainLayout->addWidget(derivativesGroup_);
    mainLayout->addWidget(gridGroup_);
    mainLayout->addWidget(gridTypeGroup_);
    mainLayout->addWidget(grid2DGroup_);
    mainLayout->addWidget(grid3DGroup_);
    mainLayout->addWidget(resolutionGroup_);
    mainLayout->addWidget(xyzGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();


}

void PotentialPage::connectSignals()
{
    connect(exactPotentialCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::potentialModeChanged);

    connect(longRangeCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::longRangeChanged);

    connect(gradientCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::gradientStateChanged);

    connect(secondDerivativesCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::secondDerivativesStateChanged);

    connect(gridCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::gridStateChanged);

    connect(gridDimensionGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PotentialPage::gridDimensionChanged);

    connect(surface2DGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PotentialPage::planeModeChanged);

    connect(planesButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PotentialPage::plane2DChanged);

    connect(resolutionButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &PotentialPage::resolutionModeChanged);

    connect(xyzCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::xyzTabulationStateChanged);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &PotentialPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
                this, &PotentialPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &PotentialPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &PotentialPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
                    runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &PotentialPage::openOutputRequested);

    connect(runner_, &ExternalProgramRunner::started,
                    this, [this]() {
                        setExecEnabled(false);
                        setStopEnabled(true);
                        emit externalProcessStarted();
                    });

    connect(runner_, &ExternalProgramRunner::finished,
            this, [this](int, QProcess::ExitStatus exitStatus) {
                setExecEnabled(true);
                setStopEnabled(false);

                const bool finishedOk = (exitStatus == QProcess::NormalExit);

                if (finishedOk) {
                    handleNormalProcessExit();
                } else if (exitStatus == QProcess::CrashExit) {
                    emit statusMessageRequested(
                        tr("Process crashed, exit code = %1").arg(exitStatus)
                    );
                }
                emit externalProcessFinished(finishedOk);
            });

    connect(runner_, &ExternalProgramRunner::errorOccurred,
            this, [this](QProcess::ProcessError) {
                setExecEnabled(true);
                setStopEnabled(false);
                emit externalProcessFinished(false);
            });
}

// ---- getters / setters ----

int PotentialPage::getPlaneCase() const { return planeResult.planeCase;}
int PotentialPage::lMax() const { return lMaxSpin_->value(); }
int PotentialPage::lMaxTop() const { return lMaxSpin_->maximum(); }
int PotentialPage::longRangeThreshold() const { return longRangeSpin_->value(); }
int PotentialPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int PotentialPage::pointsX() const { return pointsXSpin_->value();}
int PotentialPage::pointsY() const { return pointsYSpin_->value();}
int PotentialPage::pointsZ() const { return pointsZSpin_->value();}
int PotentialPage::pointsUV() const { return pointsUVSpin_->value();}
int PotentialPage::xyzTableRows() const { return xyzSheet_->tabla->rowCount();}

double PotentialPage::deltaU() const { return deltaU_;}
double PotentialPage::deltaV() const { return deltaV_;}
double PotentialPage::deltaX() const { return deltaX_;}
double PotentialPage::deltaY() const { return deltaY_;}
double PotentialPage::deltaZ() const { return deltaZ_;}

double PotentialPage::computeDelta(const char* key, double ini, double fin) const
{
    return ReadWriteOptions::computeDelta(key, ini, fin, this);
}

bool PotentialPage::is2DGrid() const { return gridDimensionGroup_->button(0)->isChecked(); }
bool PotentialPage::is3DGrid() const { return gridDimensionGroup_->button(1)->isChecked(); }
bool PotentialPage::isCustomResolution() const { return customResolutionRadio_->isChecked(); }
bool PotentialPage::isExactPotential() const { return exactPotentialCheck_->isChecked(); }
bool PotentialPage::isGradient() const { return gradientCheck_->isChecked(); }
bool PotentialPage::isGrid() const { return gridCheck_->isChecked(); }
bool PotentialPage::isHighResolution() const { return highResolutionRadio_->isChecked(); }
bool PotentialPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool PotentialPage::isLongRangePotential() const { return longRangeCheck_->isChecked(); }
bool PotentialPage::isLowResolution() const { return lowResolutionRadio_->isChecked(); }
bool PotentialPage::isMediumResolution() const { return mediumResolutionRadio_->isChecked(); }
bool PotentialPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool PotentialPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool PotentialPage::isPlane2D() const { return plane2DRadio_->isChecked(); }
bool PotentialPage::isPlaneOther() const { return planeOtherRadio_->isChecked(); }
bool PotentialPage::isPlaneXY() const { return planeXYRadio_->isChecked(); }
bool PotentialPage::isPlaneXZ() const { return planeXZRadio_->isChecked(); }
bool PotentialPage::isPlaneYZ() const { return planeYZRadio_->isChecked(); }
bool PotentialPage::isSecondDerivatives() const { return secondDerivativesCheck_->isChecked(); }
bool PotentialPage::isXYZTabulation() const { return xyzCheck_->isChecked(); }

QVector3D PotentialPage::getPlaneWu() const { return planeResult.wu; }
QVector3D PotentialPage::getPlaneWv() const { return planeResult.wv; }

void PotentialPage::clearSheet() {xyzSheet_->clear(); }
void PotentialPage::insertRow(int i) {xyzSheet_->tabla->insertRow(i); }
void PotentialPage::resizeSheet(int i) {xyzSheet_->resizeRows(i); }
void PotentialPage::setCellValue(const QString& value, int i, int j) {xyzSheet_->setcellvalue(value,i,j); }
void PotentialPage::setCustomResolution(bool checked) { resolutionButtonsGroup_->button(CUSTOM_RES)->setChecked(checked); }
void PotentialPage::setCustomResolutionEnabled(bool enabled) { customResolutionRadio_->setEnabled(enabled); }
void PotentialPage::setDerivativesOptionsVisible(bool visible)  { derivativesGroup_->setVisible(visible); }
void PotentialPage::setDeltaU(const double value) { deltaU_ = value; }
void PotentialPage::setDeltaV(const double value) { deltaV_ = value; }
void PotentialPage::setDeltaX(const double value) { deltaX_ = value; }
void PotentialPage::setDeltaY(const double value) { deltaY_ = value; }
void PotentialPage::setDeltaZ(const double value) { deltaZ_ = value; }
void PotentialPage::setEquationX(const QString& value) { equationXEdit_->setText(value); }
void PotentialPage::setEquationY(const QString& value) { equationYEdit_->setText(value); }
void PotentialPage::setEquationZ(const QString& value) { equationZEdit_->setText(value); }
void PotentialPage::setExactPotential(bool checked) { exactPotentialCheck_->setChecked(checked); }
void PotentialPage::setExactPotentialVisible(bool visible) { exactPotentialCheck_->setVisible(visible); }
void PotentialPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void PotentialPage::setGradient(bool checked) { gradientCheck_->setChecked(checked); }
void PotentialPage::setGrid(bool checked) { gridCheck_->setChecked(checked);}
void PotentialPage::setGrid2D(bool checked) { gridDimensionGroup_->button(DIM_2D)->setChecked(checked); }
void PotentialPage::setGrid2DVisible(bool visible) { grid2DGroup_->setVisible(visible); grid2DGroup_->setEnabled(visible); }
void PotentialPage::setGrid3D(bool checked) { gridDimensionGroup_->button(DIM_3D)->setChecked(checked); }
void PotentialPage::setGrid3DVisible(bool visible) { grid3DGroup_->setVisible(visible); grid3DGroup_->setEnabled(visible); }
void PotentialPage::setGridButtonsEnabled(bool enabled) { gridDimensionGroup_->button(DIM_2D)->setEnabled(enabled);
                                        gridDimensionGroup_->button(DIM_3D)->setEnabled(enabled); }
void PotentialPage::setGridTypeVisible(bool visible) { gridTypeGroup_->setVisible(visible); }
void PotentialPage::setHighResolution(bool checked) { resolutionButtonsGroup_->button(HIGH_RES)->setChecked(checked); }
void PotentialPage::setHighResolutionEnabled(bool enabled) { highResolutionRadio_->setEnabled(enabled); }
void PotentialPage::setHighResolutionTip(const QString &value) { highResolutionRadio_->setToolTip(value); }
void PotentialPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void PotentialPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void PotentialPage::setLmaxVisible(bool visible) { lmaxGroup_->setVisible(visible); }
void PotentialPage::setLongRangeMinThreshold(int number) { longRangeSpin_->setMinimum(number); }
void PotentialPage::setLongRangeEnabled(bool checked) { longRangeSpin_->setEnabled(checked); }
void PotentialPage::setLongRangePotential(bool checked) { longRangeCheck_->setChecked(checked); }
void PotentialPage::setLongRangeThreshold(int number) { longRangeSpin_->setValue(number); }
void PotentialPage::setLongRangeTopThreshold(int number) { longRangeSpin_->setMaximum(number); }
void PotentialPage::setLongRangeVisible(bool visible) { longRangeGroup_->setVisible(visible); }
void PotentialPage::setLowResolution(bool checked) { resolutionButtonsGroup_->button(LOW_RES)->setChecked(checked); }
void PotentialPage::setLowResolutionEnabled(bool enabled) { lowResolutionRadio_->setEnabled(enabled); }
void PotentialPage::setLowResolutionTip(const QString &value) { lowResolutionRadio_->setToolTip(value); }
void PotentialPage::setMediumResolution(bool checked) { resolutionButtonsGroup_->button(MEDIUM_RES)->setChecked(checked); }
void PotentialPage::setMediumResolutionEnabled(bool enabled) { mediumResolutionRadio_->setEnabled(enabled); }
void PotentialPage::setMediumResolutionTip(const QString &value) { mediumResolutionRadio_->setToolTip(value); }
void PotentialPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void PotentialPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void PotentialPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void PotentialPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void PotentialPage::setOutputButtonEnabled(bool enabled) { outputButton_->setEnabled(enabled); }
void PotentialPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void PotentialPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void PotentialPage::setParametricEquationsVisible(bool visible) { parametricEquationsGroup_->setVisible(visible); }
void PotentialPage::setPlanes2DVisible(bool visible) { planes2DGroup_->setVisible(visible); }
void PotentialPage::setPlane2D(bool checked) { plane2DRadio_->setChecked(checked); }
void PotentialPage::setPlaneA(const QString& value) { planeAEdit_->setText(value); }
void PotentialPage::setPlaneB(const QString& value) { planeBEdit_->setText(value); }
void PotentialPage::setPlaneC(const QString& value) { planeCEdit_->setText(value); }
void PotentialPage::setPlaneCoefficientsVisible(bool visible) { planeCoefficientsGroup_->setVisible(visible); }
void PotentialPage::setPlaneOther(bool checked) { planeOtherRadio_->setChecked(checked); }
void PotentialPage::setPlaneXY(bool checked) { planesButtonsGroup_->button(planeXY)->setChecked(checked); }
void PotentialPage::setPlaneXZ(bool checked) { planesButtonsGroup_->button(planeXZ)->setChecked(checked);}
void PotentialPage::setPlaneYZ(bool checked) { planesButtonsGroup_->button(planeYZ)->setChecked(checked); }
void PotentialPage::setPointsUV(int number) { pointsUVSpin_->setValue(number); }
void PotentialPage::setPointsX(int number) { pointsXSpin_->setValue(number); }
void PotentialPage::setPointsY(int number) { pointsYSpin_->setValue(number); }
void PotentialPage::setPointsZ(int number) { pointsZSpin_->setValue(number); }
void PotentialPage::setResolutionVisible(bool visible) { resolutionGroup_->setVisible(visible); }
void PotentialPage::setResolution2DVisible(bool visible) { resolution2DGroup_->setVisible(visible); }
void PotentialPage::setResolution3DVisible(bool visible) { resolution3DGroup_->setVisible(visible); }
void PotentialPage::setSecondDerivatives(bool checked) { secondDerivativesCheck_->setChecked(checked); }
void PotentialPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void PotentialPage::setUmax(const QString& value) { if (value.toDouble() > uMinValue().toDouble()) uMaxEdit_->setText(value); }
void PotentialPage::setUmin(const QString& value) { if (value.toDouble() < uMaxValue().toDouble()) uMinEdit_->setText(value); }
void PotentialPage::setVmax(const QString& value) { if (value.toDouble() > vMinValue().toDouble()) vMaxEdit_->setText(value); }
void PotentialPage::setVmin(const QString& value) { if (value.toDouble() < vMaxValue().toDouble()) vMinEdit_->setText(value); }
void PotentialPage::setXmax(const QString& value) { if (value.toDouble() > xMinValue().toDouble()) xMaxEdit_->setText(value); }
void PotentialPage::setXmin(const QString& value) { if (value.toDouble() < xMaxValue().toDouble()) xMinEdit_->setText(value); }
void PotentialPage::setXYZTabulation(bool checked) { xyzCheck_->setChecked(checked); }
void PotentialPage::setXYZEnabled(bool checked) { xyzTable_->setEnabled(checked); }
void PotentialPage::setXYZVisible(bool checked) { xyzTable_->setVisible(checked); }
void PotentialPage::setYmax(const QString& value) { if (value.toDouble() > yMinValue().toDouble()) yMaxEdit_->setText(value); }
void PotentialPage::setYmin(const QString& value) { if (value.toDouble() < yMaxValue().toDouble()) yMinEdit_->setText(value); }
void PotentialPage::setZmax(const QString& value) { if (value.toDouble() > zMinValue().toDouble()) zMaxEdit_->setText(value); }
void PotentialPage::setZmin(const QString& value) { if (value.toDouble() < zMaxValue().toDouble()) zMinEdit_->setText(value); }


//    Executes external program DAMPOT  (Computes molecular electrostatic potential from the atomic partition)
void PotentialPage::execDamPot()
{
    QString stdOutput;

    QString rootName = "DAMPOT_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMPOT_400.inp";
    QString inputSection = "DAMPOTSECT";

    const QString projectFile =
            QDir(projectFolder_).filePath(projectName_ + ".damproj");


    bool printwarns = false;

    QString warns = QString(tr("Warning: failed saving the following options") + ":\n");

    writeToFile(
            projectFile.toStdString(),
            &printwarns,
            &warns);

    runner_->inputdatafile(inputTemplate,
                          inputSection,
                          projectFile,
                          projectFolder_,
                          projectName_);

    const QString stdInput =
                QDir(projectFolder_).filePath(projectName_ + "-" + inputTemplate);

    QFile file(stdInput);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this,
                             tr("DAMQT"),
                             tr("Cannot create input file %1").arg(stdInput));
        return;
    }

    if (isInputOnly()) {
        QTextStream in(&file);

        emit showInputFileRequested(in.readAll());

        QMessageBox::information(this,
                                 tr("DAMQT"),
                                 tr("File %1 has been successfully created")
                                     .arg(stdInput));
        return;
    }

    file.close();

    ExternalProgramRunner::RunRequest request;
    if (is3DGrid()){
        request.runMpi = isMpiChecked();
    } else{
        request.runMpi = false;
    }
    if (outputPrefixEdit_->text().isEmpty()){
        request.outputPrefix = projectName_;
    } else{
        request.outputPrefix = outputPrefixEdit_->text();
    }

    request.rootName = rootName;
    request.stdInput = stdInput;
    request.stdOutput = stdOutput;
    request.subdir = subdir;
    request.nprocs = mpiProcessors();

    const QString exactsuffix = isExactPotential() ? "_exact" : "";

    const QString suffix = request.runMpi ? "_mpi" : "";

    request.suffix = exactsuffix + suffix;

    lastOutputFileName_ =
        QDir(projectFolder_).filePath(
            request.outputPrefix + "-" + request.rootName + request.suffix + ".out"
//            request.outputPrefix + exactsuffix + "-" + request.rootName + suffix + ".out"
        );

    setStopEnabled(true);
    runner_->setProjectFolder(projectFolder_);

    runner_->setMpiCommand(mpiCommand_);
    runner_->setMpiFlags(mpiFlags_);
    runner_->start(request);

    if (is2DGrid() && isPlane2D()){
        QVector3D wu;
        QVector3D wv;
        int densplanecase;
        wu = getPlaneWu();
        wv = getPlaneWv();
        QString fileinpstr=projectFolder_+projectName_+".xyz";
        QFile fileinp(fileinpstr);
        if (fileinp.open(QFile::ReadOnly | QFile::Text)) {
            densplanecase = getPlaneCase();
            if (densplanecase > 0 && densplanecase < 8){
                QVector<QString> plane;
                plane << "XY0" << "X0Z" << "0YZ" << "AB0" << "A0C" << "0BC" << "ABC";
                QString fileoutstr = projectFolder_+request.outputPrefix;
                if (isExactPotential()){
                    fileoutstr += "_exact";
                }
                fileoutstr = fileoutstr+"_"+plane.at(densplanecase-1)+"-v.plane";
                QFile fileout(fileoutstr);
                if(fileout.open(QFile::Text | QFile::WriteOnly)){
                    QTextStream in(&fileinp); // Buffer for reading from fileinput
                    QTextStream out(&fileout); // Buffer for writing to fileout
                    QString line;
                    Elements elem;
                    while (!in.atEnd()){
                        line = in.readLine();
#if QT_VERSION < 0x050E00
                        QStringList xyz = line.split(' ',QString::SkipEmptyParts);
#else
                        QStringList xyz = line.split(' ',Qt::SkipEmptyParts);
#endif
                        if (xyz.count() == 4){
                            double x = xyz[1].toDouble() * ANGSTROMTOBOHR;
                            double y = xyz[2].toDouble() * ANGSTROMTOBOHR;
                            double z = xyz[3].toDouble() * ANGSTROMTOBOHR;
                            if (qAbs(QVector3D::dotProduct(QVector3D(x,y,z),
                                QVector3D(planeA().toDouble(), planeB().toDouble(), planeC().toDouble()))) > 1.e-8)
                                continue;
                            double u = QVector3D::dotProduct(QVector3D(x,y,z),wu) / wu.length();
                            double v = QVector3D::dotProduct(QVector3D(x,y,z),wv) / wv.length();
                            int znuc = elem.getZsymbol(xyz[0]);
                            out << QString("%1  %2  %3  %4  %5  %6\n").arg(u).arg(v).arg(znuc).arg(x).arg(y).arg(z);
                        }
                    }
                    out << QString("%1 %2 %3").arg(planeA().toDouble()).arg(planeB().toDouble()).arg(planeC().toDouble());
                    fileout.close();
                }
            }
        }
    }
    rename_pot_cntfile();
}

void PotentialPage::gradientStateChanged(int state)
{
    Q_UNUSED(state);
// Extra safeguard: gradient cannot be disabled while second derivatives are active.
    if (isSecondDerivatives() && !isGradient()) {
        QSignalBlocker blocker(gradientCheck_);
        setGradient(true);
    }
}

void PotentialPage::gridDimensionChanged(int id)
{
    int resolution;
    if (id == 0){
        setGrid2DVisible(true);
        setGrid3DVisible(false);
        setGradient(false);
        setSecondDerivatives(false);
        setResolution3DVisible(false);
        if (mpiAvailable_){
            setMpiVisible(false);
            setMpiCheckEnabled(false);
            setMpiEnabled(false);
        }
        setLowResolutionTip("129x129");
        setMediumResolutionTip("257x257");
        setHighResolutionTip("513x513");
        resolution = ReadWriteOptions::whatResolution2D(this);
        if (resolution == 3){  // Custom resolution
            setResolution2DVisible(true);
        }
    }
    else{
        setGrid2DVisible(false);
        setGrid3DVisible(true);
        setResolution2DVisible(false);
        if (mpiAvailable_){
            if( !isInputOnly()){
                setMpiVisible(true);
                if (isMpiChecked()){
                    setMpiControlsEnabled(true);
                }
                else{
                    setMpiControlsEnabled(false);
                    setMpiCheckEnabled(true);
                }
            }
            else{
                setMpiVisible(false);
                setMpiCheckEnabled(false);
                setMpiControlsEnabled(false);
            }
        }
        setGradient(true);
        setLowResolutionTip("65x65x65");
        setMediumResolutionTip("129x129x129");
        setHighResolutionTip("257x257x257");
        resolution = ReadWriteOptions::whatResolution3D(this);
        if (resolution == 3){
            setResolution3DVisible(true);
        }
    }
    switch (resolution) {
        case LOW_RES:
            setLowResolution(true);
            break;
        case MEDIUM_RES:
            setMediumResolution(true);
            break;
        case HIGH_RES:
            setHighResolution(true);
            break;
        case CUSTOM_RES:
            setCustomResolution(true);
            break;
    }
    int state = isGrid() ? Qt::Checked : Qt::Unchecked;
    gridStateChanged(state);
}

void PotentialPage::gridStateChanged(int state){
    if (state == Qt::Checked){
        if (is2DGrid()){
            setGrid2DVisible(true);
            setGrid3DVisible(false);
            if (isCustomResolution()){
                setResolution2DVisible(true);
            }
            setResolution3DVisible(false);
            if (mpiAvailable_){
                setMpiVisible(false);
            }
        }
        else{
            setGrid2DVisible(false);
            setGrid3DVisible(true);
            if (isCustomResolution()){
                setResolution3DVisible(true);
            }
            setResolution2DVisible(false);
            if (mpiAvailable_){
                if( !isInputOnly()){
                    setMpiVisible(true);
                }
                else {
                    setMpiVisible(false);
                }
            }
        }
        setGridTypeVisible(true);
        setResolutionVisible(true);
        setGridButtonsEnabled(true);
        setCustomResolutionEnabled(true);
        setLowResolutionEnabled(true);
        setMediumResolutionEnabled(true);
        setHighResolutionEnabled(true);
    }
    else{
        if (mpiAvailable_){
            setMpiVisible(false);
        }
        setGridTypeVisible(false);
        setResolutionVisible(false);
        setGrid2DVisible(false);
        setGrid3DVisible(false);
        setGridButtonsEnabled(false);
        setCustomResolutionEnabled(false);
        setLowResolutionEnabled(false);
        setMediumResolutionEnabled(false);
        setHighResolutionEnabled(false);
    }
    if (is3DGrid()){
        setLowResolutionTip("65x65x65");
        setMediumResolutionTip("129x129x129");
        setHighResolutionTip("257x257x257");
    }
    else{
        setLowResolutionTip("129x129");
        setMediumResolutionTip("257x257");
        setHighResolutionTip("513x513");
    }
}

void PotentialPage::handleNormalProcessExit()
{
    QFile file(lastOutputFileName_);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(
            this,
            tr("ProcessOutput"),
            tr("File %1 cannot be read:\n%2.")
                .arg(lastOutputFileName_, file.errorString())
        );
        return;
    }

    QTextStream in(&file);
    const QString outputText = in.readAll();

    emit outputTextReady(outputText);
}

void PotentialPage::inputOnlyStateChanged(int state)
{
    if (!(state == Qt::Checked) && isGrid() && is3DGrid()){
        setMpiVisible(true);
        setMpiControlsEnabled(true);
        if (!isMpiChecked()){
            setMpiEnabled(false);
        }
    }
    else{
        setMpiVisible(false);
        setMpiControlsEnabled(false);
    }
}

void PotentialPage::loadDefault(){
    setLmaxTop(MAX_LEXP);
    setLmax(10);
    setLongRangePotential(false);
    setLongRangeMinThreshold(-10);
    setLongRangeTopThreshold(0);
    setLongRangeThreshold(-7);

    setUmin("-10.0");
    setUmax("10.0");
    setVmin("-10.0");
    setVmax("10.0");
    setXmin("-10.0");
    setXmax("10.0");
    setYmin("-10.0");
    setYmax("10.0");
    setZmin("-10.0");
    setZmax("10.0");

    setLowResolution(true);
    setGrid(true);
    setGridTypeVisible(true);
    setGrid3D(true);
    setGrid3DVisible(true);
    setGrid2DVisible(false);

    setResolution2DVisible(false);
    setResolution3DVisible(false);
}

void PotentialPage::longRangeChanged(int state)
{
    if (state == Qt::Checked){
        setLongRangeEnabled(false);
    }
    else{
        setLongRangeEnabled(true);
    }
}

void PotentialPage::mpiStateChanged(int state)
{
    Q_UNUSED(state);
    QSignalBlocker blocker(mpiCheck_);
    if (is3DGrid() && !isInputOnly()){
        setMpiControlsEnabled(true);
        setMpiVisible(true);
    }
    else if (is2DGrid()){
        setMpiChecked(false);
        setMpiEnabled(false);
        setMpiVisible(false);
    }
    else if (isInputOnly()){
        setMpiEnabled(false);
        setMpiVisible(false);
    }
    else{
        setMpiControlsEnabled(true);
    }
}

void PotentialPage::plane2DChanged(int id){
    switch (id) {
        case planeXY:
            setEquationX("u");
            setEquationY("v");
            setEquationZ("0");
            setPlaneA("0.");
            setPlaneB("0.");
            setPlaneC("1.");
            setPlaneCoefficientsVisible(false);
            break;
        case planeXZ:
            setEquationX("u");
            setEquationY("0");
            setEquationZ("v");
            setPlaneA("0.");
            setPlaneB("1.");
            setPlaneC("0.");
            setPlaneCoefficientsVisible(false);
            break;
        case planeYZ:
            setEquationX("0");
            setEquationY("u");
            setEquationZ("v");
            setPlaneA("1.");
            setPlaneB("0.");
            setPlaneC("0.");
            setPlaneCoefficientsVisible(false);
            break;
        default:
            setPlaneA("1.");
            setPlaneB("1.");
            setPlaneC("1.");
            setEquationX("u");
            setEquationY("v");
            setEquationZ("-(("+planeA()+"*u)+("+planeB()+"*v))/("+planeC()+")");
            setPlaneCoefficientsVisible(true);
    }
    planeResult = ReadWriteOptions::get_plane_case(planeA().toDouble(),
        planeB().toDouble(),planeC().toDouble());
}

void PotentialPage::planeModeChanged(int id)
{
    if (id == 0) {
        setParametricEquationsVisible(false);
        setPlanes2DVisible(true);
        planeResult = ReadWriteOptions::get_plane_case(planeA().toDouble(),planeB().toDouble(),planeC().toDouble());
    }else{
        setParametricEquationsVisible(true);
        setPlanes2DVisible(false);
    }
}

QString PotentialPage::planeSuffix(int planecase){
    switch (planecase){
    case 1:
        return QString("_XY0");
    case 2:
        return QString("_X0Z");
    case 3:
        return QString("_0YZ");
    case 4:
        return QString("_AB0");
    case 5:
        return QString("_A0C");
    case 6:
        return QString("_0BC");
    case 7:
        return QString("_ABC");
    default:
        return QString("");
    }
}

void PotentialPage::potentialModeChanged()    // If is exact potential unchecks and hides gradient and derivatives
{
    if (isExactPotential()) {
        setGradient(false);
        setSecondDerivatives(false);
        setLmaxVisible(false);
        setLongRangeVisible(false);
        setDerivativesOptionsVisible(false);
        if (activebeware_){
            QCheckBox *cb = new QCheckBox(tr("Do not display this message again"));
            QMessageBox msgbox;
            msgbox.setText(tr("Beware that MESP calculation from density matrix and basis set ")+
                    tr("is a lengthy process. \nThis option is intended only for testing purposes. ")+
                    tr("\nIt may take too much time in MESP tabulation of large systems on grids"));
            msgbox.setIcon(QMessageBox::Icon::Information);
            msgbox.addButton(QMessageBox::Ok);
            msgbox.setCheckBox(cb);
            QObject::connect(cb, &QCheckBox::stateChanged, this, [this](int state){
                if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                    this->activebeware_ = false;
                }
            });
            msgbox.exec();
        }
    }
    else{
        setLmaxVisible(true);
        setLongRangeVisible(true);
        setDerivativesOptionsVisible(true);
    }
}

void PotentialPage::readFromFile(const std::string& file)
{
    const char* section = "DAMPOTSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    setLmax(ReadWriteOptions::readSpinBox("lmaxrep", section, file));
    setLongRangePotential(ReadWriteOptions::readCheckBox("largo", section, file));

    QString value = ReadWriteOptions::readTextToLineEdit("umbrlargo", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-7");
    setLongRangeThreshold(value.toInt());

    setExactPotential(ReadWriteOptions::readCheckBox("lexact", section, file));
    setGradient(ReadWriteOptions::readCheckBox("lgradient", section, file));
    setSecondDerivatives(ReadWriteOptions::readCheckBox("lderiv2", section, file));

    ReadWriteOptions::readDoubleToLineEdit("xinf", section, xMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("xsup", section, xMaxEdit_, file);
    ReadWriteOptions::readDouble("dltx", section, &deltaX_, file);
    ReadWriteOptions::readDoubleToLineEdit("yinf", section, yMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("ysup", section, yMaxEdit_, file);
    ReadWriteOptions::readDouble("dlty", section, &deltaY_, file);
    ReadWriteOptions::readDoubleToLineEdit("zinf", section, zMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("zsup", section, zMaxEdit_, file);
    ReadWriteOptions::readDouble("dltz", section, &deltaZ_, file);

    ReadWriteOptions::readDoubleToLineEdit("uinf", section, uMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("usup", section, uMaxEdit_, file);
    ReadWriteOptions::readDouble("dltu", section, &deltaU_, file);
    ReadWriteOptions::readDoubleToLineEdit("vinf", section, vMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("vsup", section, vMaxEdit_, file);
    ReadWriteOptions::readDouble("dltv", section, &deltaV_, file);

    ReadWriteOptions::readDimensionOption("lgrid2d", section, file, this);

    int resolution;
    if (is3DGrid()){
        resolution = ReadWriteOptions::whatResolution3D(this);
        setGrid2DVisible(false);
    }
    else{
        resolution = ReadWriteOptions::whatResolution2D(this);
        setGrid3DVisible(false);
    }
    switch (resolution) {
        case LOW_RES:
            setLowResolution(true);
            break;
        case MEDIUM_RES:
            setMediumResolution(true);
            break;
        case HIGH_RES:
            setHighResolution(true);
            break;
        case CUSTOM_RES:
            setCustomResolution(true);
            break;
    }
    // ---- Equations ----
    equationXEdit_->setText(ReadWriteOptions::readTextToLineEdit("x_func_uv", section, file));
    equationYEdit_->setText(ReadWriteOptions::readTextToLineEdit("y_func_uv", section, file));
    equationZEdit_->setText(ReadWriteOptions::readTextToLineEdit("z_func_uv", section, file));

    // ---- Plane ----
    QString aux;
    aux = ReadWriteOptions::readDoubleToLineEdit("planeA", section, file);
    if (!aux.isEmpty()) setPlaneA(aux);
    aux = ReadWriteOptions::readDoubleToLineEdit("planeB", section, file);
    if (!aux.isEmpty()) setPlaneB(aux);
    aux = ReadWriteOptions::readDoubleToLineEdit("planeC", section, file);
    if (!aux.isEmpty()) setPlaneC(aux);
    updatePlaneSelectionFromCoefficients();

    // ---- Grid enable ----
    bool grid = ReadWriteOptions::readCheckBox("lgrid", section, file);
    gridCheck_->setChecked(grid);

    gridTypeGroup_->setVisible(grid);
    resolutionGroup_->setVisible(grid);

    if (grid) {
        setGridTypeVisible(true);
        setResolutionVisible(true);
        setGrid2DVisible(is2DGrid());
        setGrid3DVisible(is3DGrid());
        if (isCustomResolution()){
            setResolution2DVisible(is2DGrid());
            setResolution3DVisible(is3DGrid());
        }
        mpiGroup_->setVisible(is3DGrid());
    }
    else{
        setGridTypeVisible(false);
        setResolutionVisible(false);
    }

    // ---- Sheet for tabulation ----
    setXYZTabulation(ReadWriteOptions::readCheckBox("lpoints", section, file));

    readXYZTable(file, section);
    xyzTable_->updateGeometry();
}


void PotentialPage::readXYZTable(const std::string& file, const char* section)
{
    // ---- Number of rows ----
    const QString numText =
        QString::fromStdString(CIniFile::GetValue("numrtab", section, file));

    bool ok = false;
    const int numRows = numText.toInt(&ok);

    clearSheet();

    if (!ok || numRows <= 0) {
        setXYZEnabled(false);
        setXYZVisible(false);
        return;
    }

    resizeSheet(numRows + 1);

    for (int i = 0; i < numRows; ++i) {
        insertRow(i);
        for (int j = 0; j < 3; ++j) {
            const QString key = QString("rtab(%1,%2)").arg(j + 1).arg(i + 1);

            const QString value =
                QString::fromStdString(
                    CIniFile::GetValue(key.toStdString(), section, file)
                );

            setCellValue(value, i, j);
        }
    }

    resizeSheet(xyzTableRows());

    // ---- Visibility ----
    if (isXYZTabulation()) {
        setXYZEnabled(true);
        setXYZVisible(true);
    } else {
        setXYZEnabled(false);
        setXYZVisible(false);
    }
}

void PotentialPage::resolutionModeChanged(int id){
    if (id == CUSTOM_RES){
        if (is3DGrid()){
            setResolution2DVisible(false);
            setResolution3DVisible(true);
        }
        else{
            setResolution2DVisible(true);
            setResolution3DVisible(false);
        }
    }
    else{
        setResolution2DVisible(false);
        setResolution3DVisible(false);
    }
}

void PotentialPage::secondDerivativesStateChanged(int state)
{
    if (state == Qt::Checked) {     // If second derivatives are enabled, gradient is mandatory.
        {
            QSignalBlocker blocker(gradientCheck_);
            setGradient(true);
        }
        gradientCheck_->setEnabled(false);
    } else {
        gradientCheck_->setEnabled(true);
    }
}

void PotentialPage::xyzTabulationStateChanged(int state)
{
    if (state == Qt::Checked){
        setXYZEnabled(true);
        setXYZVisible(true);
    }else{
        setXYZEnabled(false);
        setXYZVisible(false);
    }
}

void PotentialPage::rename_pot_cntfile(){
    QString aux;
    if (isExactPotential()){
        aux = "_exact";
    }
    else{
        aux = "";
    }
    QFile filecnt(projectFolder_ + projectName_ + aux + "-v.cnt");
    int potplanecase = getPlaneCase();
    if (filecnt.exists() && planeSuffix(potplanecase) != ""){
        QFile fileold(projectFolder_ + projectName_ + aux + planeSuffix(potplanecase) + "-v.cnt");
        if (fileold.exists())
            fileold.remove();
        filecnt.rename(projectFolder_ + projectName_ + aux + planeSuffix(potplanecase) + "-v.cnt");
    }
}

void PotentialPage::setIsValence(bool valence)
{
    lvalence_ = valence;
}

void PotentialPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void PotentialPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void PotentialPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}


void PotentialPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void PotentialPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void PotentialPage::stopCurrentProcess()
{
    runner_->stop();
}

void PotentialPage::updatePlaneSelectionFromCoefficients()
{
    const double a = planeA().toDouble();
    const double b = planeB().toDouble();
    const double c = planeC().toDouble();

    if (c == 0.0) {
        if (b == 0.0) {
            setPlaneYZ(true);
        } else if (a == 0.0) {
            setPlaneXZ(true);
        }
    } else if (a == 0.0 && b == 0.0) {
        setPlaneXY(true);
    }
    else{
        setPlaneOther(true);
    }
}

//void PotentialPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool lvalence,
//                                bool* printwarns,
//                                QString* warns,
//                                const DeltaCalculator& deltaCalculator)
void PotentialPage::writeToFile(const std::string& file,
                              bool* printwarns,
                              QString* warns)
{
    const char* section = "DAMPOTSECT";

    CIniFile::DeleteSection(section, file);

    auto writeBool = [&](const char* key, bool value) {
        ReadWriteOptions::writeOption(
            key, section, value ? QString("T") : QString("F"),
            file, printwarns, warns);
    };

    auto writeText = [&](const char* key, const QString& value) {
        ReadWriteOptions::writeOption(
            key, section, value, file, printwarns, warns);
    };

    auto deltaCalculator =
                [this](const char *key, double ini, double fin) {
                    return computeDelta(key, ini, fin);
                };

    // ---- General ----
    writeBool("iswindows", iswindows_);

    if (lvalence_) {
        writeBool("lvalence", true);
    }

    if (isExactPotential()) {
        writeBool("lexact", true);
        writeBool("lgradient", false);
        writeBool("lderiv2", false);
    } else {
        writeBool("lexact", false);
        writeBool("lgradient", isGradient());
        writeBool("lderiv2", isSecondDerivatives());
    }

    writeText("lmaxrep", QString::number(lMax()));

    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

    // ---- Long-range ----
    writeBool("largo", isLongRangePotential());
    writeText("umbrlargo", QString("1.d%1").arg(longRangeThreshold()));

    // ---- Intervals ----
    if (is3DGrid()){
        setDeltaX(ReadWriteOptions::writeIntervals(
            "xinf", "xsup", "dltx", section, file,
            xMinValue(), xMaxValue(), printwarns, warns, deltaCalculator));

        setDeltaY(ReadWriteOptions::writeIntervals(
            "yinf", "ysup", "dlty", section, file,
            yMinValue(), yMaxValue(), printwarns, warns, deltaCalculator));

        setDeltaZ(ReadWriteOptions::writeIntervals(
            "zinf", "zsup", "dltz", section, file,
            zMinValue(), zMaxValue(), printwarns, warns, deltaCalculator));

        ReadWriteOptions::writeOption("uinf", section, uMinValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("usup", section, uMaxValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("dltu", section, QString("%2").arg(deltaU_), file, printwarns, warns);

        ReadWriteOptions::writeOption("vinf", section, vMinValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("vsup", section, vMaxValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("dltv", section, QString("%2").arg(deltaV_), file, printwarns, warns);
    }
    else{
        setDeltaU(ReadWriteOptions::writeIntervals(
            "uinf", "usup", "dltu", section, file,
            uMinValue(), uMaxValue(), printwarns, warns, deltaCalculator));

        setDeltaV(ReadWriteOptions::writeIntervals(
            "vinf", "vsup", "dltv", section, file,
            vMinValue(), vMaxValue(), printwarns, warns, deltaCalculator));

        ReadWriteOptions::writeOption("xinf", section, xMinValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("xsup", section, xMaxValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("dltx", section, QString("%2").arg(deltaX_), file, printwarns, warns);

        ReadWriteOptions::writeOption("yinf", section, yMinValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("ysup", section, yMaxValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("dlty", section, QString("%2").arg(deltaY_), file, printwarns, warns);

        ReadWriteOptions::writeOption("zinf", section, zMinValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("zsup", section, zMaxValue(), file, printwarns, warns);
        ReadWriteOptions::writeOption("dltz", section, QString("%2").arg(deltaZ_), file, printwarns, warns);
    }

    // ---- Grid ----
    writeBool("lgrid2d", is2DGrid());
//    setMpiControlsEnabled(is3DGrid());
//    setMpiVisible(is3DGrid());

    // ---- Parametric eq ----
    writeText("x_func_uv", quoteString(equationX()));
    writeText("y_func_uv", quoteString(equationY()));
    writeText("z_func_uv", quoteString(equationZ()));

    // ---- Plane ----
    if (isPlane2D()){
        writeText("planecase", QString::number(getPlaneCase()));
        writeText("planeA", planeA());
        writeText("planeB", planeB());
        writeText("planeC", planeC());
    }
    else{
        writeText("planecase", QString::number(-1));
    }

    // ---- XYZ table ----
    ReadWriteOptions::writeXYZTable(section, file, this, Sheet::max_sel, printwarns, warns);

    // ---- Final flags ----
    writeBool("lgrid", isGrid());
    writeBool("lpoints", isXYZTabulation());
}

QString PotentialPage::equationX() const { return equationXEdit_->text(); }
QString PotentialPage::equationY() const { return equationYEdit_->text(); }
QString PotentialPage::equationZ() const { return equationZEdit_->text(); }
QString PotentialPage::getCellValue(int i, int j) const { return xyzSheet_->getcellvalue(i,j); }
QString PotentialPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString PotentialPage::planeA() const { return planeAEdit_->text(); }
QString PotentialPage::planeB() const { return planeBEdit_->text(); }
QString PotentialPage::planeC() const { return planeCEdit_->text(); }
QString PotentialPage::uMaxValue() const { return uMaxEdit_->text(); }
QString PotentialPage::uMinValue() const { return uMinEdit_->text(); }
QString PotentialPage::vMaxValue() const { return vMaxEdit_->text(); }
QString PotentialPage::vMinValue() const { return vMinEdit_->text(); }
QString PotentialPage::xMaxValue() const { return xMaxEdit_->text(); }
QString PotentialPage::xMinValue() const { return xMinEdit_->text(); }
QString PotentialPage::yMaxValue() const { return yMaxEdit_->text(); }
QString PotentialPage::yMinValue() const { return yMinEdit_->text(); }
QString PotentialPage::zMaxValue() const { return zMaxEdit_->text(); }
QString PotentialPage::zMinValue() const { return zMinEdit_->text(); }
QString PotentialPage::numtabular() const { return QString("numrtab"); }
QString PotentialPage::tabularkey() const { return QString("rtab"); }
