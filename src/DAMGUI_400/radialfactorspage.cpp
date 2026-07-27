//  Copyright 2008-2026, Jaime Fernandez Rico, Rafael Lopez, Ignacio Ema,
//  Guillermo Ramirez, David Zorrilla, Anmol Kumar, Sachin D. Yeole, Shridhar R. Gadre
// 
//  This file is part of DAMQT.
// 
//  DAMQT is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  DAMQT is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with DAMQT.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------
//
//    Class defining the class for radial factors tabulation
//
//  File:   radialfactors.cpp
//
//      Last version: May 2026
//
#include "radialfactorspage.h"

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

static QString quoteString(QString s)
{
    s.remove("\"");
    return "\"" + s + "\"";
}

RadialFactorsPage::RadialFactorsPage(QWidget* parent)
    : QWidget(parent),
      runner_(new ExternalProgramRunner(this))
{
    buildUi();
    connectSignals();
}


void RadialFactorsPage::buildUi()
{
    // ----- Validators ----
    myDoubleValidator_ = new QDoubleValidator(nullptr);
    myDoubleValidator_->setLocale(QLocale::English);
    
    // ---- Output prefix ----
    outputGroup_ = new QGroupBox(tr("Output files prefix"), this);
    outputPrefixEdit_ = new QLineEdit(outputGroup_);

    auto* outputLayout = new QHBoxLayout(outputGroup_);
    outputLayout->addWidget(outputPrefixEdit_);

    // ---- Tabulation points ----
    tabulationGroup_ = new QGroupBox(tr("Tabulation points"), this);
    tabulationInitialLabel_ = new QLabel("Initial", tabulationGroup_);
    tabulationInitialEdit_ = new QLineEdit(tabulationGroup_);
    tabulationInitialEdit_->setValidator(myDoubleValidator_);
    tabulationInitialEdit_->setMaximumWidth(100);
    tabulationInitialEdit_->setText("0.0");
    tabulationFinalLabel_ = new QLabel("Final", tabulationGroup_);
    tabulationFinalEdit_ = new QLineEdit(tabulationGroup_);
    tabulationFinalEdit_->setValidator(myDoubleValidator_);
    tabulationFinalEdit_->setMaximumWidth(100);
    tabulationFinalEdit_->setText("5.0");
    tabulationStepLabel_ = new QLabel("Step", tabulationGroup_);
    tabulationStepEdit_ = new QLineEdit(tabulationGroup_);
    tabulationStepEdit_->setValidator(myDoubleValidator_);
    tabulationStepEdit_->setMaximumWidth(100);
    tabulationStepEdit_->setText("0.05");
    
    auto* radiusRangeLayout = new QGridLayout();
    radiusRangeLayout->addWidget(tabulationInitialLabel_,0,0);
    radiusRangeLayout->addWidget(tabulationFinalLabel_,0,1);
    radiusRangeLayout->addWidget(tabulationStepLabel_,0,2);
    radiusRangeLayout->addWidget(tabulationInitialEdit_,1,0);
    radiusRangeLayout->addWidget(tabulationFinalEdit_,1,1);
    radiusRangeLayout->addWidget(tabulationStepEdit_,1,2);
    
    extraValuesCheck_ = new QCheckBox(tr("Extra values"), tabulationGroup_);

    radiiTable_ = new QWidget();
    radiiSheet_ = new Sheet(0, 1, 0,true, radiiTable_);
    QStringList radList_;
    radList_ << "R" ;
    radiiSheet_->setHeader(radList_);
    radiiTable_->setVisible(false);
    radiiTable_->setEnabled(false);
    radiiTable_->setMaximumWidth(160);

    auto* radiiTableLayout = new QHBoxLayout();
    radiiTableLayout->addStretch(10);
    radiiTableLayout->addWidget(radiiTable_);
    radiiTableLayout->addStretch(10);
    
    auto* tabulationLayout = new QVBoxLayout(tabulationGroup_);
    tabulationLayout->addLayout(radiusRangeLayout);
    tabulationLayout->addWidget(extraValuesCheck_);
    tabulationLayout->addLayout(radiiTableLayout);
    
    // ---- Radial factors ----
    lmax_ = 25;
    mmax_ = 25;
    radialFactorsGroup_ = new QGroupBox(tr("Radial factors"), this);
    lTabulationLabel_ = new QLabel("l", radialFactorsGroup_);
    lTabulationSpin_ = new QSpinBox(radialFactorsGroup_);
    lTabulationSpin_->setRange(0, lmax_);
    lTabulationSpin_->setValue(10);
    lTabulationSpin_->setMaximumWidth(70);
    mTabulationLabel_ = new QLabel("m", radialFactorsGroup_);
    mTabulationSpin_ = new QSpinBox(radialFactorsGroup_);
    mTabulationSpin_->setRange(-mmax_, mmax_);
    mTabulationSpin_->setValue(0);
    mTabulationSpin_->setMaximumWidth(70);
    
    auto* radialFactorsLayout = new QHBoxLayout(radialFactorsGroup_);
    radialFactorsLayout->addStretch(10);
    radialFactorsLayout->addWidget(lTabulationLabel_,Qt::AlignRight);
    radialFactorsLayout->addWidget(lTabulationSpin_,Qt::AlignLeft);
    radialFactorsLayout->addWidget(mTabulationLabel_,Qt::AlignRight);
    radialFactorsLayout->addWidget(mTabulationSpin_,Qt::AlignLeft);
    radialFactorsLayout->addStretch(10);
    
    // ---- Centers ----
    centersGroup_ = new QGroupBox(tr("Centers"), this);
    centersLabel_ = new QLabel(tr("1,3-5,10,13-17,...") + ":", centersGroup_);
    centersEdit_ = new QLineEdit(centersGroup_);
    QRegExp rx("[1-9][-,\\d]*");
    centersValidator_ = new QRegExpValidator(rx, nullptr);
    centersList_ = new QStringList();
    centersEdit_->setValidator(centersValidator_);
    
    auto* centersLayout = new QVBoxLayout(centersGroup_);
    centersLayout->addWidget(centersLabel_);
    centersLayout->addWidget(centersEdit_);
        
    // ---- Derivatives ----
    derivativesGroup_ = new QGroupBox(tr("Derivatives"), this);
    gradientCheck_ = new QCheckBox(tr("First derivatives"), derivativesGroup_);
    secondDerivativesCheck_ = new QCheckBox(tr("Second derivatives"), derivativesGroup_);
    
    auto* derivativesLayout = new QVBoxLayout(derivativesGroup_);
    derivativesLayout->addWidget(gradientCheck_);
    derivativesLayout->addWidget(secondDerivativesCheck_);
        
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
    mainLayout->addWidget(tabulationGroup_);
    mainLayout->addWidget(radialFactorsGroup_);
    mainLayout->addWidget(centersGroup_);
    mainLayout->addWidget(derivativesGroup_);
    mainLayout->addWidget(inputOnlyGroup_);
    mainLayout->addLayout(buttonsLayout);
    mainLayout->addStretch();

}


void RadialFactorsPage::connectSignals()
{
    
    connect(extraValuesCheck_, &QCheckBox::stateChanged,
            this, &RadialFactorsPage::extraValuesChanged);
    
    connect(lTabulationSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &RadialFactorsPage::lTabulationChanged);

    connect(centersEdit_, &QLineEdit::textChanged, this, &RadialFactorsPage::centersChanged);
    
    connect(gradientCheck_, &QCheckBox::stateChanged,
            this, &RadialFactorsPage::gradientStateChanged);
    
    connect(secondDerivativesCheck_, &QCheckBox::stateChanged,
            this, &RadialFactorsPage::secondDerivativesStateChanged);

    connect(execButton_, &QPushButton::clicked,
            this, &RadialFactorsPage::execRequested);

//    connect(stopButton_, &QPushButton::clicked,
//            this, &RadialFactorsPage::stopRequested);

    connect(stopButton_, &QPushButton::clicked,
            runner_, &ExternalProgramRunner::stop);

    connect(outputButton_, &QPushButton::clicked,
            this, &RadialFactorsPage::openOutputRequested);    

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

int RadialFactorsPage::lMax() const { return lmax_; }
int RadialFactorsPage::mMax() const { return mmax_; }
int RadialFactorsPage::xyzTableRows() const { return radiiSheet_->tabla->rowCount();}

bool RadialFactorsPage::isExtraValues() const { return extraValuesCheck_->isChecked(); }
bool RadialFactorsPage::isGradient() const { return gradientCheck_->isChecked(); }
bool RadialFactorsPage::isInputOnly() const { return inputOnlyCheck_->isChecked(); }
bool RadialFactorsPage::isSecondDerivatives() const { return secondDerivativesCheck_->isChecked(); }

void RadialFactorsPage::clearSheet() {radiiSheet_->clear(); }
void RadialFactorsPage::insertRow(int i) {radiiSheet_->tabla->insertRow(i); }
void RadialFactorsPage::resizeSheet(int i) {radiiSheet_->resizeRows(i); }
void RadialFactorsPage::setCellValue(const QString& value, int i, int j) {radiiSheet_->setcellvalue(value,i,j); }
void RadialFactorsPage::setCentersList(const QStringList& value) { *centersList_ = value; }
void RadialFactorsPage::setExecEnabled(bool enabled) { execButton_->setEnabled(enabled); }
void RadialFactorsPage::setExtraValues(bool checked) { extraValuesCheck_->setChecked(checked); }
void RadialFactorsPage::setGradient(bool checked) { gradientCheck_->setChecked(checked); }
void RadialFactorsPage::setLmax(int lmax) { lmax_ = lmax; lTabulationSpin_->setMaximum(lmax); mTabulationSpin_->setMaximum(lmax); }
void RadialFactorsPage::setlTabulation(int value) { lTabulationSpin_->setValue(value); }
void RadialFactorsPage::setMmax(int mmax) { mmax_ = mmax; mTabulationSpin_->setMaximum(mmax); }
void RadialFactorsPage::setmTabulation(int value) { mTabulationSpin_->setValue(value); }
void RadialFactorsPage::setOutputPrefix(const QString& value) { outputPrefixEdit_->setText(value); }
void RadialFactorsPage::setPageEnabled(bool enabled){this->setEnabled(enabled);}
void RadialFactorsPage::setRadiiTableEnabled(bool enabled) { radiiTable_->setEnabled(enabled); }
void RadialFactorsPage::setRadiiTableVisible(bool checked) { radiiTable_->setVisible(checked); }
void RadialFactorsPage::setSecondDerivatives(bool checked) { secondDerivativesCheck_->setChecked(checked); }
void RadialFactorsPage::setStopEnabled(bool enabled) { stopButton_->setEnabled(enabled); }
void RadialFactorsPage::setTabulationInitial(const QString& value) { tabulationInitialEdit_->setText(value); }
void RadialFactorsPage::setTabulationFinal(const QString& value) { tabulationFinalEdit_->setText(value); }
void RadialFactorsPage::setTabulationStep(const QString& value) { tabulationStepEdit_->setText(value); }


void RadialFactorsPage::centersChanged(){
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
void RadialFactorsPage::execDamFrad()
{
    QString stdOutput;

    QString rootName = "DAMFRAD_400";
    QString subdir = "DAM_400";
    QString inputTemplate = "DAMFRAD_400.inp";
    QString inputSection = "DAMFRADSECT";

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

void RadialFactorsPage::gradientStateChanged(int state)
{
    Q_UNUSED(state);
// Extra safeguard: gradient cannot be disabled while second derivatives are active.
    if (isSecondDerivatives() && !isGradient()) {
        QSignalBlocker blocker(gradientCheck_);
        setGradient(true);
    }
}


void RadialFactorsPage::handleNormalProcessExit()
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

void RadialFactorsPage::lTabulationChanged(int lvalue)
{
    setMmax(lvalue);
}

void RadialFactorsPage::loadDefault(){
    setTabulationFinal("5.0");
    setTabulationInitial("0.0");
    setTabulationStep("0.05");
    clearSheet();
    setExtraValues(false);
    setGradient(false);
    setSecondDerivatives(false);
    setLmax(25);
    setlTabulation(0);
    setmTabulation(0);
}

void RadialFactorsPage::extraValuesChanged(int state)
{
    if (state == Qt::Checked){
        setRadiiTableVisible(true);
        setRadiiTableEnabled(true);
    }else{
        setRadiiTableVisible(false);
        setRadiiTableEnabled(false);
    }
}


void RadialFactorsPage::readFromFile(const std::string& file)
{
    const char* section = "DAMFRADSECT";

    outputPrefixEdit_->setText(
        ReadWriteOptions::readTextToLineEdit("filename", section, file)
    );

    // ---- Tabulation points ----
    ReadWriteOptions::readDoubleToLineEdit("rini", section, tabulationInitialEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("rfin", section, tabulationFinalEdit_, file);
    ReadWriteOptions::readDoubleToLineEdit("dltr", section, tabulationStepEdit_, file);
    setExtraValues(ReadWriteOptions::readCheckBox("lrlist", section, file));
    readTable(file, section);
    radiiTable_->updateGeometry();

    // ---- Radial factors ----
    setlTabulation(ReadWriteOptions::readSpinBox("ltab", section, file));
    setmTabulation(ReadWriteOptions::readSpinBox("mtab", section, file));

    // ---- Centers ----
    int ncenters;
    ReadWriteOptions::readInt("ncntab", section, &ncenters, file);
    QString qv;
    std::string key;
    if (ncenters > 0){
        centersList_->clear();
        for(int i = 0 ; i < ncenters ; ++i){
            key = "iatomsel(" + std::to_string(i + 1) + ")";
            qv = ReadWriteOptions::readTextToLineEdit(key.c_str(),section,file);
            if (!qv.isEmpty()){
                centersList_->append(qv);
            }
        }
        centersEdit_->setText(centersList_->join(","));
    }

    // ---- Derivatives ----
    setGradient(ReadWriteOptions::readCheckBox("lderiv", section, file));
    setSecondDerivatives(ReadWriteOptions::readCheckBox("lderiv2", section, file));
}


void RadialFactorsPage::readTable(const std::string& file, const char* section)
{
    // ---- Number of rows ----
    const QString numText =
        QString::fromStdString(CIniFile::GetValue("nlist", section, file));

    bool ok = false;
    const int numRows = numText.toInt(&ok);

    clearSheet();

    if (!ok || numRows <= 0) {
        setRadiiTableVisible(false);
        return;
    }

    resizeSheet(numRows + 1);

    for (int i = 0; i < numRows; ++i) {
        insertRow(i);
        const QString key = QString("rlist(%1)").arg(i + 1);

        const QString value =
            QString::fromStdString(
                CIniFile::GetValue(key.toStdString(), section, file)
            );
        setCellValue(value, i, 0);
    }

    resizeSheet(xyzTableRows());

    // ---- Visibility ----
    if (isExtraValues()) {
        setRadiiTableVisible(true);
    } else {
        setRadiiTableVisible(false);
    }
}

void RadialFactorsPage::secondDerivativesStateChanged(int state)
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


void RadialFactorsPage::setIsWindows(bool windows)
{
    iswindows_ = windows;
}

void RadialFactorsPage::setProjectData(const QString &projectFolder,
                        const QString &projectName)
{
    projectFolder_ = projectFolder;
    projectName_ = projectName;
}


//void RadialFactorsPage::writeToFile(const std::string& file,
//                                bool isWindows,
//                                bool* printwarns,
//                                QString* warns)
void RadialFactorsPage::writeToFile(const std::string& file,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "DAMFRADSECT";

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

    ReadWriteOptions::writeOption("rini", section, tabulationInitialEdit_->text(), file, printwarns, warns);
    ReadWriteOptions::writeOption("rfin", section, tabulationFinalEdit_->text(), file, printwarns, warns);
    ReadWriteOptions::writeOption("dltr", section, tabulationStepEdit_->text(), file, printwarns, warns);
    writeText("ltab", QString::number(lTabulationSpin_->value()));
    writeText("mtab", QString::number(mTabulationSpin_->value()));

    // ---- List of extra values of r ----
    writeBool("lrlist", isExtraValues());
    writeText("nlist", QString("%1").arg(xyzTableRows()));

    // ---- Extra radii table ----
    ReadWriteOptions::writeRTable(section, file, this, Sheet::max_sel, printwarns, warns);

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
            writeText(QString("%1%2%3").arg("iatomsel(").arg(ncntab).arg(")").toStdString().c_str(),
                      QString::number(list[k]));
        }
        writeText("ncntab", QString("%1").arg(ncntab));
    }

    writeBool("lderiv", isGradient());
    writeBool("lderiv2", isSecondDerivatives());
}

QString RadialFactorsPage::getCellValue(int i, int j) const { return radiiSheet_->getcellvalue(i,j); }
QString RadialFactorsPage::numtabular() const { return QString("nlist"); }
QString RadialFactorsPage::outputPrefix() const { return outputPrefixEdit_->text(); }
QString RadialFactorsPage::tabularkey() const { return QString("rlist"); }

