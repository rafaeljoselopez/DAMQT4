#include "atomicdensitiespage.h"

#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QTextStream>
#include <QTimer>

#include "externalprogramrunner.h"

#ifndef MAX_NUM_PROCESSORS
#define MAX_NUM_PROCESSORS 128
#endif

AtomicDensitiesPage::AtomicDensitiesPage(bool mpiAvailable, QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void AtomicDensitiesPage::buildUi()
{
    lmaxExpansionGroup_ = new QGroupBox(tr("Highest l in expansion"), this);
    lmaxExpansionSpin_ = new QSpinBox(lmaxExpansionGroup_);
    lmaxExpansionSpin_->setRange(0, 25);
    lmaxExpansionSpin_->setValue(10);
    lmaxExpansionSpin_->setMaximumWidth(50);
    lmaxExpansionSpin_->setToolTip(tr("Size of multipolar expansion"));

    auto* lmaxExpansionLayout = new QHBoxLayout(lmaxExpansionGroup_);
    lmaxExpansionLayout->addWidget(lmaxExpansionSpin_);

    lmaxDisplayedGroup_ = new QGroupBox(tr("Highest l to be displayed"), this);
    lmaxDisplayedSpin_ = new QSpinBox(lmaxDisplayedGroup_);
    lmaxDisplayedSpin_->setRange(0, lmaxExpansionSpin_->value());
    lmaxDisplayedSpin_->setValue(5);
    lmaxDisplayedSpin_->setMaximumWidth(50);
    lmaxDisplayedSpin_->setToolTip(
        tr("Highest order of multipoles to be displayed in output file and "
           "whose modules will be tabulated in file .mltmod"));

    auto* lmaxDisplayedLayout = new QHBoxLayout(lmaxDisplayedGroup_);
    lmaxDisplayedLayout->addWidget(lmaxDisplayedSpin_);

    fittingTypeGroup_ = new QGroupBox(tr("Type of fitting"), this);
    totalDensityRadio_ = new QRadioButton(tr("Total density"), fittingTypeGroup_);
    oneCenterRadio_ = new QRadioButton(tr("One-center terms"), fittingTypeGroup_);
    twoCenterRadio_ = new QRadioButton(tr("Two-center terms"), fittingTypeGroup_);
    totalDensityRadio_->setChecked(true);

    auto* fittingLayout = new QVBoxLayout(fittingTypeGroup_);
    fittingLayout->addWidget(totalDensityRadio_);
    fittingLayout->addWidget(oneCenterRadio_);
    fittingLayout->addWidget(twoCenterRadio_);

    thresholdsGroup_ = new QGroupBox(tr("Thresholds"), this);
    fitThresholdLabel_ = new QLabel(tr("Fitting threshold: 10^"), thresholdsGroup_);
    fitThresholdLabel_->setToolTip(tr("Threshold for truncating expansions of radial factors "));
    cutoffThresholdLabel_ = new QLabel(tr("Cutoff threshold: 10^"), thresholdsGroup_);
    cutoffThresholdLabel_->setToolTip(tr("Threshold for neglecting radial factors "));

    fitThresholdSpin_ = new QSpinBox(thresholdsGroup_);
    fitThresholdSpin_->setRange(-20, 0);
    fitThresholdSpin_->setValue(-14);
    fitThresholdSpin_->setMaximumWidth(60);

    cutoffThresholdSpin_ = new QSpinBox(thresholdsGroup_);
    cutoffThresholdSpin_->setRange(-20, 0);
    cutoffThresholdSpin_->setValue(-14);
    cutoffThresholdSpin_->setMaximumWidth(60);

    auto* thresholdsLayout = new QGridLayout(thresholdsGroup_);
    thresholdsLayout->addWidget(cutoffThresholdLabel_, 0, 0, Qt::AlignRight);
    thresholdsLayout->addWidget(cutoffThresholdSpin_, 0, 1, Qt::AlignLeft);
    thresholdsLayout->addWidget(fitThresholdLabel_, 1, 0, Qt::AlignRight);
    thresholdsLayout->addWidget(fitThresholdSpin_, 1, 1, Qt::AlignLeft);

    inputOnlyGroup_ = new QGroupBox(tr("Input only"), this);
    inputOnlyCheck_ = new QCheckBox(tr("Generate input file only"), inputOnlyGroup_);
    inputOnlyCheck_->setChecked(false);

    auto* inputOnlyLayout = new QVBoxLayout(inputOnlyGroup_);
    inputOnlyLayout->addWidget(inputOnlyCheck_);

    mpiGroup_ = new QGroupBox(tr("Parallel computing"), this);
    mpiCheck_ = new QCheckBox(tr("MPI"), mpiGroup_);
    mpiProcessorsLabel_ = new QLabel(tr("Number of processors"), mpiGroup_);
    mpiProcessorsSpin_ = new QSpinBox(mpiGroup_);
    mpiProcessorsSpin_->setRange(1, MAX_NUM_PROCESSORS);
    mpiProcessorsSpin_->setValue(1);
    mpiProcessorsSpin_->setMaximumWidth(50);

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
    mainLayout->addWidget(lmaxExpansionGroup_);
    mainLayout->addWidget(lmaxDisplayedGroup_);
    mainLayout->addWidget(fittingTypeGroup_);
    mainLayout->addWidget(thresholdsGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addWidget(mpiGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();
}

void AtomicDensitiesPage::connectSignals()
{
    connect(lmaxExpansionSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
                lmaxDisplayedSpin_->setMaximum(value);
                emit lmaxExpansionChanged(value);
            });


    connect(inputOnlyCheck_, &QCheckBox::stateChanged,
            this, &AtomicDensitiesPage::inputOnlyStateChanged);

    connect(mpiCheck_, &QCheckBox::stateChanged,
            this, &AtomicDensitiesPage::mpiStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &AtomicDensitiesPage::execRequested);

    connect(stopButton_, &QPushButton::clicked,
                runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &AtomicDensitiesPage::openOutputRequested);

    connect(runner_, &ExternalProgramRunner::started,
                this, [this]() {
                    setExecEnabled(false);
                    setStopEnabled(true);
                    emit externalProcessStarted();
                });

//    connect(runner_, &ExternalProgramRunner::finished,
//            this, [this](int, QProcess::ExitStatus exitStatus) {
//                setExecEnabled(true);
//                setStopEnabled(false);

//                const bool finishedOk = (exitStatus == QProcess::NormalExit);

//                if (finishedOk) {
//                    handleNormalProcessExit();
//                } else if (exitStatus == QProcess::CrashExit) {
//                    emit statusMessageRequested(
//                        tr("Process crashed, exit code = %1").arg(exitStatus)
//                    );
//                }

//                emit execPagesEnabledChanged(finishedOk);
//                emit externalProcessFinished(finishedOk);
//            });

//    connect(runner_, &ExternalProgramRunner::finished,
//            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {

//                const bool finishedOk =
//                    exitStatus == QProcess::NormalExit && exitCode == 0;

//                if (runningOperation_ == RunningOperation::Sgbs2Sxyz) {

//                    runningOperation_ = RunningOperation::None;

//                    if (!finishedOk) {
//                        QFile::remove(sgbs2sxyzInputFile_);

//                        setExecEnabled(true);
//                        setStopEnabled(false);

//                        emit statusMessageRequested(
//                            tr("sgbs2sxyz failed, exit code = %1")
//                                .arg(exitCode)
//                        );

//                        emit execPagesEnabledChanged(false);
//                        emit externalProcessFinished(false);
//                        return;
//                    }

//                    if (finishSgbs2Sxyz()) {
//                        startDam();
//                        return;
//                    }

//                    setExecEnabled(true);
//                    setStopEnabled(false);

//                    emit execPagesEnabledChanged(false);
//                    emit externalProcessFinished(false);
//                    return;
//                }

//                runningOperation_ = RunningOperation::None;

//                setExecEnabled(true);
//                setStopEnabled(false);

//                if (finishedOk) {
//                    handleNormalProcessExit();
//                } else {
//                    emit statusMessageRequested(
//                        tr("Process failed, exit code = %1").arg(exitCode)
//                    );
//                }

//                emit execPagesEnabledChanged(finishedOk);
//                emit externalProcessFinished(finishedOk);
//            });

    connect(runner_, &ExternalProgramRunner::finished,
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {

                const bool finishedOk =
                    exitStatus == QProcess::NormalExit && exitCode == 0;

                if (runningOperation_ == RunningOperation::Sgbs2Sxyz) {

                    qDebug() << "sgbs2sxyz finished:"
                                 << "exitCode =" << exitCode
                                 << "exitStatus =" << exitStatus;

                    qDebug() << "sxyz exists:"
                             << QFile::exists(sgbs2sxyzTargetFile_);

                    runningOperation_ = RunningOperation::None;

                    if (!finishedOk) {
                        QFile::remove(sgbs2sxyzInputFile_);

                        setExecEnabled(true);
                        setStopEnabled(false);

                        emit statusMessageRequested(
                            tr("sgbs2sxyz failed, exit code = %1")
                                .arg(exitCode)
                        );

                        emit execPagesEnabledChanged(false);
                        emit externalProcessFinished(false);
                        return;
                    }

                    if (finishSgbs2Sxyz()) {
                        QTimer::singleShot(0, this, [this]() {
                            qDebug() << "Starting DAM after sgbs2sxyz";
                            startDam();
                        });

                        return;
                    }

                    setExecEnabled(true);
                    setStopEnabled(false);

                    emit execPagesEnabledChanged(false);
                    emit externalProcessFinished(false);
                    return;
                }

                runningOperation_ = RunningOperation::None;

                setExecEnabled(true);
                setStopEnabled(false);

                if (finishedOk) {
                    handleNormalProcessExit();
                } else {
                    emit statusMessageRequested(
                        tr("Process failed, exit code = %1")
                            .arg(exitCode)
                    );
                }

                emit execPagesEnabledChanged(finishedOk);
                emit externalProcessFinished(finishedOk);
            });

    connect(runner_, &ExternalProgramRunner::errorOccurred,
            this, [this](QProcess::ProcessError) {
                setExecEnabled(true);
                setStopEnabled(false);
                emit externalProcessFinished(false);
            });
}

int AtomicDensitiesPage::cutoffThreshold() const { return cutoffThresholdSpin_->value(); }
int AtomicDensitiesPage::fitThreshold() const { return fitThresholdSpin_->value(); }
int AtomicDensitiesPage::lmaxExpansion() const { return lmaxExpansionSpin_->value(); }
int AtomicDensitiesPage::lmaxDisplayed() const { return lmaxDisplayedSpin_->value(); }
int AtomicDensitiesPage::mpiProcessors() const { return mpiProcessorsSpin_->value(); }

bool AtomicDensitiesPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool AtomicDensitiesPage::isMpiChecked() const { return mpiCheck_->isChecked(); }
bool AtomicDensitiesPage::isMpiEnabled() const { return mpiCheck_->isEnabled(); }
bool AtomicDensitiesPage::isOneCenterChecked() const { return oneCenterRadio_->isChecked(); }
bool AtomicDensitiesPage::isTotalDensityChecked() const { return totalDensityRadio_->isChecked(); }
bool AtomicDensitiesPage::isTwoCenterChecked() const { return twoCenterRadio_->isChecked(); }
bool AtomicDensitiesPage::isWindows() const { return iswindows_; }


void AtomicDensitiesPage::execDam()
{
    if (!lslater_) {
        startDam();
        return;
    }

    const QString sxyzFilename =
        QDir(projectFolder_).filePath(
            QFileInfo(projectName_).completeBaseName() + ".sxyz"
        );

    if (QFile::exists(sxyzFilename)) {
        startDam();
        return;
    }

    runSgbs2Sxyz(sxyzFilename);
}

//    Executes external program DAM (Partition of molecular density into atomic densities)
void AtomicDensitiesPage::startDam(){

//qDebug() << endl << endl << "entra en startDam" << endl << endl;

    QString stdOutput;

    QString rootName;
    QString subdir = "DAM_400";
    QString inputTemplate;
    QString inputSection;

    runningOperation_ = RunningOperation::Dam;

    if (lslater_) {
        rootName = "DAMSTO_400";
        inputTemplate = "DAMSTO_400.inp";
        inputSection = "DAMSECT";
        QString sxyzfilename = projectFolder_+projectName_+".sxyz";
        if (!(QFile::exists(sxyzfilename))){
            if (!(QFile::exists(sxyzfilename))){
                QMessageBox::warning(this, tr("DAMQT"),tr("File %1 does not exist").arg(sxyzfilename));
                return;
            }
        }
    } else {
        rootName = "DAMGTO_400";
        inputTemplate = "DAMGTO_400.inp";
        inputSection = "G-DAMSECT";
    }

    const QString projectFile =
        QDir(projectFolder_).filePath(projectName_ + ".damproj");

    bool printwarns = false;

    QString warns = QString(tr("Warning: failed saving the following options") + ":\n");

    writeToFile(
            projectFile.toStdString(),
            iswindows_,
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

        const QString inputText = in.readAll();

//        qDebug() << "inputText:" << inputText;

        emit showInputFileRequested(inputText);

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

//    const QString suffix = request.runMpi ? "_mpi" : "";

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
}

void AtomicDensitiesPage::runSgbs2Sxyz(const QString& sxyzFilename)
{
    sgbs2sxyzTargetFile_ = sxyzFilename;

    sgbs2sxyzInputFile_ = sxyzFilename;
    sgbs2sxyzInputFile_.replace(".sxyz", ".tmpinp");

    sgbs2sxyzErrorFile_ = sgbs2sxyzInputFile_;
    sgbs2sxyzErrorFile_.replace(".tmpinp", ".err");

    QFile inputFile(sgbs2sxyzInputFile_);

    if (!inputFile.open(QFile::Text | QFile::WriteOnly))
    {
        emit errorOccurred(
            tr("Unable to create temporary input file:\n%1")
                .arg(sgbs2sxyzInputFile_));
        return;
    }

    QTextStream out(&inputFile);

    const QFileInfo info(sxyzFilename);

    const QString baseName =
        info.path() + "/" + info.completeBaseName();

    out << "\"" << baseName << "\"";

#if QT_VERSION < 0x050E00
    out << endl;
#else
    out << Qt::endl;
#endif

    inputFile.close();

    startSgbs2SxyzProcess();
}

void AtomicDensitiesPage::startSgbs2SxyzProcess()
{
    ExternalProgramRunner::RunRequest request;

    request.runMpi = false;
    request.outputPrefix = projectName_;

    request.rootName = "sgbs2sxyz";
    request.stdInput = sgbs2sxyzInputFile_;
    request.stdOutput = sgbs2sxyzErrorFile_;
    request.subdir = "DAM_400";
    request.nprocs = 1;
    request.suffix = "";

    runningOperation_ = RunningOperation::Sgbs2Sxyz;

    runner_->setProjectFolder(projectFolder_);

    runner_->setMpiCommand(mpiCommand_);
    runner_->setMpiFlags(mpiFlags_);

    if (!runner_->start(request)) {
        runningOperation_ = RunningOperation::None;

        QFile::remove(sgbs2sxyzInputFile_);

        emit errorOccurred(
            tr("Could not start sgbs2sxyz.")
        );
    }
}

bool AtomicDensitiesPage::finishSgbs2Sxyz()
{
    QFile::remove(sgbs2sxyzInputFile_);

    if (!QFile::exists(sgbs2sxyzTargetFile_)) {
        emit errorOccurred(
            tr("The sgbs2sxyz process finished, but the expected file "
               "was not created:\n%1")
                .arg(sgbs2sxyzTargetFile_)
        );

        return false;
    }

    const QFileInfo targetInfo(sgbs2sxyzTargetFile_);

    const QString generatedInfoFile =
        QDir(targetInfo.path()).filePath(
            targetInfo.completeBaseName() + ".sgbs2sxyz"
        );

    if (!QFile::exists(generatedInfoFile)) {
        emit statusMessageRequested(
            tr("Warning: the auxiliary sgbs2sxyz output file was not created:\n%1")
                .arg(generatedInfoFile)
        );
    }

    return true;
}

//bool AtomicDensitiesPage::finishSgbs2Sxyz(
//    const QString& targetSxyzFile)
//{
//    if (QFile::exists(targetSxyzFile)) {
//        return true;
//    }

//    emit errorOccurred(
//        tr("The conversion finished successfully, but "
//           "the expected file was not created:\n%1")
//            .arg(targetSxyzFile)
//    );

//    return false;
//}

void AtomicDensitiesPage::handleNormalProcessExit()
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


void AtomicDensitiesPage::inputOnlyStateChanged(int state)
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

void AtomicDensitiesPage::loadDefault(){
    setLmaxExpansion(10);
    setLmaxDisplayed(5);
}

void AtomicDensitiesPage::mpiStateChanged(int state)
{
    if (state != 0 && !isInputOnly()){
        setMpiChecked(true);
        setMpiEnabled(true);
    }
    else{
        if (state == 0){
            setMpiChecked(false);
        }
        else{
            setMpiChecked(true);
            setMpiCheckEnabled(false);
        }
        setMpiEnabled(false);
    }
}

void AtomicDensitiesPage::readFromFile(const std::string& file)
{
    const char* section;
    if (lslater_)
        section = "DAMSECT";
    else
        section = "G-DAMSECT";

    setLmaxExpansion(ReadWriteOptions::readSpinBox("lmaxexp", section, file));
    setTopLmaxDisplayed(lmaxExpansion());
    setLmaxDisplayed(ReadWriteOptions::readSpinBox("lmultmx", section, file));

    QString value = ReadWriteOptions::readTextToLineEdit("ioptaj", section, file);
    if (value == "2"){
        setOneCenterChecked(true);
    }
    else if(value == "3"){
        setTwoCenterChecked(true);
    }
    else{
        setTotalDensityChecked(true);
    }

    value = ReadWriteOptions::readTextToLineEdit("umbral", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-14");
    setCutoffThreshold(value.toInt());

    value = ReadWriteOptions::readTextToLineEdit("umbralres", section, file);
    if (value.length()>3)
        value.remove(0,3);
    else
        value = QString("-14");
    setFitThreshold(value.toInt());

}

//    Reads the number of atoms from file fileName
int AtomicDensitiesPage::read_natom(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("read_natom"),tr("File %1 cannot be read").arg(fileName) +
                        QString(":\n%1.").arg(file.errorString()));
        return 0;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    if (line.size()==0){
        return 0;
    }
    else{
        return line.toInt();
    }
}

ExternalProgramRunner *AtomicDensitiesPage::runner() const
{
    return runner_;
}

void AtomicDensitiesPage::setCutoffThreshold(int value)
{
    cutoffThresholdSpin_->setValue(value);
}

void AtomicDensitiesPage::setExecEnabled(bool enabled)
{
    execButton_->setEnabled(enabled);
}

void AtomicDensitiesPage::setFitThreshold(int value)
{
    fitThresholdSpin_->setValue(value);
}

void AtomicDensitiesPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void AtomicDensitiesPage::setLmaxDisplayed(int value)
{
    lmaxDisplayedSpin_->setValue(value);
}


void AtomicDensitiesPage::setLmaxExpansion(int value)
{
    lmaxExpansionSpin_->setValue(value);
}

void AtomicDensitiesPage::setlslater(bool value)
{
   lslater_ = value;
}

void AtomicDensitiesPage::setlvalence(bool value)
{
   lvalence_ = value;
}

void AtomicDensitiesPage::setlzdo(bool value)
{
   lzdo_ = value;
}

void AtomicDensitiesPage::setMpiChecked(bool checked)
{
    mpiCheck_->setChecked(checked);
}

void AtomicDensitiesPage::setMpiCheckEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
}

void AtomicDensitiesPage::setMpiControlsEnabled(bool enabled)
{
    mpiCheck_->setEnabled(enabled);
    mpiProcessorsLabel_->setEnabled(enabled && mpiCheck_->isChecked());
    mpiProcessorsSpin_->setEnabled(enabled && mpiCheck_->isChecked());
}

void AtomicDensitiesPage::setMpiEnabled(bool enabled)
{
    mpiProcessorsLabel_->setEnabled(enabled);
    mpiProcessorsSpin_->setEnabled(enabled);
}

void AtomicDensitiesPage::setMpiProcessors(int value)
{
    mpiProcessorsSpin_->setValue(value);
}

void AtomicDensitiesPage::setMpiSettings(const QString &mpiCommand,
                    const QString &mpiFlags)
{
    mpiCommand_ = mpiCommand;
    mpiFlags_ = mpiFlags;
}

void AtomicDensitiesPage::setMpiVisible(bool visible)
{
    mpiGroup_->setVisible(visible);
}

//    sets variable natom
void AtomicDensitiesPage::set_natom(int i)
{
    AtomicDensitiesPage::natom_ = i;
}

void AtomicDensitiesPage::setOneCenterChecked(bool checked)
{
    oneCenterRadio_->setChecked(checked);
}

void AtomicDensitiesPage::setPageEnabled(bool enabled)
{
    this->setEnabled(enabled);
}

void AtomicDensitiesPage::setProjectFolder(const QString& value)
{
    projectFolder_ = value;
}

void AtomicDensitiesPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

void AtomicDensitiesPage::setStopEnabled(bool enabled)
{
    stopButton_->setEnabled(enabled);
}

void AtomicDensitiesPage::setTopLmaxDisplayed(int value)
{
    lmaxDisplayedSpin_->setRange(0, value);
}

void AtomicDensitiesPage::setTotalDensityChecked(bool checked)
{
    totalDensityRadio_->setChecked(checked);
}

void AtomicDensitiesPage::setTwoCenterChecked(bool checked)
{
    twoCenterRadio_->setChecked(checked);
}


void AtomicDensitiesPage::stopCurrentProcess()
{
    runner_->stop();
}

void AtomicDensitiesPage::writeToFile(const std::string& file,
                                bool isWindows,
                                bool* printwarns,
                                QString* warns)
{
    const char* section;
    if (lslater_)
        section = "DAMSECT";
    else
        section = "G-DAMSECT";

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
    writeBool("iswindows", isWindows);

    writeBool("lzdo", lzdo_);

    writeBool("lvalence", lvalence_);

    QString qv;
    if (isOneCenterChecked()) qv = QString("2");
    else if (isTwoCenterChecked())qv = QString("3");
    else qv = QString("1");

    writeText("lmaxexp",  QString("%1").arg(lmaxExpansion()));
    writeText("lmultmx",  QString("%1").arg(lmaxDisplayed()));
    writeText("ioptaj" ,  qv);
    writeText("umbral" ,  QString("1.d"+QString("%1").arg(cutoffThreshold())));
    writeText("umbralres",QString("1.d"+QString("%1").arg(fitThreshold())));
}
