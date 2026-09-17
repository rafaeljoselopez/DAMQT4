#include "densitygradientpage.h"

#include <QCheckBox>
#include <QComboBox>
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
#include <QToolButton>
#include <QVBoxLayout>

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

DensityGradientPage::DensityGradientPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void DensityGradientPage::buildUi()
{
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Field settings ----

    lmaxGroup_ = new QGroupBox(tr("Highest l in expansion"), this);

    lMaxSpin_ = new QSpinBox(lmaxGroup_);
    lMaxSpin_->setRange(0, 25);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);

    auto* lmaxLayout = new QHBoxLayout(lmaxGroup_);
    lmaxLayout->addWidget(lMaxSpin_);

    longRangeGroup_ = new QGroupBox(tr("Long-range"), this);
    longRangeLabel_ = new QLabel(tr("Long-range threshold"), longRangeGroup_);
    longRangeSpin_  = new QSpinBox(longRangeGroup_);
    longRangeSpin_->setRange(0, 25);
    longRangeSpin_->setValue(10);
    longRangeSpin_->setMaximumWidth(60);
    longRangeSpin_->setEnabled(true);

    auto* longRangeLayout = new QGridLayout(longRangeGroup_);
    longRangeLayout->addWidget(longRangeLabel_,0,0);
    longRangeLayout->addWidget(longRangeSpin_,0,1);


    myDoubleValidator_ = new QDoubleValidator(nullptr);
    myDoubleValidator_->setLocale(QLocale::English);
    
    // ---- Lines ----
    linesGroup_ = new QGroupBox(tr("Gradient lines"), this);

    linesPointsLabel_ = new QLabel(tr("Highest number of points"), linesGroup_);
    linesPointsEdit_ = new QLineEdit(linesGroup_);
    linesPointsEdit_->setMaximumWidth(60);

    auto* linesPointsLayout = new QHBoxLayout();
    linesPointsLayout->addWidget(linesPointsLabel_);
    linesPointsLayout->addWidget(linesPointsEdit_);

    strideLabel_ = new QLabel(tr("Stride length"), linesGroup_);
    strideEdit_ = new QLineEdit(linesGroup_);
    strideEdit_->setValidator(myDoubleValidator_);
    strideEdit_ ->setMaximumWidth(60);

    auto* strideLayout = new QHBoxLayout();
    strideLayout->addWidget(strideLabel_);
    strideLayout->addWidget(strideEdit_);


    // ---- Plot ----

    plotTypeGroup_ = new QGroupBox(tr("Plot type"), linesGroup_);
    plot2DRadio_ = new QRadioButton(tr("2D"), plotTypeGroup_);
    plot3DRadio_ = new QRadioButton(tr("3D"), plotTypeGroup_);
    plotTypeGroup_->setVisible(true);
    plot2DRadio_->setChecked(false);
    plot3DRadio_->setChecked(true);

    plotDimensionGroup_ = new QButtonGroup(plotTypeGroup_);
    plotDimensionGroup_->addButton(plot2DRadio_, DIM_2D);
    plotDimensionGroup_->addButton(plot3DRadio_, DIM_3D);


    auto* plotTypeLayout = new QHBoxLayout(plotTypeGroup_);
    plotTypeLayout->addWidget(plot2DRadio_);
    plotTypeLayout->addWidget(plot3DRadio_);

    // 3D
    plot3DGroup_ = new QGroupBox(tr("3D plot"), linesGroup_);
    setPlot3DVisible(true);

    startDirectionsLabel_= new QLabel(tr("Set of starting directions"), plot3DGroup_);
    startDirectionsCombo_ = new QComboBox(plot3DGroup_);
    startDirectionsCombo_->addItem("0");
    startDirectionsCombo_->addItem("1");
    startDirectionsCombo_->addItem("2");
    startDirectionsCombo_->addItem("3");
    startDirectionsCombo_->addItem("4");
    startDirectionsCombo_->addItem("5");
    startDirectionsCombo_->addItem("6");
    startDirectionsCombo_->addItem("7");
    startDirectionsCombo_->setCurrentIndex(1);
    startDirectionsCombo_->setMaximumWidth(50);
    startDirectionsCombo_->setMinimumWidth(50);
    startDirectionsCombo_->setToolTip(tr("Based on icosahedron vertices, C2 axes and C3 axes\n")+
            tr("1: vertices (12 points), 2: C3 axes (20 points), 3: C2 axes (30 points)")+
            tr("4: vertices + C3 axes, 5: vertices + C2 axes, 6: C2 axes + C3 axes, 7: vertices + C2 axes + C3 axes"));

    auto* startDirectionsLayout = new QHBoxLayout();
    startDirectionsLayout->addWidget(startDirectionsLabel_);
    startDirectionsLayout->addWidget(startDirectionsCombo_);

    box3DGroup_ = new QGroupBox(tr("3D boundaries"), plot3DGroup_);
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

    auto* plot3DLayout = new QVBoxLayout(plot3DGroup_);
    plot3DLayout->addLayout(startDirectionsLayout);
    plot3DLayout->addWidget(box3DGroup_);

    // 2D
    plot2DGroup_ = new QGroupBox(tr("2D plot"), linesGroup_);
    setPlot2DVisible(false);

    linesPerNucleusLabel_ = new QLabel(tr("Number of Lines per nucleus"), plot2DGroup_);
    linesPerNucleusEdit_ = new QLineEdit(plot2DGroup_);
    linesPerNucleusEdit_ ->setMaximumWidth(40);

    auto* linesPerNucleusLayout = new QHBoxLayout();
    linesPerNucleusLayout->addWidget(linesPerNucleusLabel_);
    linesPerNucleusLayout->addWidget(linesPerNucleusEdit_);

    box2DGroup_ = new QGroupBox(tr("2D boundaries"), plot2DGroup_);
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


    planes2DGroup_ = new QGroupBox(tr("2D Planes"), plot2DGroup_);

    planeXYRadio_ = new QRadioButton(tr("XY"), plot2DGroup_);
    planeXZRadio_ = new QRadioButton(tr("XZ"), plot2DGroup_);
    planeYZRadio_ = new QRadioButton(tr("YZ"), plot2DGroup_);
    planeOtherRadio_ = new QRadioButton(tr("Other"), plot2DGroup_);

    planesButtonsGroup_ = new QButtonGroup(this);
    planesButtonsGroup_->addButton(planeXYRadio_, planeXY);
    planesButtonsGroup_->addButton(planeXZRadio_, planeXZ);
    planesButtonsGroup_->addButton(planeYZRadio_, planeYZ);
    planesButtonsGroup_->addButton(planeOtherRadio_, planeOther);

    planeCoefficientsGroup_ = new QGroupBox(tr("Plane parameters"), plot2DGroup_);
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
    
    slopesRatioLabel_ = new QLabel(tr("Starting slopes ratio"), plot2DGroup_);
    
    uvRatioLabel_ = new QLabel("v/u", plot2DGroup_);
    uvRatioEdit_ = new QLineEdit(plot2DGroup_);
    uvRatioEdit_->setText("0.1");
    uvRatioEdit_->setValidator(myDoubleValidator_);
    uvRatioEdit_ ->setMaximumWidth(50);
    
    
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
    planesLayout->addWidget(slopesRatioLabel_, 3,0,1,2);
    planesLayout->addWidget(uvRatioLabel_, 4, 0);
    planesLayout->addWidget(uvRatioEdit_, 4, 1);
    

    // Lines tabulation
    extraLinesCheck_ = new QCheckBox(tr("Extra lines"), linesGroup_);
    tableLinesGroup_ = new QGroupBox("", this);
    tableLinesGroup_->setVisible(false);

    addPointsCheck_ = new QCheckBox(tr("Add starting points to table"), tableLinesGroup_);

    linesUVTable_ = new QWidget();
    linesUVSheet_ = new Sheet(0, 3, 0,true, linesUVTable_);
    QStringList linesUVList_;
    linesUVList_ << tr("center") << "u" << "v";
    linesUVSheet_->setHeader(linesUVList_);
    linesUVTable_->setVisible(false);
    linesUVTable_->setEnabled(false);

    linesXYZTable_ = new QWidget();
    linesXYZSheet_ = new Sheet(0, 4, 0,true, linesXYZTable_);
    QStringList linesXYZList_;
    linesXYZList_ << tr("center") << "x" << "y" << "z";
    linesXYZSheet_->setHeader(linesXYZList_);
    linesXYZTable_->setVisible(false);
    linesXYZTable_->setEnabled(false);


    auto* xyzLayout = new QVBoxLayout(tableLinesGroup_);
    xyzLayout->addWidget(addPointsCheck_);
    xyzLayout->addWidget(linesUVTable_);
    xyzLayout->addWidget(linesXYZTable_);
    
    // ---- Import gradient lines ----
    importLinesGroup_ = new QGroupBox(tr("Import gradient lines from file:"), this);
    importLinesEdit_ = new QLineEdit(importLinesGroup_);
    importLinesEdit_->setMaximumWidth(280);
    importLinesButton_ =  new QToolButton(importLinesGroup_);
    importLinesButton_->setText(tr("..."));
    importLinesGroup_->setVisible(false);

    auto* importLinesLayout = new QHBoxLayout(importLinesGroup_);
    importLinesLayout->addWidget(importLinesEdit_);
    importLinesLayout->addWidget(importLinesButton_);

    auto* plot2DLayout = new QVBoxLayout(plot2DGroup_);
    plot2DLayout->addLayout(linesPerNucleusLayout);
    plot2DLayout->addWidget(box2DGroup_);
    plot2DLayout->addWidget(planes2DGroup_);

    auto* linesLayout = new QVBoxLayout(linesGroup_);
    linesLayout->addLayout(linesPointsLayout);
    linesLayout->addLayout(strideLayout);
    linesLayout->addWidget(plotTypeGroup_);
    linesLayout->addWidget(plot2DGroup_);
    linesLayout->addWidget(plot3DGroup_);
    linesLayout->addWidget(extraLinesCheck_,0,Qt::AlignCenter);
    linesLayout->addWidget(tableLinesGroup_);
    linesLayout->addWidget(importLinesGroup_);
    
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
    mainLayout->addWidget(lmaxGroup_);
    mainLayout->addWidget(longRangeGroup_);
    mainLayout->addWidget(linesGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();


}

void DensityGradientPage::connectSignals()
{

    connect(plotDimensionGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityGradientPage::plotDimensionChanged);

    connect(planesButtonsGroup_, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &DensityGradientPage::plane2DChanged);

    connect(extraLinesCheck_, &QCheckBox::stateChanged,
            this, &DensityGradientPage::extraLinesChanged);

    connect(addPointsCheck_, &QCheckBox::stateChanged,
            this, &DensityGradientPage::tabulationStateChanged);

    connect(importLinesButton_, &QToolButton::clicked,
            this, &DensityGradientPage::importGradientLines);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &DensityGradientPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
                this, &DensityGradientPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &DensityGradientPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &DensityGradientPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
                    runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &DensityGradientPage::openOutputRequested);

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

int DensityGradientPage::lMax() const { return lMaxSpin_->value(); }
int DensityGradientPage::lMaxTop() const { return lMaxSpin_->maximum(); }
int DensityGradientPage::longRangeThreshold() const { return longRangeSpin_->value(); }
int DensityGradientPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int DensityGradientPage::planecase() const { return planeResult.planeCase;}
int DensityGradientPage::linesUVTableRows() const { return linesUVSheet_->tabla->rowCount();}
int DensityGradientPage::linesXYZTableRows() const { return linesXYZSheet_->tabla->rowCount();}

bool DensityGradientPage::is2DPlot() const { return plotDimensionGroup_->button(0)->isChecked(); }
bool DensityGradientPage::is3DPlot() const { return plotDimensionGroup_->button(1)->isChecked(); }
bool DensityGradientPage::isExtraLines() const { return extraLinesCheck_->isChecked(); }
bool DensityGradientPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool DensityGradientPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool DensityGradientPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool DensityGradientPage::isPlane2D() const { return plane2DRadio_->isChecked(); }
bool DensityGradientPage::isPlaneOther() const { return planeOtherRadio_->isChecked(); }
bool DensityGradientPage::isPlaneXY() const { return planeXYRadio_->isChecked(); }
bool DensityGradientPage::isPlaneXZ() const { return planeXZRadio_->isChecked(); }
bool DensityGradientPage::isPlaneYZ() const { return planeYZRadio_->isChecked(); }
bool DensityGradientPage::isTabulation() const { return addPointsCheck_->isChecked(); }

QVector3D DensityGradientPage::getPlaneWu() const { return planeResult.wu; }
QVector3D DensityGradientPage::getPlaneWv() const { return planeResult.wv; }

void DensityGradientPage::clearUVSheet() {linesUVSheet_->clear(); }
void DensityGradientPage::clearXYZSheet() {linesXYZSheet_->clear(); }
void DensityGradientPage::insertUVRow(int i) {linesUVSheet_->tabla->insertRow(i); }
void DensityGradientPage::insertXYZRow(int i) {linesXYZSheet_->tabla->insertRow(i); }
void DensityGradientPage::resizeUVSheet(int i) {linesUVSheet_->resizeRows(i); }
void DensityGradientPage::resizeXYZSheet(int i) {linesXYZSheet_->resizeRows(i); }
void DensityGradientPage::setUVCellValue(const QString& value, int i, int j) {linesUVSheet_->setcellvalue(value,i,j); }
void DensityGradientPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void DensityGradientPage::setExtraLines(bool checked) { extraLinesCheck_->setChecked(checked); }
void DensityGradientPage::setImportLinesFile(const QString &value) { importLinesEdit_->setText(value); }
void DensityGradientPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void DensityGradientPage::setPlot2D(bool checked) { plotDimensionGroup_->button(DIM_2D)->setChecked(checked); }
void DensityGradientPage::setPlot2DVisible(bool visible) { plot2DGroup_->setVisible(visible); plot2DGroup_->setEnabled(visible); }
void DensityGradientPage::setPlot3D(bool checked) { plotDimensionGroup_->button(DIM_3D)->setChecked(checked); }
void DensityGradientPage::setPlot3DVisible(bool visible) { plot3DGroup_->setVisible(visible); plot3DGroup_->setEnabled(visible); }
void DensityGradientPage::setPlotTypeVisible(bool visible) { plotTypeGroup_->setVisible(visible); }
void DensityGradientPage::setLinesPerNucleus(const QString& value) { linesPerNucleusEdit_->setText(value); }
void DensityGradientPage::setLinesPoints(const QString& value) { linesPointsEdit_->setText(value); }
void DensityGradientPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void DensityGradientPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void DensityGradientPage::setLmaxVisible(bool visible) { lmaxGroup_->setVisible(visible); }
void DensityGradientPage::setLongRangeMinThreshold(int number) { longRangeSpin_->setMinimum(number); }
void DensityGradientPage::setLongRangeThreshold(int number) { longRangeSpin_->setValue(number); }
void DensityGradientPage::setLongRangeTopThreshold(int number) { longRangeSpin_->setMaximum(number); }
void DensityGradientPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void DensityGradientPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void DensityGradientPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void DensityGradientPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void DensityGradientPage::setOutputButtonEnabled(bool enabled) { outputButton_->setEnabled(enabled); }
void DensityGradientPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void DensityGradientPage::setPlanes2DVisible(bool visible) { planes2DGroup_->setVisible(visible); }
void DensityGradientPage::setPlane2D(bool checked) { plane2DRadio_->setChecked(checked); }
void DensityGradientPage::setPlaneA(const QString& value) { planeAEdit_->setText(value); }
void DensityGradientPage::setPlaneB(const QString& value) { planeBEdit_->setText(value); }
void DensityGradientPage::setPlaneC(const QString& value) { planeCEdit_->setText(value); }
void DensityGradientPage::setPlaneCoefficientsVisible(bool visible) { planeCoefficientsGroup_->setVisible(visible); }
void DensityGradientPage::setPlaneOther(bool checked) { planeOtherRadio_->setChecked(checked); }
void DensityGradientPage::setPlaneXY(bool checked) { planesButtonsGroup_->button(planeXY)->setChecked(checked); }
void DensityGradientPage::setPlaneXZ(bool checked) { planesButtonsGroup_->button(planeXZ)->setChecked(checked);}
void DensityGradientPage::setPlaneYZ(bool checked) { planesButtonsGroup_->button(planeYZ)->setChecked(checked); }
// void DensityGradientPage::setProjectFolder(const QString& value) { projectFolder_ = value;}
void DensityGradientPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void DensityGradientPage::setStride(const QString &value) { strideEdit_->setText(value); }
void DensityGradientPage::setTabulation(bool checked) { addPointsCheck_->setChecked(checked); }
void DensityGradientPage::setUmax(const QString& value) { if (value.toDouble() > uMinValue().toDouble()) uMaxEdit_->setText(value); }
void DensityGradientPage::setUmin(const QString& value) { if (value.toDouble() < uMaxValue().toDouble()) uMinEdit_->setText(value); }
void DensityGradientPage::setUVTableEnabled(bool checked) { linesUVTable_->setEnabled(checked); }
void DensityGradientPage::setUVTableVisible(bool checked) { linesUVTable_->setVisible(checked); }
void DensityGradientPage::setVmax(const QString& value) { if (value.toDouble() > vMinValue().toDouble()) vMaxEdit_->setText(value); }
void DensityGradientPage::setVmin(const QString& value) { if (value.toDouble() < vMaxValue().toDouble()) vMinEdit_->setText(value); }
void DensityGradientPage::setXYZCellValue(const QString& value, int i, int j) {linesXYZSheet_->setcellvalue(value,i,j); }
void DensityGradientPage::setXmax(const QString& value) { if (value.toDouble() > xMinValue().toDouble()) xMaxEdit_->setText(value); }
void DensityGradientPage::setXmin(const QString& value) { if (value.toDouble() < xMaxValue().toDouble()) xMinEdit_->setText(value); }
void DensityGradientPage::setXYZTableEnabled(bool checked) { linesXYZTable_->setEnabled(checked); }
void DensityGradientPage::setXYZTableVisible(bool checked) { linesXYZTable_->setVisible(checked); }
void DensityGradientPage::setYmax(const QString& value) { if (value.toDouble() > yMinValue().toDouble()) yMaxEdit_->setText(value); }
void DensityGradientPage::setYmin(const QString& value) { if (value.toDouble() < yMaxValue().toDouble()) yMinEdit_->setText(value); }
void DensityGradientPage::setZmax(const QString& value) { if (value.toDouble() > zMinValue().toDouble()) zMaxEdit_->setText(value); }
void DensityGradientPage::setZmin(const QString& value) { if (value.toDouble() < zMaxValue().toDouble()) zMinEdit_->setText(value); }


//    Executes external program DAMDENGRAD  (Computes electron density gradient lines from the atomic partition)
void DensityGradientPage::execDamDenGrad()
{
    QString stdOutput;

    QString rootName = "DAMDENGRAD_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMDENGRAD_400.inp";
    QString inputSection = "DAMDENGRADSECT";

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

    if (is3DPlot()){
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

void DensityGradientPage::extraLinesChanged(int state)
{
    if (state == Qt::Checked){
        tableLinesGroup_->setVisible(true);
        importLinesGroup_->setVisible(true);
    }
    else{
        tableLinesGroup_->setVisible(false);
        importLinesGroup_->setVisible(false);
    }
}

void DensityGradientPage::handleNormalProcessExit()
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
void DensityGradientPage::importGradientLines()
{
    QFileDialog filedialog(this);
    filedialog.setDirectory(projectFolder_);
    filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString fullPath = filedialog.getOpenFileName(this,tr("Open file ..."),projectFolder_,
                    tr("lin files")+" (*.lin2D *.lin3D);;"+tr("All files")+" (*)");
    if (fullPath.length()==0){
        return;
    }
    importLinesEdit_->setText(fullPath);
}

void DensityGradientPage::inputOnlyStateChanged(int state)
{
    if (!(state == Qt::Checked) && is3DPlot()){
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

void DensityGradientPage::loadDefault(){
    setLmaxTop(MAX_LEXP);
    setLmax(10);
    setLongRangeMinThreshold(-10);
    setLongRangeTopThreshold(0);
    setLongRangeThreshold(-9);

    setLinesPoints("1000");
    setStride("0.02");
    setLinesPerNucleus("16");

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

    setExtraLines(false);

    setPlotTypeVisible(true);
    setPlot3D(true);
    setPlot3DVisible(true);
    setPlot2DVisible(false);
    setMpiVisible(true);

    setTabulation(false);
    setUVTableVisible(false);
    setXYZTableVisible(false);
    setImportLinesFile("");
    clearUVSheet();
    clearXYZSheet();
}


void DensityGradientPage::mpiStateChanged(int state)
{
    Q_UNUSED(state);
    QSignalBlocker blocker(mpiCheck_);
    if (is3DPlot() && !isInputOnly()){
        setMpiControlsEnabled(true);
        setMpiVisible(true);
    }
    else if (is2DPlot()){
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


void DensityGradientPage::plane2DChanged(int id){
    switch (id) {
        case planeXY:
            setPlaneA("0.");
            setPlaneB("0.");
            setPlaneC("1.");
            setPlaneCoefficientsVisible(false);
            break;
        case planeXZ:
            setPlaneA("0.");
            setPlaneB("1.");
            setPlaneC("0.");
            setPlaneCoefficientsVisible(false);
            break;
        case planeYZ:
            setPlaneA("1.");
            setPlaneB("0.");
            setPlaneC("0.");
            setPlaneCoefficientsVisible(false);
            break;
        default:
            setPlaneA("1.");
            setPlaneB("1.");
            setPlaneC("1.");
            setPlaneCoefficientsVisible(true);
    }
    planeResult = ReadWriteOptions::get_plane_case(planeA().toDouble(),
        planeB().toDouble(),planeC().toDouble());
}


void DensityGradientPage::plotDimensionChanged(int id)
{
    if (id == 0){
        setPlot2DVisible(true);
        setPlot3DVisible(false);
        if (mpiAvailable_){
            setMpiVisible(false);
        }
    }
    else{
        setPlot2DVisible(false);
        setPlot3DVisible(true);
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
    }
    setExtraLines(false);
    setTabulation(false);
}

void DensityGradientPage::readFromFile(const std::string& file)
{
    const char* section = "DAMDENGRADSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    setLmax(ReadWriteOptions::readSpinBox("lmaxrep", section, file));

    QString value = ReadWriteOptions::readTextToLineEdit("umbrlargo", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-7");
    setLongRangeThreshold(value.toInt());

    ReadWriteOptions::readDoubleToLineEdit("xinf", section, xMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("xsup", section, xMaxEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("yinf", section, yMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("ysup", section, yMaxEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("zinf", section, zMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("zsup", section, zMaxEdit_, file);

    ReadWriteOptions::readDoubleToLineEdit("uinf", section, uMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("usup", section, uMaxEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("vinf", section, vMinEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("vsup", section, vMaxEdit_, file);

    setPlot2D(ReadWriteOptions::readRadioButton("lplot2d", section, file));

    if (is3DPlot()){
        setPlot2DVisible(false);
        setPlot3DVisible(true);
    }
    else{
        setPlot2DVisible(true);
        setPlot3DVisible(false);
    }

    // ---- Plane ----
    QString aux;
    aux = ReadWriteOptions::readDoubleToLineEdit("planeA", section, file);
    if (!aux.isEmpty()) setPlaneA(aux);
    aux = ReadWriteOptions::readDoubleToLineEdit("planeB", section, file);
    if (!aux.isEmpty()) setPlaneB(aux);
    aux = ReadWriteOptions::readDoubleToLineEdit("planeC", section, file);
    if (!aux.isEmpty()) setPlaneC(aux);
    updatePlaneSelectionFromCoefficients();

    // ---- Plot enable ----

    setPlotTypeVisible(true);

    mpiGroup_->setVisible(is3DPlot());

    // ---- Extra lines ----
    setExtraLines(ReadWriteOptions::readCheckBox("lextralines", section, file));

    // ---- Sheet for tabulation ----
    if (is3DPlot()){
        readXYZLinesTable(file, section);
        if (linesXYZSheet_->tabla->rowCount() > 0){
            setTabulation(true);
            linesXYZTable_->updateGeometry();
            linesXYZTable_->setVisible(true);
        }
        else{
            setTabulation(false);
            linesXYZTable_->updateGeometry();
            linesXYZTable_->setVisible(false);
        }
    }
    else{
        readUVLinesTable(file, section);
        if (linesUVSheet_->tabla->rowCount() > 0){
            setTabulation(true);
            linesUVTable_->updateGeometry();
            linesUVTable_->setVisible(true);
        }
        else{
            setTabulation(false);
            linesUVTable_->updateGeometry();
            linesUVTable_->setVisible(false);
        }
    }

    // ---- File with lines for tabulation ----
    importLinesEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filelines", section, file)
    );
}

void DensityGradientPage::readUVLinesTable(const std::string& file, const char* section)
{
    // ---- Number of rows ----
    const QString numText =
        QString::fromStdString(CIniFile::GetValue("nlines", section, file));

    bool ok = false;
    const int numRows = numText.toInt(&ok);

    clearUVSheet();

    if (!ok || numRows <= 0) {
        setUVTableEnabled(false);
        setUVTableVisible(false);
        return;
    }

    resizeUVSheet(numRows + 1);

    for (int i = 0; i < numRows; ++i) {
        insertUVRow(i);
        const QString key = QString("icntlines(%1)").arg(i + 1);
        const QString value =
            QString::fromStdString(
                CIniFile::GetValue(key.toStdString(), section, file)
            );
        setUVCellValue(value, i, 0);
        for (int j = 1; j < 3; ++j) {
            const QString key = QString("rlines(%1,%2)").arg(j).arg(i + 1);
            const QString value =
                QString::fromStdString(
                    CIniFile::GetValue(key.toStdString(), section, file)
                );

            setUVCellValue(value, i, j);
        }
    }

    resizeUVSheet(linesUVTableRows());

    // ---- Visibility ----
    if (isTabulation()) {
        setUVTableEnabled(true);
        setUVTableVisible(true);
    } else {
        setUVTableEnabled(false);
        setUVTableVisible(false);
    }
    setXYZTableEnabled(false);
    setXYZTableVisible(false);
}


void DensityGradientPage::readXYZLinesTable(const std::string& file, const char* section)
{
    // ---- Number of rows ----
    const QString numText =
        QString::fromStdString(CIniFile::GetValue("nlines", section, file));

    bool ok = false;
    const int numRows = numText.toInt(&ok);

    clearXYZSheet();

    if (!ok || numRows <= 0) {
        setXYZTableEnabled(false);
        setXYZTableVisible(false);
        return;
    }

    resizeXYZSheet(numRows + 1);

    for (int i = 0; i < numRows; ++i) {
        insertXYZRow(i);
        const QString key = QString("icntlines(%1)").arg(i + 1);
        const QString value =
            QString::fromStdString(
                CIniFile::GetValue(key.toStdString(), section, file)
            );
        setXYZCellValue(value, i, 0);
        for (int j = 1; j < 4; ++j) {
            const QString key = QString("rlines(%1,%2)").arg(j).arg(i + 1);
            const QString value =
                QString::fromStdString(
                    CIniFile::GetValue(key.toStdString(), section, file)
                );

            setXYZCellValue(value, i, j);
        }
    }

    resizeXYZSheet(linesXYZTableRows());

    // ---- Visibility ----
    if (isTabulation()) {
        setXYZTableEnabled(true);
        setXYZTableVisible(true);
    } else {
        setXYZTableEnabled(false);
        setXYZTableVisible(false);
    }
    setUVTableEnabled(false);
    setUVTableVisible(false);
}


void DensityGradientPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void DensityGradientPage::stopCurrentProcess()
{
    runner_->stop();
}

void DensityGradientPage::tabulationStateChanged(int state)
{
    if (state == Qt::Checked){
        if (is3DPlot()){
            setXYZTableEnabled(true);
            setXYZTableVisible(true);
            setUVTableEnabled(false);
            setUVTableVisible(false);
        } else{
            setUVTableEnabled(true);
            setUVTableVisible(true);
            setXYZTableEnabled(false);
            setXYZTableVisible(false);
        }
    }else{
        setUVTableEnabled(false);
        setUVTableVisible(false);
        setXYZTableEnabled(false);
        setXYZTableVisible(false);
    }
}


void DensityGradientPage::setIsValence(bool valence)
{
    lvalence_ = valence;
}

void DensityGradientPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void DensityGradientPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void DensityGradientPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void DensityGradientPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void DensityGradientPage::updatePlaneSelectionFromCoefficients()
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

//void DensityGradientPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool lvalence,
//                                bool* printwarns,
//                                QString* warns)
void DensityGradientPage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMDENGRADSECT";

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

    // ---- General ----
    writeBool("iswindows", iswindows_);

    if (lvalence_) {
        writeBool("lvalence", true);
    }


    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

    // ---- Lmax in expansion ----
    writeText("lmaxrep", QString::number(lMax()));

    // ---- Long-range ----
    writeText("umbrlargo", QString("1.d%1").arg(longRangeThreshold()));

    // ---- Field lines ----
    writeText("numpnt", linesPointsEdit_->text());
    writeText("dlt0", strideEdit_->text());

    // ---- Plot type ----
    writeBool("lplot2d", is2DPlot());
    writeText("nlinpernuc", linesPerNucleusEdit_->text());
    writeText("ioplines3D", startDirectionsCombo_->currentText());

    // ---- Intervals ----
    ReadWriteOptions::writeOption("uinf", section, uMinValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("usup", section, uMaxValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("vinf", section, vMinValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("vsup", section, vMaxValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("xinf", section, xMinValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("xsup", section, xMaxValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("yinf", section, yMinValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("ysup", section, yMaxValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("zinf", section, zMinValue(), file, printwarns, warns);
    ReadWriteOptions::writeOption("zsup", section, zMaxValue(), file, printwarns, warns);

//    // ---- Plane ----
    if (is2DPlot()){
        writeText("planeA", planeA());
        writeText("planeB", planeB());
        writeText("planeC", planeC());
    }

    writeText("uvratio", uvRatioEdit_->text());

    writeBool("lextralines", isExtraLines());
    if (is2DPlot() && isTabulation()){
        ReadWriteOptions::writeUVLinesTable(section, file, this, Sheet::max_sel, printwarns, warns);
    } else if (is3DPlot() && isTabulation()){
        ReadWriteOptions::writeXYZLinesTable(section, file, this, Sheet::max_sel, printwarns, warns);
    }

    if (!importLinesEdit_->text().isEmpty())
        writeText("filelines", quoteString(importLinesEdit_->text()));
}

QString DensityGradientPage::getUVCellValue(int i, int j) const { return linesUVSheet_->getcellvalue(i,j); }
QString DensityGradientPage::getXYZCellValue(int i, int j) const { return linesXYZSheet_->getcellvalue(i,j); }
QString DensityGradientPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString DensityGradientPage::planeA() const { return planeAEdit_->text(); }
QString DensityGradientPage::planeB() const { return planeBEdit_->text(); }
QString DensityGradientPage::planeC() const { return planeCEdit_->text(); }
QString DensityGradientPage::uMaxValue() const { return uMaxEdit_->text(); }
QString DensityGradientPage::uMinValue() const { return uMinEdit_->text(); }
QString DensityGradientPage::vMaxValue() const { return vMaxEdit_->text(); }
QString DensityGradientPage::vMinValue() const { return vMinEdit_->text(); }
QString DensityGradientPage::xMaxValue() const { return xMaxEdit_->text(); }
QString DensityGradientPage::xMinValue() const { return xMinEdit_->text(); }
QString DensityGradientPage::yMaxValue() const { return yMaxEdit_->text(); }
QString DensityGradientPage::yMinValue() const { return yMinEdit_->text(); }
QString DensityGradientPage::zMaxValue() const { return zMaxEdit_->text(); }
QString DensityGradientPage::zMinValue() const { return zMinEdit_->text(); }
QString DensityGradientPage::numtabular() const { return QString("nlines"); }
QString DensityGradientPage::tabularkey() const { return QString("rlines"); }
