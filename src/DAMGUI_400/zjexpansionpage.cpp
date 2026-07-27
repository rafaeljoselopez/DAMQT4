#include "zjexpansionpage.h"

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

#include "readwriteoptions.h"
#include "IniFile.h"

#ifndef MAX_NUM_PROCESSORS
#define MAX_NUM_PROCESSORS 128
#endif
#ifndef SIZE4
#define SIZE4   400
#endif
#ifndef SIZE20
#define SIZE20 2000
#endif
#ifndef MAX_KEXPZJ
#define MAX_KEXPZJ 40
#endif

ZJExpansionPage::ZJExpansionPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void ZJExpansionPage::buildUi()
{
    // ---- Ball size ----
    ballSizeGroup_ = new QGroupBox(tr("Ball size (bohr)"), this);

    ballSizeEdit_ = new QLineEdit(ballSizeGroup_);
    QDoubleValidator *ballSizeValidator = new QDoubleValidator(nullptr);
    ballSizeValidator->setLocale(QLocale::English);
    ballSizeValidator->setBottom(0.0);
    ballSizeEdit_->setText("10.0");
    ballSizeEdit_->setAlignment(Qt::AlignCenter);
    ballSizeEdit_->setValidator(ballSizeValidator);
    ballSizeEdit_->setMaximumWidth(80);
    
    ballRadioTypeGroup_ = new QGroupBox(tr(""), ballSizeGroup_);
    ballRadioTypeGroup_->setMaximumSize(QSize(350, 2000));
    ballAbsoluteRadio_ = new QRadioButton(tr("Absolute"), ballRadioTypeGroup_);
    ballAbsoluteRadio_->setChecked(true);
    ballAbsoluteRadio_->setToolTip(tr("Value of the ball radius in bohr"));
    ballRelativeRadio_ = new QRadioButton(tr("Relative"), ballRadioTypeGroup_);
    ballRelativeRadio_->setToolTip(tr("Value added to the largest distance of nuclei to origin to define the ball radius"));
    ballRadioGroup_ = new QButtonGroup(this);
    ballRadioGroup_->addButton(ballAbsoluteRadio_, ABSOLUTE);
    ballRadioGroup_->addButton(ballRelativeRadio_, RELATIVE);
    
    auto* ballRadioTypeLayout = new QVBoxLayout(ballRadioTypeGroup_);
    ballRadioTypeLayout->addWidget(ballAbsoluteRadio_);
    ballRadioTypeLayout->addWidget(ballRelativeRadio_);

    auto* ballSizeVerticalLayout = new QVBoxLayout(ballSizeGroup_);
    ballSizeVerticalLayout->addWidget(ballSizeEdit_, 0, Qt::AlignHCenter);
    ballSizeVerticalLayout->addWidget(ballRadioTypeGroup_, 0, Qt::AlignHCenter);
    
    // ---- Expansion length ----
    expansionLengthGroup_ = new QGroupBox(tr("Expansion length"), this);
    expansionLengthGroup_->setMaximumSize(QSize(SIZE4, SIZE20));
    
    lMaxLabel_ = new QLabel(tr("Highest l"), expansionLengthGroup_);
    lMaxSpin_  = new QSpinBox(expansionLengthGroup_);
    lMaxSpin_->setRange(0, 22);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);
    lMaxSpin_->setToolTip(tr("Size of expansion"));
    
    kMaxLabel_ = new QLabel(tr("Highest k"), expansionLengthGroup_);
    kMaxSpin_  = new QSpinBox(expansionLengthGroup_);
    kMaxSpin_->setRange(0, MAX_KEXPZJ);
    kMaxSpin_->setValue(10);
    kMaxSpin_->setMaximumWidth(60);
    kMaxSpin_->setToolTip(tr("Size of expansion"));
    
    echelonRadio_ = new QRadioButton(tr("Echelon type"), expansionLengthGroup_);
    echelonRadio_->setToolTip(tr("Number of functions per l equal to max(highest l+1, highest k)-l"));
    echelonRadio_->setChecked(false);
    
    auto* lmaxLayout = new QHBoxLayout();
    lmaxLayout->addWidget(lMaxLabel_, 0, Qt::AlignRight);
    lmaxLayout->addWidget(lMaxSpin_, 0, Qt::AlignLeft);
    
    auto* kmaxLayout = new QHBoxLayout();
    kmaxLayout->addWidget(kMaxLabel_, 0, Qt::AlignRight);
    kmaxLayout->addWidget(kMaxSpin_, 0, Qt::AlignLeft);
    
    auto* expansionLengthLayout = new QVBoxLayout(expansionLengthGroup_);
    expansionLengthLayout->addLayout(lmaxLayout);
    expansionLengthLayout->addLayout(kmaxLayout);
    expansionLengthLayout->addWidget(echelonRadio_, 0, Qt::AlignCenter);
    
    // ---- Type of fitting ----
    fittingTypeGroup_ = new QGroupBox(tr("Expansion length"), this);
    zernike3DRadio_ = new QRadioButton(tr("Zernike 3D"),fittingTypeGroup_);
    zernike3DRadio_->setChecked(true);
    jacobiRadio_ = new QRadioButton(tr("Jacobi"),fittingTypeGroup_);
    
    fittingGroup_ = new QButtonGroup(this);
    fittingGroup_->addButton(zernike3DRadio_, ZERNIKE);
    fittingGroup_->addButton(jacobiRadio_, JACOBI);
    
    auto* fittingTypeLayout = new QVBoxLayout(fittingTypeGroup_);
    fittingTypeLayout->addWidget(zernike3DRadio_);
    fittingTypeLayout->addWidget(jacobiRadio_);
    
    // ---- Quadrature length ----
    quadratureGroup_ = new QGroupBox(tr("Quadrature length"), this);
    quadratureGroup_->setMaximumSize(QSize(SIZE4, SIZE20));
    
    quadratureLabel_ = new QLabel(tr("No. of sampling points"), quadratureGroup_);
    quadratureSpin_ = new QSpinBox(quadratureGroup_);
    quadratureSpin_->setMinimum(128);
    quadratureSpin_->setMaximum(8192);
    quadratureSpin_->setValue(256);
    quadratureSpin_->setSingleStep(128);
    quadratureSpin_->setMaximumWidth(100);
    quadratureSpin_->setToolTip(tr("Number of points for sampling radial factors in projection"));
    
    auto* quadratureGroupLayout = new QHBoxLayout(quadratureGroup_);
    quadratureGroupLayout->addWidget(quadratureLabel_);
    quadratureGroupLayout->addWidget(quadratureSpin_);
    
    // ---- Thresholds ----
    thresholdsGroup_ = new QGroupBox(tr("Thresholds"), this);
    
    multipoleCutoffLabel_ = new QLabel(tr("Multipole cutoff: 10^"), thresholdsGroup_);
    multipoleCutoffSpin_ = new QSpinBox(thresholdsGroup_);
    multipoleCutoffSpin_->setRange(-15, 0);
    multipoleCutoffSpin_->setValue(-10);
    multipoleCutoffSpin_->setMaximumWidth(65);
    multipoleCutoffSpin_->setToolTip(tr("Cutoff for multipole printing"));
    
    distributionsCutoffLabel_ = new QLabel(tr("Distributions cutoff: 10^"), thresholdsGroup_);
    distributionsCutoffSpin_ = new QSpinBox(thresholdsGroup_);
    distributionsCutoffSpin_->setRange(-15, 0);
    distributionsCutoffSpin_->setValue(-12);
    distributionsCutoffSpin_->setMaximumWidth(65);
    distributionsCutoffSpin_->setToolTip(tr("Overlap cutoff for neglecting distributions"));

    auto* multipoleCutoffLayout = new QHBoxLayout();
    multipoleCutoffLayout->addWidget(multipoleCutoffLabel_, 0, Qt::AlignRight);
    multipoleCutoffLayout->addWidget(multipoleCutoffSpin_, 0, Qt::AlignRight);

    auto* distributionsCutoffLayout = new QHBoxLayout();
    distributionsCutoffLayout->addWidget(distributionsCutoffLabel_, 0, Qt::AlignRight);
    distributionsCutoffLayout->addWidget(distributionsCutoffSpin_, 0, Qt::AlignRight);

    auto* thresholdsLayout = new QVBoxLayout(thresholdsGroup_);
    thresholdsLayout->addLayout(multipoleCutoffLayout);
    thresholdsLayout->addLayout(distributionsCutoffLayout);
    
    // ---- Input only ----
    inputOnlyGroup_ = new QGroupBox(tr("Input only"), this);
    inputOnlyCheck_ = new QCheckBox(tr("Generate input file only"), inputOnlyGroup_);
    inputOnlyCheck_->setChecked(false);

    auto* inputOnlyLayout = new QVBoxLayout(inputOnlyGroup_);
    inputOnlyLayout->addWidget(inputOnlyCheck_);

    // ---- MPI ----
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

    // ---- Buttons ----
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
    mainLayout->addWidget(ballSizeGroup_);
    mainLayout->addWidget(expansionLengthGroup_);
    mainLayout->addWidget(fittingTypeGroup_);
    mainLayout->addWidget(quadratureGroup_);
    mainLayout->addWidget(thresholdsGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();
    
}
    
void ZJExpansionPage::connectSignals()
{
    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &ZJExpansionPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
                this, &ZJExpansionPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &ZJExpansionPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &ZJExpansionPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);

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
 
int ZJExpansionPage::distributionsCutoff() const { return distributionsCutoffSpin_->value(); }
int ZJExpansionPage::kMax() const { return kMaxSpin_->value(); }
int ZJExpansionPage::kMaxTop() const { return kMaxSpin_->maximum(); }
int ZJExpansionPage::lMax() const { return lMaxSpin_->value(); }
int ZJExpansionPage::lMaxTop() const { return lMaxSpin_->maximum(); }
int ZJExpansionPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }
int ZJExpansionPage::multipoleCutoff() const { return multipoleCutoffSpin_->value(); }
int ZJExpansionPage::nquadPoints() const { return quadratureSpin_->value(); }

bool ZJExpansionPage::isAbsoluteRadio() const { return ballAbsoluteRadio_->isChecked(); }
bool ZJExpansionPage::isEchelon() const { return echelonRadio_->isChecked(); }
bool ZJExpansionPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool ZJExpansionPage::isJacobi() const { return jacobiRadio_->isChecked(); }
bool ZJExpansionPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool ZJExpansionPage::isRelativeRadio() const { return ballRelativeRadio_->isChecked(); }
bool ZJExpansionPage::isZernike() const { return zernike3DRadio_->isChecked(); }

void ZJExpansionPage::setAbsoluteRadio(bool checked) { ballAbsoluteRadio_->setChecked(checked); }
void ZJExpansionPage::setBallSize(const QString &value) { ballSizeEdit_->setText(value); }
void ZJExpansionPage::setDistributionsCutoff(int value) { distributionsCutoffSpin_->setValue(value); }
void ZJExpansionPage::setDistributionsCutoffMinimum(int value) { distributionsCutoffSpin_->setMinimum(value); }
void ZJExpansionPage::setEchelon(bool checked) { echelonRadio_->setChecked(checked); }
void ZJExpansionPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void ZJExpansionPage::setJacobi(bool checked) { jacobiRadio_->setChecked(checked); }
void ZJExpansionPage::setKmax(int lmax) { lMaxSpin_->setValue(lmax); }
void ZJExpansionPage::setKmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void ZJExpansionPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void ZJExpansionPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void ZJExpansionPage::setMpiChecked(bool checked) { mpiCheck_->setChecked(checked); }
void ZJExpansionPage::setMpiCheckEnabled(bool enabled) { mpiCheck_->setEnabled(enabled); }
void ZJExpansionPage::setMpiProcessors(int value) { mpiProcessorsSpin_->setValue(value); }
void ZJExpansionPage::setMpiVisible(bool visible) { mpiGroup_->setVisible(visible); }
void ZJExpansionPage::setMultipoleCutoff(int value) { multipoleCutoffSpin_->setValue(value); }
void ZJExpansionPage::setMultipoleCutoffMinimum(int value) { multipoleCutoffSpin_->setMinimum(value); }
void ZJExpansionPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void ZJExpansionPage::setQuadratureLength(int value) { quadratureSpin_->setValue(value); }
void ZJExpansionPage::setQuadratureMaximum(int value) { quadratureSpin_->setMaximum(value); }
void ZJExpansionPage::setQuadratureMinimum(int value) { quadratureSpin_->setMinimum(value); }
void ZJExpansionPage::setRelativeRadio(bool checked) { ballRelativeRadio_->setChecked(checked); }
void ZJExpansionPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void ZJExpansionPage::setZernike(bool checked) { zernike3DRadio_->setChecked(checked); }


//    Executes external program DAMDEN  (Computes molecular density or deformations from the atomic partition)
void ZJExpansionPage::execDamZJ()
{
    QString stdOutput;

    QString rootName;
    if (lslater_){
        rootName = QString("DAMZERNIKE-JACOBI_STO");
    } else {
        rootName = QString("DAMZERNIKE-JACOBI_GTO");
    }
    QString subdir = "DAMZERNIKE_400";
    QString inputTemplate = "DAMZJ_400.inp";
    QString inputSection = "DAMZJSECT";

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

    request.outputPrefix = projectName_;

    request.rootName = rootName;
    request.stdInput = stdInput;
    request.stdOutput = stdOutput;
    request.subdir = subdir;
    request.nprocs = mpiProcessors();
    const QString suffixtype = isZernike() ? "-Zernike" : "-Jacobi";
    const QString suffix = request.runMpi ? "_mpi" : "";

    request.suffix = suffixtype + suffix;

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


void ZJExpansionPage::handleNormalProcessExit()
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

void ZJExpansionPage::inputOnlyStateChanged(int state)
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

void ZJExpansionPage::loadDefault(){
    setBallSize("10.0");
    setRelativeRadio(false);
    setLmaxTop(MAX_LEXPZJ);
    setLmax(10);
    setKmax(10);
    setEchelon(false);
    setJacobi(false);
    setQuadratureLength(256);
    setQuadratureMinimum(128);
    setQuadratureMaximum(8192);
    setDistributionsCutoffMinimum(-15);
    setDistributionsCutoff(-10);
    setMultipoleCutoffMinimum(-15);
    setMultipoleCutoff(-12);
}


void ZJExpansionPage::mpiStateChanged(int state)
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

void ZJExpansionPage::readFromFile(const std::string& file)
{
    const char* section = "DAMZJSECT";
    
    ReadWriteOptions::readDoubleToLineEdit("rstar", section, ballSizeEdit_, file);
    setRelativeRadio(ReadWriteOptions::readRadioButton("lrstarrel", section, file));
    setAbsoluteRadio(!isRelativeRadio());
    
    setLmax(ReadWriteOptions::readSpinBox("lexpansion", section, file));
    setKmax(ReadWriteOptions::readSpinBox("kexpansion", section, file));
    setEchelon(ReadWriteOptions::readRadioButton("lechelon", section, file));
    
    setJacobi(ReadWriteOptions::readRadioButton("ljacobi", section, file));
    setZernike(!isJacobi());
    
    setQuadratureLength(ReadWriteOptions::readSpinBox("nquadpoints", section, file));
    
    QString value = ReadWriteOptions::readTextToLineEdit("thresmult", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-10)");
    setMultipoleCutoff(value.toInt());
    
    value = ReadWriteOptions::readTextToLineEdit("thresoverlap", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-12)");
    setDistributionsCutoff(value.toInt());
    
}

void ZJExpansionPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void ZJExpansionPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void ZJExpansionPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void ZJExpansionPage::setProjectFolder(const QString& value)
{
    projectFolder_ = value;
}

void ZJExpansionPage::setSlater(bool slater)
{
    lslater_ = slater;
}


void ZJExpansionPage::stopCurrentProcess()
{
    runner_->stop();
}

void ZJExpansionPage::writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns)
{
    const char* section = "DAMZJSECT";

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
    
    writeText("rstar", ballSizeEdit_->text());

    writeBool("lrstarrel", isRelativeRadio());

    writeText("lexpansion", QString::number(lMax()));
    writeText("kexpansion", QString::number(kMax()));
    writeBool("lechelon", isEchelon());

    writeBool("ljacobi", isJacobi());

    writeText("nquadpoints", QString::number(nquadPoints()));

    writeText("thresmult", QString("1.d%1").arg(multipoleCutoff()));
    writeText("thresoverlap", QString("1.d%1").arg(distributionsCutoff()));

}

    
void ZJExpansionPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void ZJExpansionPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}
    
    
    
    
