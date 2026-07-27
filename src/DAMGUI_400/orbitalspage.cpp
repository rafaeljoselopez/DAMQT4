#include "orbitalspage.h"

#include <QCheckBox>
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

OrbitalsPage::OrbitalsPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void OrbitalsPage::buildUi()
{
    // ---- Import data file ----
    importGroup_ = new QGroupBox(tr(""), this);
    importLabel_ = new QLabel(tr("Import data from") + ":", importGroup_);
    importEdit_ = new QLineEdit(importGroup_);
    importBrowseButton_ = new QToolButton(importGroup_);
    importBrowseButton_->setText(tr("..."));

    auto* importFileLayout = new QHBoxLayout();
    importFileLayout->addWidget(importEdit_);
    importFileLayout->addWidget(importBrowseButton_);

    auto* importLayout = new QVBoxLayout(importGroup_);
    importLayout->addWidget(importLabel_);
    importLayout->addLayout(importFileLayout);

    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Molecular orbitals selection ----
    molecularOrbitalsGroup_ = new QGroupBox(tr("Molecular orbitals selection"), this);
    molecularOrbitalsLabel_ = new QLabel(tr("1,3-5,10,13-17,...") + ":", molecularOrbitalsGroup_);
    molecularOrbitalsEdit_ = new QLineEdit(molecularOrbitalsGroup_);
    QRegExp rx("[1-9][-,\\d]*");
    molecularOrbitalsValidator_ = new QRegExpValidator(rx, nullptr);
    molecularOrbitalsList_ = new QStringList();
    molecularOrbitalsEdit_->setValidator(molecularOrbitalsValidator_);

    auto* molecularOrbitalsLayout = new QVBoxLayout(molecularOrbitalsGroup_);
    molecularOrbitalsLayout->addWidget(molecularOrbitalsLabel_);
    molecularOrbitalsLayout->addWidget(molecularOrbitalsEdit_);

    // ---- Derivatives ----
    derivativesGroup_ = new QGroupBox(tr("Derivatives"), this);
    gradientCheck_ = new QCheckBox(tr("Gradient"), derivativesGroup_);

    auto* derivativesLayout = new QVBoxLayout(derivativesGroup_);
    derivativesLayout->addWidget(gradientCheck_);

    // ---- Grid ----
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
    mainLayout->addWidget(importGroup_);
    mainLayout->addWidget(outputGroup_);
    mainLayout->addWidget(molecularOrbitalsGroup_);
    mainLayout->addWidget(derivativesGroup_);
    mainLayout->addWidget(gridTypeGroup_);
    mainLayout->addWidget(grid2DGroup_);
    mainLayout->addWidget(grid3DGroup_);
    mainLayout->addWidget(resolutionGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();


}

void OrbitalsPage::connectSignals()
{
    connect(importBrowseButton_, &QPushButton::clicked, this, &OrbitalsPage::importFileNameMO);

    connect(molecularOrbitalsEdit_, &QLineEdit::textChanged, this, &OrbitalsPage::molecularOrbitalsChanged);

    connect(gridDimensionGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OrbitalsPage::gridDimensionChanged);

    connect(surface2DGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OrbitalsPage::planeModeChanged);

    connect(planesButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OrbitalsPage::plane2DChanged);

    connect(resolutionButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &OrbitalsPage::resolutionModeChanged);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &OrbitalsPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
                this, &OrbitalsPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &OrbitalsPage::execRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &OrbitalsPage::openOutputRequested);

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

int OrbitalsPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int OrbitalsPage::getPlaneCase() const { return planeResult.planeCase;}
int OrbitalsPage::pointsX() const { return pointsXSpin_->value();}
int OrbitalsPage::pointsY() const { return pointsYSpin_->value();}
int OrbitalsPage::pointsZ() const { return pointsZSpin_->value();}
int OrbitalsPage::pointsUV() const { return pointsUVSpin_->value();}

double OrbitalsPage::computeDelta(const char* key, double ini, double fin) const
{
    return ReadWriteOptions::computeDelta(key, ini, fin, this);
}
double OrbitalsPage::deltaU() const { return deltaU_;}
double OrbitalsPage::deltaV() const { return deltaV_;}
double OrbitalsPage::deltaX() const { return deltaX_;}
double OrbitalsPage::deltaY() const { return deltaY_;}
double OrbitalsPage::deltaZ() const { return deltaZ_;}

bool OrbitalsPage::is2DGrid() const { return gridDimensionGroup_->button(0)->isChecked(); }
bool OrbitalsPage::is3DGrid() const { return gridDimensionGroup_->button(1)->isChecked(); }
bool OrbitalsPage::isCustomResolution() const { return customResolutionRadio_->isChecked(); }
bool OrbitalsPage::isGradient() const { return gradientCheck_->isChecked(); }
bool OrbitalsPage::isHighResolution() const { return highResolutionRadio_->isChecked(); }
bool OrbitalsPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool OrbitalsPage::isLowResolution() const { return lowResolutionRadio_->isChecked(); }
bool OrbitalsPage::isMediumResolution() const { return mediumResolutionRadio_->isChecked(); }
bool OrbitalsPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool OrbitalsPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool OrbitalsPage::isPlane2D() const { return plane2DRadio_->isChecked(); }
bool OrbitalsPage::isPlaneOther() const { return planeOtherRadio_->isChecked(); }
bool OrbitalsPage::isPlaneXY() const { return planeXYRadio_->isChecked(); }
bool OrbitalsPage::isPlaneXZ() const { return planeXZRadio_->isChecked(); }
bool OrbitalsPage::isPlaneYZ() const { return planeYZRadio_->isChecked(); }

QVector3D OrbitalsPage::getPlaneWu() const { return planeResult.wu; }
QVector3D OrbitalsPage::getPlaneWv() const { return planeResult.wv; }

void OrbitalsPage::setCustomResolution(bool checked) { resolutionButtonsGroup_->button(CUSTOM_RES)->setChecked(checked); }
void OrbitalsPage::setCustomResolutionEnabled(bool enabled) { customResolutionRadio_->setEnabled(enabled); }
void OrbitalsPage::setDerivativesOptionsVisible(bool visible)  { derivativesGroup_->setVisible(visible); }
void OrbitalsPage::setDeltaU(const double value) { deltaU_ = value; }
void OrbitalsPage::setDeltaV(const double value) { deltaV_ = value; }
void OrbitalsPage::setDeltaX(const double value) { deltaX_ = value; }
void OrbitalsPage::setDeltaY(const double value) { deltaY_ = value; }
void OrbitalsPage::setDeltaZ(const double value) { deltaZ_ = value; }
void OrbitalsPage::setEquationX(const QString& value) { equationXEdit_->setText(value); }
void OrbitalsPage::setEquationY(const QString& value) { equationYEdit_->setText(value); }
void OrbitalsPage::setEquationZ(const QString& value) { equationZEdit_->setText(value); }
void OrbitalsPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void OrbitalsPage::setGradient(bool checked) { gradientCheck_->setChecked(checked); }
void OrbitalsPage::setGrid2D(bool checked) { gridDimensionGroup_->button(DIM_2D)->setChecked(checked); }
void OrbitalsPage::setGrid2DVisible(bool visible) { grid2DGroup_->setVisible(visible); grid2DGroup_->setEnabled(visible); }
void OrbitalsPage::setGrid3D(bool checked) { gridDimensionGroup_->button(DIM_3D)->setChecked(checked); }
void OrbitalsPage::setGrid3DVisible(bool visible) { grid3DGroup_->setVisible(visible); grid3DGroup_->setEnabled(visible); }
void OrbitalsPage::setHighResolution(bool checked) { resolutionButtonsGroup_->button(HIGH_RES)->setChecked(checked); }
void OrbitalsPage::setHighResolutionEnabled(bool enabled) { highResolutionRadio_->setEnabled(enabled); }
void OrbitalsPage::setHighResolutionTip(const QString &value) { highResolutionRadio_->setToolTip(value); }
void OrbitalsPage::setImport(const QString& value) { importEdit_->setText(value); }
void OrbitalsPage::setLowResolution(bool checked) { resolutionButtonsGroup_->button(LOW_RES)->setChecked(checked); }
void OrbitalsPage::setLowResolutionEnabled(bool enabled) { lowResolutionRadio_->setEnabled(enabled); }
void OrbitalsPage::setLowResolutionTip(const QString &value) { lowResolutionRadio_->setToolTip(value); }
void OrbitalsPage::setMediumResolution(bool checked) { resolutionButtonsGroup_->button(MEDIUM_RES)->setChecked(checked); }
void OrbitalsPage::setMediumResolutionEnabled(bool enabled) { mediumResolutionRadio_->setEnabled(enabled); }
void OrbitalsPage::setMediumResolutionTip(const QString &value) { mediumResolutionRadio_->setToolTip(value); }
void OrbitalsPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void OrbitalsPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void OrbitalsPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void OrbitalsPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void OrbitalsPage::setOutputButtonEnabled(bool enabled) { outputButton_->setEnabled(enabled); }
void OrbitalsPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void OrbitalsPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void OrbitalsPage::setParametricEquationsVisible(bool visible) { parametricEquationsGroup_->setVisible(visible); }
void OrbitalsPage::setPlanes2DVisible(bool visible) { planes2DGroup_->setVisible(visible); }
void OrbitalsPage::setPlane2D(bool checked) { plane2DRadio_->setChecked(checked); }
void OrbitalsPage::setPlaneA(const QString& value) { planeAEdit_->setText(value); }
void OrbitalsPage::setPlaneB(const QString& value) { planeBEdit_->setText(value); }
void OrbitalsPage::setPlaneC(const QString& value) { planeCEdit_->setText(value); }
void OrbitalsPage::setPlaneCoefficientsVisible(bool visible) { planeCoefficientsGroup_->setVisible(visible); }
void OrbitalsPage::setPlaneOther(bool checked) { planeOtherRadio_->setChecked(checked); }
void OrbitalsPage::setPlaneXY(bool checked) { planesButtonsGroup_->button(planeXY)->setChecked(checked); }
void OrbitalsPage::setPlaneXZ(bool checked) { planesButtonsGroup_->button(planeXZ)->setChecked(checked);}
void OrbitalsPage::setPlaneYZ(bool checked) { planesButtonsGroup_->button(planeYZ)->setChecked(checked); }
void OrbitalsPage::setPointsUV(int number) { pointsUVSpin_->setValue(number); }
void OrbitalsPage::setPointsX(int number) { pointsXSpin_->setValue(number); }
void OrbitalsPage::setPointsY(int number) { pointsYSpin_->setValue(number); }
void OrbitalsPage::setPointsZ(int number) { pointsZSpin_->setValue(number); }
void OrbitalsPage::setResolution2DVisible(bool visible) { resolution2DGroup_->setVisible(visible); }
void OrbitalsPage::setResolution3DVisible(bool visible) { resolution3DGroup_->setVisible(visible); }
void OrbitalsPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void OrbitalsPage::setUmax(const QString& value) { if (value.toDouble() > uMinValue().toDouble()) uMaxEdit_->setText(value); }
void OrbitalsPage::setUmin(const QString& value) { if (value.toDouble() < uMaxValue().toDouble()) uMinEdit_->setText(value); }
void OrbitalsPage::setVmax(const QString& value) { if (value.toDouble() > vMinValue().toDouble()) vMaxEdit_->setText(value); }
void OrbitalsPage::setVmin(const QString& value) { if (value.toDouble() < vMaxValue().toDouble()) vMinEdit_->setText(value); }
void OrbitalsPage::setXmax(const QString& value) { if (value.toDouble() > xMinValue().toDouble()) xMaxEdit_->setText(value); }
void OrbitalsPage::setXmin(const QString& value) { if (value.toDouble() < xMaxValue().toDouble()) xMinEdit_->setText(value); }
void OrbitalsPage::setYmax(const QString& value) { if (value.toDouble() > yMinValue().toDouble()) yMaxEdit_->setText(value); }
void OrbitalsPage::setYmin(const QString& value) { if (value.toDouble() < yMaxValue().toDouble()) yMinEdit_->setText(value); }
void OrbitalsPage::setZmax(const QString& value) { if (value.toDouble() > zMinValue().toDouble()) zMaxEdit_->setText(value); }
void OrbitalsPage::setZmin(const QString& value) { if (value.toDouble() < zMaxValue().toDouble()) zMinEdit_->setText(value); }


//    Executes external program DAMORB  (Tabulates molecular orbitals on a grid)
void OrbitalsPage::execDamOrb()
{
    QString stdOutput;

    QString rootName = "DAMORB_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMORB_400.inp";
    QString inputSection = "DAMORBSECT";

    const QString projectFile =
            QDir(projectFolder_).filePath(projectName_ + ".damproj");
    if (importFile().isEmpty()){
        QMessageBox::warning(this, tr("DAMQT"),tr("Choose a file with molecular orbitals coefficients"));
        return;
    }

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

    request.suffix = request.runMpi ? "_mpi" : "";

    lastOutputFileName_ =
        QDir(projectFolder_).filePath(
            request.outputPrefix + "-" + request.rootName + request.suffix + ".out"
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
                fileoutstr = fileoutstr+"_"+plane.at(densplanecase-1)+"-o.plane";
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

}

void OrbitalsPage::gridDimensionChanged(int id)
{
    int resolution;
    if (id == 0){
        setGrid2DVisible(true);
        setGrid3DVisible(false);
        setResolution3DVisible(false);
        setGradient(false);
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
        setGrid3DVisible(true);
        setGrid2DVisible(false);
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
}

void OrbitalsPage::handleNormalProcessExit()
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

/* Import name of a file with molecular orbitals*/
void OrbitalsPage::importFileNameMO()
{
    QFileDialog filedialog(this);
    filedialog.setDirectory(projectFolder_);
    filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString fullPath = filedialog.getOpenFileName(this,tr("Open file ..."),projectFolder_,
            tr("Import data from")+" (*.orb *.GAorba *.GAorbb *.SLorba *.SLorbb);;"+
            tr("All files")+" (*)");
    if (fullPath.length()==0){
        return;
    }
    QFileInfo fileInfo(fullPath);
    importEdit_->setText(fileInfo.fileName());
}

void OrbitalsPage::inputOnlyStateChanged(int state)
{
    if (!(state == Qt::Checked) && is3DGrid()){
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

void OrbitalsPage::loadDefault(){
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
    setGrid3D(true);
    setGrid3DVisible(true);
    setGrid2DVisible(false);

    setResolution2DVisible(false);
    setResolution3DVisible(false);

    setImport("");
    molecularOrbitalsList_->clear();
    molecularOrbitalsEdit_->setText("");
}

void OrbitalsPage::molecularOrbitalsChanged(){
    QString string = QString(molecularOrbitalsEdit_->text());
    QStringList stringlist1 = string.split(",");
    molecularOrbitalsList_->clear();
    for (int i = 0 ; i < stringlist1.length() ; ++i){
        if (stringlist1.at(i).length() != 0){
            if (stringlist1.at(i).contains(QString("-"))){
                QStringList stringlist2 = stringlist1.at(i).split("-");
                if (stringlist2.at(0).length() != 0 && stringlist2.at(1).length() != 0
                        && stringlist2.at(1).toInt() > stringlist2.at(0).toInt()){
                    for (int k = stringlist2.at(0).toInt() ; k <= stringlist2.at(1).toInt() ; k++){
                        molecularOrbitalsList_->append(QString("%1").arg(k));
                    }
                }
            }
            else{
                molecularOrbitalsList_->append(stringlist1.at(i));
            }
        }
    }
    molecularOrbitalsList_->sort();
    molecularOrbitalsList_->removeDuplicates();
}

void OrbitalsPage::mpiStateChanged(int state)
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

void OrbitalsPage::plane2DChanged(int id){
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

void OrbitalsPage::planeModeChanged(int id)
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

void OrbitalsPage::readFromFile(const std::string& file)
{
    const char* section = "DAMORBSECT";

    importEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("fileMOname", section, file)
    );
    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    setGradient(ReadWriteOptions::readCheckBox("lgradient", section, file));

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
        setGrid3DVisible(true);
    }
    else{
        resolution = ReadWriteOptions::whatResolution2D(this);
        setGrid2DVisible(true);
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
    grid2DGroup_->setVisible(is2DGrid());
    grid3DGroup_->setVisible(is3DGrid());
    if (isCustomResolution()){
        setResolution2DVisible(is2DGrid());
        setResolution3DVisible(is3DGrid());
    }
    mpiGroup_->setVisible(is3DGrid());

    int norb;
    ReadWriteOptions::readInt("norbs", section, &norb, file);
    QString qv;
    std::string key;
    if (norb > 0){
        molecularOrbitalsList_->clear();
        for(int i = 0 ; i < norb ; ++i){
            key = "iorbsinp(" + std::to_string(i + 1) + ")";
            qv = ReadWriteOptions::readTextToLineEdit(key.c_str(),section,file);
            if (!qv.isEmpty()){
                molecularOrbitalsList_->append(qv);
            }
        }
        molecularOrbitalsEdit_->setText(molecularOrbitalsList_->join(","));
    }
}

void OrbitalsPage::resolutionModeChanged(int id){
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

void OrbitalsPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void OrbitalsPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void OrbitalsPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void OrbitalsPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void OrbitalsPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void OrbitalsPage::setProjectFolder(const QString& value)
{
    projectFolder_ = value;
}


void OrbitalsPage::stopCurrentProcess()
{
    runner_->stop();
}

void OrbitalsPage::updatePlaneSelectionFromCoefficients()
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

//void OrbitalsPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool* printwarns,
//                                QString* warns,
//                                const DeltaCalculator& deltaCalculator)
void OrbitalsPage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMORBSECT";

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

    writeText("fileMOname",quoteString(importFile()));

    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

    // ---- Gradient ----
    writeBool("lgradient", isGradient());

    // ---- List of orbitals indices ----
    if (molecularOrbitalsList_->count() < 1){
        writeText("norbs", QString::number(1));
        writeText("iorbsinp(1)", QString::number(1));
    }
    else{
        QList <int> list;
        for (int i = 0 ; i < molecularOrbitalsList_->count() ; ++i){
            list.append(molecularOrbitalsList_->at(i).toInt());
        }
        std::sort(list.begin(), list.end());
        int norbs = 0;
        for (int k=0 ; k < list.count() ; k++){
            if (list[k] <= 0 ) continue;
            norbs++;
            writeText(QString("%1%2%3").arg("iorbsinp(").arg(norbs).arg(")").toStdString().c_str(),
                      QString::number(list[k]));
        }
        writeText("norbs", QString("%1").arg(norbs));
    }

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

}

QString OrbitalsPage::equationX() const { return equationXEdit_->text(); }
QString OrbitalsPage::equationY() const { return equationYEdit_->text(); }
QString OrbitalsPage::equationZ() const { return equationZEdit_->text(); }
QString OrbitalsPage::importFile() const { return importEdit_->text(); }
QString OrbitalsPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString OrbitalsPage::planeA() const { return planeAEdit_->text(); }
QString OrbitalsPage::planeB() const { return planeBEdit_->text(); }
QString OrbitalsPage::planeC() const { return planeCEdit_->text(); }
QString OrbitalsPage::uMaxValue() const { return uMaxEdit_->text(); }
QString OrbitalsPage::uMinValue() const { return uMinEdit_->text(); }
QString OrbitalsPage::vMaxValue() const { return vMaxEdit_->text(); }
QString OrbitalsPage::vMinValue() const { return vMinEdit_->text(); }
QString OrbitalsPage::xMaxValue() const { return xMaxEdit_->text(); }
QString OrbitalsPage::xMinValue() const { return xMinEdit_->text(); }
QString OrbitalsPage::yMaxValue() const { return yMaxEdit_->text(); }
QString OrbitalsPage::yMinValue() const { return yMinEdit_->text(); }
QString OrbitalsPage::zMaxValue() const { return zMaxEdit_->text(); }
QString OrbitalsPage::zMinValue() const { return zMinEdit_->text(); }


