#include "densitypage.h"

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

DensityPage::DensityPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void DensityPage::buildUi()
{
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Density settings ----
    densitySettingsGroup_ = new QGroupBox(tr("Density settings"), this);

    dataOriginGroup_ = new QGroupBox(tr("Data origin"), densitySettingsGroup_);
    originalDensityRadio_ = new QRadioButton(tr("Original density"), dataOriginGroup_);
    fittedDensityRadio_ = new QRadioButton(tr("Fitted density"), dataOriginGroup_);
    originalDensityRadio_->setToolTip(tr("Original density without fitting"));
    fittedDensityRadio_->setToolTip(tr("Density expansion in atomic contributions"));
    fittedDensityRadio_->setChecked(true);

    auto* dataOriginLayout = new QVBoxLayout(dataOriginGroup_);
    dataOriginLayout->addWidget(originalDensityRadio_);
    dataOriginLayout->addWidget(fittedDensityRadio_);

    densityOptionsGroup_ = new QGroupBox(tr("Density options"), densitySettingsGroup_);
    fullDensityRadio_ = new QRadioButton(tr("Full electron density"), densityOptionsGroup_);
    deformationDensityRadio_ = new QRadioButton(tr("Density deformations"), densityOptionsGroup_);
    lRangeDensityRadio_ = new QRadioButton(tr("Contributions to density"), densityOptionsGroup_);
    fullDensityRadio_->setChecked(true);

    auto* densityOptionsLayout = new QVBoxLayout(densityOptionsGroup_);
    densityOptionsLayout->addWidget(fullDensityRadio_);
    densityOptionsLayout->addWidget(deformationDensityRadio_);
    densityOptionsLayout->addWidget(lRangeDensityRadio_);

    atomicTermsGroup_ = new QGroupBox(tr("Atomic terms"), densitySettingsGroup_);
    lMaxLabel_ = new QLabel(tr("Highest l"), atomicTermsGroup_);
    lMinLabel_ = new QLabel(tr("Lowest l"), atomicTermsGroup_);

    lMaxSpin_ = new QSpinBox(atomicTermsGroup_);
    lMaxSpin_->setRange(0, 25);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);

    lMinSpin_ = new QSpinBox(atomicTermsGroup_);
    lMinSpin_->setRange(0, 25);
    lMinSpin_->setValue(0);
    lMinSpin_->setMaximumWidth(60);
    lMinSpin_->setEnabled(false);

    auto* lmaxLayout = new QHBoxLayout();
    lmaxLayout->addWidget(lMaxLabel_);
    lmaxLayout->addWidget(lMaxSpin_);

    auto* lminLayout = new QHBoxLayout();
    lminLayout->addWidget(lMinLabel_);
    lminLayout->addWidget(lMinSpin_);

    auto* atomicTermsLayout = new QVBoxLayout(atomicTermsGroup_);
    atomicTermsLayout->addLayout(lmaxLayout);
    atomicTermsLayout->addLayout(lminLayout);

    auto* densitySettingsLayout = new QVBoxLayout(densitySettingsGroup_);
    densitySettingsLayout->addWidget(dataOriginGroup_);
    densitySettingsLayout->addWidget(densityOptionsGroup_);
    densitySettingsLayout->addWidget(atomicTermsGroup_);

    // ---- Derivatives ----
    derivativesGroup_ = new QGroupBox(tr("Derivatives"), this);
    gradientCheck_ = new QCheckBox(tr("Gradient"), derivativesGroup_);
    secondDerivativesCheck_ = new QCheckBox(tr("Second derivatives"), derivativesGroup_);
    laplacianCheck_ = new QCheckBox(tr("Laplacian"), derivativesGroup_);

    auto* derivativesLayout = new QVBoxLayout(derivativesGroup_);
    derivativesLayout->addWidget(gradientCheck_);
    derivativesLayout->addWidget(secondDerivativesCheck_);
    derivativesLayout->addWidget(laplacianCheck_);

    // ---- Fragments ----
    fragmentsGroup_ = new QGroupBox(tr("Fragments"), this);
    molecularFragmentsCheck_ = new QCheckBox(tr("Molecule"), fragmentsGroup_);
    atomicFragmentsCheck_ = new QCheckBox(tr("Atomic fragments"), fragmentsGroup_);
    functionalGroupCheck_ = new QCheckBox(tr("Density accumulation"), fragmentsGroup_);
    setMolecularFragments(true);
    setAtomicFragments(false);
    setFunctionalGroup(false);

    atomsGroup_ = new QGroupBox(tr("Atoms"), fragmentsGroup_);
    atomsLabel_ = new QLabel("1,3-5,10,13-17,...", atomsGroup_);
    atomsEdit_ = new QLineEdit(atomsGroup_);

    QRegExp densrx("[1-9][-,\\d]*");
    densityAtomsValidator_ = new QRegExpValidator(densrx, nullptr);
    densityAtomsList_= new QStringList();
    atomsEdit_->setValidator(densityAtomsValidator_);

    auto* atomsLayout = new QVBoxLayout(atomsGroup_);
    atomsLayout->addWidget(atomsLabel_);
    atomsLayout->addWidget(atomsEdit_);

    auto* fragmentsLayout = new QVBoxLayout(fragmentsGroup_);
    fragmentsLayout->addWidget(molecularFragmentsCheck_);
    fragmentsLayout->addWidget(atomicFragmentsCheck_);
    fragmentsLayout->addWidget(functionalGroupCheck_);
    fragmentsLayout->addWidget(atomsGroup_);

    atomsGroup_->setVisible(false);

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

    auto* gridTypeLayout = new QHBoxLayout(gridTypeGroup_);
    gridTypeLayout->addWidget(grid2DRadio_);
    gridTypeLayout->addWidget(grid3DRadio_);

    // 3D
    grid3DGroup_ = new QGroupBox(tr("3D grid"), this);
    setGrid3DVisible(true);

    gridDimensionGroup_ = new QButtonGroup(this);
    gridDimensionGroup_->addButton(grid2DRadio_, DIM_2D);
    gridDimensionGroup_->addButton(grid3DRadio_, DIM_3D);

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

    xMinEdit_->setText("-10.0");
    xMinEdit_->setAlignment(Qt::AlignRight);
    xMinEdit_->setValidator(myDoubleValidator_);

    xMaxEdit_->setText("10.0");
    xMaxEdit_->setAlignment(Qt::AlignRight);
    xMaxEdit_->setValidator(myDoubleValidator_);

    yMinEdit_->setText("-10.0");
    yMinEdit_->setAlignment(Qt::AlignRight);
    yMinEdit_->setValidator(myDoubleValidator_);

    yMaxEdit_->setText("10.0");
    yMaxEdit_->setAlignment(Qt::AlignRight);
    yMaxEdit_->setValidator(myDoubleValidator_);

    zMinEdit_->setText("-10.0");
    zMinEdit_->setAlignment(Qt::AlignRight);
    zMinEdit_->setValidator(myDoubleValidator_);

    zMaxEdit_->setText("10.0");
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

    inf2DLabel_ = new QLabel(tr("Min"), box2DGroup_);
    sup2DLabel_ = new QLabel(tr("Max"), box2DGroup_);

    uLabel_ = new QLabel(tr("u"), box2DGroup_);
    vLabel_ = new QLabel(tr("v"), box2DGroup_);

    uMinEdit_ = new QLineEdit(box2DGroup_);
    uMinEdit_->setText("-10.0");
    uMinEdit_->setAlignment(Qt::AlignRight);
    uMinEdit_->setValidator(myDoubleValidator_);

    uMaxEdit_ = new QLineEdit(box2DGroup_);
    uMaxEdit_->setText("10.0");
    uMaxEdit_->setAlignment(Qt::AlignRight);
    uMaxEdit_->setValidator(myDoubleValidator_);

    vMinEdit_ = new QLineEdit(box2DGroup_);
    vMinEdit_->setText("-10.0");
    vMinEdit_->setAlignment(Qt::AlignRight);
    vMinEdit_->setValidator(myDoubleValidator_);

    vMaxEdit_ = new QLineEdit(box2DGroup_);
    vMaxEdit_->setText("10.0");
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
    mainLayout->addWidget(densitySettingsGroup_);
    mainLayout->addWidget(derivativesGroup_);
    mainLayout->addWidget(fragmentsGroup_);
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

void DensityPage::connectSignals()
{
    connect(originalDensityRadio_, &QRadioButton::toggled,
            this, &DensityPage::dataOriginChanged);

    connect(fittedDensityRadio_, &QRadioButton::toggled,
            this, &DensityPage::dataOriginChanged);

    connect(fullDensityRadio_, &QRadioButton::toggled,
            this, &DensityPage::densityModeChanged);

    connect(deformationDensityRadio_, &QRadioButton::toggled,
            this, &DensityPage::densityModeChanged);

    connect(lRangeDensityRadio_, &QRadioButton::toggled,
            this, &DensityPage::densityModeChanged);

    connect(lMaxSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DensityPage::lMaxChanged);

    connect(lMinSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DensityPage::lMinChanged);

    connect(gradientCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::gradientStateChanged);

    connect(secondDerivativesCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::secondDerivativesStateChanged);

    connect(atomicFragmentsCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::fragmentsStateChanged);

    connect(functionalGroupCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::fragmentsStateChanged);

    connect(molecularFragmentsCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::fragmentsStateChanged);

    connect(gridCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::gridStateChanged);

    connect(gridDimensionGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityPage::gridDimensionChanged);

    connect(surface2DGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityPage::planeModeChanged);

    connect(planesButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityPage::plane2DChanged);

    connect(resolutionButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityPage::resolutionModeChanged);

    connect(xyzCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::xyzTabulationStateChanged);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
            this, &DensityPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &DensityPage::execRequested);

    connect(stopButton_, &QPushButton::clicked,
                runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &DensityPage::openOutputRequested);

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

int DensityPage::getPlaneCase() const { return planeResult.planeCase;}
int DensityPage::lMax() const { return lMaxSpin_->value(); }
int DensityPage::lMaxTop() const { return lMaxSpin_->maximum(); }
int DensityPage::lMin() const { return lMinSpin_->value(); }
int DensityPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int DensityPage::pointsX() const { return pointsXSpin_->value();}
int DensityPage::pointsY() const { return pointsYSpin_->value();}
int DensityPage::pointsZ() const { return pointsZSpin_->value();}
int DensityPage::pointsUV() const { return pointsUVSpin_->value();}
int DensityPage::xyzTableRows() const { return xyzSheet_->tabla->rowCount();}

double DensityPage::deltaU() const { return deltaU_;}
double DensityPage::deltaV() const { return deltaV_;}
double DensityPage::deltaX() const { return deltaX_;}
double DensityPage::deltaY() const { return deltaY_;}
double DensityPage::deltaZ() const { return deltaZ_;}

double DensityPage::computeDelta(const char* key, double ini, double fin) const
{
    return ReadWriteOptions::computeDelta(key, ini, fin, this);
}

bool DensityPage::is2DGrid() const { return grid2DRadio_->isChecked(); }
bool DensityPage::is3DGrid() const { return grid3DRadio_->isChecked(); }
bool DensityPage::isAtomicFragments() const { return atomicFragmentsCheck_->isChecked(); }
bool DensityPage::isCustomResolution() const { return customResolutionRadio_->isChecked(); }
bool DensityPage::isDeformationDensity() const { return deformationDensityRadio_->isChecked(); }
bool DensityPage::isExactDensity() const { return originalDensityRadio_->isChecked(); }
bool DensityPage::isFittedDensity() const { return fittedDensityRadio_->isChecked(); }
bool DensityPage::isFullDensity() const { return fullDensityRadio_->isChecked(); }
bool DensityPage::isFunctionalGroup() const { return functionalGroupCheck_->isChecked(); }
bool DensityPage::isGradient() const { return gradientCheck_->isChecked(); }
bool DensityPage::isGrid() const { return gridCheck_->isChecked(); }
bool DensityPage::isHighResolution() const { return highResolutionRadio_->isChecked(); }
bool DensityPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool DensityPage::isLaplacian() const { return laplacianCheck_->isChecked(); }
bool DensityPage::isLRangeDensity() const { return lRangeDensityRadio_->isChecked(); }
bool DensityPage::isLowResolution() const { return lowResolutionRadio_->isChecked(); }
bool DensityPage::isMediumResolution() const { return mediumResolutionRadio_->isChecked(); }
bool DensityPage::isMolecularFragments() const { return molecularFragmentsCheck_->isChecked(); }
bool DensityPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool DensityPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool DensityPage::isPlane2D() const { return plane2DRadio_->isChecked(); }
bool DensityPage::isPlaneOther() const { return planeOtherRadio_->isChecked(); }
bool DensityPage::isPlaneXY() const { return planeXYRadio_->isChecked(); }
bool DensityPage::isPlaneXZ() const { return planeXZRadio_->isChecked(); }
bool DensityPage::isPlaneYZ() const { return planeYZRadio_->isChecked(); }
bool DensityPage::isSecondDerivatives() const { return secondDerivativesCheck_->isChecked(); }
bool DensityPage::isXYZTabulation() const { return xyzCheck_->isChecked(); }

QVector3D DensityPage::getPlaneWu() const { return planeResult.wu; }
QVector3D DensityPage::getPlaneWv() const { return planeResult.wv; }

void DensityPage::clearSheet() {xyzSheet_->clear(); }
void DensityPage::insertRow(int i) {xyzSheet_->tabla->insertRow(i); }
void DensityPage::resizeSheet(int i) {xyzSheet_->resizeRows(i); }
void DensityPage::setAtomicFragments(bool checked) { atomicFragmentsCheck_->setChecked(checked); }
void DensityPage::setAtomicTermsVisible(bool visible)  { atomicTermsGroup_->setVisible(visible); }
void DensityPage::setAtomsFrameVisible(bool visible) { atomsGroup_->setVisible(visible); }
void DensityPage::setCellValue(const QString& value, int i, int j) {xyzSheet_->setcellvalue(value,i,j); }
void DensityPage::setCustomResolution(bool checked) { resolutionButtonsGroup_->button(CUSTOM_RES)->setChecked(checked); }
void DensityPage::setCustomResolutionEnabled(bool enabled) { customResolutionRadio_->setEnabled(enabled); }
void DensityPage::setDeformationDensity(bool checked) { deformationDensityRadio_->setChecked(checked); }
void DensityPage::setDeltaU(const double value) { deltaU_ = value; }
void DensityPage::setDeltaV(const double value) { deltaV_ = value; }
void DensityPage::setDeltaX(const double value) { deltaX_ = value; }
void DensityPage::setDeltaY(const double value) { deltaY_ = value; }
void DensityPage::setDeltaZ(const double value) { deltaZ_ = value; }
void DensityPage::setDensityOptionsVisible(bool visible)  { densityOptionsGroup_->setVisible(visible); }
void DensityPage::setDerivativesOptionsVisible(bool visible)  { derivativesGroup_->setVisible(visible); }
void DensityPage::setDensityAtomsList(const QStringList& value) { *densityAtomsList_ = value; }
void DensityPage::setEquationX(const QString& value) { equationXEdit_->setText(value); }
void DensityPage::setEquationY(const QString& value) { equationYEdit_->setText(value); }
void DensityPage::setEquationZ(const QString& value) { equationZEdit_->setText(value); }
void DensityPage::setExactDensity(bool checked) { originalDensityRadio_->setChecked(checked); }
void DensityPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void DensityPage::setFittedDensity(bool checked) { fittedDensityRadio_->setChecked(checked); }
void DensityPage::setFragmentOptionsVisible(bool visible)  { fragmentsGroup_->setVisible(visible); }
void DensityPage::setFragmentAtomsText(const QString& value) { atomsEdit_->setText(value); }
void DensityPage::setFullDensity(bool checked) { fullDensityRadio_->setChecked(checked); }
void DensityPage::setFunctionalGroup(bool checked) { functionalGroupCheck_->setChecked(checked); }
void DensityPage::setGradient(bool checked) { gradientCheck_->setChecked(checked); }
void DensityPage::setGrid(bool checked) { gridCheck_->setChecked(checked);}
void DensityPage::setGrid2D(bool checked) { gridDimensionGroup_->button(DIM_2D)->setChecked(checked); }
void DensityPage::setGrid2DVisible(bool visible) { grid2DGroup_->setVisible(visible); grid2DGroup_->setEnabled(visible); }
void DensityPage::setGrid3D(bool checked) { gridDimensionGroup_->button(DIM_3D)->setChecked(checked); }
void DensityPage::setGrid3DVisible(bool visible) { grid3DGroup_->setVisible(visible); grid3DGroup_->setEnabled(visible); }
void DensityPage::setGridButtonsEnabled(bool enabled) { gridDimensionGroup_->button(DIM_2D)->setEnabled(enabled);
                                        gridDimensionGroup_->button(DIM_3D)->setEnabled(enabled); }
void DensityPage::setGridTypeVisible(bool visible) { gridTypeGroup_->setVisible(visible); }
void DensityPage::setHighResolution(bool checked) { highResolutionRadio_->setChecked(checked); }
void DensityPage::setHighResolutionEnabled(bool enabled) { highResolutionRadio_->setEnabled(enabled); }
void DensityPage::setHighResolutionTip(const QString &value) { highResolutionRadio_->setToolTip(value); }
void DensityPage::setLaplacian(bool checked) { laplacianCheck_->setChecked(checked); }
void DensityPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void DensityPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void DensityPage::setLmin(int lmin) { lMinSpin_->setValue(lmin); }
void DensityPage::setLminEnabled(bool enabled) { lMinSpin_->setEnabled(enabled); }
void DensityPage::setLminTop(int ltop) { lMinSpin_->setRange(0,ltop); }
void DensityPage::setLowResolution(bool checked) { resolutionButtonsGroup_->button(LOW_RES)->setChecked(checked); }
void DensityPage::setLowResolutionEnabled(bool enabled) { lowResolutionRadio_->setEnabled(enabled); }
void DensityPage::setLowResolutionTip(const QString &value) { lowResolutionRadio_->setToolTip(value); }
void DensityPage::setMediumResolution(bool checked) { resolutionButtonsGroup_->button(MEDIUM_RES)->setChecked(checked); }
void DensityPage::setMediumResolutionEnabled(bool enabled) { mediumResolutionRadio_->setEnabled(enabled); }
void DensityPage::setMediumResolutionTip(const QString &value) { mediumResolutionRadio_->setToolTip(value); }
void DensityPage::setMolecularFragments(bool checked) { molecularFragmentsCheck_->setChecked(checked); }
void DensityPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void DensityPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void DensityPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void DensityPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void DensityPage::setOutputButtonEnabled(bool enabled) { outputButton_->setEnabled(enabled); }
void DensityPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void DensityPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void DensityPage::setParametricEquationsVisible(bool visible) { parametricEquationsGroup_->setVisible(visible); }
void DensityPage::setPlanes2DVisible(bool visible) { planes2DGroup_->setVisible(visible); }
void DensityPage::setPlane2D(bool checked) { plane2DRadio_->setChecked(checked); }
void DensityPage::setPlaneA(const QString& value) { planeAEdit_->setText(value); }
void DensityPage::setPlaneB(const QString& value) { planeBEdit_->setText(value); }
void DensityPage::setPlaneC(const QString& value) { planeCEdit_->setText(value); }
void DensityPage::setPlaneCoefficientsVisible(bool visible) { planeCoefficientsGroup_->setVisible(visible); }
void DensityPage::setPlaneOther(bool checked) { planeOtherRadio_->setChecked(checked); }
void DensityPage::setPlaneXY(bool checked) { planesButtonsGroup_->button(planeXY)->setChecked(checked); }
void DensityPage::setPlaneXZ(bool checked) { planesButtonsGroup_->button(planeXZ)->setChecked(checked);}
void DensityPage::setPlaneYZ(bool checked) { planesButtonsGroup_->button(planeYZ)->setChecked(checked); }
void DensityPage::setPointsUV(int number) { pointsUVSpin_->setValue(number); }
void DensityPage::setPointsX(int number) { pointsXSpin_->setValue(number); }
void DensityPage::setPointsY(int number) { pointsYSpin_->setValue(number); }
void DensityPage::setPointsZ(int number) { pointsZSpin_->setValue(number); }
void DensityPage::setLRangeDensity(bool checked) { lRangeDensityRadio_->setChecked(checked); }
void DensityPage::setResolutionVisible(bool visible) { resolutionGroup_->setVisible(visible); }
void DensityPage::setResolution2DVisible(bool visible) { resolution2DGroup_->setVisible(visible); }
void DensityPage::setResolution3DVisible(bool visible) { resolution3DGroup_->setVisible(visible); }
void DensityPage::setSecondDerivatives(bool checked) { secondDerivativesCheck_->setChecked(checked); }
void DensityPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void DensityPage::setUmax(const QString& value) { if (value.toDouble() > uMinValue().toDouble()) uMaxEdit_->setText(value); }
void DensityPage::setUmin(const QString& value) { if (value.toDouble() < uMaxValue().toDouble()) uMinEdit_->setText(value); }
void DensityPage::setVmax(const QString& value) { if (value.toDouble() > vMinValue().toDouble()) vMaxEdit_->setText(value); }
void DensityPage::setVmin(const QString& value) { if (value.toDouble() < vMaxValue().toDouble()) vMinEdit_->setText(value); }
void DensityPage::setXmax(const QString& value) { if (value.toDouble() > xMinValue().toDouble()) xMaxEdit_->setText(value); }
void DensityPage::setXmin(const QString& value) { if (value.toDouble() < xMaxValue().toDouble()) xMinEdit_->setText(value); }
void DensityPage::setXYZTabulation(bool checked) { xyzCheck_->setChecked(checked); }
void DensityPage::setXYZEnabled(bool checked) { xyzTable_->setEnabled(checked); }
void DensityPage::setXYZVisible(bool checked) { xyzTable_->setVisible(checked); }
void DensityPage::setYmax(const QString& value) { if (value.toDouble() > yMinValue().toDouble()) yMaxEdit_->setText(value); }
void DensityPage::setYmin(const QString& value) { if (value.toDouble() < yMaxValue().toDouble()) yMinEdit_->setText(value); }
void DensityPage::setZmax(const QString& value) { if (value.toDouble() > zMinValue().toDouble()) zMaxEdit_->setText(value); }
void DensityPage::setZmin(const QString& value) { if (value.toDouble() < zMaxValue().toDouble()) zMinEdit_->setText(value); }


void DensityPage::dataOriginChanged()
{
    if (isExactDensity()) {
        setDensityOptionsVisible(false);
        setAtomicTermsVisible(false);
        setDerivativesOptionsVisible(false);
        setFragmentOptionsVisible(false);
        setFullDensity(true);
    }else{
        setDensityOptionsVisible(true);
        setAtomicTermsVisible(true);
        setDerivativesOptionsVisible(true);
        setFragmentOptionsVisible(true);
    }
}

void DensityPage::densityModeChanged(){
    if (isFullDensity()) {
        setLmin(0);
        setLminEnabled(false);
    }
    else if (isDeformationDensity()){
        setLmin(1);
        setLminEnabled(false);
    }
    else if (isLRangeDensity()){
        setLminEnabled(true);
    }
}


//    Executes external program DAMDEN  (Computes molecular density or deformations from the atomic partition)
void DensityPage::execDamDen()
{
    QString stdOutput;

    QString rootName = "DAMDEN_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMDEN_400.inp";
    QString inputSection = "DAMDENSECT";

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

    const QString exactsuffix = isExactDensity() ? "_exact" : "";

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
                if (isExactDensity()){
                    fileoutstr += "_exact";
                }
                fileoutstr = fileoutstr+"_"+plane.at(densplanecase-1)+"-d.plane";
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
    rename_density_cntfile();
}

void DensityPage::fragmentsStateChanged(int state)
{
    if (state == Qt::Checked || isFunctionalGroup()) {
        setAtomsFrameVisible(true);
    } else {
        setAtomsFrameVisible(false);
    }
}

void DensityPage::gradientStateChanged(int state)
{
    Q_UNUSED(state);
// Extra safeguard: gradient cannot be disabled while second derivatives are active.
    if (isSecondDerivatives() && !isGradient()) {
        QSignalBlocker blocker(gradientCheck_);
        setGradient(true);
    }
}

void DensityPage::gridDimensionChanged(int id)
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

void DensityPage::gridStateChanged(int state){
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


void DensityPage::handleNormalProcessExit()
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

void DensityPage::inputOnlyStateChanged(int state)
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

void DensityPage::lMaxChanged()
{
    if (lMax() < lMin())
        setLmax(lMin());
}

void DensityPage::lMinChanged()
{
    if (lMin() > lMax())
        setLmin(lMax());
}

void DensityPage::loadDefault(){
    setFullDensity(true);
    setLmaxTop(MAX_LEXP);
    setLmax(10);
    setLminTop(MAX_LEXP);
    setLmin(0);
    setFittedDensity(true);

    setMolecularFragments(true);
    setAtomicFragments(false);
    setFunctionalGroup(false);

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

void DensityPage::mpiStateChanged(int state)
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

void DensityPage::plane2DChanged(int id){
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

void DensityPage::planeModeChanged(int id)
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


QString DensityPage::planeSuffix(int planecase){
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

void DensityPage::readFromFile(const std::string& file)
{
    const char* section = "DAMDENSECT";

    // ---- Basic options ----
    setExactDensity(ReadWriteOptions::readRadioButton("lexact", section, file));
    setFittedDensity(!isExactDensity());

    setDeformationDensity(ReadWriteOptions::readRadioButton("ldeform", section, file));
    setLmax(ReadWriteOptions::readSpinBox("lmaxrep", section, file));
    setLmin(ReadWriteOptions::readSpinBox("lminrep", section, file));

    if (lMin() > 1)
        setLRangeDensity(!isDeformationDensity());
    else if (lMin() == 1)
        setDeformationDensity(true);
    else
        setFullDensity(!isDeformationDensity());

    setGradient(ReadWriteOptions::readCheckBox("lgradient", section, file));
    setSecondDerivatives(ReadWriteOptions::readCheckBox("lderiv2", section, file));
    setLaplacian(ReadWriteOptions::readCheckBox("laplacian", section, file));

    setMolecularFragments(ReadWriteOptions::readCheckBox("lmolec", section, file));
    setAtomicFragments(ReadWriteOptions::readCheckBox("latomics", section, file));
    setFunctionalGroup(ReadWriteOptions::readCheckBox("ldensacc", section, file));

    readSelectedAtoms(file, section);

    // ---- Output ----
    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    // ---- Limits ----
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

    // ---- Grid dimension ----
    ReadWriteOptions::readDimensionOption("lgrid2d", section, file, this);

    // ---- Resolution ----

    int resolution;
    if (is3DGrid()){
        resolution = ReadWriteOptions::whatResolution3D(this);
        setGrid3DVisible(true);
        setGrid2DVisible(false);
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

void DensityPage::readSelectedAtoms(const std::string& file, const char* section)
{
    QStringList selectedAtoms;

    const QString nselText =
        QString::fromStdString(CIniFile::GetValue("nsel", section, file));

    bool ok = false;
    const int nsel = nselText.toInt(&ok);

    if (ok && nsel > 0) {
        for (int i = 0; i < nsel; ++i) {
            const QString key = QString("iatomsel(%1)").arg(i + 1);
            const QString atomText =
                QString::fromStdString(CIniFile::GetValue(key.toStdString(), section, file));

            if (!atomText.isEmpty()) {
                selectedAtoms.append(atomText);
            }
        }
    }

    setFragmentAtomsText(selectedAtoms.join(","));
}

void DensityPage::readXYZTable(const std::string& file, const char* section)
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

void DensityPage::rename_density_cntfile(){
    QString aux;
    if (isFullDensity()) {
        if (isExactDensity()){
            aux = "_exact";
        }
        else{
            aux = "";
        }
    }
    else{
        aux = "_deform";
    }
    QString path = projectFolder_;
    QDir directorio(path);
    QStringList filtro;
    int densplanecase = getPlaneCase();
    filtro << projectName_ + aux + "*" + planeSuffix(densplanecase) + "-d.cnt";
    QStringList archivos = directorio.entryList(filtro, QDir::Files);
    QFile filecnt(projectFolder_ + projectName_ + aux + "-d.cnt");
    qDebug() << "filecnt = " << projectFolder_ + projectName_ + aux + "-d.cnt";
    qDebug() << "filecnt.exists: " << filecnt.exists();
    if (filecnt.exists() && planeSuffix(densplanecase) != ""){
        QFile fileold(projectFolder_ + projectName_ + aux + planeSuffix(densplanecase) + "-d.cnt");
        if (fileold.exists())
            fileold.remove();
        filecnt.rename(projectFolder_ + projectName_ + aux + planeSuffix(densplanecase) + "-d.cnt");
    }
    QString planefile = projectName_ + aux + planeSuffix(densplanecase) + "-d.plane";
    if (QFile::exists(projectFolder_ + planefile)){
        QString planebase = planefile;
        planebase.replace(".plane", "");
        for (const QString& archivo : archivos){
            QString fileauxname = archivo;
            fileauxname.replace(".cnt", "");
            if (fileauxname == planebase) continue;
            QString newfilename = fileauxname + ".plane";
            if (QFile::exists(projectFolder_ + newfilename))
                QFile::remove(projectFolder_ + newfilename);
            if (!QFile::copy(projectFolder_ + planefile, projectFolder_ + newfilename)){
                qDebug() << "Error copying " << projectFolder_ + planefile << " to " << projectFolder_ + newfilename;
                continue;
            }
        }
    } else {
        qDebug() << "The file "<< projectFolder_ + planefile << " DOES NOT exist";

    }
}

void DensityPage::resolutionModeChanged(int id){
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

void DensityPage::secondDerivativesStateChanged(int state)
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

void DensityPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}


void DensityPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void DensityPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void DensityPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void DensityPage::setNatoms(int n)
{
    natoms_ = n;
}

void DensityPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void DensityPage::stopCurrentProcess()
{
    runner_->stop();
}

void DensityPage::updatePlaneSelectionFromCoefficients()
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


void DensityPage::writeSelectedAtoms(const std::string& file,
                                     int maxAtomCount,
                                     bool* printwarns,
                                     QString* warns) const
{
    const char* section = "DAMDENSECT";

    QString text = fragmentAtomsText().trimmed();
    if (text.isEmpty()) {
        ReadWriteOptions::writeOption("nsel", section, "0", file, printwarns, warns);
        return;
    }

    QStringList pieces = text.split(",", Qt::SkipEmptyParts);

    QList<int> values;

    for (QString& piece : pieces) {
        QString trimmedPiece = piece.trimmed();
        if (trimmedPiece.isEmpty()) {
            continue;
        }

        // Intervalos tipo 3-7
        if (trimmedPiece.contains('-')) {
            QStringList limits = trimmedPiece.split('-', Qt::SkipEmptyParts);
            if (limits.size() == 2) {
                bool ok1 = false;
                bool ok2 = false;

                int first = limits[0].trimmed().toInt(&ok1);
                int last  = limits[1].trimmed().toInt(&ok2);

                if (ok1 && ok2 && first > 0 && last > 0) {
                    if (first > last) {
                        std::swap(first, last);
                    }

                    for (int v = first; v <= last; ++v) {
                        if (v > 0 && v <= maxAtomCount) {
                            values.append(v);
                        }
                    }
                }
            }
        }
        // Índice individual
        else {
            bool ok = false;
            int v = trimmedPiece.toInt(&ok);
            if (ok && v > 0 && v <= maxAtomCount) {
                values.append(v);
            }
        }
    }

    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());

    int count = 0;
    for (int value : values) {
        ++count;
        const QString key = QString("iatomsel(%1)").arg(count);

        ReadWriteOptions::writeOption(
            key.toStdString().c_str(),
            section,
            QString::number(value),
            file,
            printwarns,
            warns);
    }

    ReadWriteOptions::writeOption(
        "nsel", section, QString::number(count),
        file, printwarns, warns);
}


void DensityPage::writeToFile(const std::string& file,
                              bool* printwarns,
                              QString* warns)
{
    const char* section = "DAMDENSECT";

    auto deltaCalculator =
            [this](const char *key, double ini, double fin) {
                return computeDelta(key, ini, fin);
            };

    CIniFile::DeleteSection(section, file);

    auto writeBool = [&](const char* key, bool value) {
        ReadWriteOptions::writeOption(
            key, section, value ? "T" : "F",
            file, printwarns, warns);
    };

    auto writeText = [&](const char* key, const QString& value) {
        ReadWriteOptions::writeOption(
            key, section, value, file, printwarns, warns);
    };

    // ---- General ----
    writeBool("iswindows", iswindows_);

    if (isExactDensity()) {
        writeBool("lexact", true);
        writeBool("ldeform", false);
        writeBool("lgradient", false);
        writeBool("lderiv2", false);
        writeBool("lmolec", false);
        writeBool("latomics", false);
        writeBool("ldensacc", false);
        writeBool("latomsel", false);
    } else {
        writeBool("lexact", false);
        writeBool("ldeform", isDeformationDensity());

        if (isDeformationDensity())
            writeText("lminrep", "1");
        else
            writeText("lminrep", QString::number(lMin()));

        writeBool("lgradient", isGradient());
        writeBool("lderiv2", isSecondDerivatives());
        writeBool("laplacian", isLaplacian());
        writeBool("lmolec", isMolecularFragments());
        writeBool("latomics", isAtomicFragments());
        writeBool("ldensacc", isFunctionalGroup());
        writeBool("latomsel", isAtomicFragments() || isFunctionalGroup());
    }

    writeText("lmaxrep", QString::number(lMax()));

    // ---- Selected atoms ----
    writeSelectedAtoms(file, natoms_, printwarns, warns);

    // ---- Output ----
    writeText("filename", quoteString(outputPrefix()));

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

    // ---- Bounds ----
    writeBool("lboundsx", false);
    writeText("xboundinf","0.0");
    writeText("xboundsup","0.0");

    writeBool("lboundsy", false);
    writeText("yboundinf","0.0");
    writeText("yboundsup","0.0");

    writeBool("lboundsz", false);
    writeText("zboundinf","0.0");
    writeText("zboundsup","0.0");

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

void DensityPage::xyzTabulationStateChanged(int state)
{
    if (state == Qt::Checked){
        setXYZEnabled(true);
        setXYZVisible(true);
    }else{
        setXYZEnabled(false);
        setXYZVisible(false);
    }
}

QString DensityPage::equationX() const { return equationXEdit_->text(); }
QString DensityPage::equationY() const { return equationYEdit_->text(); }
QString DensityPage::equationZ() const { return equationZEdit_->text(); }
QString DensityPage::getCellValue(int i, int j) const { return xyzSheet_->getcellvalue(i,j); }
QString DensityPage::fragmentAtomsText() const { return atomsEdit_->text(); }
QString DensityPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString DensityPage::planeA() const { return planeAEdit_->text(); }
QString DensityPage::planeB() const { return planeBEdit_->text(); }
QString DensityPage::planeC() const { return planeCEdit_->text(); }
QString DensityPage::uMaxValue() const { return uMaxEdit_->text(); }
QString DensityPage::uMinValue() const { return uMinEdit_->text(); }
QString DensityPage::vMaxValue() const { return vMaxEdit_->text(); }
QString DensityPage::vMinValue() const { return vMinEdit_->text(); }
QString DensityPage::xMaxValue() const { return xMaxEdit_->text(); }
QString DensityPage::xMinValue() const { return xMinEdit_->text(); }
QString DensityPage::yMaxValue() const { return yMaxEdit_->text(); }
QString DensityPage::yMinValue() const { return yMinEdit_->text(); }
QString DensityPage::zMaxValue() const { return zMaxEdit_->text(); }
QString DensityPage::zMinValue() const { return zMinEdit_->text(); }
QString DensityPage::numtabular() const { return QString("numrtab"); }
QString DensityPage::tabularkey() const { return QString("rtab"); }
