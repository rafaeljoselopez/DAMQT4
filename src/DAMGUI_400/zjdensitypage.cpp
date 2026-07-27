#include "zjdensitypage.h"

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

#include "externalprogramrunner.h"
#include "readwriteoptions.h"
#include "IniFile.h"

#ifndef MAX_NUM_PROCESSORS
#define MAX_NUM_PROCESSORS 128
#endif

static QString quoteString(QString s)
{
    s.remove("\"");
    return "\"" + s + "\"";
}

ZJDensityPage::ZJDensityPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void ZJDensityPage::buildUi()
{
    
    // ---- Import data ----
    importDataGroup_ = new QGroupBox(tr("Import data from:"), this);
    importDataEdit_ = new QLineEdit(importDataGroup_);
    importDataEdit_->setMaximumWidth(280);
    importDataButton_ =  new QToolButton(importDataGroup_);
    importDataButton_->setText(tr("..."));
    
    auto* importDataLayout = new QHBoxLayout(importDataGroup_);
    importDataLayout->addWidget(importDataEdit_);
    importDataLayout->addWidget(importDataButton_);
    
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Density expansion terms ----
    densityExpansionGroup_ = new QGroupBox(tr("Density expansion terms"), this);
    
    lkRangesGroup_ = new QGroupBox("", this);
    lMaxLabel_ = new QLabel(tr("Highest l"), lkRangesGroup_);
    lMinLabel_ = new QLabel(tr("Lowest l"), lkRangesGroup_);
    kMaxLabel_ = new QLabel(tr("Highest k"), lkRangesGroup_);

    lMaxSpin_ = new QSpinBox(lkRangesGroup_);
    lMaxSpin_->setRange(0, 25);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);

    lMinSpin_ = new QSpinBox(lkRangesGroup_);
    lMinSpin_->setRange(0, lMaxSpin_->value());
    lMinSpin_->setValue(0);
    lMinSpin_->setMaximumWidth(60);
    
    kMaxSpin_ = new QSpinBox(lkRangesGroup_);
    kMaxSpin_->setRange(0, 25);
    kMaxSpin_->setValue(10);
    kMaxSpin_->setMaximumWidth(60);

    auto* lkrangesLayout = new  QGridLayout();
    lkrangesLayout->addWidget(lMaxLabel_,0,0);
    lkrangesLayout->addWidget(lMaxSpin_,0,1);
    lkrangesLayout->addWidget(lMinLabel_,1,0);
    lkrangesLayout->addWidget(lMinSpin_,1,1);
    lkrangesLayout->addWidget(kMaxLabel_,2,0);
    lkrangesLayout->addWidget(kMaxSpin_,2,1);

    auto* rangesLayout = new QHBoxLayout(lkRangesGroup_);
    rangesLayout->addStretch();
    rangesLayout->addLayout(lkrangesLayout);
    rangesLayout->addStretch();

    
    allFunctionsRadio_ = new QRadioButton(tr("Choose all functions"), densityExpansionGroup_);
    
    chooseLIndicesRadio_ = new QRadioButton(tr("Choose l indices"), densityExpansionGroup_);
    chooseLIndicesLabel_ = new QLabel(tr("Select indices: 1, 3-5, 10, 13-17,..."), densityExpansionGroup_);
    chooseLIndicesLabel_->setVisible(false);
    QRegExp rxL("[0-9][-,\\d]*");
    chooseLIndicesValidator_ = new QRegExpValidator(rxL, nullptr);
    
    chooseKIndicesRadio_ = new QRadioButton(tr("Choose k indices"), densityExpansionGroup_);
    chooseKIndicesLabel_ = new QLabel(tr("Select indices: 1, 3-5, 10, 13-17,..."), densityExpansionGroup_);
    chooseKIndicesLabel_->setVisible(false);
    QRegExp rxK("[0-9][-,\\d]*");
    chooseKIndicesValidator_ = new QRegExpValidator(rxK, nullptr);
    
    chooseLKIndicesRadio_ = new QRadioButton(tr("Choose pairs of (l,k) indices"), densityExpansionGroup_);
    chooseLKIndicesLabel_ = new QLabel(tr("Select pairs of indices: (0,0), (1,2),..."), densityExpansionGroup_);
    chooseLKIndicesLabel_->setVisible(false);
    QRegExp rxLK("^\\(?[0-9]*[\\(,\\d*\\)]*");
    chooseLKIndicesValidator_ = new QRegExpValidator(rxLK, nullptr);
    
    chooseLKMIndicesRadio_ = new QRadioButton(tr("Choose triads of (l,k,m) indices"), densityExpansionGroup_);
    chooseLKMIndicesLabel_ = new QLabel(tr("Select triads of indices: (0,0,0), (1,2,-1),..."), densityExpansionGroup_);
    chooseLKMIndicesLabel_->setVisible(false);
    QRegExp rxLKM("^\\(?[0-9]*[\\(,-\\d*\\)]*");
    chooseLKMIndicesValidator_ = new QRegExpValidator(rxLKM, nullptr);

    chooseIndicesEdit_ = new QLineEdit(densityExpansionGroup_);
    chooseIndicesEdit_->setVisible(false);
    chooseIndicesList_ = new QStringList();

    chooseIndicesGroup_ = new QButtonGroup(this);
    chooseIndicesGroup_->addButton(allFunctionsRadio_, ALLINDICES);
    chooseIndicesGroup_->addButton(chooseLIndicesRadio_, LINDICES);
    chooseIndicesGroup_->addButton(chooseKIndicesRadio_, KINDICES);
    chooseIndicesGroup_->addButton(chooseLKIndicesRadio_, LKINDICES);
    chooseIndicesGroup_->addButton(chooseLKMIndicesRadio_, LKMINDICES);
    
    auto* densityExpansionLayout = new QVBoxLayout(densityExpansionGroup_);
    densityExpansionLayout->addWidget(allFunctionsRadio_);
    densityExpansionLayout->addWidget(lkRangesGroup_);
    densityExpansionLayout->addWidget(chooseLIndicesRadio_);
    densityExpansionLayout->addWidget(chooseKIndicesRadio_);
    densityExpansionLayout->addWidget(chooseLKIndicesRadio_);
    densityExpansionLayout->addWidget(chooseLKMIndicesRadio_);
    densityExpansionLayout->addWidget(chooseLIndicesLabel_);
    densityExpansionLayout->addWidget(chooseKIndicesLabel_);
    densityExpansionLayout->addWidget(chooseLKIndicesLabel_);
    densityExpansionLayout->addWidget(chooseLKMIndicesLabel_);
    densityExpansionLayout->addWidget(chooseIndicesEdit_);

    // ---- Derivatives ----
    derivativesGroup_ = new QGroupBox(tr("Derivatives"), this);
    gradientCheck_ = new QCheckBox(tr("Gradient"), derivativesGroup_);

    auto* derivativesLayout = new QVBoxLayout(derivativesGroup_);
    derivativesLayout->addWidget(gradientCheck_);

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
    grid3DGroup_->setVisible(true);

    gridDimensionGroup_ = new QButtonGroup(this);
    gridDimensionGroup_->addButton(grid2DRadio_, DIM_2D);
    gridDimensionGroup_->addButton(grid3DRadio_, DIM_3D);

    box3DGroup_ = new QGroupBox(tr("3D boundaries"), grid3DGroup_);
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
    grid2DGroup_->setVisible(false);

    box2DGroup_ = new QGroupBox(tr("2D boundaries"), grid2DGroup_);
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

    setOutputGroupEnabled(false);
    setDensityExpansionEnabled(false);
    setDerivativesEnabled(false);
    setGridEnabled(false);
    setMpiCheckEnabled(false);
    setExecEnabled(false);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(importDataGroup_);
    mainLayout->addWidget(outputGroup_);
    mainLayout->addWidget(densityExpansionGroup_);
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

void ZJDensityPage::connectSignals()
{
    connect(importDataEdit_, &QLineEdit::textChanged,
            this, &ZJDensityPage::importZJDensityChanged);

    connect(importDataButton_, &QToolButton::clicked,
            this, &ZJDensityPage::importZJDensity);

    connect(lMaxSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ZJDensityPage::lMaxChanged);

    connect(lMinSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ZJDensityPage::lMinChanged);
    
    connect(chooseIndicesGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ZJDensityPage::indicesChanged);

    connect(gridCheck_, &QCheckBox::stateChanged,
            this, &ZJDensityPage::gridStateChanged);

    connect(gridDimensionGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ZJDensityPage::gridDimensionChanged);

    connect(surface2DGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ZJDensityPage::planeModeChanged);

    connect(planesButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ZJDensityPage::plane2DChanged);

    connect(resolutionButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ZJDensityPage::resolutionModeChanged);

    connect(xyzCheck_, &QCheckBox::stateChanged,
            this, &ZJDensityPage::xyzTabulationStateChanged);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &ZJDensityPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
            this, &ZJDensityPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &ZJDensityPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &ZJDensityPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
                runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &ZJDensityPage::openOutputRequested);

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

int ZJDensityPage::kMax() const { return kMaxSpin_->value(); }
int ZJDensityPage::lMax() const { return lMaxSpin_->value(); }
int ZJDensityPage::lMaxTop() const { return lMaxSpin_->maximum(); }
int ZJDensityPage::lMin() const { return lMinSpin_->value(); }
int ZJDensityPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int ZJDensityPage::pointsX() const { return pointsXSpin_->value();}
int ZJDensityPage::pointsY() const { return pointsYSpin_->value();}
int ZJDensityPage::pointsZ() const { return pointsZSpin_->value();}
int ZJDensityPage::pointsUV() const { return pointsUVSpin_->value();}
int ZJDensityPage::planecase() const { return planeResult.planeCase;}
int ZJDensityPage::xyzTableRows() const { return xyzSheet_->tabla->rowCount();}

double ZJDensityPage::deltaU() const { return deltaU_;}
double ZJDensityPage::deltaV() const { return deltaV_;}
double ZJDensityPage::deltaX() const { return deltaX_;}
double ZJDensityPage::deltaY() const { return deltaY_;}
double ZJDensityPage::deltaZ() const { return deltaZ_;}

double ZJDensityPage::computeDelta(const char* key, double ini, double fin) const
{
    return ReadWriteOptions::computeDelta(key, ini, fin, this);
}

bool ZJDensityPage::is2DGrid() const { return grid2DRadio_->isChecked(); }
bool ZJDensityPage::is3DGrid() const { return grid3DRadio_->isChecked(); }
bool ZJDensityPage::isCustomResolution() const { return customResolutionRadio_->isChecked(); }
bool ZJDensityPage::isGradient() const { return gradientCheck_->isChecked(); }
bool ZJDensityPage::isGrid() const { return gridCheck_->isChecked(); }
bool ZJDensityPage::isHighResolution() const { return highResolutionRadio_->isChecked(); }
bool ZJDensityPage::isJacobi() const { return importDataEdit_->text().contains("jacobi"); }
bool ZJDensityPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool ZJDensityPage::isLowResolution() const { return lowResolutionRadio_->isChecked(); }
bool ZJDensityPage::isMediumResolution() const { return mediumResolutionRadio_->isChecked(); }
bool ZJDensityPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool ZJDensityPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool ZJDensityPage::isPlane2D() const { return plane2DRadio_->isChecked(); }
bool ZJDensityPage::isPlaneOther() const { return planeOtherRadio_->isChecked(); }
bool ZJDensityPage::isPlaneXY() const { return planeXYRadio_->isChecked(); }
bool ZJDensityPage::isPlaneXZ() const { return planeXZRadio_->isChecked(); }
bool ZJDensityPage::isPlaneYZ() const { return planeYZRadio_->isChecked(); }
bool ZJDensityPage::isXYZTabulation() const { return xyzCheck_->isChecked(); }
bool ZJDensityPage::isZernike() const { return importDataEdit_->text().contains("zernike"); }

QVector3D ZJDensityPage::getPlaneWu() const { return planeResult.wu; }
QVector3D ZJDensityPage::getPlaneWv() const { return planeResult.wv; }

void ZJDensityPage::clearSheet() {xyzSheet_->clear(); }
void ZJDensityPage::insertRow(int i) {xyzSheet_->tabla->insertRow(i); }
void ZJDensityPage::resizeSheet(int i) {xyzSheet_->resizeRows(i); }
void ZJDensityPage::setCellValue(const QString& value, int i, int j) {xyzSheet_->setcellvalue(value,i,j); }
void ZJDensityPage::setChooseAllIndices(bool checked) { allFunctionsRadio_->setChecked(checked);}
void ZJDensityPage::setChooseKIndices(bool checked) { chooseKIndicesRadio_->setChecked(checked);}
void ZJDensityPage::setChooseLIndices(bool checked) { chooseLIndicesRadio_->setChecked(checked);}
void ZJDensityPage::setChooseLKIndices(bool checked) { chooseLKIndicesRadio_->setChecked(checked);}
void ZJDensityPage::setChooseLKMIndices(bool checked) { chooseLKMIndicesRadio_->setChecked(checked);}
void ZJDensityPage::setCustomResolution(bool checked) { resolutionButtonsGroup_->button(CUSTOM_RES)->setChecked(checked); }
void ZJDensityPage::setCustomResolutionEnabled(bool enabled) { customResolutionRadio_->setEnabled(enabled); }
void ZJDensityPage::setDeltaU(const double value) { deltaU_ = value; }
void ZJDensityPage::setDeltaV(const double value) { deltaV_ = value; }
void ZJDensityPage::setDeltaX(const double value) { deltaX_ = value; }
void ZJDensityPage::setDeltaY(const double value) { deltaY_ = value; }
void ZJDensityPage::setDeltaZ(const double value) { deltaZ_ = value; }
void ZJDensityPage::setDensityExpansionEnabled(bool enabled) { densityExpansionGroup_->setEnabled(enabled); }
void ZJDensityPage::setDerivativesEnabled(bool enabled)  { derivativesGroup_->setEnabled(enabled); }
void ZJDensityPage::setEquationX(const QString& value) { equationXEdit_->setText(value); }
void ZJDensityPage::setEquationY(const QString& value) { equationYEdit_->setText(value); }
void ZJDensityPage::setEquationZ(const QString& value) { equationZEdit_->setText(value); }
void ZJDensityPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void ZJDensityPage::setGradient(bool checked) { gradientCheck_->setChecked(checked); }
void ZJDensityPage::setGrid(bool checked) { gridCheck_->setChecked(checked);}
void ZJDensityPage::setGrid2D(bool checked) { gridDimensionGroup_->button(DIM_2D)->setChecked(checked); }
void ZJDensityPage::setGrid2DVisible(bool visible) { grid2DGroup_->setVisible(visible); grid2DGroup_->setEnabled(visible); }
void ZJDensityPage::setGrid3D(bool checked) { gridDimensionGroup_->button(DIM_3D)->setChecked(checked); }
void ZJDensityPage::setGrid3DVisible(bool visible) { grid3DGroup_->setVisible(visible); grid3DGroup_->setEnabled(visible); }
void ZJDensityPage::setGridButtonsEnabled(bool enabled) { gridDimensionGroup_->button(DIM_2D)->setEnabled(enabled);
                                        gridDimensionGroup_->button(DIM_3D)->setEnabled(enabled); }
void ZJDensityPage::setGridEnabled(bool enabled) { gridGroup_->setEnabled(enabled);}
void ZJDensityPage::setGridTypeVisible(bool visible) { gridTypeGroup_->setVisible(visible); }
void ZJDensityPage::setHighResolution(bool checked) { highResolutionRadio_->setChecked(checked); }
void ZJDensityPage::setHighResolutionEnabled(bool enabled) { highResolutionRadio_->setEnabled(enabled); }
void ZJDensityPage::setHighResolutionTip(const QString &value) { highResolutionRadio_->setToolTip(value); }
void ZJDensityPage::setImport(const QString& value) { importDataEdit_->setText(value); }
void ZJDensityPage::setKmax(int lmax) { kMaxSpin_->setValue(lmax); }
void ZJDensityPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void ZJDensityPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void ZJDensityPage::setLmin(int lmin) { lMinSpin_->setValue(lmin); }
void ZJDensityPage::setLminEnabled(bool enabled) { lMinSpin_->setEnabled(enabled); }
void ZJDensityPage::setLminTop(int ltop) { lMinSpin_->setRange(0,ltop); }
void ZJDensityPage::setLowResolution(bool checked) { resolutionButtonsGroup_->button(LOW_RES)->setChecked(checked); }
void ZJDensityPage::setLowResolutionEnabled(bool enabled) { lowResolutionRadio_->setEnabled(enabled); }
void ZJDensityPage::setLowResolutionTip(const QString &value) { lowResolutionRadio_->setToolTip(value); }
void ZJDensityPage::setMediumResolution(bool checked) { resolutionButtonsGroup_->button(MEDIUM_RES)->setChecked(checked); }
void ZJDensityPage::setMediumResolutionEnabled(bool enabled) { mediumResolutionRadio_->setEnabled(enabled); }
void ZJDensityPage::setMediumResolutionTip(const QString &value) { mediumResolutionRadio_->setToolTip(value); }
void ZJDensityPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void ZJDensityPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void ZJDensityPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void ZJDensityPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void ZJDensityPage::setOutputGroupEnabled(bool enabled) { outputGroup_->setEnabled(enabled); }
void ZJDensityPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void ZJDensityPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void ZJDensityPage::setParametricEquationsVisible(bool visible) { parametricEquationsGroup_->setVisible(visible); }
void ZJDensityPage::setPlanes2DVisible(bool visible) { planes2DGroup_->setVisible(visible); }
void ZJDensityPage::setPlane2D(bool checked) { plane2DRadio_->setChecked(checked); }
void ZJDensityPage::setPlaneA(const QString& value) { planeAEdit_->setText(value); }
void ZJDensityPage::setPlaneB(const QString& value) { planeBEdit_->setText(value); }
void ZJDensityPage::setPlaneC(const QString& value) { planeCEdit_->setText(value); }
void ZJDensityPage::setPlaneCoefficientsVisible(bool visible) { planeCoefficientsGroup_->setVisible(visible); }
void ZJDensityPage::setPlaneOther(bool checked) { planeOtherRadio_->setChecked(checked); }
void ZJDensityPage::setPlaneXY(bool checked) { planesButtonsGroup_->button(planeXY)->setChecked(checked); }
void ZJDensityPage::setPlaneXZ(bool checked) { planesButtonsGroup_->button(planeXZ)->setChecked(checked);}
void ZJDensityPage::setPlaneYZ(bool checked) { planesButtonsGroup_->button(planeYZ)->setChecked(checked); }
void ZJDensityPage::setPointsUV(int number) { pointsUVSpin_->setValue(number); }
void ZJDensityPage::setPointsX(int number) { pointsXSpin_->setValue(number); }
void ZJDensityPage::setPointsY(int number) { pointsYSpin_->setValue(number); }
void ZJDensityPage::setPointsZ(int number) { pointsZSpin_->setValue(number); }
void ZJDensityPage::setProjectFolder(const QString& value) { projectFolder_ = value;}
void ZJDensityPage::setResolutionVisible(bool visible) { resolutionGroup_->setVisible(visible); }
void ZJDensityPage::setResolution2DVisible(bool visible) { resolution2DGroup_->setVisible(visible); }
void ZJDensityPage::setResolution3DVisible(bool visible) { resolution3DGroup_->setVisible(visible); }
void ZJDensityPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void ZJDensityPage::setUmax(const QString& value) { if (value.toDouble() > uMinValue().toDouble()) uMaxEdit_->setText(value); }
void ZJDensityPage::setUmin(const QString& value) { if (value.toDouble() < uMaxValue().toDouble()) uMinEdit_->setText(value); }
void ZJDensityPage::setVmax(const QString& value) { if (value.toDouble() > vMinValue().toDouble()) vMaxEdit_->setText(value); }
void ZJDensityPage::setVmin(const QString& value) { if (value.toDouble() < vMaxValue().toDouble()) vMinEdit_->setText(value); }
void ZJDensityPage::setXmax(const QString& value) { if (value.toDouble() > xMinValue().toDouble()) xMaxEdit_->setText(value); }
void ZJDensityPage::setXmin(const QString& value) { if (value.toDouble() < xMaxValue().toDouble()) xMinEdit_->setText(value); }
void ZJDensityPage::setXYZTabulation(bool checked) { xyzCheck_->setChecked(checked); }
void ZJDensityPage::setXYZEnabled(bool checked) { xyzTable_->setEnabled(checked); }
void ZJDensityPage::setXYZVisible(bool checked) { xyzTable_->setVisible(checked); }
void ZJDensityPage::setYmax(const QString& value) { if (value.toDouble() > yMinValue().toDouble()) yMaxEdit_->setText(value); }
void ZJDensityPage::setYmin(const QString& value) { if (value.toDouble() < yMaxValue().toDouble()) yMinEdit_->setText(value); }
void ZJDensityPage::setZmax(const QString& value) { if (value.toDouble() > zMinValue().toDouble()) zMaxEdit_->setText(value); }
void ZJDensityPage::setZmin(const QString& value) { if (value.toDouble() < zMaxValue().toDouble()) zMinEdit_->setText(value); }

//    Executes external program DAMDEN  (Computes molecular density or deformations from the atomic partition)
void ZJDensityPage::execDamDenZJ()
{
    QString stdOutput;

    QString rootName = "DAMDENZJ_400";
    QString subdir = "DAMZERNIKE_400";
    QString inputTemplate = "DAMDENZJ_400.inp";
    QString inputSection = "DAMDENZJSECT";

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

    const QString suffix = request.runMpi ? "_mpi" : "";

    request.suffix = suffix;

    lastOutputFileName_ =
        QDir(projectFolder_).filePath(
            request.outputPrefix + "-" + request.rootName + request.suffix + ".out"
        );

    setStopEnabled(true);
    runner_->setProjectFolder(projectFolder_);

    runner_->setMpiCommand(mpiCommand_);
    runner_->setMpiFlags(mpiFlags_);
    runner_->start(request);

}

void ZJDensityPage::gridDimensionChanged(int id)
{
    int resolution;
    if (id == 0){
        setGrid2DVisible(true);
        setGrid3DVisible(false);
        setGradient(false);
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


void ZJDensityPage::gridStateChanged(int state){
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

void ZJDensityPage::handleNormalProcessExit()
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

void ZJDensityPage::hideIndicesEditors(){
    lkRangesGroup_->setVisible(false);
    chooseKIndicesLabel_->setVisible(false);
    chooseLIndicesLabel_->setVisible(false);
    chooseLKIndicesLabel_->setVisible(false);
    chooseLKMIndicesLabel_->setVisible(false);
}

void ZJDensityPage::importZJDensity()
{
    QFileDialog filedialog(this);
    filedialog.setDirectory(projectFolder_);
    filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString fullPath = filedialog.getOpenFileName(this,tr("Open file ..."),projectFolder_,
            tr("Import data from")+" (*.zernike *.jacobi);;"+
            tr("All files")+" (*)");
    if (fullPath.length()==0){
        return;
    }
    QFileInfo fileInfo(fullPath);
    importDataEdit_->setText(fileInfo.fileName());
}

void ZJDensityPage::importZJDensityChanged(){
    if (importDataEdit_->text().isEmpty()){
        setOutputGroupEnabled(false);
        setDensityExpansionEnabled(false);
        setDerivativesEnabled(false);
        setGridEnabled(false);
        setMpiCheckEnabled(false);
        setExecEnabled(false);
    }
    else{
        setOutputGroupEnabled(true);
        setDensityExpansionEnabled(true);
        setDerivativesEnabled(true);
        setGridEnabled(true);
        setMpiCheckEnabled(true);
        setExecEnabled(true);
    }
}


void ZJDensityPage::indicesChanged(int id)
{
    hideIndicesEditors();
    if (id == ALLINDICES){
        lkRangesGroup_->setVisible(true);
        chooseIndicesEdit_->setVisible(false);
    }
    else if (id == LINDICES){
        chooseLIndicesLabel_->setVisible(true);
        chooseIndicesEdit_->setVisible(true);
        chooseIndicesEdit_->setValidator(chooseLIndicesValidator_);
    }
    else if (id == KINDICES){
        chooseKIndicesLabel_->setVisible(true);
        chooseIndicesEdit_->setVisible(true);
        chooseIndicesEdit_->setValidator(chooseKIndicesValidator_);
    }
    else if (id == LKINDICES){
        chooseLKIndicesLabel_->setVisible(true);
        chooseIndicesEdit_->setVisible(true);
        chooseIndicesEdit_->setValidator(chooseLKIndicesValidator_);
    }
    else if (id == LKMINDICES){
        chooseLKMIndicesLabel_->setVisible(true);
        chooseIndicesEdit_->setVisible(true);
        chooseIndicesEdit_->setValidator(chooseLKMIndicesValidator_);
    }
}

void ZJDensityPage::inputOnlyStateChanged(int state)
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

void ZJDensityPage::lMaxChanged()
{
    if (lMax() < lMin())
        setLmax(lMin());
    setLminTop(lMax());
}

void ZJDensityPage::lMinChanged()
{
    if (lMin() > lMax())
        setLmin(lMax());
}

void ZJDensityPage::loadDefault(){
    setImport("");
    setLmin(0);

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

void ZJDensityPage::mpiStateChanged(int state)
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

void ZJDensityPage::planeModeChanged(int id)
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


void ZJDensityPage::plane2DChanged(int id){
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

void ZJDensityPage::resolutionModeChanged(int id){
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

void ZJDensityPage::xyzTabulationStateChanged(int state)
{
    if (state == Qt::Checked){
        setXYZEnabled(true);
        setXYZVisible(true);
    }else{
        setXYZEnabled(false);
        setXYZVisible(false);
    }
}


void ZJDensityPage::readFromFile(const std::string& file)
{
    const char* section = "DAMDENZJSECT";

    // ---- Import data ----
    importDataEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("fileZJname", section, file)
    );
    
    // ---- Output prefix ----
    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );
    
    // ---- Density expansion terms ----
    setChooseAllIndices(true);    // default
    setChooseKIndices(ReadWriteOptions::readRadioButton("lindividk", section, file));
    setChooseLIndices(ReadWriteOptions::readRadioButton("lindividl", section, file));
    setChooseLKIndices(ReadWriteOptions::readRadioButton("lindividlk", section, file));
    setChooseLKMIndices(ReadWriteOptions::readRadioButton("lindividlkm", section, file));
    
    setLmax(ReadWriteOptions::readSpinBox("lmaxrep", section, file));
    setLmin(ReadWriteOptions::readSpinBox("lminrep", section, file));
    setKmax(ReadWriteOptions::readSpinBox("kmaxrep", section, file));
    
    setGradient(ReadWriteOptions::readCheckBox("lgradient", section, file));

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
    }
    else{
        resolution = ReadWriteOptions::whatResolution2D(this);
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


void ZJDensityPage::readXYZTable(const std::string& file, const char* section)
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

void ZJDensityPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void ZJDensityPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void ZJDensityPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void ZJDensityPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void ZJDensityPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void ZJDensityPage::stopCurrentProcess()
{
    runner_->stop();
}

void ZJDensityPage::updatePlaneSelectionFromCoefficients()
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

//void ZJDensityPage::writeToFile(const std::string& file,
//                              bool isWindows,
//                              bool* printwarns,
//                              QString* warns,
//                              const DeltaCalculator& deltaCalculator)
void ZJDensityPage::writeToFile(const std::string& file,
                              bool* printwarns,
                              QString* warns)
{
    const char* section = "DAMDENZJSECT";

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

    // ---- Import data ----
    writeText("fileZJname", quoteString(importFile()));

    // ---- Output prefix ----
    writeText("filename", quoteString(outputPrefix()));

    if (allFunctionsRadio_->isChecked()) {
        writeBool("lindividl", false);
        writeBool("lindividk", false);
        writeBool("lindividlk", false);
        writeBool("lindividlkm", false);
        writeText("kmaxrep", QString::number(kMax()));
        writeText("lmaxrep", QString::number(lMax()));
        writeText("lminrep", QString::number(lMin()));
    } else {
        if (chooseLIndicesRadio_->isChecked()) {
            writeBool("lindividl", true);
            writeBool("lindividk", false);
            writeBool("lindividlk", false);
            writeBool("lindividlkm", false);
        }
        else if (chooseKIndicesRadio_->isChecked()) {
            writeBool("lindividl", false);
            writeBool("lindividk", true);
            writeBool("lindividlk", false);
            writeBool("lindividlkm", false);
        }
        else if (chooseLKIndicesRadio_->isChecked()) {
            writeBool("lindividl", false);
            writeBool("lindividk", false);
            writeBool("lindividlk", true);
            writeBool("lindividlkm", false);
        }
        else if (chooseLKMIndicesRadio_->isChecked()) {
            writeBool("lindividl", false);
            writeBool("lindividk", false);
            writeBool("lindividlk", false);
            writeBool("lindividlkm", true);
        }
        QString string = QString(chooseIndicesEdit_->text());
        string.remove("(",Qt::CaseInsensitive);
            string.remove(")",Qt::CaseInsensitive);
        #if QT_VERSION < 0x050E00
            QStringList stringlist1 = string.split(' ',QString::SkipEmptyParts);
        #else
            QStringList stringlist1 = string.split(' ',Qt::SkipEmptyParts);
        #endif
        chooseIndicesList_->clear();
        for (int i = 0 ; i < stringlist1.length() ; ++i){
            if (stringlist1.at(i).length() != 0){
                if ((chooseLIndicesRadio_->isChecked() || chooseKIndicesRadio_->isChecked())
                        && stringlist1.at(i).contains(QString("-"))){
                    QStringList stringlist2 = stringlist1.at(i).split("-");
                    if (stringlist2.at(0).length() != 0 && stringlist2.at(1).length() != 0
                            && stringlist2.at(1).toInt() > stringlist2.at(0).toInt()){
                        for (int k = stringlist2.at(0).toInt() ; k <= stringlist2.at(1).toInt() ; k++){
                            chooseIndicesList_->append(QString("%1").arg(k));
                        }
                    }
                }
                else{
                    chooseIndicesList_->append(stringlist1.at(i));
                }
            }
        }
        if (chooseLIndicesRadio_->isChecked() || chooseKIndicesRadio_->isChecked()){
            chooseIndicesList_->sort();
            chooseIndicesList_->removeDuplicates();
        }
        QString str = chooseIndicesList_->join(",");
        writeText("indices", str);
        writeText("kmaxrep", QString::number(MAX_KEXPZJ));
        writeText("lmaxrep", QString::number(MAX_LEXPZJ));
        writeText("lminrep", QString("0"));
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

    // ---- Parametric eq ----
    writeText("x_func_uv", quoteString(equationX()));
    writeText("y_func_uv", quoteString(equationY()));
    writeText("z_func_uv", quoteString(equationZ()));

    // ---- Plane ----
    if (isPlane2D()){
        writeText("planecase", QString::number(planecase()));
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

QString ZJDensityPage::equationX() const { return equationXEdit_->text(); }
QString ZJDensityPage::equationY() const { return equationYEdit_->text(); }
QString ZJDensityPage::equationZ() const { return equationZEdit_->text(); }
QString ZJDensityPage::getCellValue(int i, int j) const { return xyzSheet_->getcellvalue(i,j); }
QString ZJDensityPage::importFile() const { return importDataEdit_->text(); }
QString ZJDensityPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString ZJDensityPage::planeA() const { return planeAEdit_->text(); }
QString ZJDensityPage::planeB() const { return planeBEdit_->text(); }
QString ZJDensityPage::planeC() const { return planeCEdit_->text(); }
QString ZJDensityPage::uMaxValue() const { return uMaxEdit_->text(); }
QString ZJDensityPage::uMinValue() const { return uMinEdit_->text(); }
QString ZJDensityPage::vMaxValue() const { return vMaxEdit_->text(); }
QString ZJDensityPage::vMinValue() const { return vMinEdit_->text(); }
QString ZJDensityPage::xMaxValue() const { return xMaxEdit_->text(); }
QString ZJDensityPage::xMinValue() const { return xMinEdit_->text(); }
QString ZJDensityPage::yMaxValue() const { return yMaxEdit_->text(); }
QString ZJDensityPage::yMinValue() const { return yMinEdit_->text(); }
QString ZJDensityPage::zMaxValue() const { return zMaxEdit_->text(); }
QString ZJDensityPage::zMinValue() const { return zMinEdit_->text(); }
QString ZJDensityPage::numtabular() const { return QString("numrtab"); }
QString ZJDensityPage::tabularkey() const { return QString("rtab"); }
