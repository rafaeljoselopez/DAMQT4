#include "topographypage.h"

#include <QtDebug>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRadioButton>
#include <QTableWidget>
#include <QTextStream>
#include <QToolButton>

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

TopographyPage::TopographyPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void TopographyPage::buildUi()
{
    myDoubleValidator_ = new QDoubleValidator(nullptr);
    myDoubleValidator_->setLocale(QLocale::English);
    
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);
    
    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);
    
    // ---- Topography type ----
    topoTypeGroup_ = new QGroupBox(tr("Topography type"), this);
    
    densityRadio_ = new QRadioButton(tr("Molecular density"), topoTypeGroup_);
    potentialRadio_ = new QRadioButton(tr("Molecular potential"), topoTypeGroup_);
    topoButtonsGroup_ = new QButtonGroup(this);
    topoButtonsGroup_->addButton(densityRadio_, 0);
    topoButtonsGroup_->addButton(potentialRadio_, 1);
    
    lMaxLabel_ = new QLabel(tr("Highest l in expansion"), topoTypeGroup_);
    lMaxSpin_ = new QSpinBox(topoTypeGroup_);
    lMaxSpin_->setFixedWidth(80);
    
    auto* lMaxLayout = new QHBoxLayout();
    lMaxLayout->addWidget(lMaxLabel_);
    lMaxLayout->addWidget(lMaxSpin_);
    
    auto* topoTypeLayout = new QVBoxLayout(topoTypeGroup_);
    topoTypeLayout->addWidget(densityRadio_);
    topoTypeLayout->addWidget(potentialRadio_);
    topoTypeLayout->addLayout(lMaxLayout);
        
    // ---- Topography mapping ----
    mappingGroup_ = new QGroupBox(tr("Topography mapping"), this);
    mappingGroup_->setMaximumSize(QSize(SIZE4, SIZE20));
    
    mapCriticalCheck_ = new QCheckBox(tr("Map critical points"), mappingGroup_);
    
    boxMarginsSizeLabel_ = new QLabel(tr("Box margins size"), mappingGroup_);
    boxMarginsSizeEdit_ = new QLineEdit(mappingGroup_);
    boxMarginsSizeEdit_->setText("1.0");
    boxMarginsSizeEdit_->setValidator(myDoubleValidator_);
    boxMarginsSizeEdit_->setToolTip(tr("Box size around default guess points"));
    boxMarginsSizeEdit_->setFixedWidth(100);
    
    auto* boxMarginsSizeLayout = new QHBoxLayout();
    boxMarginsSizeLayout->addWidget(boxMarginsSizeLabel_);
    boxMarginsSizeLayout->addWidget(boxMarginsSizeEdit_);
    
    convergenceThresholdLabel_ = new QLabel(tr("Convergence threshold"), mappingGroup_);
    convergenceThresholdEdit_ = new QLineEdit(mappingGroup_);
    convergenceThresholdEdit_->setText("4.0e-15");
    convergenceThresholdEdit_->setValidator(myDoubleValidator_);
    convergenceThresholdEdit_->setToolTip(tr("Convergence threshold. Recommended: 4.0E-12 for MESP and 4.0E-15 for MED"));
    convergenceThresholdEdit_->setFixedWidth(100);

    auto* convergenceThresholdLayout = new QHBoxLayout();
    convergenceThresholdLayout->addWidget(convergenceThresholdLabel_);
    convergenceThresholdLayout->addWidget(convergenceThresholdEdit_);
    
    // ---- Guess points ----
    guessPointsGroup_ = new QGroupBox(tr("Guess points for CPs"), this);
    guessPointsGroup_->setMaximumSize(QSize(SIZE4, SIZE20));
    guessPointsGroup_->setVisible(false);
    
    guessPointsCheck_ = new QCheckBox(tr("Add guess points for CPs"), mappingGroup_);
    guessPointsCheck_->setChecked(false);
    
    // ---- XYZ tabulation ----
    guessPointsToTableGroup_ = new QGroupBox("", this);
    guessPointsToTableGroup_->setMaximumSize(QSize(SIZE4, SIZE20));
    
    addPointsToTableCheck_ = new QCheckBox(tr("Add guess points to table"), guessPointsToTableGroup_);

    xyzTable_ = new QWidget();
    xyzSheet_ = new Sheet(0, 3, 0,true, xyzTable_);

    QStringList xyzList_;
    xyzList_ << "x" << "y" << "z";
    xyzSheet_->setHeader(xyzList_);
    
    auto* guessPointsToTableLayout = new QVBoxLayout(guessPointsToTableGroup_);
    guessPointsToTableLayout->setContentsMargins(5, 0, 5, 5);
    guessPointsToTableLayout->addWidget(addPointsToTableCheck_);
    guessPointsToTableLayout->addWidget(xyzTable_);
    
    // ---- Guess points from file ----
    guessFromFileGroup_ = new QGroupBox(tr("Load guess points from file"), this);


    guessFileEdit_ = new QLineEdit(guessFromFileGroup_);
    guessFileButton_ =  new QToolButton(guessFromFileGroup_);
    guessFileButton_->setText(tr("..."));
    
    auto* guessFileLayout = new QHBoxLayout(guessFromFileGroup_);
    guessFileLayout->addWidget(guessFileEdit_);
    guessFileLayout->addWidget(guessFileButton_);
    
    auto* guessPointsLayout = new QVBoxLayout(guessPointsGroup_);
    guessPointsLayout->addWidget(guessPointsToTableGroup_);
    guessPointsLayout->addWidget(guessFromFileGroup_);
    
    boxSizeLabel_ = new QLabel(tr("Box size"), mappingGroup_);
    boxSizeEdit_ = new QLineEdit(mappingGroup_);
    boxSizeEdit_->setText("2.0");
    boxSizeEdit_->setValidator(myDoubleValidator_);
    boxSizeEdit_->setToolTip(tr("Box size around optional guess points"));
    boxSizeEdit_->setFixedWidth(100);
    
    stepSizeLabel_ = new QLabel(tr("Step size"), mappingGroup_);
    stepSizeEdit_ = new QLineEdit(mappingGroup_);
    stepSizeEdit_->setText("2.0");
    stepSizeEdit_->setValidator(myDoubleValidator_);
    stepSizeEdit_->setFixedWidth(100);

    auto* boxStepLayout = new QGridLayout();
    boxStepLayout->setColumnStretch(2, 1);
    boxStepLayout->setHorizontalSpacing(8);
    boxStepLayout->setVerticalSpacing(5);
    boxStepLayout->addWidget(boxSizeLabel_, 0, 0);
    boxStepLayout->addWidget(boxSizeEdit_, 0, 1);
    boxStepLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);
    boxStepLayout->addWidget(stepSizeLabel_, 1, 0);
    boxStepLayout->addWidget(stepSizeEdit_, 1, 1);
    boxStepLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 2);

    // ---- Molecular graph ----
    buildGraphGroup_ = new QGroupBox("", this);
    buildGraphGroup_->setVisible(false);
    buildGraphCheck_ = new QCheckBox(tr("Construct molecular graph"), mappingGroup_);
    buildGraphCheck_->setChecked(false);
    
    boxGraphSizeLabel_ = new QLabel(tr("Box size"), buildGraphGroup_);
    boxGraphSizeEdit_ = new QLineEdit(buildGraphGroup_);
    boxGraphSizeEdit_->setText("4.5");
    boxGraphSizeEdit_->setValidator(myDoubleValidator_);
    boxGraphSizeEdit_->setToolTip(tr("Square box size computed as distance of farthest CP plus this margin size"));
    boxGraphSizeEdit_->setFixedWidth(100);

    auto* boxGraphSizeLayout = new QHBoxLayout();
    boxGraphSizeLayout->addWidget(boxGraphSizeLabel_);
    boxGraphSizeLayout->addWidget(boxGraphSizeEdit_);
    
    convergenceGradLabel_ = new QLabel(tr("Gradient convergence"), buildGraphGroup_);
    convergenceGradEdit_ = new QLineEdit(buildGraphGroup_);
    convergenceGradEdit_->setText("1.0e-5");
    convergenceGradEdit_->setMinimumWidth(60);
    convergenceGradEdit_->setValidator(myDoubleValidator_);
    convergenceGradEdit_->setToolTip(tr("Gradient convergence threshold for finalizing lines or basins"));
    
    auto* convergenceGradLayout = new QHBoxLayout();
    convergenceGradLayout->addWidget(convergenceGradLabel_);
    convergenceGradLayout->addWidget(convergenceGradEdit_);
 
    auto* buildGraphLayout = new QVBoxLayout(buildGraphGroup_);
    buildGraphLayout->addWidget(buildGraphCheck_);
    buildGraphLayout->addLayout(boxGraphSizeLayout,Qt::AlignCenter);
    buildGraphLayout->addLayout(convergenceGradLayout,Qt::AlignCenter);
    
    // ---- Atomic basins ----
    buildBasinGroup_ = new QGroupBox("", this);
    buildBasinGroup_->setVisible(false);
    buildBasinCheck_ = new QCheckBox(tr("Construct 3D atomic basin"), mappingGroup_);
    buildBasinCheck_->setChecked(false);
    
    boxBasinSizeLabel_ = new QLabel(tr("Box size"), buildBasinGroup_);
    boxBasinSizeEdit_ = new QLineEdit(buildBasinGroup_);
    boxBasinSizeEdit_->setText("6.0");
    boxBasinSizeEdit_->setValidator(myDoubleValidator_);
    boxBasinSizeEdit_->setToolTip(tr("Square box size computed as distance of farthest CP plus this margin size"));
    boxBasinSizeEdit_->setFixedWidth(100);

    auto* boxBasinSizeLayout = new QGridLayout();
    boxBasinSizeLayout->setColumnStretch(2, 1);
    boxBasinSizeLayout->setHorizontalSpacing(8);
    boxBasinSizeLayout->setVerticalSpacing(5);
    boxBasinSizeLayout->addWidget(boxBasinSizeLabel_, 0, 0);
    boxBasinSizeLayout->addWidget(boxBasinSizeEdit_, 0, 1);
    boxBasinSizeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);

    extraConnectionsGroup_= new QGroupBox("", this);
    extraConnectionsGroup_->setVisible(false);
    
    extraConnectionsCheck_ = new QCheckBox(tr("Extra connections"), buildBasinGroup_);
    
    connectionsThresholdLabel_ = new QLabel(tr("Connection threshold"), buildBasinGroup_);
    connectionsThresholdEdit_ = new QLineEdit(buildBasinGroup_);
    connectionsThresholdEdit_->setText("3.0");
    connectionsThresholdEdit_->setValidator(myDoubleValidator_);
    connectionsThresholdEdit_->setToolTip(tr("Used to determine the number of connecting lines in basin"));
    connectionsThresholdEdit_->setFixedWidth(100);

    auto* connectionsThresholdLayout = new QHBoxLayout(extraConnectionsGroup_);
    connectionsThresholdLayout->setContentsMargins(5, 0, 5, 5);
    connectionsThresholdLayout->addWidget(connectionsThresholdLabel_);
    connectionsThresholdLayout->addWidget(connectionsThresholdEdit_);
    
    initialStepLabel_ = new QLabel(tr("Initial step"), mappingGroup_);
    initialStepEdit_ = new QLineEdit(mappingGroup_);
    initialStepEdit_->setText("0.02");
    initialStepEdit_->setValidator(myDoubleValidator_);
    initialStepEdit_->setFixedWidth(100);
    
    auto* initialStepLayout = new QHBoxLayout();
    initialStepLayout->addWidget(initialStepLabel_);
    initialStepLayout->addWidget(initialStepEdit_);
    
    auto* buildBasinLayout = new QVBoxLayout(buildBasinGroup_);
    buildBasinLayout->addLayout(boxBasinSizeLayout,Qt::AlignCenter);
    buildBasinLayout->addWidget(extraConnectionsCheck_);
    buildBasinLayout->addWidget(extraConnectionsGroup_);
    
    
    
    auto* topoMapLayout = new QVBoxLayout(mappingGroup_);
    topoMapLayout->addWidget(mapCriticalCheck_);
    topoMapLayout->addLayout(boxMarginsSizeLayout);
    topoMapLayout->addLayout(convergenceThresholdLayout);
    topoMapLayout->addWidget(guessPointsCheck_);
    topoMapLayout->addWidget(guessPointsGroup_);
    topoMapLayout->addLayout(boxStepLayout);
    topoMapLayout->addWidget(buildGraphCheck_);
    topoMapLayout->addWidget(buildGraphGroup_);
    topoMapLayout->addWidget(buildBasinCheck_);
    topoMapLayout->addWidget(buildBasinGroup_);
    topoMapLayout->addLayout(initialStepLayout,Qt::AlignCenter);
    
    
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
    mainLayout->addWidget(topoTypeGroup_);
    mainLayout->addWidget(mappingGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();
  
}
    
void TopographyPage::connectSignals()
{
    connect(densityRadio_, &QRadioButton::toggled,
            this, &TopographyPage::topoTypeChanged);
    
    connect(guessPointsCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::topoAddGuessChanged);
    
    connect(addPointsToTableCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::topoXYZChanged);
    
    connect(buildBasinCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::topoBasinChanged);
    
    connect(buildGraphCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::topoMolGraphChanged);
    
    connect(extraConnectionsCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::extraConnectionsChanged);
    
    connect(guessFileButton_, &QPushButton::clicked,
            this, &TopographyPage::importFile);
    
    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::inputOnlyStateChanged);
    
    connect(mpiCheck_, &QCheckBox::stateChanged,
            this, &TopographyPage::mpiStateChanged);
    
    connect(execButton_, &QPushButton::clicked,
            this, &TopographyPage::execRequested);
    
    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);
    
    connect(outputButton_, &QPushButton::clicked,
            this, &TopographyPage::openOutputRequested);

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

int TopographyPage::lMax() const { return lMaxSpin_->value(); }
int TopographyPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int TopographyPage::xyzTableRows() const { return xyzSheet_->tabla->rowCount();}

bool TopographyPage::isBuildBasin() const { return buildBasinCheck_->isChecked(); }
bool TopographyPage::isBuildGraph() const { return buildGraphCheck_->isChecked(); }
bool TopographyPage::isExtraConnections() const { return extraConnectionsCheck_->isChecked(); }
bool TopographyPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool TopographyPage::isGuessPoints() const { return guessPointsCheck_->isChecked(); }
bool TopographyPage::isMapCritical() const { return mapCriticalCheck_->isChecked(); }
bool TopographyPage::isMED() const { return topoButtonsGroup_->button(0)->isChecked(); }
bool TopographyPage::isMESP() const { return topoButtonsGroup_->button(1)->isChecked(); }
bool TopographyPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool TopographyPage::isTopoDensity() const { return densityRadio_->isChecked(); }
bool TopographyPage::isXYZTabulation() const { return addPointsToTableCheck_->isChecked(); }

void TopographyPage::clearSheet() {xyzSheet_->clear(); }
void TopographyPage::insertRow(int i) {xyzSheet_->tabla->insertRow(i); }
void TopographyPage::resizeSheet(int i) {xyzSheet_->resizeRows(i); }
void TopographyPage::setAddPointsToTable(bool checked) { addPointsToTableCheck_->setChecked(checked); }
void TopographyPage::setAddPointsToTableVisible(bool visible) { addPointsToTableCheck_->setVisible(visible); }
void TopographyPage::setBuildBasinCheck(bool checked) { buildBasinCheck_->setChecked(checked); }
void TopographyPage::setBuildGraphCheck(bool checked) { buildGraphCheck_->setChecked(checked); }
void TopographyPage::setBoxBasinSize(const QString& value) { boxBasinSizeEdit_->setText(value); }
void TopographyPage::setCellValue(const QString& value, int i, int j) {xyzSheet_->setcellvalue(value,i,j); }
void TopographyPage::setConnectionsThreshold(const QString& value) { connectionsThresholdEdit_->setText(value); }
void TopographyPage::setDensityRadioChecked(bool checked) { topoButtonsGroup_->button(0)->setChecked(checked); }
void TopographyPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void TopographyPage::setExtraConnectionsCheck(bool checked) { extraConnectionsCheck_->setChecked(checked); }
void TopographyPage::setGuessPoints(bool checked) { guessPointsCheck_->setChecked(checked); }
void TopographyPage::setInitialStep(const QString& value) { initialStepEdit_->setText(value); }
void TopographyPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void TopographyPage::setMapCriticalCheck(bool checked) { mapCriticalCheck_->setChecked(checked); }
void TopographyPage::setPotentialRadioChecked(bool checked) { topoButtonsGroup_->button(1)->setChecked(checked); }
void TopographyPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void TopographyPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void TopographyPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void TopographyPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void TopographyPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void TopographyPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void TopographyPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void TopographyPage::setXYZEnabled(bool checked) { xyzTable_->setEnabled(checked); }
void TopographyPage::setXYZVisible(bool visible) { xyzTable_->setVisible(visible); }


//    Executes external program DAMDEN  (Computes molecular density or deformations from the atomic partition)
void TopographyPage::execDamTopography()
{
    QString stdOutput;

    QString rootName = "DAMTOPOGRAPHY";
    QString subdir = "TDAM_400";
    QString inputTemplate = "DAMTOPO_400.inp";
    QString inputSection = "DAMTOPOSECT";

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
    request.runMpi = isMpiChecked();
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
    const QString suffixtype = isMED() ? "-d" : "-v";
    const QString suffix = request.runMpi ? "_mpi" : "";

    request.suffix = suffixtype + suffix;

    lastOutputFileName_ =
        QDir(projectFolder_).filePath(
            request.outputPrefix + "-" + request.rootName + request.suffix + ".out"
//            request.outputPrefix + "-" + request.rootName + suffix + ".out"
        );

    setStopEnabled(true);
    runner_->setProjectFolder(projectFolder_);
    runner_->setMpiCommand(mpiCommand_);
    runner_->setMpiFlags(mpiFlags_);
    runner_->start(request);
}

void TopographyPage::extraConnectionsChanged(int state)
{
    if (state == Qt::Checked){
        extraConnectionsGroup_->setVisible(true);
    }
    else{
        extraConnectionsGroup_->setVisible(false);
    }
}

void TopographyPage::importFile()
{
    QFileDialog filedialog(this);
    filedialog.setDirectory(projectFolder_);
    filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString fileName = filedialog.getOpenFileName(this,tr("Open file ..."),projectFolder_,
            tr("Import data from")+" (*.xyz);;"+
            tr("All files")+" (*)");
    if (fileName.length()==0){
        return;
    }
    guessFileEdit_->setText(fileName);
}

void TopographyPage::handleNormalProcessExit()
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

void TopographyPage::inputOnlyStateChanged(int state)
{
    if (!(state == Qt::Checked)){
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

void TopographyPage::loadDefault(){
    setBuildBasinCheck(false);
    setBuildGraphCheck(false);
    setDensityRadioChecked(true);
    clearSheet();
}

void TopographyPage::mpiStateChanged(int state)
{
    Q_UNUSED(state);
    QSignalBlocker blocker(mpiCheck_);
    if (isInputOnly()){
        setMpiEnabled(false);
        setMpiVisible(false);
    }
    else{
        setMpiControlsEnabled(true);
    }
}


void TopographyPage::readFromFile(const std::string& file)
{
    const char* section = "DAMTOPOSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    setDensityRadioChecked(ReadWriteOptions::readRadioButton("MED", section, file));
    setPotentialRadioChecked(ReadWriteOptions::readRadioButton("MESP", section, file));

    setLmax(ReadWriteOptions::readSpinBox("lmaxi", section, file));

    setMapCriticalCheck(ReadWriteOptions::readCheckBox("TOPOGRAPH", section, file));
    ReadWriteOptions::readDoubleToLineEdit("BOXL", section, boxMarginsSizeEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("CNVG", section, convergenceThresholdEdit_, file);

    setGuessPoints(ReadWriteOptions::readCheckBox("addguess", section, file));
    setAddPointsToTable(ReadWriteOptions::readCheckBox("addguess", section, file));

    readXYZTable(file, section);
    xyzTable_->updateGeometry();

    if (addPointsToTableCheck_->isChecked() && xyzTableRows() > 0){
        xyzTable_->setVisible(true);
    }
    else {
        setAddPointsToTable(false);
        xyzTable_->setVisible(false);
    }

    guessFileEdit_->setText(ReadWriteOptions::readTextToLineEdit("guessfile", section, file));

    ReadWriteOptions::readDoubleToLineEdit("BOXT", section, boxSizeEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("stepszt", section, stepSizeEdit_, file);

    setBuildGraphCheck(ReadWriteOptions::readCheckBox("gradpath", section, file));
    ReadWriteOptions::readDoubleToLineEdit("BOXG", section, boxGraphSizeEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("DRCUTCP", section, convergenceGradEdit_, file);

    setBuildBasinCheck(ReadWriteOptions::readCheckBox("basin", section, file));
    setBoxBasinSize(ReadWriteOptions::readTextToLineEdit("BOXB", section, file));
    setExtraConnectionsCheck(ReadWriteOptions::readCheckBox("exdraw", section, file));
    setConnectionsThreshold(ReadWriteOptions::readTextToLineEdit("exln", section, file));
    setInitialStep(ReadWriteOptions::readTextToLineEdit("FDISP", section, file));

}

void TopographyPage::readXYZTable(const std::string& file, const char* section)
{
    // ---- Number of rows ----
    const QString numText =
        QString::fromStdString(CIniFile::GetValue("ncntguess", section, file));

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
            const QString key = QString("rcntguess(%1,%2)").arg(j + 1).arg(i + 1);

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


void TopographyPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void TopographyPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void TopographyPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void TopographyPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void TopographyPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void TopographyPage::setProjectFolder(const QString& value)
{
    projectFolder_ = value;
}

void TopographyPage::stopCurrentProcess()
{
    runner_->stop();
}

void TopographyPage::topoAddGuessChanged(int state)
{
    if (state == Qt::Checked){
        guessPointsGroup_->setVisible(true);
    }
    else{
        guessPointsGroup_->setVisible(false);
    }
}

void TopographyPage::topoBasinChanged(int state)
{
    if (state == Qt::Checked){
        buildBasinGroup_->setVisible(true);
    }
    else{
        buildBasinGroup_->setVisible(false);
    }
}

void TopographyPage::topoMolGraphChanged(int state)
{
    if (state == Qt::Checked){
        buildGraphGroup_->setVisible(true);
    }
    else{
        buildGraphGroup_->setVisible(false);
    }
}
    
void TopographyPage::topoTypeChanged(){
    if (isTopoDensity()) {
        convergenceThresholdEdit_->setText("4.0e-15");
        boxMarginsSizeEdit_->setText("1.0");
    }else{
        convergenceThresholdEdit_->setText("4.0e-12");
        boxMarginsSizeEdit_->setText("1.0");
    }
    guessPointsCheck_->setChecked(false);
    guessFileEdit_->setText("");
}

void TopographyPage::topoXYZChanged(int state)
{
    if (state == Qt::Checked){
        setXYZEnabled(true);
        setXYZVisible(true);
    }
    else{
        setXYZEnabled(false);
        setXYZVisible(false);
    }
}


//void TopographyPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool* printwarns,
//                                QString* warns)
void TopographyPage::writeToFile(const std::string& file,
                              bool* printwarns,
                              QString* warns)
{
    const char* section = "DAMTOPOSECT";

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

    writeBool("MED", isMED());
    writeBool("MESP", isMESP());

    // ---- lmax ----
    writeText("lmaxi", QString::number(lMax()));

    // ---- Topography ----
    writeBool("TOPOGRAPH", isMapCritical());
    writeText("BOXL",boxMarginsSizeEdit_->text());
    writeText("CNVG",convergenceThresholdEdit_->text());

    // ---- Guess points ----
    writeBool("addguess", isGuessPoints());

    // ---- XYZ table ----
    ReadWriteOptions::writeXYZTable(section, file, this, Sheet::max_sel, printwarns, warns);

    // ---- Guess file ----
    writeText("guessfile",quoteString(guessFile()));

    ReadWriteOptions::writeOption("BOXT", section, boxSizeEdit_->text(), file, printwarns, warns);
    ReadWriteOptions::writeOption("stepszt", section, stepSizeEdit_->text(), file, printwarns, warns);

    // ---- Molecular graph ----
    writeBool("gradpath", isBuildGraph());
    ReadWriteOptions::writeOption("BOXG", section, boxGraphSizeEdit_->text(), file, printwarns, warns);
    ReadWriteOptions::writeOption("DRCUTCP", section, convergenceGradEdit_->text(), file, printwarns, warns);

    // ---- Atoms basins ----
    writeBool("basin", isBuildBasin());
    ReadWriteOptions::writeOption("BOXB", section, boxBasinSizeEdit_->text(), file, printwarns, warns);
    writeBool("exdraw", isExtraConnections());
    ReadWriteOptions::writeOption("exln", section, connectionsThresholdEdit_->text(), file, printwarns, warns);
    ReadWriteOptions::writeOption("FDISP", section, initialStepEdit_->text(), file, printwarns, warns);

    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

}

QString TopographyPage::getCellValue(int i, int j) const { return xyzSheet_->getcellvalue(i,j); }
QString TopographyPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString TopographyPage::guessFile() const { return guessFileEdit_->text(); }
QString TopographyPage::numtabular() const { return QString("ncntguess"); }
QString TopographyPage::tabularkey() const { return QString("rcntguess"); }
