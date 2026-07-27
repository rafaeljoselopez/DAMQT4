#include "orientedmultipolespage.h"

#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextStream>

#include "readwriteoptions.h"
#include "IniFile.h"

static QString quoteString(QString s)
{
    s.remove("\"");
    return "\"" + s + "\"";
}

OrientedMultipolesPage::OrientedMultipolesPage(QWidget* parent)
    : QWidget(parent),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}

void OrientedMultipolesPage::buildUi()
{
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);
        
    // ---- Multipoles ----
    multipolesGroup_ = new QGroupBox(tr("Multipoles"), this);
    lMaxLabel_ = new QLabel(tr("Highest l"), multipolesGroup_);
    lMinLabel_ = new QLabel(tr("Lowest l"), multipolesGroup_);

    lMaxSpin_ = new QSpinBox(multipolesGroup_);
    lMaxSpin_->setRange(0, 25);
    lMaxSpin_->setValue(10);
    lMaxSpin_->setMaximumWidth(60);

    lMinSpin_ = new QSpinBox(multipolesGroup_);
    lMinSpin_->setRange(0, 25);
    lMinSpin_->setValue(0);
    lMinSpin_->setMaximumWidth(60);
    
    auto* lmaxLayout = new QHBoxLayout();
    lmaxLayout->addWidget(lMaxLabel_);
    lmaxLayout->addWidget(lMaxSpin_);

    auto* lminLayout = new QHBoxLayout();
    lminLayout->addWidget(lMinLabel_);
    lminLayout->addWidget(lMinSpin_);

    auto* multipolesLayout = new QVBoxLayout(multipolesGroup_);
    multipolesLayout->addLayout(lmaxLayout);
    multipolesLayout->addLayout(lminLayout);
    
    // ---- Centers defining plane ----
    centersPlaneGroup_ = new QGroupBox(tr("Centers defining plane"), this);
    leftLabel_ = new QLabel("Left", centersPlaneGroup_);
    leftSpin_ = new QSpinBox(multipolesGroup_);
    leftSpin_->setRange(1,3);
    leftSpin_->setValue(1);
    rotleft_ = leftSpin_->value();
    middleLabel_ = new QLabel("Left", centersPlaneGroup_);
    middleSpin_ = new QSpinBox(multipolesGroup_);
    middleSpin_->setRange(1,3);
    middleSpin_->setValue(2);
    rotmiddle_ = middleSpin_->value();
    rightLabel_ = new QLabel("Right", centersPlaneGroup_);
    rightSpin_ = new QSpinBox(multipolesGroup_);
    rightSpin_->setRange(1,3);
    rightSpin_->setValue(3);
    rotright_ = rightSpin_->value();
    numatoms_ = 3;

    auto* centersPlaneLayout = new QGridLayout(centersPlaneGroup_);
    centersPlaneLayout->addWidget(leftLabel_,0,0);
    centersPlaneLayout->addWidget(middleLabel_,0,1);
    centersPlaneLayout->addWidget(rightLabel_,0,2);
    centersPlaneLayout->addWidget(leftSpin_,1,0);
    centersPlaneLayout->addWidget(middleSpin_,1,1);
    centersPlaneLayout->addWidget(rightSpin_,1,2);
    
    // ---- Atomic fragments selection ----
    centersGroup_ = new QGroupBox(tr("Atomic fragments"), this);
    centersLabel_ = new QLabel(tr("1,3-5,10,13-17,...") + ":", centersGroup_);
    centersEdit_ = new QLineEdit(centersGroup_);
    QRegExp rx("[1-9][-,\\d]*");
    centersValidator_ = new QRegExpValidator(rx, nullptr);
    centersList_ = new QStringList();
    centersEdit_->setValidator(centersValidator_);
    
    auto* centersLayout = new QVBoxLayout(centersGroup_);
    centersLayout->addWidget(centersLabel_);
    centersLayout->addWidget(centersEdit_);

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
    mainLayout->addWidget(multipolesGroup_);
    mainLayout->addWidget(centersPlaneGroup_);
    mainLayout->addWidget(centersGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

}

void OrientedMultipolesPage::connectSignals()
{

    connect(centersEdit_, &QLineEdit::textChanged, this, &OrientedMultipolesPage::centersChanged);
    
    connect(lMaxSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OrientedMultipolesPage::lMaxChanged);
    
    connect(lMinSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OrientedMultipolesPage::lMinChanged);
    
    connect(leftSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OrientedMultipolesPage::leftCenterChanged);
        
    connect(middleSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OrientedMultipolesPage::middleCenterChanged);
        
    connect(rightSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &OrientedMultipolesPage::rightCenterChanged);
    
    connect(centersEdit_, &QLineEdit::textChanged, 
            this, &OrientedMultipolesPage::centersChanged);
    
    connect(execButton_, &QPushButton::clicked,
            this, &OrientedMultipolesPage::execRequested);
    
//    connect(stopButton_, &QPushButton::clicked,
//            this, &OrientedMultipolesPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);
    
    connect(outputButton_, &QPushButton::clicked,
            this, &OrientedMultipolesPage::openOutputRequested);

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
bool OrientedMultipolesPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }

int OrientedMultipolesPage::leftIndex() const { return leftSpin_->value(); }
int OrientedMultipolesPage::lMax() const { return lMaxSpin_->value(); }
int OrientedMultipolesPage::lMin() const { return lMinSpin_->value(); }
int OrientedMultipolesPage::middleIndex() const { return middleSpin_->value(); }
int OrientedMultipolesPage::rightIndex() const { return rightSpin_->value(); }

void OrientedMultipolesPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void OrientedMultipolesPage::setLeftSpinMax(int nmax) { leftSpin_->setMaximum(nmax); }
void OrientedMultipolesPage::setLeftSpinValue(int value) { leftSpin_->setValue(value); }
void OrientedMultipolesPage::setLmax(int lmax) { lMaxSpin_->setValue(lmax); }
void OrientedMultipolesPage::setLmaxTop(int ltop) { lMaxSpin_->setRange(0,ltop); }
void OrientedMultipolesPage::setLmin(int lmin) { lMinSpin_->setValue(lmin); }
void OrientedMultipolesPage::setLminTop(int ltop) { lMinSpin_->setRange(0,ltop); }
void OrientedMultipolesPage::setMiddleSpinMax(int nmax) { middleSpin_->setMaximum(nmax); }
void OrientedMultipolesPage::setMiddleSpinValue(int value) { middleSpin_->setValue(value); }
void OrientedMultipolesPage::setNumAtoms(int value) { numatoms_ = value; }
void OrientedMultipolesPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void OrientedMultipolesPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void OrientedMultipolesPage::setRightSpinMax(int nmax) { rightSpin_->setMaximum(nmax); }
void OrientedMultipolesPage::setRightSpinValue(int value) { rightSpin_->setValue(value); }
void OrientedMultipolesPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }


void OrientedMultipolesPage::centersChanged(){
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
void OrientedMultipolesPage::execDamMultRot()
{
    QString stdOutput;

    QString rootName = "DAMMULTROT_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMMULTROT_400.inp";
    QString inputSection = "DAMMULTROTSECT";

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

void OrientedMultipolesPage::handleNormalProcessExit()
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

void OrientedMultipolesPage::leftCenterChanged()
{
    if (leftSpin_->value() > rotleft_){    // The value of leftSpin_->value() is increasing
        if (leftSpin_->value() == middleSpin_->value()){
            if(rightSpin_->value() != (middleSpin_->value()+1)){
                if (middleSpin_->value() < numatoms_)
                    leftSpin_->setValue(middleSpin_->value()+1);
                else
                    leftSpin_->setValue(rotleft_);
            }
            else if(rightSpin_->value() < numatoms_)
                leftSpin_->setValue(rightSpin_->value()+1);
            else
                leftSpin_->setValue(rotleft_);
        }
        else if(leftSpin_->value()==rightSpin_->value()){
            if(middleSpin_->value() != rightSpin_->value()+1){
                if (rightSpin_->value() < numatoms_)
                    leftSpin_->setValue(rightSpin_->value()+1);
                else
                    leftSpin_->setValue(rotleft_);
            }
            else if(middleSpin_->value() < numatoms_)
                leftSpin_->setValue(middleSpin_->value()+1);
            else
                leftSpin_->setValue(rotleft_);
        }
    }
    else{    // The value of leftSpin_->value() is decreasing
        if (leftSpin_->value() == middleSpin_->value()){
            if(rightSpin_->value() != middleSpin_->value()-1){
                if (middleSpin_->value() > 1)
                    leftSpin_->setValue(middleSpin_->value()-1);
                else
                    leftSpin_->setValue(rotleft_);
            }
            else if(rightSpin_->value() > 1)
                leftSpin_->setValue(rightSpin_->value()-1);
            else
                leftSpin_->setValue(rotleft_);
        }
        else if(leftSpin_->value()==rightSpin_->value()){
            if(middleSpin_->value() != rightSpin_->value()-1){
                if (rightSpin_->value() > 1)
                    leftSpin_->setValue(rightSpin_->value()-1);
                else
                    leftSpin_->setValue(rotleft_);
            }
            else if(middleSpin_->value() > 1)
                leftSpin_->setValue(middleSpin_->value()-1);
            else
                leftSpin_->setValue(rotleft_);
        }
    }
    rotleft_ = leftSpin_->value();
}


void OrientedMultipolesPage::lMaxChanged()
{
    if (lMinSpin_->value() > lMaxSpin_->value())
        lMaxSpin_->setValue(lMinSpin_->value());
    lMinSpin_->setMaximum(lMaxSpin_->value());
}

void OrientedMultipolesPage::lMinChanged()
{
    if (lMinSpin_->value() > lMaxSpin_->value())
        lMinSpin_->setValue(lMaxSpin_->value());
    lMaxSpin_->setMinimum(lMinSpin_->value());
}

void OrientedMultipolesPage::loadDefault(){
    setLmin(0);
    setLmax(0);
    setLeftSpinValue(1);
    setMiddleSpinValue(2);
    setRightSpinValue(3);
}

void OrientedMultipolesPage::middleCenterChanged()
{
    if (middleSpin_->value() > rotmiddle_){    // The value of middleSpin_->value() is increasing
        if (middleSpin_->value() == leftSpin_->value()){
            if(rightSpin_->value() != leftSpin_->value()+1){
                if (leftSpin_->value() < numatoms_)
                    middleSpin_->setValue(leftSpin_->value()+1);
                else
                    middleSpin_->setValue(rotmiddle_);
            }
            else if(rightSpin_->value() < numatoms_)
                middleSpin_->setValue(rightSpin_->value()+1);
            else
                middleSpin_->setValue(rotmiddle_);
        }
        else if(middleSpin_->value() == rightSpin_->value()){
            if(leftSpin_->value() != rightSpin_->value()+1){
                if (rightSpin_->value() < numatoms_)
                    middleSpin_->setValue(rightSpin_->value()+1);
                else
                    middleSpin_->setValue(rotmiddle_);
            }
            else if(leftSpin_->value() < numatoms_)
                middleSpin_->setValue(leftSpin_->value()+1);
            else
                middleSpin_->setValue(rotmiddle_);
        }
    }
    else{    // The value of middleSpin_->value() is decreasing
        if (middleSpin_->value() == leftSpin_->value()){
            if(rightSpin_->value() != leftSpin_->value()-1){
                if (leftSpin_->value() > 1)
                    middleSpin_->setValue(leftSpin_->value()-1);
                else
                    middleSpin_->setValue(rotmiddle_);
            }
            else if(rightSpin_->value() > 1)
                middleSpin_->setValue(rightSpin_->value()-1);
            else
                middleSpin_->setValue(rotmiddle_);
        }
        else if(middleSpin_->value() == rightSpin_->value()){
            if(leftSpin_->value() != rightSpin_->value()-1){
                if (rightSpin_->value() > 1)
                    middleSpin_->setValue(rightSpin_->value()-1);
                else
                    middleSpin_->setValue(rotmiddle_);
            }
            else if(leftSpin_->value() > 1)
                middleSpin_->setValue(leftSpin_->value()-1);
            else
                middleSpin_->setValue(rotmiddle_);
        }
    }
    rotmiddle_ = middleSpin_->value();
}

void OrientedMultipolesPage::rightCenterChanged()
{
    if (rightSpin_->value() > rotright_){    // The value of rightSpin_->value() is increasing
        if (rightSpin_->value() == leftSpin_->value()){
            if(middleSpin_->value() != leftSpin_->value()+1){
                if (leftSpin_->value() < numatoms_)
                    rightSpin_->setValue(leftSpin_->value()+1);
                else
                    rightSpin_->setValue(rotright_);
            }
            else if(middleSpin_->value() <numatoms_)
                rightSpin_->setValue(middleSpin_->value()+1);
            else
                rightSpin_->setValue(rotright_);
        }
        else if(rightSpin_->value() == middleSpin_->value()){
            if(leftSpin_->value() != middleSpin_->value()+1){
                if (middleSpin_->value() < numatoms_)
                    rightSpin_->setValue(middleSpin_->value()+1);
                else
                    rightSpin_->setValue(rotright_);
            }
            else if(leftSpin_->value() < numatoms_)
                rightSpin_->setValue(leftSpin_->value()+1);
            else
                rightSpin_->setValue(rotright_);
        }
    }
    else{    // The value of rightSpin_->value() is decreasing
        if (rightSpin_->value() == leftSpin_->value()){
            if(middleSpin_->value() != leftSpin_->value()-1){
                if (leftSpin_->value() > 1)
                    rightSpin_->setValue(leftSpin_->value()-1);
                else
                    rightSpin_->setValue(rotright_);
            }
            else if(middleSpin_->value() > 1)
                rightSpin_->setValue(middleSpin_->value()-1);
            else
                rightSpin_->setValue(rotright_);
        }
        else if(rightSpin_->value() == middleSpin_->value()){
            if(leftSpin_->value() != middleSpin_->value()-1){
                if (middleSpin_->value() > 1)
                    rightSpin_->setValue(middleSpin_->value()-1);
                else
                    rightSpin_->setValue(rotright_);
            }
            else if(leftSpin_->value() > 1)
                rightSpin_->setValue(leftSpin_->value()-1);
            else
                rightSpin_->setValue(rotright_);
        }
    }
    rotright_ = rightSpin_->value();
}



void OrientedMultipolesPage::readFromFile(const std::string& file)
{
    const char* section = "DAMMULTROTSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    // ---- Multipoles ----
    setLmax(ReadWriteOptions::readSpinBox("lmax", section, file));
    setLmin(ReadWriteOptions::readSpinBox("lmin", section, file));

    // ---- Centers defining plane ----
    setLeftSpinMax(numatoms_);
    setMiddleSpinMax(numatoms_);
    setRightSpinMax(numatoms_);
    setLeftSpinValue(ReadWriteOptions::readSpinBox("i1", section, file));
    setMiddleSpinValue(ReadWriteOptions::readSpinBox("i2", section, file));
    setRightSpinValue(ReadWriteOptions::readSpinBox("i3", section, file));

    // ---- Atomic fragments selection ----

    int ncenters;
    ReadWriteOptions::readInt("ncntab", section, &ncenters, file);
    QString qv;
    std::string key;
    if (ncenters > 0){
        centersList_->clear();
        for(int i = 0 ; i < ncenters ; ++i){
            key = "icntab(" + std::to_string(i + 1) + ")";
            qv = ReadWriteOptions::readTextToLineEdit(key.c_str(),section,file);
            if (!qv.isEmpty()){
                centersList_->append(qv);
            }
        }
        centersEdit_->setText(centersList_->join(","));
    }
}


void OrientedMultipolesPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void OrientedMultipolesPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}

//void OrientedMultipolesPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool* printwarns,
//                                QString* warns)
void OrientedMultipolesPage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMMULTROTSECT";

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
    
    writeText("lmax", QString::number(lMax()));
    writeText("lmin", QString::number(lMin()));
    writeText("i1", QString::number(leftIndex()));
    writeText("i2", QString::number(middleIndex()));
    writeText("i3", QString::number(rightIndex()));
    
    // ---- Output ----
    writeText("filename",quoteString(outputPrefix()));

    // ---- List of selected atoms indices ----
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
            writeText(QString("%1%2%3").arg("icntab(").arg(ncntab).arg(")").toStdString().c_str(),
                      QString::number(list[k]));
        }
        writeText("ncntab", QString("%1").arg(ncntab));
    }
}

QString OrientedMultipolesPage::outputPrefix() const { return outputPrefixEdit_->text(); }
