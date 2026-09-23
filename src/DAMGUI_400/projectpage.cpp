#include "projectpage.h"

#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include <QtDebug>

ProjectPage::ProjectPage(bool mpiAvailable,
                         const QString& mpiCommand,
                         QWidget* parent)
    : QWidget(parent),
      mpiAvailable_(mpiAvailable),
      mpiCommandDefault_(mpiCommand)
{
    buildUi();
    connectSignals();
}

void ProjectPage::buildUi()
{
    projectGroup_ = new QGroupBox(tr("Create Project"), this);

    importLabel_ = new QLabel(tr("Import data from") + ":", projectGroup_);
    importEdit_ = new QLineEdit(projectGroup_);
    importBrowseButton_ = new QToolButton(projectGroup_);
    importBrowseButton_->setText(tr("..."));

    projectFolderLabel_ = new QLabel(tr("Project folder") + ":", projectGroup_);
    projectFolderEdit_ = new QLineEdit(projectGroup_);

    projectNameLabel_ = new QLabel(tr("Project name") + ":", projectGroup_);
    projectNameEdit_ = new QLineEdit(projectGroup_);

    execImportButton_ = new QPushButton(QIcon(":/images/exec.png"), tr("Exec"), projectGroup_);
    execImportButton_->setEnabled(false);

    auto* projectLayout = new QVBoxLayout(projectGroup_);

    auto* row1 = new QHBoxLayout();
    row1->addWidget(importLabel_);
    projectLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout();
    row2->addWidget(importEdit_);
    row2->addWidget(importBrowseButton_);
    projectLayout->addLayout(row2);

    auto* row3 = new QHBoxLayout();
    row3->addWidget(projectFolderLabel_);
    projectLayout->addLayout(row3);

    auto* row4 = new QHBoxLayout();
    row4->addWidget(projectFolderEdit_);
    projectLayout->addLayout(row4);

    auto* row5 = new QHBoxLayout();
    row5->addWidget(projectNameLabel_);
    projectLayout->addLayout(row5);

    auto* row6 = new QHBoxLayout();
    row6->addWidget(projectNameEdit_);
    projectLayout->addLayout(row6);

    auto* row7 = new QHBoxLayout();
    row7->addWidget(execImportButton_, 0, Qt::AlignRight);
    projectLayout->addLayout(row7);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(projectGroup_);

    if (mpiAvailable_) {
        mpiGroup_ = new QGroupBox(tr("MPI options"), this);

        mpiCommandLabel_ = new QLabel(tr("MPI command") + ":", mpiGroup_);
        mpiFlagsLabel_ = new QLabel(tr("MPI flags") + ":", mpiGroup_);
        mpiCommandEdit_ = new QLineEdit(mpiGroup_);
        mpiFlagsEdit_ = new QLineEdit(mpiGroup_);

        mpiCommandEdit_->setText(mpiCommandDefault_);

        auto* mpiLayout = new QVBoxLayout(mpiGroup_);

        auto* row8 = new QHBoxLayout();
        row8->addWidget(mpiCommandLabel_);
        mpiLayout->addLayout(row8);

        auto* row9 = new QHBoxLayout();
        row9->addWidget(mpiCommandEdit_);
        mpiLayout->addLayout(row9);

        auto* row10 = new QHBoxLayout();
        row10->addWidget(mpiFlagsLabel_);
        mpiLayout->addLayout(row10);

        auto* row11 = new QHBoxLayout();
        row11->addWidget(mpiFlagsEdit_);
        mpiLayout->addLayout(row11);

        mainLayout->addWidget(mpiGroup_);
    }

    mainLayout->addStretch();
}

void ProjectPage::connectSignals()
{
    connect(importEdit_, &QLineEdit::textChanged,
            this, &ProjectPage::importFileChanged);

    connect(projectFolderEdit_, &QLineEdit::textChanged,
            this, &ProjectPage::projectFolderChanged);

    // connect(projectNameEdit_, &QLineEdit::textChanged,
    //         this, &ProjectPage::projectNameChanged);

    // connect(projectNameEdit_, &QLineEdit::textChanged,
    //         this,
    //         [this](const QString& text) {
    //             qDebug() << "projectNameEdit_ changed:" << text;
    //             emit projectNameChanged(text);
    //         });

    connect(projectNameEdit_, &QLineEdit::textChanged,
            this, [this](const QString& text) {
                emit projectNameChanged(text);
            });

    connect(importBrowseButton_, &QToolButton::clicked,
            this, &ProjectPage::browseImportRequested);

    connect(execImportButton_, &QPushButton::clicked,
            this, &ProjectPage::execImportRequested);

    if (mpiCommandEdit_) {
        connect(mpiCommandEdit_, &QLineEdit::textChanged,
                this, &ProjectPage::mpiCommandChanged);
    }

    if (mpiFlagsEdit_) {
        connect(mpiFlagsEdit_, &QLineEdit::textChanged,
                this, &ProjectPage::mpiFlagsChanged);
    }
}

QString ProjectPage::importFile() const
{
    return importEdit_->text();
}

QString ProjectPage::projectFolder() const
{
    return projectFolderEdit_->text();
}

QString ProjectPage::projectName() const
{
    return projectNameEdit_->text();
}

QString ProjectPage::mpiCommand() const
{
    return mpiCommandEdit_ ? mpiCommandEdit_->text() : QString();
}

QString ProjectPage::mpiFlags() const
{
    return mpiFlagsEdit_ ? mpiFlagsEdit_->text() : QString();
}

void ProjectPage::setBrowseEnabled(bool enabled)
{
    importBrowseButton_->setEnabled(enabled);
}

void ProjectPage::setImportEnabled(bool enabled)
{
    importEdit_->setEnabled(enabled);
}

void ProjectPage::setImportFile(const QString& value)
{
    importEdit_->setText(value);
}

void ProjectPage::setProjectFolder(const QString& value)
{
    projectFolderEdit_->setText(value);
}

void ProjectPage::setProjectFolderEnabled(bool enabled)
{
    projectFolderEdit_->setEnabled(enabled);
}

void ProjectPage::setProjectName(const QString& value)
{
    projectNameEdit_->setText(value);
}

void ProjectPage::setProjectNameEnabled(bool enabled)
{
    projectNameEdit_->setEnabled(enabled);
}

void ProjectPage::setExecEnabled(bool enabled)
{
    execImportButton_->setEnabled(enabled);
}


void ProjectPage::loadDefault(){
    setImportFile("");
    setImportEnabled(true);
    setBrowseEnabled(true);

    setProjectFolder("");
    setProjectFolderEnabled(false);

    setProjectName("");
    setProjectNameEnabled(true);

    setExecEnabled(false);
}

void ProjectPage::readFromFile(const std::string& file, const QString ImportFolder)
{
    const char* section = "PROJECTSECT";
    QLineEdit projectFolderEdit;
    projectFolderEdit.setText(
        ReadWriteOptions::readTextToLineEdit("ProjectFolder", section, file)
    );
    if (projectFolderEdit.text().isEmpty())
        setProjectFolder(ImportFolder);
    else
        setProjectFolder(projectFolderEdit.text());

    QLineEdit projectNameEdit;
    projectNameEdit.setText(
        ReadWriteOptions::readTextToLineEdit("ProjectName", section, file)
    );
    if (projectNameEdit.text().isEmpty())
        setProjectName(QFileInfo(QString(file.c_str())).fileName());
    else
        setProjectName(projectNameEdit.text());

    string v = CIniFile::GetValue("ImportFile","PROJECTSECT",file);
    QString qv = QString(v.c_str());
    if (!qv.isEmpty()){
        setImportFile(ImportFolder+'/'+qv);
    }
    else{
        setImportFile(ImportFolder+'/'+projectName());
    }
    setProjectFolderEnabled(true);
    setProjectNameEnabled(true);
}

void ProjectPage::writeToFile(const std::string& file,
                                bool isWindows,
                                bool* printwarns,
                                QString* warns)
{
    const char* section = "PROJECTSECT";

    auto writeText = [&](const char* key, const QString& value) {
        ReadWriteOptions::writeOption(
            key, section, value, file, printwarns, warns);
    };

    writeText("ImportFolder", QFileInfo(importFile()).path());
    writeText("ImportFile", QFileInfo(importFile()).fileName());
    writeText("ProjectFolder", projectFolder());
    writeText("ProjectName", projectName());
}
