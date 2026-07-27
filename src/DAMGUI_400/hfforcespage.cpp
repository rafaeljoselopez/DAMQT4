#include "hfforcespage.h"

#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>

#include "readwriteoptions.h"
#include "IniFile.h"

static QString quoteString(QString s)
{
    s.remove("\"");
    return "\"" + s + "\"";
}

HFForcesPage::HFForcesPage(QWidget* parent)
    : QWidget(parent),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void HFForcesPage::buildUi()
{
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);
    
    // ---- Atomic fragments selection ----
    atomicFragmentsGroup_ = new QGroupBox(tr("Atomic contributions"), this);
    atomicFragmentsCheck_ = new QCheckBox(tr("Atomic fragments"), atomicFragmentsGroup_);
    atomicFragmentsCheck_->setChecked(false);
    centersGroup_ = new QGroupBox(tr("Centers"), atomicFragmentsGroup_);
    centersLabel_ = new QLabel(tr("1,3-5,10,13-17,...") + ":", centersGroup_);
    centersEdit_ = new QLineEdit(centersGroup_);
    QRegExp rx("[1-9][-,\\d]*");
    centersValidator_ = new QRegExpValidator(rx, nullptr);
    centersList_ = new QStringList();
    centersEdit_->setValidator(centersValidator_);
    
    auto* centersLayout = new QVBoxLayout(centersGroup_);
    centersLayout->addWidget(centersLabel_);
    centersLayout->addWidget(centersEdit_);
    centersEdit_->setEnabled(false);
    
    auto* atomicFragmentsLayout = new QVBoxLayout(atomicFragmentsGroup_);
    atomicFragmentsLayout->addWidget(atomicFragmentsCheck_);
    atomicFragmentsLayout->addWidget(centersGroup_);

    // Input only
    inputOnlyGroup_ = new QGroupBox(tr("Input only"), this);
    inputOnlyCheck_ = new QCheckBox(tr("Generate input file only"), inputOnlyGroup_);
    inputOnlyCheck_->setChecked(false);

    auto* inputOnlyLayout = new QVBoxLayout(inputOnlyGroup_);
    inputOnlyLayout->addWidget(inputOnlyCheck_);
    
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
    mainLayout->addWidget(atomicFragmentsGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

}

void HFForcesPage::connectSignals()
{
    connect(atomicFragmentsCheck_, &QCheckBox::stateChanged,
            this, &HFForcesPage::atomicFragmentsChanged);

    connect(centersEdit_, &QLineEdit::textChanged,
            this, &HFForcesPage::centersChanged);
    
    connect(execButton_, &QPushButton::clicked,
            this, &HFForcesPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &HFForcesPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &HFForcesPage::openOutputRequested);

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
bool HFForcesPage::isAtomicFragments() const { return atomicFragmentsCheck_->isChecked(); }
bool HFForcesPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }

void HFForcesPage::setAtomicFragments(bool checked) { atomicFragmentsCheck_->setChecked(checked); }
void HFForcesPage::setCentersEnabled(bool enabled) { centersEdit_->setEnabled(enabled); }
void HFForcesPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void HFForcesPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void HFForcesPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void HFForcesPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }


void HFForcesPage::atomicFragmentsChanged(int state)
{
    if (state == Qt::Checked){
        setCentersEnabled(true);
    }
    else{
        setCentersEnabled(false);
    }
}


void HFForcesPage::centersChanged(){
    QString string = QString(centersEdit_->text());
    QStringList stringlist1 = string.split(",");
    centersList_->clear();
    for (int i = 0 ; i < stringlist1.length() ; ++i){
        if (stringlist1.at(i).length() != 0){
            if (stringlist1.at(i).contains(QString("-"))){
                QStringList stringlist2 = stringlist1.at(i).split("-");
                if (stringlist2.at(0).length() != 0 && stringlist2.at(1).length() != 0
                        && stringlist2.at(1).toInt() > stringlist2.at(0).toInt()){
                    for (int k = stringlist2.at(0).toInt() ; k <= stringlist2.at(1).toInt() ; k++){
                        centersList_->append(QString("%1").arg(k));
                    }
                }
            }
            else{
                centersList_->append(stringlist1.at(i));
            }
        }
    }
    centersList_->sort();
    centersList_->removeDuplicates();
}


//    Executes external program DAMFRAD  (Computes radial factors of density expansion)
void HFForcesPage::execDamForces()
{
    QString stdOutput;

    QString rootName = "DAMFORCES_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMFORCES_400.inp";
    QString inputSection = "DAMFORCESSECT";

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

    request.runMpi = false;
    if (outputPrefixEdit_->text().isEmpty()){
        request.outputPrefix = projectName_;
    } else{
        request.outputPrefix = outputPrefixEdit_->text();
    }

    request.rootName = rootName;
    request.stdInput = stdInput;
    request.stdOutput = stdOutput;
    request.subdir = subdir;

    lastOutputFileName_ =
        QDir(projectFolder_).filePath(
            request.outputPrefix + "-" + request.rootName + ".out"
        );

    setStopEnabled(true);
    runner_->setProjectFolder(projectFolder_);
    runner_->start(request);
}


void HFForcesPage::handleNormalProcessExit()
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

void HFForcesPage::loadDefault(){
    setAtomicFragments(false);
}

void HFForcesPage::readFromFile(const std::string& file)
{
    const char* section = "DAMFORCESSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    setAtomicFragments(ReadWriteOptions::readCheckBox("latomsel", section, file));

    // ---- Centers ----

    int nforces;
    ReadWriteOptions::readInt("ncntab", section, &nforces, file);
    QString qv;
    std::string key;
    if (nforces > 0){
        centersList_->clear();
        for(int i = 0 ; i < nforces ; ++i){
            key = "iatomsel(" + std::to_string(i + 1) + ")";
            qv = ReadWriteOptions::readTextToLineEdit(key.c_str(),section,file);
            if (!qv.isEmpty()){
                centersList_->append(qv);
            }
        }
        centersEdit_->setText(centersList_->join(","));
    }
}

void HFForcesPage::setIsValence(bool valence)
{
    lvalence_ = valence;
}

void HFForcesPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void HFForcesPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}


//void HFForcesPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool lvalence,
//                                bool* printwarns,
//                                QString* warns)
void HFForcesPage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMFORCESSECT";

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
    
    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));
    
    // ---- Is valence calculation ----
    if (lvalence_) {
        writeBool("lvalence", true);
    }

    // ---- List of selected atoms indices ----
    writeBool("latomsel", isAtomicFragments());
    
    if (centersList_->count() < 1){
        writeText("ncntab", QString::number(0));
    }
    else{
        QList <int> list;
        for (int i = 0 ; i < centersList_->count() ; ++i){
            list.append(centersList_->at(i).toInt());
        }
        std::sort(list.begin(), list.end());
        int ncntab = 0;
        for (int k=0 ; k < list.count() ; k++){
            if (list[k] <= 0 ) continue;
            ncntab++;
            writeText(QString("%1%2%3").arg("iatomsel(").arg(ncntab).arg(")").toStdString().c_str(),
                      QString::number(list[k]));
        }
        writeText("ncntab", QString("%1").arg(ncntab));
    }
}

QString HFForcesPage::outputPrefix() const { return outputPrefixEdit_->text(); }
