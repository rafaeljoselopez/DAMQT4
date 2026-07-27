#include "sigmaholepage.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSpinBox>
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

SigmaHolePage::SigmaHolePage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}


void SigmaHolePage::buildUi()
{
    myDoubleValidator_ = new QDoubleValidator(nullptr);
    myDoubleValidator_->setLocale(QLocale::English);
    
    // ---- Import density grid ----
    importDensityGroup_ = new QGroupBox(tr("Import density grid from file:"), this);
    importDensityEdit_ = new QLineEdit(importDensityGroup_);
    importDensityEdit_->setMaximumWidth(280);
    importDensityButton_ =  new QToolButton(importDensityGroup_);
    importDensityButton_->setText(tr("..."));
    
    auto* importDensityLayout = new QHBoxLayout(importDensityGroup_);
    importDensityLayout->addWidget(importDensityEdit_);
    importDensityLayout->addWidget(importDensityButton_);
    
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);
    outputPrefixEdit_->setMaximumWidth(330);
    
    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);
    
    // ---- Thresholds and other options ----
    optionsGroup_ = new QGroupBox("", this);
    densityValueLabel_ = new QLabel(tr("Density value:"), optionsGroup_);
    densityValueEdit_ = new QLineEdit(optionsGroup_);
    densityValueEdit_->setText("0.001");
    densityValueEdit_->setValidator(myDoubleValidator_);
    densityValueEdit_->setToolTip(tr("Density value on isosurface (it defines molecular boundaries)"));
    densityValueEdit_->setMaximumWidth(60);
    
    geometryThresholdLabel_ = new QLabel(tr("Geometry threshold: 10^"), optionsGroup_);
    geometryThresholdSpin_ = new QSpinBox(optionsGroup_);
    geometryThresholdSpin_->setRange(-10, 0);
    geometryThresholdSpin_->setValue(-5);
    geometryThresholdSpin_->setMaximumWidth(60);
   
    longRangeThresholdLabel_ = new QLabel(tr("Long-range threshold 10^"), optionsGroup_);
    longRangeThresholdSpin_ = new QSpinBox(optionsGroup_);
    longRangeThresholdSpin_->setRange(-10, 0);
    longRangeThresholdSpin_->setValue(-9);
    longRangeThresholdSpin_->setMaximumWidth(60);
    
    localMaxThresholdLabel_ = new QLabel(tr("Threshold for maxima:"), optionsGroup_);
    localMaxThresholdLabel_->setToolTip(tr("Defines regions for local maxima search"));
    localMaxThresholdSpin_ = new QSpinBox(optionsGroup_);
    localMaxThresholdSpin_->setRange(20, 99);
    localMaxThresholdSpin_->setValue(70);
    localMaxThresholdSpin_->setSingleStep(5);
    localMaxThresholdSpin_->setMaximumWidth(60);
    
    localMinThresholdLabel_ = new QLabel(tr("Threshold for minima:"), optionsGroup_);
    localMinThresholdLabel_->setToolTip(tr("Defines regions for local minima search"));
    localMinThresholdSpin_ = new QSpinBox(optionsGroup_);
    localMinThresholdSpin_->setRange(20, 99);
    localMinThresholdSpin_->setValue(70);
    localMinThresholdSpin_->setSingleStep(5);
    localMinThresholdSpin_->setMaximumWidth(60);
    
    extremaSeparationLabel_ = new QLabel(tr("Extrema separation"), optionsGroup_);
    extremaSeparationEdit_ = new QLineEdit(this);
    extremaSeparationEdit_->setText("3.0");
    extremaSeparationEdit_->setValidator(myDoubleValidator_);
    extremaSeparationEdit_->setMaximumWidth(60);
    extremaSeparationEdit_->setToolTip(tr("Minimum separation allowed between maxima or minima"));
    
    auto* optionsLayout = new QGridLayout(optionsGroup_);
    optionsLayout->setColumnStretch(2, 1);
    optionsLayout->setHorizontalSpacing(8);
    optionsLayout->setVerticalSpacing(5);
    optionsLayout->addWidget(densityValueLabel_, 0, 0);
    optionsLayout->addWidget(densityValueEdit_, 0, 1);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);
    optionsLayout->addWidget(geometryThresholdLabel_, 1, 0);
    optionsLayout->addWidget(geometryThresholdSpin_, 1, 1);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 2);
    optionsLayout->addWidget(longRangeThresholdLabel_, 2, 0);
    optionsLayout->addWidget(longRangeThresholdSpin_, 2, 1);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 2);
    optionsLayout->addWidget(localMaxThresholdLabel_, 3, 0);
    optionsLayout->addWidget(localMaxThresholdSpin_, 3, 1);
    optionsLayout->addWidget(new QLabel("x 10<sup>-2</sup>"), 3, 2);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 3, 2);
    optionsLayout->addWidget(localMinThresholdLabel_, 4, 0);
    optionsLayout->addWidget(localMinThresholdSpin_, 4, 1);
    optionsLayout->addWidget(new QLabel("x 10<sup>-2</sup>"), 4, 2);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 4, 2);
    optionsLayout->addWidget(extremaSeparationLabel_, 5, 0);
    optionsLayout->addWidget(extremaSeparationEdit_, 5, 1);
    optionsLayout->addWidget(new QLabel("bohr"), 5, 2);
    optionsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 5, 2);
    
    exactPotentialCheck_ = new QCheckBox(tr("Exact potential"), this);
    
    potentialExpansionGroup_ = new QGroupBox(tr("Potential expansion"), this);
    potentialExpansionLabel_ = new QLabel(tr("Highest l"), potentialExpansionGroup_);
    potentialExpansionSpin_ = new QSpinBox(potentialExpansionGroup_);
    potentialExpansionSpin_->setRange(0, 25);
    potentialExpansionSpin_->setValue(10);
    potentialExpansionSpin_->setMaximumWidth(60);

    auto* potentialExpansionLayout = new QHBoxLayout(potentialExpansionGroup_);
    potentialExpansionLayout->addWidget(potentialExpansionLabel_);
    potentialExpansionLayout->addWidget(potentialExpansionSpin_);
    
    
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
    mainLayout->addWidget(importDensityGroup_);
    mainLayout->addWidget(outputGroup_);
    mainLayout->addWidget(optionsGroup_);
    mainLayout->addWidget(exactPotentialCheck_);
    mainLayout->addWidget(potentialExpansionGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();
    
}

void SigmaHolePage::connectSignals()
{

    connect(importDensityEdit_, &QLineEdit::textChanged,
            this, &SigmaHolePage::importSGholeDensityChanged);

    connect(importDensityButton_, &QToolButton::clicked,
            this, &SigmaHolePage::importSGdensity);

    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &SigmaHolePage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
                this, &SigmaHolePage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &SigmaHolePage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &SigmaHolePage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &SigmaHolePage::openOutputRequested);


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

int SigmaHolePage::geometryThreshold() const { return geometryThresholdSpin_->value(); }
int SigmaHolePage::lMax() const { return potentialExpansionSpin_->value(); }
int SigmaHolePage::localMaxThreshold() const { return localMaxThresholdSpin_->value(); }
int SigmaHolePage::localMinThreshold() const { return localMinThresholdSpin_->value(); }
int SigmaHolePage::longRangeThreshold() const { return longRangeThresholdSpin_->value(); }
int SigmaHolePage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }

bool SigmaHolePage::isExactPotential() const { return exactPotentialCheck_->isChecked(); }
bool SigmaHolePage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool SigmaHolePage::isMpiChecked() const { return mpiCheck_->isChecked(); }

void SigmaHolePage::setDensityValue(const QString& value) { densityValueEdit_->setText(value); }
void SigmaHolePage::setExactPotential(bool checked) { exactPotentialCheck_->setChecked(checked); }
void SigmaHolePage::setExactPotentialEnabled(bool enabled) { exactPotentialCheck_->setEnabled(enabled); }
void SigmaHolePage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void SigmaHolePage::setExtremaSeparation(const QString& value) { extremaSeparationEdit_->setText(value); }
void SigmaHolePage::setGeometryThreshold(int value) { geometryThresholdSpin_->setValue(value); }
void SigmaHolePage::setImport(const QString& value) { importDensityEdit_->setText(value); }
void SigmaHolePage::setLmax(int lmax) { potentialExpansionSpin_->setValue(lmax); }
void SigmaHolePage::setLmaxTop(int ltop) { potentialExpansionSpin_->setRange(0,ltop); }
void SigmaHolePage::setLocalMaxThreshold(int value) { localMaxThresholdSpin_->setValue(value); }
void SigmaHolePage::setLocalMinThreshold(int value) { localMinThresholdSpin_->setValue(value); }
void SigmaHolePage::setLongRangeThreshold(int value) { longRangeThresholdSpin_->setValue(value); }
void SigmaHolePage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void SigmaHolePage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void SigmaHolePage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void SigmaHolePage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void SigmaHolePage::setOptionsEnabled(bool enabled) { optionsGroup_->setEnabled(enabled); }
void SigmaHolePage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void SigmaHolePage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void SigmaHolePage::setPotentialExpansionEnabled(bool enabled) { potentialExpansionGroup_->setEnabled(enabled); }
void SigmaHolePage::setProjectFolder(const QString& value) { projectFolder_ = value;}
void SigmaHolePage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }


//    Executes external program DAMDENSGHOLE  (Computes electrostatic potential on molecular surface)
void SigmaHolePage::execDamSGhole()
{
    QString stdOutput;

    QString rootName = "DAMSGHOLE_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMSGHOLE_400.inp";
    QString inputSection = "DAMSGHOLESECT";

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

    const QString exactsuffix = isExactPotential() ? "_exact" : "";

    const QString suffix = request.runMpi ? "_mpi" : "";

    request.suffix = exactsuffix + suffix;

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


void SigmaHolePage::handleNormalProcessExit()
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

void SigmaHolePage::importSGholeDensityChanged(){
    if (importDensityEdit_->text().isEmpty()){
        setOptionsEnabled(false);
        setPotentialExpansionEnabled(false);
        setMpiCheckEnabled(false);
        setExecEnabled(false);
    }
    else{
        setOptionsEnabled(true);
        setPotentialExpansionEnabled(true);
        setMpiCheckEnabled(true);
        setExecEnabled(true);
    }
}

/* Import name of a file with molecular orbitals*/
void SigmaHolePage::importSGdensity()
{
    QFileDialog filedialog(this);
    filedialog.setDirectory(projectFolder_);
    filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
    QString fullPath = filedialog.getOpenFileName(this,tr("Open file ..."),projectFolder_,
            tr("Import data from")+" (*-d.plt);;"+
            tr("All files")+" (*)");
    if (fullPath.length()==0){
        return;
    }
    QFileInfo fileInfo(fullPath);
    importDensityEdit_->setText(fileInfo.fileName());
}


void SigmaHolePage::inputOnlyStateChanged(int state)
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

void SigmaHolePage::loadDefault(){
    setImport("");
    setDensityValue("0.001");
    setGeometryThreshold(-5);
    setLongRangeThreshold(-9);
    setLocalMaxThreshold(70);
    setLocalMinThreshold(70);
    setExtremaSeparation("3.0");
}


void SigmaHolePage::mpiStateChanged(int state)
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

void SigmaHolePage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}


void SigmaHolePage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void SigmaHolePage::readFromFile(const std::string& file)
{
    const char* section = "DAMSGHOLESECT";

    importDensityEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("gridname", section, file)
    );
    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );
    ReadWriteOptions::readDoubleToLineEdit("contourval", section, densityValueEdit_, file);

    QString value = ReadWriteOptions::readTextToLineEdit("geomthr", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-5");
    setGeometryThreshold(value.toInt());

    value = ReadWriteOptions::readTextToLineEdit("umbrlargo", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-9");
    setLongRangeThreshold(value.toInt());

    setLocalMaxThreshold(ReadWriteOptions::readSpinBox("thrslocalmax", section, file));
    setLocalMinThreshold(ReadWriteOptions::readSpinBox("thrslocalmin", section, file));
    ReadWriteOptions::readDoubleToLineEdit("separation", section, extremaSeparationEdit_, file);
    setExactPotential(ReadWriteOptions::readCheckBox("lexact", section, file));
    setLmax(ReadWriteOptions::readSpinBox("lmaxrep", section, file));
}

void SigmaHolePage::setIsValence(bool valence)
{
    lvalence_ = valence;
}

void SigmaHolePage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void SigmaHolePage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void SigmaHolePage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void SigmaHolePage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMSGHOLESECT";

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

    // ---- Density file ----
    writeText("gridname",quoteString(importFile()));

    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

    writeText("contourval", densityValueEdit_->text());
    writeText("geomthr", QString("1.d%1").arg(geometryThreshold()));
    writeText("umbrlargo", QString("1.d%1").arg(longRangeThreshold()));
    writeText("thrslocalmax", QString("%1.d-2").arg(localMaxThreshold()));
    writeText("thrslocalmin", QString("%1.d-2").arg(localMinThreshold()));
    writeText("separation", extremaSeparationEdit_->text());
    writeBool("lexact", isExactPotential());
    writeText("lmaxrep", QString::number(lMax()));

}

QString SigmaHolePage::importFile() const { return importDensityEdit_->text(); }
QString SigmaHolePage::outputPrefix() const { return outputPrefixEdit_->text(); }
