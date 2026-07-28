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
//    Class defining the main window. Manages the graphic interface,
//    executes the auxiliary programs and displays the results
//
//  File:   mainwindow.cpp
//
//      Last version: May 2026
//
#include <string.h>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QtGlobal>
#include <QApplication>
#include <QPrinter>
#include <QPrintDialog>

#include <QtDebug>
#include <QLineEdit>
#include <QStringList>
#include <QBoxLayout>
#include <QtCore/qprocess.h>
#include <QPixmap>


#include <QProcess>
#include <QStandardPaths>
#include <QMessageBox>

#include "filedialogutils.h"
#include "mainwindow.h"
#include "GlobalInfo.h"
#include <QOperatingSystemVersion>

#include "projectpage.h"
#include "atomicdensitiespage.h"
#include "densitypage.h"
#include "potentialpage.h"
#include "orbitalspage.h"
#include "topographypage.h"
#include "sigmaholepage.h"
#include "fieldlinespage.h"
#include "densitygradientpage.h"
#include "hfforcespage.h"
#include "radialfactorspage.h"
#include "orientedmultipolespage.h"
#include "zjexpansionpage.h"
#include "zjdensitypage.h"

#include "fchkimporter.h"
#include "molproimporter.h"
#include "singlepassimporter.h"

#ifndef ANGSTROMTOBOHR
#define ANGSTROMTOBOHR 1.88971616463
#endif
#define SIZE4   400
#define SIZE20 2000

namespace
{

bool sameFilePath(const QString& first, const QString& second)
{
    return QDir::cleanPath(QFileInfo(first).absoluteFilePath()) ==
           QDir::cleanPath(QFileInfo(second).absoluteFilePath());
}

bool copyReplacing(const QString& source, const QString& target)
{
    if (sameFilePath(source, target))
        return true;

    if (!QFileInfo::exists(source))
        return false;

    if (QFileInfo::exists(target) && !QFile::remove(target))
        return false;

    return QFile::copy(source, target);
}

}

bool isWindowsPlatform()
{
    #ifdef Q_OS_WIN
        return true;
    #else
        return false;
    #endif
}

bool checkMpiCommand(QString& mpiCommand)
{
    QProcess process;
    QString shell;
    QString cmdPrefix;
    QStringList candidates;

    #ifdef Q_OS_WIN
        shell = "cmd";
        cmdPrefix = "/C";
        candidates = QStringList() << "where mpirun" << "where mpiexec";
    #else
        shell = "sh";
        cmdPrefix = "-c";
        candidates = QStringList() << "command -v mpirun" << "command -v mpiexec";
    #endif

    for (const QString& cmd : candidates) {
        process.start(shell, QStringList() << cmdPrefix << cmd);
        process.waitForFinished(-1);
        QByteArray output = process.readAllStandardOutput();

        if (!output.isEmpty()) {
            if (cmd.contains("mpirun"))
                mpiCommand = "mpirun";
            else
                mpiCommand = "mpiexec";
            return true;
        }
    }

    return false;
}

/* Sets initial values */
MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
{
    QPixmap pixmap;
    pixmap.load(":/images/splash_4_en.png");

    activebeware = false;
    denslist = new QStringList();
    lzdo = false;
    lvalence = false;
    plotsknt = 1;
    topindex = -1;
    widgetsknt = 1;
    maxnumprocessors = MAX_NUM_PROCESSORS;
    BTNraiseplotslist.clear();
    BTNshowplotsslist.clear();
    BTNraisewidgetslist.clear();
    BTNshowwidgetslist.clear();
    connections2D.clear();
    connections3D.clear();

    fchkImporter_ = new FchkImporter(this);

    connect(fchkImporter_, &FchkImporter::importFinished,
        this, &MainWindow::onFchkImportFinished
    );

    connect(fchkImporter_, &FchkImporter::outputTextReady,
        this,
        [this](const QString& text) {
            textEdit->setFont(QFont("Courier", 10));
            textEdit->setPlainText(text);
        });

    connect(fchkImporter_, &FchkImporter::outputFileError,
        this,
        [this](const QString& fileName,
               const QString& errorString) {
            QMessageBox::warning(
                this,
                tr("ProcessOutput"),
                tr("File %1 cannot be read:\n%2.")
                    .arg(fileName, errorString)
            );
        });

    molproImporter_ = new MolproImporter(this);

    connect(molproImporter_, &MolproImporter::importFinished,
        this, &MainWindow::onMolproImportFinished);

    connect(molproImporter_, &MolproImporter::outputTextReady,
        this,
        [this](const QString& text) {
            textEdit->setFont(QFont("Courier", 10));
            textEdit->setPlainText(text);
        });

    connect(molproImporter_, &MolproImporter::outputFileError,
        this,
        [this](const QString& fileName,
               const QString& errorString) {
            QMessageBox::warning(
                this,
                tr("ProcessOutput"),
                tr("File %1 cannot be read:\n%2.")
                    .arg(fileName, errorString)
            );
        });

    singlePassImporter_ = new SinglePassImporter(this);

    connect(singlePassImporter_, &SinglePassImporter::processStarted,
        this, &MainWindow::processStart);

    connect(singlePassImporter_, &SinglePassImporter::importFinished,
        this, &MainWindow::onSinglePassImportFinished);

    connect(singlePassImporter_, &SinglePassImporter::importFailed,
        this, &MainWindow::onSinglePassImportFailed);

    connect(
        singlePassImporter_,
        &SinglePassImporter::outputTextReady,
        this,
        [this](const QString& text) {
            textEdit->setFont(QFont("Courier", 10));
            textEdit->setPlainText(text);
        });

    connect(
        singlePassImporter_,
        &SinglePassImporter::outputFileError,
        this,
        [this](const QString& fileName,
               const QString& errorString) {
            QMessageBox::warning(
                this,
                tr("ProcessOutput"),
                tr("File %1 cannot be read:\n%2.")
                    .arg(fileName, errorString)
            );
        });

    splash = new QSplashScreen(pixmap);
    splash->show();

    FRMlanguage = new QDialog();
    FRMlanguage->setWhatsThis(tr("Check a language and push Start to start DAMQT."));
    FRMlanguage->setWindowTitle(tr("Language"));
    FRMlanguage->setFont(QFont("Helvetica",15));
    FRMlanguage->setMinimumWidth(480);
    FRMlanguage->setAttribute(Qt::WA_DeleteOnClose);

    createLanguageMenu();

    LBLlanguage = new QLabel();
    LBLlanguage->setText(tr("Choose language and push Start"));
    BTNlangstart=new QPushButton(QIcon(":/images/empezar.png"), tr("Start"));
    BTNlangstart->setMinimumWidth(120);
//    connect(BTNlangstart, SIGNAL(clicked()), this, SLOT(start()));
    connect(BTNlangstart, &QPushButton::clicked,
            this, &MainWindow::start);

    QHBoxLayout *languageLBLLayout = new QHBoxLayout();
    languageLBLLayout->addWidget(LBLlanguage);
    languageLBLLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *languageHLayout=new QHBoxLayout();
    languageHLayout->addWidget(languageMenu);
    languageHLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *languageBTNLayout=new QHBoxLayout();
    languageBTNLayout->addWidget(BTNlangstart);
    languageBTNLayout->setAlignment(Qt::AlignRight);

    QVBoxLayout *languageVLayout = new QVBoxLayout(FRMlanguage);
    languageVLayout->addLayout(languageLBLLayout);
    languageVLayout->addSpacing(20);
    languageVLayout->addLayout(languageHLayout);
    languageVLayout->addSpacing(10);
    languageVLayout->addLayout(languageBTNLayout);
    languageVLayout->addSpacing(10);

    FRMlanguage->exec();

//    QString path=QApplication::applicationDirPath();

    iswindows = isWindowsPlatform();

    mpi = checkMpiCommand(mpicommand);

    if (mpi) {
        qDebug() << "MPI is available using:" << mpicommand;
        // usa `mpicommand` para lanzar los procesos
    } else {
        qDebug() << "MPI not found in PATH";
        // desactiva opciones MPI en GUI si hace falta
    }


    setMinimumSize(900,600);
    setWindowState(Qt::WindowMaximized);
    setWindowIcon(QIcon(":/images/icon.png"));
    setWindowTitle(tr("DAMQT"));

    TAWprincipal = new QTabWidget(this);
    textEdit = new QTextEdit();
    textEdit->setReadOnly(true);

    int tabIndex=TAWprincipal->addTab(textEdit,QIcon(":/images/document_text.png"),tr("Results"));
    TAWprincipal->setCurrentIndex(tabIndex);
//    connect(TAWprincipal, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(TAWprincipal, &QTabWidget::currentChanged,
            this, &MainWindow::tabChanged);

    setCentralWidget(TAWprincipal);    

    CreateActions();
    CreateMenus();
    CreateToolBars();
    CreateStatusBar();
    CreateLeftMenu();    // Creates the left menu
    CreateRightMenu();    // Creates the right menu

    readSettings();

    SetCurrentFile("",true,false);

    mden = 0;
}

void MainWindow::finishsplash(){
    splash->finish(this);
}

/* Defines action for TAB change */
void MainWindow::tabChanged(int index)
{
    if(index==0){
        AccPrint->setEnabled(true);
        AccPdf->setEnabled(true);
    }else{
        AccPrint->setEnabled(false);
        AccPdf->setEnabled(false);
    }
}

/* Ends all events */
void MainWindow::closeEvent(QCloseEvent *event)
{
    if(QDLwidget3D && !end_Viewer3DDialog()){
       event->ignore();
       return;
    }
    if(QDLviewer2D && !end_Viewer2DDialog()){
        event->ignore();
        return;
    }
//    if (mustSave()) {
//        writeSettings();
//        event->accept();
//    }
//    else {
//        event->ignore();
//    }

    if (!mustSave()) {
        event->ignore();
        return;
    }

    writeSettings();
    event->accept();

    delete QDLviewer2D;
    QDLviewer2D = nullpointer;
    delete QDLwidget3D;
    QDLwidget3D = nullpointer;
}

/* Changes a string to a qstring */
QString MainWindow::toQString(string v)
{
    QString qv=QString(v.c_str());
    return qv;
}

/* Changes a qstring to a string */
string MainWindow::toString(QString qv)
{
    QByteArray ba = qv.toLatin1();
    //QByteArray ba = qv.toUtf8();
    const char *v=ba.data();
    return v;
}

void MainWindow::update_textedit(QString a){
    QFile file(a);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox msgBox;
        msgBox.setText(tr("submitOutput"));
        msgBox.setInformativeText(QString(tr("Failed opening %1 output file. Error: %2\n")
                        .arg(a).arg(file.errorString())));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
    else{
        QTextStream in(&file);
        textEdit->setFont(QFont("Courier",10));
        textEdit->setPlainText(in.readAll());
    }
}

/*****************************************************************************************************/
/******************************* ACTIONS  ************************************************************/
/*****************************************************************************************************/
/* Creates program actions (Language, New, Open, Save, Save as, Print, Print PDF, 2D Viewer, 3D Viewer, Exit, Help, About) */
void MainWindow::CreateActions()
{
//    New
    AccNew = new QAction(QIcon(":/images/Nuevo.png"),tr("&New project"), this);
    AccNew->setShortcut(tr("Ctrl+N"));
    AccNew->setStatusTip(tr("Opens a new project"));
//    connect(AccNew, SIGNAL(triggered()), this, SLOT(newProject()));
    connect(AccNew, &QAction::triggered,
            this, &MainWindow::newProject);
//    Open
    AccOpen = new QAction(QIcon(":/images/Abrir.png"), tr("&Open project..."), this);
    AccOpen->setShortcut(tr("Ctrl+A"));
    AccOpen->setStatusTip(tr("Open project file"));
//    connect(AccOpen, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(AccOpen, &QAction::triggered,
            this, &MainWindow::openProject);
//    Save
    AccSave = new QAction(QIcon(":/images/Guardar.png"),tr("&Save project"), this);
    AccSave->setShortcut(tr("Ctrl+S"));
    AccSave->setStatusTip(tr("Save project file"));
//    connect(AccSave, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(AccSave, &QAction::triggered,
            this, &MainWindow::saveProject);
//    Save as
    AccSaveAs = new QAction(tr("Save project &as..."), this);
    AccSaveAs->setStatusTip(tr("Saves project file as"));
//    connect(AccSaveAs, SIGNAL(triggered()), this, SLOT(SaveProjectAs()));
    connect(AccSaveAs, &QAction::triggered,
        this, &MainWindow::saveProjectAs);
//    Print
    AccPrint = new QAction(QIcon(":/images/printer.png"),tr("&Print"), this);
    AccPrint->setShortcut(tr("Ctrl+P"));
    AccPrint->setStatusTip(tr("Print output file"));
//    connect(AccPrint, SIGNAL(triggered()), this, SLOT(PrintFile()));
    connect(AccPrint, &QAction::triggered,
        this, &MainWindow::PrintFile);
//    Print to PDF file
    AccPdf = new QAction(QIcon(":/images/acrobat.png"), tr("&Create Pdf"), this);
    AccPdf->setShortcut(tr("Ctrl+D"));
    AccPdf->setStatusTip(tr("Print output file as Pdf"));
//    connect(AccPdf, SIGNAL(triggered()), this, SLOT(PrintFilePdf()));
    connect(AccPdf, &QAction::triggered,
        this, &MainWindow::PrintFilePdf);
//    External packages
    AccExternal = new QAction(QIcon(":/images/External_program.png"),tr("E&xternal"), this);
    AccExternal->setShortcut(tr("Ctrl+E"));
    AccExternal->setStatusTip(tr("External packages"));
//    connect(AccExternal, SIGNAL(triggered()), this, SLOT(external_package()));
    connect(AccExternal, &QAction::triggered,
        this, &MainWindow::external_package);
//    2D Viewer2D
    Acc2Dplot = new QAction(QIcon(":/images/plot2D_tiny.png"),tr("&2D Viewer"), this);
    Acc2Dplot->setStatusTip(tr("2D Viewer"));
//    connect(Acc2Dplot, SIGNAL(triggered()), this, SLOT(addviewer2D()));
    connect(Acc2Dplot, &QAction::triggered,
        this, &MainWindow::addviewer2D);
//    3D Viewer
    Acc3Dview = new QAction(QIcon(":/images/cube_molecule.png"),tr("&3D Viewer"), this);
    Acc3Dview->setStatusTip(tr("3D Viewer"));
//    connect(Acc3Dview, SIGNAL(triggered()), this, SLOT(addglWidget()));
    connect(Acc3Dview, &QAction::triggered,
        this, &MainWindow::addglWidget);

//    Recent files
    for (int i = 0; i < MAX_ARCHIVOS_RECIENTES; ++i) {
        if (!AccRecentFiles[i]){
            AccRecentFiles[i] = new QAction(this);
        }
        AccRecentFiles[i]->setVisible(false);
        connect(AccRecentFiles[i], SIGNAL(triggered()), this, SLOT(openRecentProjects()));
    }
//    Exit
    AccExit = new QAction(QIcon(":/images/Salir.png"),tr("&Exit"), this);
    AccExit->setShortcut(tr("Ctrl+Q"));
    AccExit->setStatusTip(tr("Quit"));
//    connect(AccExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(AccExit, &QAction::triggered,
            this, &QWidget::close);
//    Help
    AccHelp = new QAction(QIcon(":/images/ayuda.png"),tr("&Help"), this);
    AccHelp->setStatusTip(tr("Program help"));
//    connect(AccHelp, SIGNAL(triggered()), this, SLOT(Help()));
    connect(AccHelp, &QAction::triggered,
            this, &MainWindow::Help);
//    About
    AccAbout = new QAction(QIcon(":/images/icon.png"),tr("&About DAMQT"), this);
    AccAbout->setStatusTip(tr("About DAMQT"));
//    connect(AccAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(AccAbout, &QAction::triggered,
            this, &MainWindow::about);
//    About
//    AccPerformance = new QAction(createModernGearIcon(QSize(32, 32)),tr("&Performance settings"), this);
    AccPerformance = new QAction(QIcon(":/images/GearIcon32x32.png"),tr("&Performance settings"), this);
    AccPerformance->setStatusTip(tr("Performance settings"));
//    connect(AccPerformance, SIGNAL(triggered()), this, SLOT(showPerformanceSettings()));
    connect(AccPerformance, &QAction::triggered,
            this, &MainWindow::showPerformanceSettings);
//    About Qt
    AccAboutQt = new QAction(QIcon(":/images/qtlogo.png"),tr("About &Qt"), this);
    AccAboutQt->setStatusTip(tr("About QT Library"));
//    connect(AccAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(AccAboutQt, &QAction::triggered,
            qApp, &QApplication::aboutQt);
}

/* Action: New file */
void MainWindow::newProject()
{
    if (mustSave()) {
        disable_pages();
        textEdit->clear();
        loadDefault(0);
        SetCurrentFile("",true,false);
    }
    lzdo = false;
    lvalence = false;
}

void MainWindow::openProject()
{
    if (!mustSave())
        return;

    const QString initialDir =
        FileDialogUtils::initialDirectory(ProjectFolder);

    QFileDialog fileDialog(
        this,
        tr("Open input file"),
        initialDir
    );

    FileDialogUtils::configureLocalFileDialog(fileDialog);

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    fileDialog.setNameFilters({
        tr("Project files") + " (*.damproj)",
        tr("All files") + " (*)"
    });

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QStringList selectedFiles = fileDialog.selectedFiles();

    if (selectedFiles.isEmpty())
        return;

    const QString fileName = selectedFiles.first();

    disable_pages();

    if (!Open(fileName))
        return;

    defineRanges();

    QMessageBox::information(
        this,
        tr("DAMQT"),
        tr("Project %1 open").arg(ProjectName),
        QMessageBox::Ok
    );

    atomicDensitiesPage_->setEnabled(true);

    if (QFile::exists(fileName + QStringLiteral("_2016.damqt")))
        setPostDamPagesEnabled(true);

    orbitalsPage_->setEnabled(true);
}

/* Action: Open file ggbs or sgbs*/
bool MainWindow::Open(const QString &fileName)
{
    textEdit->clear();
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Open"),tr("File %1 cannot be read").arg(fileName)
                    +QString(": \n%1").arg(file.errorString()));
            return false;
    }
// Checks whether _2016.damqt, .ggbs or .sgbs, .xyz and .den files exist
    QString path = Path(fileName);
    if (path.at(path.length()-1) != '/') path.append('/');
    QString damqtfilename = path + FileWithoutExt(fileName)+"_2016.damqt";
    QString ggbsfilename  = path + FileWithoutExt(fileName)+".ggbs";
    QString sgbsfilename  = path + FileWithoutExt(fileName)+".sgbs";
    QString sgbsgzfilename  = path + FileWithoutExt(fileName)+".sgbs.gz";
    QString sgbsdenfilename  = path + FileWithoutExt(fileName)+".sgbsden";
    QString sgbsdengzfilename  = path + FileWithoutExt(fileName)+".sgbsden.gz";
    QString xyzfilename   = path + FileWithoutExt(fileName)+".xyz";
    QString denfilename   = path + FileWithoutExt(fileName)+".den";
    QString dengzfilename   = path + FileWithoutExt(fileName)+".den.gz";
    QString densprsbinfilename   = path + FileWithoutExt(fileName)+".densprsbin";
    if (((QFile::exists(ggbsfilename) || QFile::exists(sgbsfilename) || QFile::exists(sgbsgzfilename)) &&
                    (QFile::exists(denfilename) || QFile::exists(dengzfilename) || QFile::exists(densprsbinfilename)))
            || QFile::exists(sgbsdenfilename) || QFile::exists(sgbsdengzfilename) ){
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (QFile::exists(sgbsfilename) || QFile::exists(sgbsgzfilename)
                || QFile::exists(sgbsdenfilename) || QFile::exists(sgbsdengzfilename) ){
            lslater = true;
            potentialPage_->setExactPotentialVisible(false);
        }
        else{ 
            lslater = false;
            potentialPage_->setExactPotentialVisible(true);
        }
        atomicDensitiesPage_->setlslater(lslater);
        potentialPage_->setExactPotential(false);
        projectPage_->setImportFile(fileName);
        ImportFile = FileWithoutPath(projectPage_->importFile());
        ImportFolder = path;
        projectPage_->setProjectFolder(path);
        projectPage_->setProjectName(FileWithoutExt(fileName));
        if (lslater){
            QString sxyzfilename = path + FileWithoutExt(fileName)+".sxyz";
            if (!(QFile::exists(sxyzfilename))){
                execsgbs2sxyz(sxyzfilename);
            }
            set_natom(read_natom(sxyzfilename));
        }
        else{
            set_natom(read_natom(ggbsfilename));
        }
        if(Extension(fileName)!="sgbs" && Extension(fileName)!="ggbs" && Extension(FileWithoutExt(fileName))!="sgbs"
                && Extension(fileName)!="sgbsden" && Extension(FileWithoutExt(fileName))!="sgbsden"){
            loadDefault(0);
            readOptions(fileName);
        }
        QApplication::restoreOverrideCursor();

        atomicDensitiesPage_->setEnabled(true);
        if (QFile::exists(damqtfilename)){
            setPostDamPagesEnabled(true);
        }else{
            setPostDamPagesEnabled(false);
        }
        QStringList filenames(QString(FileWithoutExt(fileName) +".GAorb*"));
        filenames << QString(FileWithoutExt(fileName) +".SLorb*");
        filenames << QString(FileWithoutExt(fileName) +".orb*");
        QStringList files;
        files = QDir(ProjectFolder).entryList(QStringList(filenames),QDir::Files);
        if (!files.isEmpty()) 
            orbitalsPage_->setEnabled(true);
        else 
            orbitalsPage_->setEnabled(false);

//        SetCurrentFile(fileName,true,false);

        const QFileInfo fileInfo(fileName);

        const bool isProjectFile =
            fileInfo.suffix().compare(
                QStringLiteral("damproj"),
                Qt::CaseInsensitive
            ) == 0;

        SetCurrentFile(
            isProjectFile ? fileName : QString(),
            true,
            false
        );

        statusBar()->showMessage(tr("File succesfully loaded"), 2000);
        QMessageBox::information(this,tr("DAMQT"),tr("File succesfully loaded"), QMessageBox::Ok, 0);
        defineRanges();
        return true;
    }
    else{
        QMessageBox::warning(this,tr("DAMQT"),tr("Files %1 and/or %2 not found").arg(FileWithoutPath(ggbsfilename))
                .arg(FileWithoutPath(denfilename)));
        atomicDensitiesPage_->setEnabled(false);
        orbitalsPage_->setEnabled(false);
        setPostDamPagesEnabled(false);
        statusBar()->showMessage(tr("Error loading file"), 2000);
        return false;
    }

}

/* Action: Open a recent file */
void MainWindow::openRecentProjects()
{
    QAction *action=qobject_cast<QAction *>(sender());
    if (action){
        QString filezdo = ProjectFolder + "zdo";
//        if (QFileInfo(filezdo).exists()){
        if (QFileInfo::exists(filezdo)){
            lzdo = true;
        }
        else{
            lzdo = false;
        }
        QString filevalence = ProjectFolder + "valence";
//        if (QFileInfo(filevalence).exists()){
        if (QFileInfo::exists(filevalence)){
            lvalence = true;
        }
        else{
            lvalence = false;
        }
        bool res=Open(action->data().toString());
        QStringList filenames(QString(ProjectName +".GAorb*"));
        filenames << QString(ProjectName +".SLorb*");
        filenames << QString(ProjectName +".orb*");
        QStringList files;
        files = QDir(ProjectFolder).entryList(QStringList(filenames),QDir::Files);
        if ((QFile::exists(QString(ProjectFolder+ProjectName+".ggbs")) ||
             QFile::exists(QString(ProjectFolder+ProjectName+".ggbs.gz")) ||
             QFile::exists(QString(ProjectFolder+ProjectName+".sgbs")) ||
                QFile::exists(QString(ProjectFolder+ProjectName+".sgbs.gz")))
                && (QFile::exists(QString(ProjectFolder+ProjectName+".den")) ||
                    QFile::exists(QString(ProjectFolder+ProjectName+".den.gz") ))){
            atomicDensitiesPage_->setEnabled(true);
            if (res){
                QMessageBox::information(this,tr("DAMQT"),tr("Project %1 open").arg(ProjectName), QMessageBox::Ok, 0);
                if (QFile::exists(QString(ProjectFolder+ProjectName+"_2016.damqt"))){
                    setPostDamPagesEnabled(true);
                    readOptions(QString(ProjectFolder+ProjectName+".damproj"));
                }
            }
            if (!files.isEmpty())
                orbitalsPage_->setEnabled(true);
            else
                orbitalsPage_->setEnabled(false);
        }
        else{
            QMessageBox::information(this,tr("DAMQT"),tr("Project %1 cannot be opened").arg(ProjectName), QMessageBox::Ok, 0);
            disable_pages();
        }
    }
    else
        return;
}

/* Action: Update recent files */
void MainWindow::UpdateRecentFiles()
{
    QMutableStringListIterator i(ArchivosRecientes);
    while (i.hasNext()) {
        if (!QFile::exists(i.next())){
            i.remove();
        }
    }
    for (int j = 0; j < MAX_ARCHIVOS_RECIENTES; ++j) {
        if (j < ArchivosRecientes.count()) {
            QString text = tr("&%1 %2").arg(j+1).arg(FileWithoutPath(ArchivosRecientes[j]));
            AccRecentFiles[j]->setText(text);
            AccRecentFiles[j]->setData(ArchivosRecientes[j]);
            AccRecentFiles[j]->setVisible(true);
        } 
        else {
            AccRecentFiles[j]->setVisible(false);
        }
    }
}

bool MainWindow::saveProject()
{
    if (ProjectName.isEmpty())
        return saveProjectAs();

    if (ProjectFilePath.isEmpty()) {
        const QString defaultProjectFile =
            QDir(ProjectFolder).filePath(
                ProjectName + QStringLiteral(".damproj")
            );

        return saveProjectToFile(defaultProjectFile);
    }

    return saveProjectToFile(ProjectFilePath);
}

/* Action: Save project as */
bool MainWindow::saveProjectAs()
{
    const QString initialDir =
        FileDialogUtils::initialDirectory(ProjectFolder);

    QFileDialog fileDialog(
        this,
        tr("Save project as"),
        initialDir
    );

    FileDialogUtils::configureLocalFileDialog(fileDialog);

    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setDefaultSuffix(QStringLiteral("damproj"));

    const QString projectFilter =
        tr("Project files") + QStringLiteral(" (*.damproj)");

    fileDialog.setNameFilters({
        projectFilter,
        tr("All files") + QStringLiteral(" (*)")
    });

    fileDialog.selectNameFilter(projectFilter);

    if (fileDialog.exec() != QDialog::Accepted)
        return false;

    const QStringList selectedFiles = fileDialog.selectedFiles();

    if (selectedFiles.isEmpty())
        return false;

    const QFileInfo selectedFileInfo(selectedFiles.first());

    const QString fullFileName =
        QDir(selectedFileInfo.absolutePath()).filePath(
            selectedFileInfo.completeBaseName()
            + QStringLiteral(".damproj")
        );

    return saveProjectToFile(fullFileName);
}

/* Saves the project to the specified file */
bool MainWindow::saveProjectToFile(const QString& fullFileName)
{
    const QFileInfo fileInfo(fullFileName);
    const QString directoryPath = fileInfo.absolutePath();

    QDir directory(directoryPath);

    if (!directory.exists()) {
        QMessageBox messageBox(this);

        messageBox.setWindowTitle(tr("DAMQT"));
        messageBox.setInformativeText(
            tr("Directory %1 was not found.\n"
               "Do you wish to create it?")
                .arg(QDir::toNativeSeparators(directoryPath))
        );

        messageBox.setStandardButtons(
            QMessageBox::Yes |
            QMessageBox::No |
            QMessageBox::Cancel
        );

        messageBox.setDefaultButton(QMessageBox::Cancel);
        messageBox.setIcon(QMessageBox::Warning);

        if (messageBox.exec() != QMessageBox::Yes)
            return false;

        if (!QDir().mkpath(directoryPath)) {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Directory %1 could not be created.")
                    .arg(QDir::toNativeSeparators(directoryPath))
            );

            return false;
        }

        statusBar()->showMessage(
            tr("Project directory successfully created"),
            2000
        );

        setPostDamPagesEnabled(false);
    }

    QFile file(fullFileName);

    if (!file.open(
            QFile::WriteOnly |
            QFile::Text |
            QFile::Truncate)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("File %1 could not be saved:\n%2.")
                .arg(
                    QDir::toNativeSeparators(fullFileName),
                    file.errorString()
                )
        );

        return false;
    }

    file.close();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    saveOptions(fullFileName);
    QApplication::restoreOverrideCursor();

    ProjectFolder =
        QDir::cleanPath(directoryPath) + QDir::separator();

    ProjectName = fileInfo.completeBaseName();

    projectPage_->setProjectFolder(ProjectFolder);
    projectPage_->setProjectName(ProjectName);

    SetCurrentFile(fullFileName, true, false);

    statusBar()->showMessage(
        tr("File successfully saved"),
        2000
    );

    return true;
}

/* Returns whether a file must be saved or not */
bool MainWindow::mustSave()
{
    if (changes) {
        QMessageBox msgBox;
        msgBox.setInformativeText(tr("Document has been modified")+"\n"+tr("Do you want to save changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) return saveProject();
        else if (ret == QMessageBox::Cancel) return false;
    }
    return true;
}

/* Action: About */
void MainWindow::about()
{
    QMessageBox::about(this,tr("About DAMQT"), "<h2>"+tr("DAMQT 3.2")+"</h2>" "<p>"
        +tr("Copyright &copy; 2008-2025")+"</p>"
        "<p>"+tr("DAMQT is a program for the analysis of electron molecular density, "
        "electrostatic potential and field and Hellman-Feynman forces on nuclei.")
        +"</p>" "<p>"+tr("Developed in the Departamento de Quimica-Fisica Aplicada of "
        "the Universidad Autonoma de Madrid (Spain) in collaboration with the Departamento de "
        "Quimica-Fisica of the Universidad de Cadiz (Spain) and with the "
        "Indian Institute of Technology Kanpur (India).")+"</p>");
}

/* Action: Print */
void MainWindow::PrintFile()
{
    if (TAWprincipal->currentIndex()==0){
        if (!textEdit->document()->isEmpty()){
            QTextDocument *document = textEdit->document();
            QPrinter printer(QPrinter::HighResolution);
            QPrintDialog *dialog = new QPrintDialog(&printer,this);
            dialog->setWindowTitle(tr("Printing options"));
            if (dialog->exec() != QDialog::Accepted)return;
            printer.setFullPage(true);
            printer.setPageSize(QPrinter::A4);
            document->print(&printer);
        }
    }
}

/* Action: Print PDF */
void MainWindow::PrintFilePdf()
{
    if (TAWprincipal->currentIndex()==0){
        if (!textEdit->document()->isEmpty()){
            QTextDocument *document = textEdit->document();
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            QFileDialog filedialog(this);
            filedialog.setDirectory(ProjectFolder);
            filedialog.setWindowFlags(Qt::WindowStaysOnTopHint);
            QString fileName = filedialog.getSaveFileName(this,tr("Save pdf..."),ProjectFolder,
                    tr("pdf files")+" (*.pdf);;"+tr("All files")+" (*)");
            if (fileName.isEmpty())return;
            if (QFile::exists(fileName)) {
                QMessageBox msgBox;
                msgBox.setInformativeText(tr("File %1 exists").arg(QDir::toNativeSeparators(fileName))
                        +"\n"+tr("Do you want to overwrite?"));
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
                msgBox.setButtonText(QMessageBox::No, tr("No"));
                msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
                msgBox.setIcon(QMessageBox::Warning);
                int ret = msgBox.exec();
                if (!(ret==QMessageBox::Yes)) return;
            }
            printer.setOutputFileName(fileName);
            printer.setFullPage(true);
            printer.setPaperSize(QPrinter::A4);
            document->print(&printer);
        }
    }
}

/* Action: Help */
void MainWindow::Help()
{
    QString path;
    if (QApplication::applicationDirPath() == "/usr/local/bin")
        path=QApplication::applicationDirPath()+"/../doc/DAMQT_3.2.0_manual.pdf";
    else{
        path=QApplication::applicationDirPath()+"/DAMQT_3.2.0_manual.pdf";
        if (!QFileInfo::exists(path))
            path = QCoreApplication::applicationDirPath()+"/../../doc/manual"+"/DAMQT_3.2.0_manual.pdf";
    }
    bool r = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    if (!r)
        QMessageBox::warning(this,tr("DAMQT"),tr("Help file does not exist in ")+path,
                QMessageBox::Ok);
}

/* Action: viewer2D */
void MainWindow::menu_viewer2D()
{
//    for (int i = 0 ; i < connections2D.size() ; i++){
//        QObject::disconnect(connections2D.at(i));
//    }
//    connections2D.clear();
    delete QDLviewer2D;

    QDLviewer2D = new ViewerDialog(this);
    QDLviewer2D->setMinimumSize(250,80);

    plots.clear();

    BTNnewplot = new QPushButton(tr("New 2D Plotter"));
    BTNnewplot->setToolTip(tr("Creates a new window for 2D plotting"));
//    connections2D << connect(BTNnewplot, SIGNAL(clicked()), this, SLOT(addviewer2D()));
    connect(BTNnewplot, &QPushButton::clicked,
            this, &MainWindow::addviewer2D);

    auto *label2D = new QLabel(tr("2D plotters"));
    label2D->setStyleSheet("QLabel { color : blue; }");

    auto *layout2=new QVBoxLayout(QDLviewer2D);
    layout2->addWidget(label2D);
    layout2->addWidget(BTNnewplot);
    layout2->addStretch();
}

/* Action: viewer3D */
//void MainWindow::menu_viewer3D()
//{
////    for (int i = 0 ; i < connections3D.size() ; i++){
////        QObject::disconnect(connections3D.at(i));
////    }
////    connections3D.clear();
//    widgets = new QList<glWidget*>();
//    QDLwidget3D = new ViewerDialog();
//    QDLwidget3D->setMinimumSize(250,80);
//    BTNnewwidget = new QPushButton(tr("New 3D Window"));
//    BTNnewwidget->setToolTip(tr("Creates a new window for 3D Viewer"));
////    connections3D << connect(BTNnewwidget, SIGNAL(clicked()), this, SLOT(addglWidget()));
//    connect(BTNnewwidget, &QPushButton::clicked,
//                this, &MainWindow::addglWidget);

//    auto *label3D = new QLabel(tr("3D viewers"));
//    label3D->setStyleSheet("QLabel { color : red; }");

//    auto *layout2 = new QVBoxLayout(QDLwidget3D);
//    layout2->addWidget(label3D);
//    layout2->addWidget(BTNnewwidget);
//    layout2->addStretch();

//}

void MainWindow::menu_viewer3D()
{
    widgets.clear();

    delete QDLwidget3D;
    QDLwidget3D = new ViewerDialog(this);
    QDLwidget3D->setMinimumSize(250, 80);

    BTNnewwidget = new QPushButton(tr("New 3D Window"), QDLwidget3D);
    BTNnewwidget->setToolTip(tr("Creates a new window for 3D Viewer"));

    connect(BTNnewwidget, &QPushButton::clicked,
            this, &MainWindow::addglWidget);

    auto *label3D = new QLabel(tr("3D viewers"), QDLwidget3D);
    label3D->setStyleSheet("QLabel { color : red; }");

    auto *layout = new QVBoxLayout(QDLwidget3D);
    layout->addWidget(label3D);
    layout->addWidget(BTNnewwidget);
    layout->addStretch();
}

/***************************************************************************/
/******************************* MENUS *************************************/
/***************************************************************************/
/* Creates the menus of the graphic interface*/
void MainWindow::CreateMenus()
{
    FileMenu = menuBar()->addMenu(tr("&File"));
    FileMenu->addAction(AccNew);
    FileMenu->addAction(AccOpen);
    FileMenu->addAction(AccSave);
    FileMenu->addAction(AccSaveAs);
    FileMenu->addSeparator();
    FileMenu->addAction(AccPrint);
    FileMenu->addAction(AccPdf);
    FileMenu->addAction(AccExternal);
    if(MAX_ARCHIVOS_RECIENTES>0){
        FileMenu->addSeparator();
        for (int i = 0; i < MAX_ARCHIVOS_RECIENTES; ++i){
            FileMenu->addAction(AccRecentFiles[i]);
        }
    }
    FileMenu->addSeparator();
    FileMenu->addAction(AccExit);
    UpdateRecentFiles();
    menuBar()->addSeparator();
    GraphicsMenu = menuBar()->addMenu(tr("&Graphics"));
    GraphicsMenu->addAction(Acc2Dplot);
    GraphicsMenu->addAction(Acc3Dview);
    menuBar()->addSeparator();
    HelpMenu = menuBar()->addMenu(tr("&Help"));
    HelpMenu->addAction(AccHelp);
    HelpMenu->addSeparator();
    HelpMenu->addAction(AccAbout);
    HelpMenu->addAction(AccPerformance);
    HelpMenu->addAction(AccAboutQt);
}

/***************************************************************************/
/******************************* TOOLBARS **********************************/
/***************************************************************************/
/* Toolbars */
void MainWindow::CreateToolBars()
{
    ToolBarFile = addToolBar(tr("Project folder"));
    ToolBarFile->addAction(AccNew);
    ToolBarFile->addAction(AccOpen);
    ToolBarFile->addAction(AccSave);
    ToolBarFile->addAction(AccPrint);
    ToolBarFile->addAction(AccPdf);
    ToolBarFile->addAction(AccExternal);
    ToolBarHelp=addToolBar(tr("Graphics"));
    ToolBarHelp->addAction(Acc2Dplot);
    ToolBarHelp->addAction(Acc3Dview);
    ToolBarHelp=addToolBar(tr("Help"));
    ToolBarHelp->addAction(AccHelp);
    ToolBarHelp->addAction(AccAbout);
    ToolBarHelp->addAction(AccPerformance);
    ToolBarHelp->addAction(AccExit);
}

/* Statusbars*/
void MainWindow::CreateStatusBar()
{
    statusBar()->showMessage(tr("DAMQT"));
}

void MainWindow::update_statusbar(QString a){
    statusBar()->showMessage(a);
}

/*******************************************************************************************************/
/******************************** OTHER ****************************************************************/
/*******************************************************************************************************/
/* Reads initial settings */
void MainWindow::readSettings()
{
    QSettings settings("DAMQT", "Densidades");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(SIZE4, SIZE4)).toSize();
    resize(size);
    move(pos);
    ArchivosRecientes = settings.value("recentFiles").toStringList();
    UpdateRecentFiles();
}

/* Writes settings for next session */
void MainWindow::writeSettings()
{
    QSettings settings("DAMQT", "Densidades");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    QStringList NewArchRecent;
    for (int i = 0 ; i < min(2*MAX_ARCHIVOS_RECIENTES,ArchivosRecientes.size()) ; i++ ){
        NewArchRecent << ArchivosRecientes.at(i);
    }
    settings.setValue("recentFiles", NewArchRecent);
}

// Nuevo slot para configuración de rendimiento
void MainWindow::showPerformanceSettings() {
    ConfigDialog configDialog(this);
    configDialog.exec();
}

void MainWindow::SetCurrentFile(
    const QString& fileName,
    bool useFileName,
    bool modified)
{
    changes = modified;

    if (useFileName)
        ProjectFilePath = fileName;

    QString displayName = tr("Unnamed");

    if (!ProjectFilePath.isEmpty()) {
        displayName = QFileInfo(ProjectFilePath).fileName();

        ArchivosRecientes.removeAll(ProjectFilePath);
        ArchivosRecientes.prepend(ProjectFilePath);

        UpdateRecentFiles();
    }

    setWindowTitle(
        tr("%1 - %2 [*]")
            .arg(tr("DAMQT"), displayName)
    );

    setWindowModified(changes);
}

/***************************************************************************/
/*****************************  DOCK  WINDOWS ******************************/
/***************************************************************************/

/* Creates DockWindows */

//      CreateLeftMenu
//
void MainWindow::CreateLeftMenu()
{
    QDoubleValidator *myDoubleValidator = new QDoubleValidator(nullpointer);
    myDoubleValidator->setLocale(QLocale::English);
    QDockWidget *dock = new QDockWidget(tr("Options"),this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setMaximumSize(QSize(500, 2000));

    toolBox = new mainmenu(dock);
    toolBox->resize(QSize(SIZE4, SIZE20));
    toolBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

//    page_project: PROJECT

    projectPage_ = new ProjectPage(mpi, mpicommand, toolBox);
    projectPage_->setEnabled(true);

    connect(projectPage_, &ProjectPage::importFileChanged,
            this, &MainWindow::TXTImport_changed);

    connect(projectPage_, &ProjectPage::projectFolderChanged,
            this, &MainWindow::TXTProjectFolder_changed);

    connect(projectPage_, &ProjectPage::projectNameChanged,
            this, &MainWindow::TXTProjectName_changed);

    connect(projectPage_, &ProjectPage::mpiCommandChanged,
            this, &MainWindow::TXTmpicommand_changed);

    connect(projectPage_, &ProjectPage::mpiFlagsChanged,
            this, &MainWindow::TXTmpiflags_changed);

    connect(projectPage_, &ProjectPage::browseImportRequested,
            this, &MainWindow::importFile);

    connect(projectPage_, &ProjectPage::execImportRequested,
            this, &MainWindow::execImport);

    toolBox->addItem(projectPage_, QIcon(":/images/icon.png"), tr("Project"));

//    atomicDensitiesPage_: ATOMIC DENSITIES

    atomicDensitiesPage_ = new AtomicDensitiesPage(mpi, toolBox);
    atomicDensitiesPage_->setEnabled(false);

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::lmaxExpansionChanged,
            this, &MainWindow::atdenslmaxexp_changed);

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::statusMessageRequested,
            this, [this](const QString &message) {
                statusBar()->showMessage(message);
            });

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::execPagesEnabledChanged,
            this, [this](bool enabled) {
                setPostDamPagesEnabled(enabled);
            });

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::externalProcessStarted,
            this, &MainWindow::onExternalDamStarted);

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::externalProcessFinished,
            this, &MainWindow::onExternalDamFinished);

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::execRequested,
            this, &MainWindow::execDam);

    connect(atomicDensitiesPage_, &AtomicDensitiesPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(atomicDensitiesPage_, QIcon(":/images/icon.png"), tr("Atomic densities"));

//    densityPage_:   MOLECULAR DENSITY

    densityPage_ = new DensityPage(mpi, toolBox);
    densityPage_->setEnabled(false);

    postDamPages_ << densityPage_;

    connect(densityPage_, &DensityPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(densityPage_, &DensityPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(densityPage_, &DensityPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(densityPage_, &DensityPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(densityPage_, &DensityPage::execRequested,
            this, &MainWindow::execDamden);

    connect(densityPage_, &DensityPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(densityPage_, QIcon(":/images/icon.png"), tr("Density"));

//    potentialPage_: ELECTROSTATIC POTENTIAL

    potentialPage_ = new PotentialPage(mpi, toolBox);
    potentialPage_->setEnabled(false);

    postDamPages_ << potentialPage_;

    connect(potentialPage_, &PotentialPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(potentialPage_, &PotentialPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(potentialPage_, &PotentialPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(potentialPage_, &PotentialPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(potentialPage_, &PotentialPage::execRequested,
            this, &MainWindow::execDampot);

    connect(potentialPage_, &PotentialPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(potentialPage_,QIcon(":/images/icon.png"),tr("Electrostatic potential"));

//    orbitalsPage_: MOLECULAR ORBITALS

    orbitalsPage_ = new OrbitalsPage(mpi, toolBox);
    orbitalsPage_->setEnabled(false);

    connect(orbitalsPage_, &OrbitalsPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(orbitalsPage_, &OrbitalsPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(orbitalsPage_, &OrbitalsPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(orbitalsPage_, &OrbitalsPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(orbitalsPage_, &OrbitalsPage::execRequested,
            this, &MainWindow::execDamorb);

    connect(orbitalsPage_, &OrbitalsPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(orbitalsPage_, QIcon(":/images/icon.png"), tr("Molecular orbitals"));

//    topographyPage_: MOLECULAR TOPOGRAPHY

    topographyPage_ = new TopographyPage(mpi, toolBox);
    topographyPage_->setEnabled(false);

    postDamPages_ << topographyPage_;

    connect(topographyPage_, &TopographyPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(topographyPage_, &TopographyPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(topographyPage_, &TopographyPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(topographyPage_, &TopographyPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(topographyPage_, &TopographyPage::execRequested,
            this, &MainWindow::execDamTopography);

//    connect(topographyPage_, &TopographyPage::stopRequested,
//            this, &MainWindow::processStop);

    connect(topographyPage_, &TopographyPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(topographyPage_, QIcon(":/images/icon.png"), tr("Molecular topography"));

//    sigmaHolePage_: surface potential (MESP sigma hole)

    sigmaHolePage_ = new SigmaHolePage(mpi, toolBox);
    sigmaHolePage_->setEnabled(false);

    postDamPages_ << sigmaHolePage_;

    connect(sigmaHolePage_, &SigmaHolePage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(sigmaHolePage_, &SigmaHolePage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(sigmaHolePage_, &SigmaHolePage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(sigmaHolePage_, &SigmaHolePage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(sigmaHolePage_, &SigmaHolePage::execRequested,
            this, &MainWindow::execDamSGhole);

    connect(sigmaHolePage_, &SigmaHolePage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(sigmaHolePage_, QIcon(":/images/icon.png"), tr("Surface potential"));

//    fieldLinesPage_: ELECTRIC FIELD

    fieldLinesPage_ = new FieldLinesPage(mpi, toolBox);
    fieldLinesPage_->setEnabled(false);

    postDamPages_ << fieldLinesPage_;


    connect(fieldLinesPage_, &FieldLinesPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(fieldLinesPage_, &FieldLinesPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(fieldLinesPage_, &FieldLinesPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(fieldLinesPage_, &FieldLinesPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(fieldLinesPage_, &FieldLinesPage::execRequested,
            this, &MainWindow::execDamfield);

    connect(fieldLinesPage_, &FieldLinesPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(fieldLinesPage_, QIcon(":/images/icon.png"), tr("Electric field"));

//    densityGradientPage_: DENSITY GRADIENT

    densityGradientPage_ = new DensityGradientPage(mpi, toolBox);
    densityGradientPage_->setEnabled(false);

    postDamPages_ << densityGradientPage_;

    connect(densityGradientPage_, &DensityGradientPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(densityGradientPage_, &DensityGradientPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(densityGradientPage_, &DensityGradientPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(densityGradientPage_, &DensityGradientPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);


    connect(densityGradientPage_, &DensityGradientPage::execRequested,
            this, &MainWindow::execDamdengrad);

    connect(densityGradientPage_, &DensityGradientPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(densityGradientPage_, QIcon(":/images/icon.png"), tr("Density gradient"));

//    hfForcesPage_: HELLMANN-FEYNMAN FORCES ON NUCLEI

    hfForcesPage_ = new HFForcesPage(toolBox);
    hfForcesPage_->setEnabled(false);

    postDamPages_ << hfForcesPage_;

    connect(hfForcesPage_, &HFForcesPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(hfForcesPage_, &HFForcesPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(hfForcesPage_, &HFForcesPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(hfForcesPage_, &HFForcesPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(hfForcesPage_, &HFForcesPage::execRequested,
            this, &MainWindow::execDamforces);

    connect(hfForcesPage_, &HFForcesPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(hfForcesPage_, QIcon(":/images/icon.png"), tr("Hellmann-Feynman forces on nuclei"));

//    radialFactorsPage_: RADIAL FACTORS

    radialFactorsPage_ = new RadialFactorsPage(toolBox);
    radialFactorsPage_->setEnabled(false);

    postDamPages_ << radialFactorsPage_;

    connect(radialFactorsPage_, &RadialFactorsPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(radialFactorsPage_, &RadialFactorsPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(radialFactorsPage_, &RadialFactorsPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(radialFactorsPage_, &RadialFactorsPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(radialFactorsPage_, &RadialFactorsPage::execRequested,
            this, &MainWindow::execDamfrad);

    connect(radialFactorsPage_, &RadialFactorsPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(radialFactorsPage_, QIcon(":/images/icon.png"), tr("Radial factors"));

//    orientedMultipolesPage_: ORIENTED MULTIPOLES

    orientedMultipolesPage_ = new OrientedMultipolesPage(toolBox);
    orientedMultipolesPage_->setEnabled(false);

    postDamPages_ << orientedMultipolesPage_;

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::execRequested,
            this, &MainWindow::execDammultrot);

    connect(orientedMultipolesPage_, &OrientedMultipolesPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(orientedMultipolesPage_, QIcon(":/images/icon.png"), tr("Oriented multipoles"));

//    zjExpansionPage_: ZERNIKE-JACOBI EXPANSIONS OF DENSITY

    zjExpansionPage_ = new ZJExpansionPage(toolBox);
    zjExpansionPage_->setEnabled(false);

    postDamPages_ << zjExpansionPage_;

    connect(zjExpansionPage_, &ZJExpansionPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(zjExpansionPage_, &ZJExpansionPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(zjExpansionPage_, &ZJExpansionPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(zjExpansionPage_, &ZJExpansionPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(zjExpansionPage_, &ZJExpansionPage::execRequested,
            this, &MainWindow::execDamZJ);

    connect(zjExpansionPage_, &ZJExpansionPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(zjExpansionPage_, QIcon(":/images/icon.png"), tr("Zernike-Jacobi density expansion"));
    
//    zjDensityPage_: TABULATION OF DENSITY FROM ZERNIKE-JACOBI EXPANSIONS

    zjDensityPage_ = new ZJDensityPage(toolBox);
    zjDensityPage_->setEnabled(false);

    postDamPages_ << zjDensityPage_;

    connect(zjDensityPage_, &ZJDensityPage::outputTextReady,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
            });

    connect(zjDensityPage_, &ZJDensityPage::showInputFileRequested,
            this, [this](const QString &text) {
                textEdit->setFont(QFont("Courier", 10));
                textEdit->setPlainText(text);
                });

    connect(zjDensityPage_, &ZJDensityPage::externalProcessStarted,
            this, &MainWindow::onExternalProcessStarted);

    connect(zjDensityPage_, &ZJDensityPage::externalProcessFinished,
            this, &MainWindow::onExternalProcessFinished);

    connect(zjDensityPage_, &ZJDensityPage::execRequested,
            this, &MainWindow::execDamdenZJ);

    connect(zjDensityPage_, &ZJDensityPage::openOutputRequested,
            this, &MainWindow::importOUT);

    toolBox->addItem(zjDensityPage_, QIcon(":/images/icon.png"), tr("Zernike-Jacobi density tabulation"));

    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(dock->sizePolicy().hasHeightForWidth());
    dock->setSizePolicy(sizePolicy1);
    dock->setWidget(toolBox);
    addDockWidget(Qt::LeftDockWidgetArea,dock);

//    LOAD DEFAULT OPTIONS

    loadDefault(0);

    disable_pages();
}
    
//      End of CreateLeftMenu
//      =====================

//      CreateRightMenu
//

void MainWindow::CreateRightMenu()
{    
    if (BTNnewplot){
        delete BTNnewplot;
        BTNnewplot = nullpointer;
    }
    if (FRMplots){
        delete FRMplots;
        FRMplots = nullpointer;
    }
    if (QDLviewer2D){
        delete QDLviewer2D;
        QDLviewer2D = nullpointer;
    }

    if (BTNnewwidget){
        delete BTNnewwidget;
        BTNnewwidget = nullpointer;
    }
    if (FRMviewers){
        delete FRMviewers;
        FRMviewers = nullpointer;
    }
    if (QDLwidget3D){
        delete QDLwidget3D;
        QDLwidget3D = nullpointer;
    }
    if ( BTNraiseviewers){
        delete  BTNraiseviewers;
         BTNraiseviewers = nullpointer;
    }
    if (dockright){
        delete dockright;
        dockright = nullpointer;
    }
    dockright = new QDockWidget(tr(""),this);
    dockright->setAllowedAreas(Qt::RightDockWidgetArea);
    dockright->resize(QSize(500, this->height()));
    dockright->setFeatures(QDockWidget::DockWidgetMovable);
    dockright->setFeatures(QDockWidget::DockWidgetFloatable);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(1000);
    sizePolicy1.setHeightForWidth(dockright->sizePolicy().hasHeightForWidth());
    dockright->setSizePolicy(sizePolicy1);
    addDockWidget(Qt::RightDockWidgetArea,dockright);

    menu_viewer2D();
    menu_viewer3D();

    BTNraiseviewers = new QPushButton(tr("Raise viewers"));
    BTNraiseviewers->setToolTip(tr("Moves viewers to front"));
    BTNraiseviewers->setStyleSheet("QPushButton {background-color: darkGreen; color: white;}");
    connect(BTNraiseviewers, SIGNAL(clicked()), this, SLOT(updatewindowsoverlay()));

    QVBoxLayout *layout1 = new QVBoxLayout();
    layout1->addWidget(BTNraiseviewers);

    QWidget *btnwidget = new QWidget();
    btnwidget->setLayout(layout1);

    dockwidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(dockwidget);
    layout->addWidget(btnwidget);
    layout->addWidget(QDLviewer2D);
    layout->addWidget(QDLwidget3D);
    layout->addStretch();

    dockright->setWidget(dockwidget);
    updatewindowsoverlay();
}

//      End of CreateRightMenu
//      =========================

/***************************************************************************/
/*  page_project: PROJECT                                             */
/***************************************************************************/

void MainWindow::chooseLanguage(QAction *action)
{
    QString locale = action->data().toString();
    QString qmPath = ":/translations/";
    if (locale.isEmpty()) locale.append("en");
    appTranslator.load("DAMQT_"+locale,qmPath);
    appTranslator.setObjectName("DAMQT_"+locale);
    qApp->installTranslator(&appTranslator);
    qtTranslator.load("qt_"+locale,qmPath);
    qApp->installTranslator(&qtTranslator);
    FRMlanguage->setWhatsThis(tr("Check a language and push Start to start DAMQT."));
    FRMlanguage->setWindowTitle(tr("Language"));
    LBLlanguage->setText(tr("Choose language and push Start"));
    languageMenu->setWhatsThis(tr("Check a language and push Start to start DAMQT."));
    BTNlangstart->setText(tr("Start"));
    if (locale == QString("es")){
        QPixmap pixmap;
        pixmap.load(QString(":/images/splash_4_%1.png").arg(locale));
        splash->setPixmap(pixmap);
        splash->show();
    }
    else{
        QPixmap pixmap;
        pixmap.load(QString(":/images/splash_4_en.png"));
        splash->setPixmap(pixmap);
        splash->show();
    }
}

void MainWindow::createLanguageMenu(){
    languageMenu = new QMenu(this);
    languageMenu->setWhatsThis(tr("Check a language and push Start to start DAMQT."));
    languageActionGroup = new QActionGroup(this);
//    connect(languageActionGroup,SIGNAL(triggered(QAction *)),this, SLOT(chooseLanguage(QAction *)));

    connect(languageActionGroup, &QActionGroup::triggered,
        this, &MainWindow::chooseLanguage);

    QDir qmDir = QDir(":/translations");
    QStringList fileNames = qmDir.entryList(QStringList("DAMQT_*.qm"));
//    qDebug() << "fileNames = " << fileNames;
    for (int i = 0 ; i < fileNames.size(); ++i){
        QString locale = fileNames[i];
//        qDebug() << "locale = " << locale;
        locale.remove(0,locale.indexOf('_')+1);
        locale.chop(3);
        QTranslator translator;
        translator.load(fileNames[i], qmDir.absolutePath());
        QString language = translator.translate("MainWindow", "English");
//        qDebug() << "language = " << language;
        QAction *action = new QAction(tr("&%1 %2").arg(i+1).arg(language),this);
        action->setCheckable(true);
        action->setData(locale);
        languageMenu->addAction(action);
        languageActionGroup->addAction(action);
        if (language == "English")
            action->setChecked(true);
    }
}

/* Imports file */
void MainWindow::execImport()
{
    QString DirNombreImport = projectPage_->importFile();
    if (DirNombreImport.size()==0){
        QMessageBox::warning(this, tr("DAMQT"),tr("Navigate to a directory with a suitable import file "));
        return;
    }
//    QFile file(DirNombreImport);
    if (!QFile::exists(DirNombreImport)){
        QMessageBox::warning(this, tr("DAMQT"),
                     tr("File %1 not found.").arg(DirNombreImport));
        return;
    }
    if (ProjectFolder.size()==0){
        QMessageBox::warning(this, tr("DAMQT"),tr("Introduce project folder"));
        return;
    }
    ImportFolder = QFileInfo(DirNombreImport).path();
    if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');
    ImportFile = QFileInfo(DirNombreImport).fileName();

    QString suffix=Extension(DirNombreImport);
    bool isgzipped = false;
    if (suffix =="gz"){
//        int ios = QProcess::execute("gunzip "+DirNombreImport);
        const int ios = QProcess::execute(QStringLiteral("gunzip"),QStringList{DirNombreImport});
        if (ios != 0){
            return;
        }
        DirNombreImport = DirNombreImport.remove(-3,3);
        suffix=Extension(DirNombreImport);
        isgzipped = true;
    }
    if (suffix=="ggbs"){
        lslater = false;
        execGgbsDen();
    }else if (suffix=="sgbs" || suffix =="sgbsden"){
        lslater = true;
        execSxyzDen();
    }else if (suffix=="fchk"){
        lslater = false;
        readFchk();
    }else if (suffix=="mos" || suffix=="coord" || suffix=="basis"){
        lslater = false;
        readTurbom();
    }else if (suffix=="mkl"){
        lslater = false;
        readMOLEKEL();
    }else if (suffix=="out" || suffix=="xml"){
        readMolpro();
        lslater = false;
    }else if (suffix=="aux"){
        lslater = true;
        readMopac();
    }else if (suffix=="nwcout"){
        lslater = false;
        readNWChem();
    }
    atomicDensitiesPage_->setlslater(lslater);
    if (lslater){
        potentialPage_->setExactPotentialVisible(false);
    }
    else{
        potentialPage_->setExactPotentialVisible(true);
    }
    potentialPage_->setExactPotential(false);

    if (isgzipped){
        QProcess::execute(QStringLiteral("gzip"),QStringList{DirNombreImport});
    }
}

/* Import name of a file */
void MainWindow::importFile()
{
    lzdo = false;
    lvalence = false;

    const QString initialDir =
        FileDialogUtils::initialDirectory(ImportFolder);

    QFileDialog fileDialog(
        this,
        tr("Open file ..."),
        initialDir
    );

    FileDialogUtils::configureLocalFileDialog(fileDialog);

    fileDialog.setWindowFlags(
        fileDialog.windowFlags() | Qt::WindowStaysOnTopHint
    );

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    // Set only local locations in the sidebar.
    // This avoids warnings produced by non-local URLs
    // when using the non-native QFileDialog.

//    QList<QUrl> sidebarUrls;

    fileDialog.setNameFilters({
        tr("Import data from") +
            " (*.ggbs *.sgbs *.sgbsden *.sgbsden.gz *.fchk *.coord "
            "*.basis *.mos *.mkl *.out *.xml *.aux *.nwcout)",

        tr("Geometry and basis set files") +
            " (*.ggbs *.sgbs *.sgbsden *.sgbsden.gz *.coord *.basis)",

        tr("fchk files") + " (*.fchk)",

        tr("All files") + " (*)"
    });

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QStringList selectedFiles =
        fileDialog.selectedFiles();

    if (selectedFiles.isEmpty())
        return;

    const QString fileName =
        selectedFiles.first();

    const QFileInfo fileInfo(fileName);

    projectPage_->setImportFile(fileName);

    ImportFolder = fileInfo.absolutePath();
    ImportFile = fileInfo.completeBaseName();

    projectPage_->setProjectFolder(
        fileInfo.absolutePath()
    );

    ProjectFolder =
        projectPage_->projectFolder();

    /*
     * The following code contatenates the file names directly,
     * so that we keep the final separator.
     */
    if (!ImportFolder.endsWith(QDir::separator()))
        ImportFolder += QDir::separator();

    if (!ProjectFolder.endsWith(QDir::separator()))
        ProjectFolder += QDir::separator();

    lzdo = QFileInfo::exists(
        ProjectFolder + QStringLiteral("zdo")
    );

    lvalence = QFileInfo::exists(
        ProjectFolder + QStringLiteral("valence")
    );

    projectPage_->setProjectName(
        fileInfo.completeBaseName()
    );

    ProjectName =
        projectPage_->projectName();

    const QString suffix =
        fileInfo.suffix().toLower();

    if (suffix == QStringLiteral("ggbs") ||
        suffix == QStringLiteral("sgbs") ||
        suffix == QStringLiteral("sgbsden")) {

        const bool opened = Open(fileName);

        if (opened) {
            QMessageBox::information(
                this,
                tr("DAMQT"),
                tr("Project %1 open").arg(ProjectName),
                QMessageBox::Ok
            );
        }
        else {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Cannot open project %1")
                    .arg(ProjectName)
            );

            return;
        }
    }

    projectPage_->setExecEnabled(true);
    atomicDensitiesPage_->setEnabled(true);

    const QString damqtFile =
        ImportFolder +
        fileInfo.completeBaseName() +
        QStringLiteral("_2016.damqt");

    setPostDamPagesEnabled(
        QFileInfo::exists(damqtFile)
    );

    QStringList nameFilters;

    nameFilters
        << fileInfo.completeBaseName() + QStringLiteral(".GAorb*")
        << fileInfo.completeBaseName() + QStringLiteral(".SLorb*")
        << fileInfo.completeBaseName() + QStringLiteral(".orb*");

    const QStringList orbitalFiles =
        QDir(ImportFolder).entryList(
            nameFilters,
            QDir::Files
        );

    orbitalsPage_->setEnabled(
        !orbitalFiles.isEmpty()
    );

    zjExpansionPage_->setEnabled(true);
}

/* Textbox for introducing the mpi command */
void MainWindow::TXTmpicommand_changed()
{
    mpicommand = projectPage_->mpiCommand();
}

/* Textbox for introducing the mpi flags */
void MainWindow::TXTmpiflags_changed()
{
    mpiflags = projectPage_->mpiFlags();
}

/* Textbox for introducing the file to import */
void MainWindow::TXTImport_changed()
{
    if(projectPage_->importFile().isEmpty()){
        projectPage_->setExecEnabled(false);
    }else{
        ImportFile = FileWithoutPath(projectPage_->importFile());
        ImportFolder = Path(projectPage_->importFile());
        if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');
        projectPage_->setExecEnabled(true);
        projectPage_->setProjectFolderEnabled(true);
    }
    if (projectPage_->projectName().isEmpty() && !projectPage_->importFile().isEmpty()){
        projectPage_->setProjectName(FileWithoutExt(projectPage_->importFile()));
        projectPage_->setProjectNameEnabled(true);
    }
    atomicDensitiesPage_->setEnabled(false);
}

/* Textbox for input data file name */
void MainWindow::TXTProjectFolder_changed(const QString &cad)
{
    ProjectFolder = cad;
    if (cad.mid(cad.length()-1,1) != "/"){
            ProjectFolder.append("/");
    }
    atomicDensitiesPage_->setProjectFolder(ProjectFolder);
    orbitalsPage_->setProjectFolder(ProjectFolder);
    topographyPage_->setProjectFolder(ProjectFolder);
    sigmaHolePage_->setProjectFolder(ProjectFolder);
    fieldLinesPage_->setProjectFolder(ProjectFolder);
    zjDensityPage_->setProjectFolder(ProjectFolder);
}

/* Textbox with project name */

void MainWindow::TXTProjectName_changed(const QString& name)
{
    if (projectPage_->projectName().isEmpty()) {
        projectPage_->setExecEnabled(false);
    } else if (!projectPage_->importFile().isEmpty()) {
        projectPage_->setExecEnabled(true);
    }

    ProjectName = name;

    SetCurrentFile(QString(), false, true);

    densityPage_->setOutputPrefix(ProjectName);
    potentialPage_->setOutputPrefix(ProjectName);
    hfForcesPage_->setOutputPrefix(ProjectName);
    fieldLinesPage_->setOutputPrefix(ProjectName);
    sigmaHolePage_->setOutputPrefix(ProjectName);
    topographyPage_->setOutputPrefix(ProjectName);
    radialFactorsPage_->setOutputPrefix(ProjectName);
    orientedMultipolesPage_->setOutputPrefix(ProjectName);
    orbitalsPage_->setOutputPrefix(ProjectName);
    zjDensityPage_->setOutputPrefix(ProjectName);
}


/**************************************************************************************************/
/********************** DIRECT IMPORT OF FILES WITH GEOMETRY; BASIS SET AND DENSITY  **************/
/**************************************************************************************************/

/* Gaussian data files */
void MainWindow::execGgbsDen()
{
    QDir projectDir(ProjectFolder);

    if (!projectDir.exists()) {
        QMessageBox msgBox(this);

        msgBox.setInformativeText(
            tr("Project %1 not found").arg(ProjectFolder) +
            QStringLiteral("\n") +
            tr("Do you wish to create?")
        );

        msgBox.setStandardButtons(
            QMessageBox::Yes |
            QMessageBox::No |
            QMessageBox::Cancel
        );

        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);

        if (msgBox.exec() != QMessageBox::Yes)
            return;

        createDir(ProjectFolder);

        projectDir.setPath(ProjectFolder);

        if (!projectDir.exists()) {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Project folder %1 could not be created.")
                    .arg(ProjectFolder)
            );
            return;
        }

        statusBar()->showMessage(
            tr("Project successfully created"),
            2000
        );

        setPostDamPagesEnabled(false);
    }

    const QDir importDir(ImportFolder);
    const QDir outputDir(ProjectFolder);

    const QString projectFile =
        outputDir.filePath(
            ProjectName + QStringLiteral(".damproj")
        );

    existsinp(projectFile, 1, true);

    const QString sourceGgbs =
        importDir.filePath(ImportFile);

    const QString targetGgbs =
        outputDir.filePath(
            ProjectName + QStringLiteral(".ggbs")
        );

    if (!copyReplacing(sourceGgbs, targetGgbs)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Cannot copy file %1 to %2.")
                .arg(sourceGgbs, targetGgbs)
        );
        return;
    }

    const QString importBaseName =
        QFileInfo(ImportFile).completeBaseName();

    QString sourceDensity =
        importDir.filePath(
            importBaseName + QStringLiteral(".den")
        );

    QString targetDensity =
        outputDir.filePath(
            ProjectName + QStringLiteral(".den")
        );

    if (!QFileInfo::exists(sourceDensity)) {
        sourceDensity += QStringLiteral(".gz");
        targetDensity += QStringLiteral(".gz");
    }

    if (!copyReplacing(sourceDensity, targetDensity)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Cannot copy density file %1 to %2.")
                .arg(sourceDensity, targetDensity)
        );
        return;
    }

    const QString sourceOptions =
        importDir.filePath(
            importBaseName + QStringLiteral("_2016.damqt")
        );

    const QString targetOptions =
        outputDir.filePath(
            ProjectName + QStringLiteral("_2016.damqt")
        );

    const bool optionsExist =
        QFileInfo::exists(sourceOptions);

    if (optionsExist &&
        !copyReplacing(sourceOptions, targetOptions)) {

        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Cannot copy options file %1 to %2.")
                .arg(sourceOptions, targetOptions)
        );
        return;
    }

    defineRanges();

    atomicDensitiesPage_->setEnabled(true);
    setPostDamPagesEnabled(optionsExist);
    orbitalsPage_->setEnabled(true);
}

/* Slater data files */
void MainWindow::execSxyzDen()
{
    QDir projectDir(ProjectFolder);

    if (!projectDir.exists()) {
        QMessageBox msgBox(this);

        msgBox.setInformativeText(
            tr("Project %1 not found").arg(ProjectFolder) +
            QStringLiteral("\n") +
            tr("Do you wish to create?")
        );

        msgBox.setStandardButtons(
            QMessageBox::Yes |
            QMessageBox::No |
            QMessageBox::Cancel
        );

        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);

        if (msgBox.exec() != QMessageBox::Yes)
            return;

        createDir(ProjectFolder);

        projectDir.setPath(ProjectFolder);

        if (!projectDir.exists()) {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Project folder %1 could not be created.")
                    .arg(ProjectFolder)
            );
            return;
        }

        statusBar()->showMessage(
            tr("Project successfully created"),
            2000
        );

        setPostDamPagesEnabled(false);
    }

    const QDir importDir(ImportFolder);
    const QDir outputDir(ProjectFolder);

    const QString projectFile =
        outputDir.filePath(
            ProjectName + QStringLiteral(".damproj")
        );

    existsinp(projectFile, 1, true);

    const QString sourceImport =
        importDir.filePath(ImportFile);

    const QFileInfo importInfo(ImportFile);
    const QString importSuffix =
        importInfo.suffix().toLower();

    const QString importBaseName =
        importInfo.completeBaseName();

    QString targetExtension;

    if (importSuffix == QStringLiteral("sgbs")) {
        targetExtension = QStringLiteral(".sgbs");
    }
    else if (importSuffix == QStringLiteral("sgbsden")) {
        targetExtension = QStringLiteral(".sgbsden");
    }
    else {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Unsupported Slater file type: %1")
                .arg(ImportFile)
        );
        return;
    }

    const QString targetImport =
        outputDir.filePath(
            ProjectName + targetExtension
        );

    if (!copyReplacing(sourceImport, targetImport)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Cannot copy file %1 to %2.")
                .arg(sourceImport, targetImport)
        );
        return;
    }

    /*
     * A separate density file is required for .sgbs files,
     * but not for .sgbsden files.
     */
    if (importSuffix == QStringLiteral("sgbs")) {
        QString sourceDensity =
            importDir.filePath(
                importBaseName + QStringLiteral(".den")
            );

        QString targetDensity =
            outputDir.filePath(
                ProjectName + QStringLiteral(".den")
            );

        if (!QFileInfo::exists(sourceDensity)) {
            sourceDensity += QStringLiteral(".gz");
            targetDensity += QStringLiteral(".gz");
        }

        if (!copyReplacing(sourceDensity, targetDensity)) {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Cannot copy density file %1 to %2.")
                    .arg(sourceDensity, targetDensity)
            );
            return;
        }
    }

    const QString sourceOptions =
        importDir.filePath(
            importBaseName + QStringLiteral("_2016.damqt")
        );

    const QString targetOptions =
        outputDir.filePath(
            ProjectName + QStringLiteral("_2016.damqt")
        );

    const bool optionsExist =
        QFileInfo::exists(sourceOptions);

    if (optionsExist &&
        !copyReplacing(sourceOptions, targetOptions)) {

        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Cannot copy options file %1 to %2.")
                .arg(sourceOptions, targetOptions)
        );
        return;
    }

    defineRanges();

    atomicDensitiesPage_->setEnabled(true);
    setPostDamPagesEnabled(optionsExist);
    orbitalsPage_->setEnabled(true);
}
    
/**************************************************************************************************/
/********************** INTERFACES TO OTHER PROGRAMS FOR DATA IMPORT  *****************************/
/**************************************************************************************************/

/* Reads data from a GAUSSIAN fchk file */
void MainWindow::readFchk()
{
    if (ImportFolder.at(ImportFolder.length()-1) != '/')
        ImportFolder.append('/');
    QString DirNombreImport = ImportFolder+ImportFile;
//    QFile file(DirNombreImport);
    QDir path(ProjectFolder);
    QString filepath=ProjectFolder + FileWithoutPath(DirNombreImport); // + ".fchk";
    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;
        msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            createDir(ProjectFolder);
            statusBar()->showMessage(tr("Project succesfully created"), 2000);
            setPostDamPagesEnabled(false);
        }else{
            return;
        }
    }
    QString processname = "GAUSS_interface.exe";
    QString execName = get_execName(processname, QString("interfaces_400"));
    if (execName.isEmpty())
        return;

    ImportRequest request;

    request.importFilePath = DirNombreImport;
    request.projectFolder = ProjectFolder;
    request.projectName = ProjectName;
    request.executablePath = execName;
    request.isWindows = iswindows;

    fchkImporter_->start(request);

}

void MainWindow::create_damproj(int exitCode, QProcess::ExitStatus exitStatus){
    if(exitStatus == QProcess::NormalExit && exitCode == 0){
        QString damprojFile = ProjectFolder+ProjectName+".damproj";
        if (!QFile::exists(damprojFile))  {
            loadDefault(1);
            saveOptions(damprojFile);
        }
    }
}

/* Reads data from an MOLEKEL .mkl  file */
void MainWindow::readMOLEKEL()
{
    QDir path(ProjectFolder);

    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;
        msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            createDir(ProjectFolder);
            statusBar()->showMessage(tr("Project succesfully created"), 2000);
            setPostDamPagesEnabled(false);
        }else{
            return;
        }
    }
    QString DirNombreArchivo = ProjectFolder+ProjectName+".damproj";
    existsinp(DirNombreArchivo,1,true);
    QStringList Parameters;
    if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');

//    Parameters << QFileInfo(ImportFile).completeBaseName() << ImportFolder  << ProjectFolder << ProjectName  ;
//    QString strprocess;

    QStringList arguments;

    arguments
            << QFileInfo(ImportFile).completeBaseName()
            << ImportFolder
            << ProjectFolder
            << ProjectName;

    QString processname = "MOLEKEL_interface.exe";

    QString execName = get_execName(processname, QString("interfaces_400"));
    if (execName.isEmpty())
        return;

    SinglePassImportRequest request;
    request.executablePath = execName;
    request.arguments = arguments;

    request.outputFilePath =
        QDir(ProjectFolder).filePath(
            ProjectName
            + QStringLiteral("-MOLEKEL_interface.out")
        );

    singlePassImporter_->start(request);
}

/* Reads data from a MOLPRO out file */
void MainWindow::readMolpro()
{
    if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');
    QString DirNombreImport = ImportFolder+ImportFile;
    QString suffix=Extension(DirNombreImport);
    QFile file(DirNombreImport);
    QDir path(ProjectFolder);
    QString filepath=ProjectFolder + FileWithoutPath(DirNombreImport);
    QStringList Parameters;
    if (suffix=="out"){
        if (!path.exists(ProjectFolder)) {
            QMessageBox msgBox;
            msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
            msgBox.setButtonText(QMessageBox::No, tr("No"));
            msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
            msgBox.setIcon(QMessageBox::Warning);
            const int ret = msgBox.exec();
            if (ret == QMessageBox::Yes){
                createDir(ProjectFolder);
                statusBar()->showMessage(tr("Project succesfully created"), 2000);
                setPostDamPagesEnabled(false);
            }else{
                return;
            }
        }
        const QString projectFile = ProjectFolder + ProjectName + ".damproj";

        existsinp(projectFile, 1, true);

        file.copy(DirNombreImport,filepath);
        int ierror=file.error();
        while (ierror!=0)
        {
            ierror=file.error();
        }
        const QString processname = "MOLPRO_out_interface.exe";
        const QString execName = get_execName(processname, QString("interfaces_400"));
        if (execName.isEmpty())
            return;

        MolproImportRequest request;

        request.importFilePath = DirNombreImport;
        request.projectFolder = ProjectFolder;
        request.projectName = ProjectName;
        request.executablePath = execName;

        molproImporter_->start(request);

        return;

    }
    else if(suffix=="xml"){
        readMolproXml(ImportFile, ImportFolder);
    }
}

/* Reads data from a MOPAC aux file */
void MainWindow::readMopac()
{
    if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');
    QString DirNombreImport = ImportFolder+ImportFile;
//    QFile file(DirNombreImport);
    QDir path(ProjectFolder);
    QString filepath=ProjectFolder + FileWithoutPath(DirNombreImport);
    QStringList Parameters;
    QString strprocess;
    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;
        msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            createDir(ProjectFolder);
            statusBar()->showMessage(tr("Project succesfully created"), 2000);
            setPostDamPagesEnabled(false);
        }else{
            return;
        }
    }
    QString DirNombreArchivo = ProjectFolder+ProjectName+".damproj";
    existsinp(DirNombreArchivo,1,true);
    if (ImportFolder.at(ImportFolder.length()-1) != '/') ImportFolder.append('/');

    QStringList arguments;

    arguments
            << QFileInfo(ImportFile).completeBaseName()
            << ImportFolder
            << ProjectFolder
            << ProjectName;

//    Parameters << QFileInfo(ImportFile).completeBaseName() << ImportFolder  << ProjectFolder << ProjectName  ;
    QString processname = "Mopac_aux_interface.exe";
    QString execName = get_execName(processname, QString("interfaces_400"));
    if (execName.isEmpty())
        return;

    SinglePassImportRequest request;
    request.executablePath = execName;
    request.arguments = arguments;

    request.outputFilePath =
        QDir(ProjectFolder).filePath(
            ProjectName
            + QStringLiteral("-MOPAC_aux_interface.out")
        );

    request.postProcess = [this]()
    {
        lzdo = true;
        lvalence = true;
    };

    singlePassImporter_->start(request);
}

/* Reads data from a TURBOMOLE coord, mos and basis  files */
void MainWindow::readTurbom()
{
    QDir path(ProjectFolder);

    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;
        msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            createDir(ProjectFolder);
            statusBar()->showMessage(tr("Project succesfully created"), 2000);
            setPostDamPagesEnabled(false);
        }else{
                return;
        }
    }
    QString DirNombreArchivo = ProjectFolder+ProjectName+".damproj";
    existsinp(DirNombreArchivo,1,true);

    if (ImportFolder.at(ImportFolder.length() - 1) != '/')
        ImportFolder.append('/');

    QStringList arguments;

    arguments
        << QFileInfo(ImportFile).completeBaseName()
        << ImportFolder
        << ProjectFolder
        << ProjectName;

    const QString processName = "TURBOMOLE_interface.exe";

    const QString execName =
        get_execName(processName, QString("interfaces_400"));

    if (execName.isEmpty())
        return;

    SinglePassImportRequest request;
    request.executablePath = execName;
    request.arguments = arguments;

    request.outputFilePath =
        QDir(ProjectFolder).filePath(
            ProjectName
            + QStringLiteral("-TURBOMOLE_interface.out")
        );

    singlePassImporter_->start(request);

}

/* Reads data from a NWChem output file (IMPORTANT! extension must be nwcout) */
void MainWindow::readNWChem()
{
    QDir path(ProjectFolder);

    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;
        msgBox.setInformativeText(QString(tr("Project %1 not found")).arg(ProjectFolder)+"\n"+tr("Do you wish to create?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
        msgBox.setIcon(QMessageBox::Warning);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes){
            createDir(ProjectFolder);
            statusBar()->showMessage(tr("Project succesfully created"), 2000);
            setPostDamPagesEnabled(false);
        }else{
                return;
        }
    }
    QString DirNombreArchivo = ProjectFolder+ProjectName+".damproj";
    existsinp(DirNombreArchivo,1,true);

    QStringList arguments;

    if (ImportFolder.at(ImportFolder.length() - 1) != '/')
        ImportFolder.append('/');

    arguments
        << QFileInfo(ImportFile).completeBaseName()
        << ImportFolder
        << ProjectFolder
        << ProjectName;

    const QString processName = "NWChem_interface.exe";
    const QString execName =
        get_execName(processName, QString("interfaces_400"));

    if (execName.isEmpty())
        return;

    SinglePassImportRequest request;
    request.executablePath = execName;
    request.arguments = arguments;

    request.outputFilePath =
        QDir(ProjectFolder).filePath(
            ProjectName
            + QStringLiteral("-NWChem_interface.out")
        );

    singlePassImporter_->start(request);

}

/**************************************************************************************************/
/**********************  FUNCTIONS FOR READING AND WRITING OPTIONS IN PROJECT FILE  ***************/
/**************************************************************************************************/


/*    LOADS DEFAULT VALUES in the widgets of the mainmenu        */
void MainWindow::loadDefault(int all)
{
    if (all==0){      
        projectPage_->loadDefault();    // projectPage_: Project
        set_natom(0);
    }
    if (!projectPage_->projectName().isEmpty()){
        densityPage_->setOutputPrefix(projectPage_->projectName());
        potentialPage_->setOutputPrefix(projectPage_->projectName());
        sigmaHolePage_->setOutputPrefix(projectPage_->projectName());
        topographyPage_->setOutputPrefix(projectPage_->projectName());
        hfForcesPage_->setOutputPrefix(projectPage_->projectName());
        fieldLinesPage_->setOutputPrefix(projectPage_->projectName());
        densityGradientPage_->setOutputPrefix(projectPage_->projectName());
        radialFactorsPage_->setOutputPrefix(projectPage_->projectName());
        orientedMultipolesPage_->setOutputPrefix(projectPage_->projectName());
        orbitalsPage_->setOutputPrefix(projectPage_->projectName());
        zjDensityPage_->setOutputPrefix(projectPage_->projectName());
    }
    else{
        densityPage_->setOutputPrefix("");
        potentialPage_->setOutputPrefix("");
        sigmaHolePage_->setOutputPrefix("");
        topographyPage_->setOutputPrefix("");
        hfForcesPage_->setOutputPrefix("");
        fieldLinesPage_->setOutputPrefix("");
        densityGradientPage_->setOutputPrefix("");
        radialFactorsPage_->setOutputPrefix("");
        orientedMultipolesPage_->setOutputPrefix("");
        orbitalsPage_->setOutputPrefix("");
        zjDensityPage_->setOutputPrefix("");
    }
    atomicDensitiesPage_->loadDefault();
    densityPage_->loadDefault();
    potentialPage_->loadDefault();
    sigmaHolePage_->loadDefault();
    topographyPage_->loadDefault();
    hfForcesPage_->loadDefault();
    fieldLinesPage_->loadDefault();
    densityGradientPage_->loadDefault();
    radialFactorsPage_->loadDefault();
    orientedMultipolesPage_->loadDefault();
    orientedMultipolesPage_->setNumAtoms(get_natom());
    orbitalsPage_->loadDefault();
    zjExpansionPage_->loadDefault();
    zjDensityPage_->loadDefault();
    zjDensityPage_->setKmax(zjExpansionPage_->kMax());
    zjDensityPage_->setLmax(zjExpansionPage_->lMax());

//      End of defaults loading

    updateMpiControls();

    SetCurrentFile("",true,false);
}


/*    READS OPTIONS FROM A PROJECT FILE (*.damproj)                */

void MainWindow::readOptions(const QString &fullFileName)
{
    QFile files(fullFileName);

    if (!files.isOpen()){
        if (!files.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("readOptions"),tr("File %1 cannot be read")
                .arg(fullFileName)+QString(":\n%1.").arg(files.errorString()));
            return;
        }
    }
    string file = toString(fullFileName);

    string v = CIniFile::GetValue("ImportFolder","PROJECTSECT",file);
    ImportFolder = QString(v.c_str());
    if (ImportFolder.isEmpty()) ImportFolder = Path(fullFileName);

    projectPage_->readFromFile(file, ImportFolder); //    projectPage_: Project
    initializeNames();

    atomicDensitiesPage_->readFromFile(file);    //    page_atdens: Atomic densities

    densityPage_->readFromFile(file);            //    densityPage_: Density

    potentialPage_->readFromFile(file);          //    potentialPage_: Electrostatic potential

    orbitalsPage_->readFromFile(file);           //    orbitalsPage_: Molecular orbitals

    topographyPage_->readFromFile(file);         //    topographyPage_: Topography

    sigmaHolePage_->readFromFile(file);          //    sigmaHolePage_: MESP sigma hole

    fieldLinesPage_->readFromFile(file);         //    fieldLinesPage_: Electric field

    densityGradientPage_->readFromFile(file);    //    densityGradientPage_: Density gradient

    hfForcesPage_->readFromFile(file);           //    hfForcesPage_: Hellmann-Feynman forces

    radialFactorsPage_->readFromFile(file);      //    radialFactorsPage_: Radial factors

    orientedMultipolesPage_->readFromFile(file); //    orientedMultipolesPage_: Oriented multipoles

    zjExpansionPage_->readFromFile(file);        //    zjExpansionPage_: Zernike-Jacobi expansions

    zjDensityPage_->readFromFile(file);          //    zjDensityPage_: Zernike-Jacobi density
            
//      End of options read

    updateMpiControls();

    atomicDensitiesPage_->setEnabled(true);
    orbitalsPage_->setEnabled(true);
    setPostDamPagesEnabled(true);

    files.close();
    SetCurrentFile("",false,false);
}

void MainWindow::initializeNames()
{
    //    Default names are project name. Will be overwritten below if an alternative name has been given
    densityPage_->setOutputPrefix(projectPage_->projectName());
    hfForcesPage_->setOutputPrefix(projectPage_->projectName());
    fieldLinesPage_->setOutputPrefix(projectPage_->projectName());
    potentialPage_->setOutputPrefix(projectPage_->projectName());
    topographyPage_->setOutputPrefix(projectPage_->projectName());
    radialFactorsPage_->setOutputPrefix(projectPage_->projectName());
    orientedMultipolesPage_->setOutputPrefix(projectPage_->projectName());
    orbitalsPage_->setOutputPrefix(projectPage_->projectName());

    QString sgbsfile =      ProjectFolder+"/"+projectPage_->projectName()+".sgbs";
    QString sgbsgzfile =    ProjectFolder+"/"+projectPage_->projectName()+".sgbs.gz";
    QString sgbsdenfile =   ProjectFolder+"/"+projectPage_->projectName()+".sgbsden";
    QString sgbsdengzfile = ProjectFolder+"/"+projectPage_->projectName()+".sgbsden.gz";
    QString ggbsfile =      ProjectFolder+"/"+projectPage_->projectName()+".ggbs";

    if (QFile::exists(sgbsfile) || QFile::exists(sgbsgzfile)
            || QFile::exists(sgbsdenfile) || QFile::exists(sgbsdengzfile) ){
        lslater = true;
        potentialPage_->setExactPotentialVisible(false);
    }
    else{
        lslater = false;
        potentialPage_->setExactPotentialVisible(true);
    }
    atomicDensitiesPage_->setlslater(lslater);
    potentialPage_->setExactPotential(false);
    if (lslater){
        QString fileaux = FileWithoutExt(projectPage_->importFile());
        if (Extension(fileaux) == "sgbs" || Extension(fileaux) == "sgbsden" ) fileaux = FileWithoutExt(fileaux);
        QString sxyzfilename = ProjectFolder+"/"+ProjectName+".sxyz";
        if (!(QFile::exists(sxyzfilename))){
            execsgbs2sxyz(sxyzfilename);
        }
        set_natom(read_natom(sxyzfilename));
    }
    else{
        set_natom(read_natom(ggbsfile));
    }
}

// Reads the content of a section in options .damproj file
QByteArray MainWindow::ReadSectionOptions(const char *SectionName, QFile *FileName)
{
    QByteArray buff, line;
    bool lreadend = false;    // True when reading section ended
    buff.append("&OPTIONS\n");
    while(!(*FileName).atEnd() && !lreadend){
        line = (*FileName).readLine(50);
        if( line.contains(SectionName) ) {
            while(!(*FileName).atEnd()){
                line = (*FileName).readLine(50);
                if( line.contains("[") ) {
                    lreadend = true;
                    break;
                }
                buff.append(line);
            }
        }
    }
    buff.append("&END\n");
    return buff;
}

//    Saves options in a project file (*.damproj)
void MainWindow::saveOptions(const QString &fullFileName)
{
    QString filezdo = ProjectFolder + "zdo";
//    if (QFileInfo(filezdo).exists()){
    if (QFileInfo::exists(filezdo)){
        lzdo = true;
    }
    else{
        lzdo = false;
    }
    QString filevalence = ProjectFolder + "valence";
//    if (QFileInfo(filevalence).exists()){
    if (QFileInfo::exists(filevalence)){
        lvalence = true;
    }
    else{
        lvalence = false;
    }
    string file = toString(fullFileName);
    bool *printwarns = new bool;

    QString *warns = new QString(tr("Warning: failed saving the following options") + ":\n");
    QFile files(fullFileName);
    if (!files.isOpen()){
        files.open(QFile::Append | QFile::WriteOnly);
    }
    *printwarns = false;

    saveOptionsProject(file, printwarns, warns);
    saveOptionsDam(file, printwarns, warns);
    saveOptionsDamden(file, printwarns, warns);
    saveOptionsDampot(file, printwarns, warns);
    saveOptionsDamforces(file, printwarns, warns);
    saveOptionsDamfield(file, printwarns, warns);
    saveOptionsDamfrad(file, printwarns, warns);
    saveOptionsDammultrot(file, printwarns, warns);
    saveOptionsOrbitals(file, printwarns, warns);
    saveOptionsTopography(file, printwarns, warns);
    saveOptionsZJExpansion(file, printwarns, warns);
    saveOptionsZJDensity(file, printwarns, warns);
    saveOptionsDamdenGrad(file, printwarns, warns);
    saveOptionsSGhole(file, printwarns, warns);

    if (*printwarns) {
        QMessageBox::warning(
            this,
            tr("Error saving options"),
            *warns
        );
    }

    files.close();
//    SetCurrentFile(fullFileName,true,false);
}

//    Saves options only clase 0: Project
void MainWindow::saveOptionsProject(string file, bool *printwarns, QString *warns)
{
    projectPage_->writeToFile(
        file,
        iswindows,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 1: Project || G-DAM || DAM
void MainWindow::saveOptionsDam(string file, bool *printwarns, QString *warns){
    atomicDensitiesPage_->writeToFile(
        file,
        iswindows,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 2: Project || DAMDEN

void MainWindow::saveOptionsDamden(std::string file, bool* printwarns, QString* warns)
{
    densityPage_->writeToFile(
        file,
        printwarns,
        warns
    );
}

//    Saves options clase 0 || clase 3: Project || DAMPOT
void MainWindow::saveOptionsDampot(string file, bool *printwarns, QString *warns)
{
    potentialPage_->writeToFile(
        file,
        printwarns,
        warns
    );
}

//    Saves options clase 0 || clase 8: Project || DAMORB
void MainWindow::saveOptionsOrbitals(string file, bool *printwarns, QString *warns)
{
    orbitalsPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 9: Project || DAMTOPO
void MainWindow::saveOptionsTopography(string file, bool *printwarns, QString *warns)
{
    topographyPage_->writeToFile(
        file,
        printwarns,
        warns);
}


//    Saves options clase 0 || clase 13: Project || DAMSGHOLE
void MainWindow::saveOptionsSGhole(string file, bool *printwarns, QString *warns)
{
    sigmaHolePage_->writeToFile(
        file,
        printwarns,
        warns);
}


//    Saves options clase 0 || clase 5: Project || DAMFIELD
void MainWindow::saveOptionsDamfield(string file, bool *printwarns, QString *warns)
{
    fieldLinesPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 12: Project || DAMDENGRAD

void MainWindow::saveOptionsDamdenGrad(string file, bool *printwarns, QString *warns)
{
    densityGradientPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 4: Project || DAMFORCES
void MainWindow::saveOptionsDamforces(string file, bool *printwarns, QString *warns)
{
    hfForcesPage_->writeToFile(
        file,
        printwarns,
        warns);
}


//    Saves options clase 0 || clase 6: Project || DAMFRAD
void MainWindow::saveOptionsDamfrad(string file, bool *printwarns, QString *warns)
{
    radialFactorsPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 7: Project || DAMMULTROT
void MainWindow::saveOptionsDammultrot(string file, bool *printwarns, QString *warns)
{
    orientedMultipolesPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 10: Project || DAMZJ
void MainWindow::saveOptionsZJExpansion(string file, bool *printwarns, QString *warns)
{
    zjExpansionPage_->writeToFile(
        file,
        printwarns,
        warns);
}

//    Saves options clase 0 || clase 11: Project || DAMDENZJ
void MainWindow::saveOptionsZJDensity(string file, bool *printwarns, QString *warns)
{
    zjDensityPage_->writeToFile(
        file,
        printwarns,
        warns);
}

/*******************************************************************************************************/
/******************************  PROGRAMS FOR DAM ANALYSIS *********************************************/
/*******************************************************************************************************/


/***********************************************************************/
/*  atomicDensitiesPage_: ATOMIC DENSITIES                                      */
/***********************************************************************/


void MainWindow::CHKatdensinput_changed(int state)     
{
    if (state == 0){
        atomicDensitiesPage_->setMpiCheckEnabled(true);
        atomicDensitiesPage_->setMpiVisible(true);
        if (atomicDensitiesPage_->isMpiChecked()){
            atomicDensitiesPage_->setMpiEnabled(true);
        }
        else{
            atomicDensitiesPage_->setMpiEnabled(false);
        }
    }
    else{
        atomicDensitiesPage_->setMpiVisible(false);
        atomicDensitiesPage_->setMpiCheckEnabled(false);
        atomicDensitiesPage_->setMpiEnabled(false);
    }
}

//    Executes external program DAM (Partition of molecular density into atomic densities)
void MainWindow::execDam(){
    atomicDensitiesPage_->setIsWindows(iswindows);
    atomicDensitiesPage_->setProjectData(ProjectFolder,ProjectName);
    atomicDensitiesPage_->setMpiSettings(mpicommand, mpiflags);
    atomicDensitiesPage_->setlzdo(lzdo);
    atomicDensitiesPage_->setlvalence(lvalence);
    atomicDensitiesPage_->setlslater(lslater);

    atomicDensitiesPage_->execDam();
    defineRanges();
}

void MainWindow::atdenslmaxexp_changed()
{
    int valor = atomicDensitiesPage_->lmaxExpansion();
    int valor1 = densityPage_->lMax();
    int valor2 = radialFactorsPage_->lMax();
    int valor3 = potentialPage_->lMax();
    int valor4 = fieldLinesPage_->lMax();

    if (valor < valor1){
        densityPage_->setLmax(valor);
    }
    densityPage_->setLmaxTop(valor);
    if (valor < valor2) radialFactorsPage_->setlTabulation(valor);
    radialFactorsPage_->setLmax(valor);
    if (valor < valor3){
        potentialPage_->setLmax(valor);
    }
    potentialPage_->setLmaxTop(valor);
    if (valor < valor4){
        fieldLinesPage_->setLmax(valor);
    }
    fieldLinesPage_->setLmaxTop(valor);

    densityPage_->setLminTop(0);

    orientedMultipolesPage_->setLmaxTop(atomicDensitiesPage_->lmaxExpansion());
    orientedMultipolesPage_->setLminTop(atomicDensitiesPage_->lmaxExpansion());

    atomicDensitiesPage_->setTopLmaxDisplayed(valor);
    if (valor < atomicDensitiesPage_->lmaxDisplayed()) atomicDensitiesPage_->setLmaxDisplayed(valor);
}

/***************************************************************************/
/*  densityPage_: DENSITY                                                      */
/***************************************************************************/

//    Executes external program DAMDEN  (Computes molecular density or deformations from the atomic partition)
void MainWindow::execDamden()
{
    densityPage_->setIsWindows(iswindows);
    densityPage_->setProjectData(ProjectFolder,ProjectName);
    densityPage_->setMpiSettings(mpicommand, mpiflags);
    densityPage_->setNatoms(natom);
    densityPage_->execDamDen();
}

void MainWindow::rename_density_cntfile(){
    QString aux;
    if (densityPage_->isFullDensity()) {
        if (densityPage_->isExactDensity()){
            aux = "_exact";
        }
        else{
            aux = "";
        }
    }
    else{
        aux = "_deform";
    }
    QString path = ProjectFolder;
    QDir directorio(path);
    QStringList filtro;
    densplanecase = densityPage_->getPlaneCase();
    filtro << ProjectName + aux + "*" + planesuffix(densplanecase) + "-d.cnt";
    QStringList archivos = directorio.entryList(filtro, QDir::Files);
    QFile filecnt(ProjectFolder + ProjectName + aux + "-d.cnt");
//qDebug() << "filecnt = " << ProjectFolder + ProjectName + aux + "-d.cnt";
//qDebug() << "filecnt.exists: " << filecnt.exists();
    if (filecnt.exists() && planesuffix(densplanecase) != ""){
        QFile fileold(ProjectFolder + ProjectName + aux + planesuffix(densplanecase) + "-d.cnt");
        if (fileold.exists())
            fileold.remove();
        filecnt.rename(ProjectFolder + ProjectName + aux + planesuffix(densplanecase) + "-d.cnt");
    }
    QString planefile = ProjectName + aux + planesuffix(densplanecase) + "-d.plane";
    if (QFile::exists(ProjectFolder + planefile)){
        QString planebase = planefile;
        planebase.replace(".plane", "");
        for (const QString& archivo : archivos){
            QString fileauxname = archivo;
            fileauxname.replace(".cnt", "");
            if (fileauxname == planebase) continue;
            QString newfilename = fileauxname + ".plane";
            if (QFile::exists(ProjectFolder + newfilename))
                QFile::remove(ProjectFolder + newfilename);
            if (!QFile::copy(ProjectFolder + planefile, ProjectFolder + newfilename)){
                qDebug() << "Error copying " << ProjectFolder + planefile << " to " << ProjectFolder + newfilename;
                continue;
            }
        }
    } else {
        qDebug() << "The file "<< ProjectFolder + planefile << " DOES NOT exist";

    }
}


/***************************************************************************/
/*  potentialPage_: ELECTROSTATIC POTENTIAL                                     */
/***************************************************************************/

//    Executes external program DAMPOT  (Computes molecular electrostatic potential from the atomic partition)
void MainWindow::execDampot()
{
    potentialPage_->setIsWindows(iswindows);
    potentialPage_->setIsValence(lvalence);
    potentialPage_->setProjectData(ProjectFolder,ProjectName);
    potentialPage_->setMpiSettings(mpicommand, mpiflags);
    potentialPage_->execDamPot();
}

void MainWindow::rename_pot_cntfile(){
    QString aux;
    if (potentialPage_->isExactPotential()){
        aux = "_exact";
    }
    else{
        aux = "";
    }
    QFile filecnt(ProjectFolder + ProjectName + aux + "-v.cnt");
    if (filecnt.exists() && planesuffix(potplanecase) != ""){
        QFile fileold(ProjectFolder + ProjectName + aux + planesuffix(potplanecase) + "-v.cnt");
        if (fileold.exists())
            fileold.remove();
        filecnt.rename(ProjectFolder + ProjectName + aux + planesuffix(potplanecase) + "-v.cnt");
    }
}


/***************************************************************************/
/*  orbitalsPage_: MOLECULAR ORBITALS                                            */
/***************************************************************************/


//    Executes external program DAMORB  (Computes molecular orbitals)
void MainWindow::execDamorb()
{
    orbitalsPage_->setIsWindows(iswindows);
    orbitalsPage_->setProjectData(ProjectFolder,ProjectName);
    orbitalsPage_->setMpiSettings(mpicommand, mpiflags);
    orbitalsPage_->execDamOrb();
}

/***************************************************************************/
/*  topographyPage_: TOPOGRAPHY                                                  */
/***************************************************************************/

//    Executes external program DAMTOPOGRAPHER  (Carries out topography analysis)
void MainWindow::execDamTopography()
{
    topographyPage_->setIsWindows(iswindows);
    topographyPage_->setProjectData(ProjectFolder,ProjectName);
    topographyPage_->setMpiSettings(mpicommand, mpiflags);
    topographyPage_->execDamTopography();
}

/***************************************************************************/
/*               sigmaHolePage_: MESP SIGMA HOLE GENERATION                   */
/***************************************************************************/

//    Executes external program DAMDENZJ  (Computes molecular density or deformations from the Zernike or Jacobi expansion)
void MainWindow::execDamSGhole()
{
    sigmaHolePage_->setIsWindows(iswindows);
    sigmaHolePage_->setIsValence(lvalence);
    sigmaHolePage_->setProjectData(ProjectFolder,ProjectName);
    sigmaHolePage_->setMpiSettings(mpicommand, mpiflags);
    sigmaHolePage_->execDamSGhole();
}

/***************************************************************************/
/*  fieldLinesPage_: ELECTRIC FIELD                                            */
/***************************************************************************/


//    Executes external program DAMFIELD  (Computes electric field lines from the atomic partition)
void MainWindow::execDamfield()
{
    fieldLinesPage_->setIsWindows(iswindows);
    fieldLinesPage_->setIsValence(lvalence);
    fieldLinesPage_->setProjectData(ProjectFolder,ProjectName);
    fieldLinesPage_->setMpiSettings(mpicommand, mpiflags);
    fieldLinesPage_->execDamField();
}


/***************************************************************************/
/*  densityGradientPage_: DENSITY GRADIENT                                        */
/***************************************************************************/

//    Executes external program DAMDENGRAD  (Computes density gradient lines from the atomic partition)
void MainWindow::execDamdengrad()
{
    densityGradientPage_->setIsWindows(iswindows);
    densityGradientPage_->setIsValence(lvalence);
    densityGradientPage_->setProjectData(ProjectFolder,ProjectName);
    densityGradientPage_->setMpiSettings(mpicommand, mpiflags);
    densityGradientPage_->execDamDenGrad();

}

/***************************************************************************/
/*  hfForcesPage_: HELLMANN-FEYNMAN FORCES                                 */
/***************************************************************************/

//    Executes external program DAMFORCES  (Computes Hellmann-Feynman forces on nuclei from the atomic partition)
void MainWindow::execDamforces()
{
    hfForcesPage_->setIsWindows(iswindows);
    hfForcesPage_->setIsValence(lvalence);
    hfForcesPage_->setProjectData(ProjectFolder,ProjectName);
    hfForcesPage_->execDamForces();
}


/****************************************************************************/
/*  radialFactorsPage_: RADIAL FACTORS                                               */
/****************************************************************************/


//    Executes external program DAMFRAD  (Computes radial factors of the atomic partition)
void MainWindow::execDamfrad()
{
    radialFactorsPage_->setIsWindows(iswindows);
    radialFactorsPage_->setProjectData(ProjectFolder,ProjectName);
    radialFactorsPage_->execDamFrad();
}


/****************************************************************************/
/*  orientedMultipolesPage_: ORIENTED MULTIPOLES                                       */
/****************************************************************************/

//    Executes external program DAMMULTROT  (Orients multipoles in a frame with the Z axis orthogonal to the plane defined by three selected atoms)
void MainWindow::execDammultrot()
{
    orientedMultipolesPage_->setIsWindows(iswindows);
    orientedMultipolesPage_->setProjectData(ProjectFolder,ProjectName);
    orientedMultipolesPage_->execDamMultRot();
}


/***************************************************************************/
/*  page_ZJdens: ZERNIKE 3D-JACOBI EXPANSIONS                              */
/***************************************************************************/

//    Executes external program DAM (Partition of molecular density into atomic densities)
void MainWindow::execDamZJ(){
    defineRanges();
    zjExpansionPage_->setIsWindows(iswindows);
    zjExpansionPage_->setSlater(lslater);
    zjExpansionPage_->setProjectData(ProjectFolder,ProjectName);
    zjExpansionPage_->setMpiSettings(mpicommand, mpiflags);
    zjExpansionPage_->execDamZJ();
}


/***************************************************************************/
/*  page_ZJtab: DENSITY TABULATION FROM ZERNIKE 3D-JACOBI EXPANSIONS       */
/***************************************************************************/

//    Executes external program DAMDENZJ  (Computes molecular density or deformations from the Zernike or Jacobi expansion)
void MainWindow::execDamdenZJ()
{
    zjDensityPage_->setIsWindows(iswindows);
    zjDensityPage_->setProjectData(ProjectFolder,ProjectName);
    zjDensityPage_->setMpiSettings(mpicommand, mpiflags);
    zjDensityPage_->execDamDenZJ();
}


//    Executes external program sgbs2sxyz  (extracts the number of centers and geometry from .sgbs file to file .sxyz)

void MainWindow::execsgbs2sxyz(const QString& sxyzFileName)
{
    QString inputFile =
        QString(sxyzFileName).replace(
            QStringLiteral(".sxyz"),
            QStringLiteral(".tmpinp")
        );

    QFile file(inputFile);

    if (!file.open(QFile::WriteOnly |
                   QFile::Text |
                   QFile::Truncate)) {
        QMessageBox::warning(
            this,
            tr("sgbs2sxyz"),
            tr("Cannot create input file %1:\n%2")
                .arg(inputFile, file.errorString())
        );

        return;
    }

    QTextStream stream(&file);

    stream << '"'
           << QFileInfo(sxyzFileName).path()
              + QDir::separator()
              + QFileInfo(sxyzFileName).completeBaseName()
           << '"'
           << Qt::endl;

    file.close();

    const QString executable =
        get_execName(
            QStringLiteral("sgbs2sxyz.exe"),
            QStringLiteral("DAM_400")
        );

    if (executable.isEmpty())
        return;

    QString errorFile = inputFile;
    errorFile.replace(
        QStringLiteral(".tmpinp"),
        QStringLiteral(".err")
    );

    QProcess process;

    process.setStandardInputFile(inputFile);
    process.setStandardOutputFile(
        errorFile,
        QIODevice::Truncate
    );
    process.setStandardErrorFile(
        errorFile,
        QIODevice::Append
    );

    process.start(executable, QStringList());

    if (!process.waitForStarted(3000)) {
        QMessageBox::warning(
            this,
            tr("sgbs2sxyz"),
            tr("The process could not be started:\n%1")
                .arg(process.errorString())
        );

        QFile::remove(inputFile);
        return;
    }

    if (!process.waitForFinished(20000)) {
        process.kill();
        process.waitForFinished();

        QMessageBox::warning(
            this,
            tr("sgbs2sxyz"),
            tr("The process did not finish within the expected time.")
        );

        QFile::remove(inputFile);
        return;
    }

    if (process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        QMessageBox::warning(
            this,
            tr("sgbs2sxyz"),
            tr("The process failed. See:\n%1")
                .arg(errorFile)
        );

        QFile::remove(inputFile);
        return;
    }

    QFile::remove(inputFile);

    const QString importedRoot =
        QDir(ProjectFolder).filePath(
            QFileInfo(ImportFile).completeBaseName()
        );

    if (importedRoot + QStringLiteral(".sxyz")
        != sxyzFileName) {

        QFile::rename(
            importedRoot + QStringLiteral(".sxyz"),
            sxyzFileName
        );

        QFile::rename(
            importedRoot + QStringLiteral(".sgbs2sxyz"),
            QDir(ProjectFolder).filePath(
                ProjectName
                + QStringLiteral(".sgbs2sxyz")
            )
        );
    }
}



/**************************************************************************************************/
/********** FUNCTIONS FOR EXTERNAL PACKAGES DIALOG                      ***************************/
/**************************************************************************************************/

void MainWindow::external_package(){
    Externals *external = new Externals(this);
    connect(external, SIGNAL(computing(QString)), this, SLOT(update_statusbar(QString)));
    connect(external, SIGNAL(updatetextedit(QString)), this, SLOT(update_textedit(QString)));
}

/**************************************************************************************************/
/***************************************  ANCILLARY FUNCTIONS *************************************/
/**************************************************************************************************/


//    Creates a folder. Returns whether it has succeded or not
bool MainWindow::createDir(QString &fullPathName)
{
    QDir dir(fullPathName);
    if(!dir.exists(fullPathName)){
        dir.mkpath(fullPathName);
        return true;
    }else{
        return false;
    }
}

//    Sets plot ranges
void MainWindow::defineRanges()
{
    QVector<double> x;
    QVector<double> y;
    QVector<double> z;
    QVector<int> ncarga;
    int nat;
    double min;
    double max;
    QString qv;
    readGeometry(nat,x,y,z,ncarga);
    if (nat == 0) return;
    set_natom(nat);
    dminmax(x,min,max);
    xmax = qRound(max);
    xmin = qRound(min);
    dminmax(y,min,max);
    ymax = qRound(max);
    ymin = qRound(min);
    dminmax(z,min,max);
    zmax = qRound(max);
    zmin = qRound(min);
    double xyztop;
    QVector<double> vaux;
    vaux << xmax << ymax << zmax << std::abs(xmin) << std::abs(ymin) << std::abs(zmin);
    dmax(vaux,xyztop);
    xmin = -xyztop;
    ymin = -xyztop;
    zmin = -xyztop;
    xmax =  xyztop;
    ymax =  xyztop;
    zmax =  xyztop;

    densityPage_->setUmin(qv.setNum(xmin-5,'g',3));
    densityPage_->setUmax(qv.setNum(xmax+5,'g',3));
    densityPage_->setVmin(qv.setNum(zmin-5,'g',3));
    densityPage_->setVmax(qv.setNum(zmax+5,'g',3));
    densityPage_->setXmin(qv.setNum(xmin-5,'g',3));
    densityPage_->setXmax(qv.setNum(xmax+5,'g',3));
    densityPage_->setYmin(qv.setNum(ymin-5,'g',3));
    densityPage_->setYmax(qv.setNum(ymax+5,'g',3));
    densityPage_->setZmin(qv.setNum(zmin-5,'g',3));
    densityPage_->setZmax(qv.setNum(zmax+5,'g',3));

    potentialPage_->setUmin(qv.setNum(2.0*(xmin-5),'g',3));
    potentialPage_->setUmax(qv.setNum(2.0*(xmax+5),'g',3));
    potentialPage_->setVmin(qv.setNum(2.0*(zmin-5),'g',3));
    potentialPage_->setVmax(qv.setNum(2.0*(zmax+5),'g',3));
    potentialPage_->setXmin(qv.setNum(2.0*(xmin-5),'g',3));
    potentialPage_->setXmax(qv.setNum(2.0*(xmax+5),'g',3));
    potentialPage_->setYmin(qv.setNum(2.0*(ymin-5),'g',3));
    potentialPage_->setYmax(qv.setNum(2.0*(ymax+5),'g',3));
    potentialPage_->setZmin(qv.setNum(2.0*(zmin-5),'g',3));
    potentialPage_->setZmax(qv.setNum(2.0*(zmax+5),'g',3));

    orbitalsPage_->setUmin(qv.setNum(2.0*(xmin-5),'g',3));
    orbitalsPage_->setUmax(qv.setNum(2.0*(xmax+5),'g',3));
    orbitalsPage_->setVmin(qv.setNum(2.0*(zmin-5),'g',3));
    orbitalsPage_->setVmax(qv.setNum(2.0*(zmax+5),'g',3));
    orbitalsPage_->setXmin(qv.setNum(2.0*(xmin-5),'g',3));
    orbitalsPage_->setXmax(qv.setNum(2.0*(xmax+5),'g',3));
    orbitalsPage_->setYmin(qv.setNum(2.0*(ymin-5),'g',3));
    orbitalsPage_->setYmax(qv.setNum(2.0*(ymax+5),'g',3));
    orbitalsPage_->setZmin(qv.setNum(2.0*(zmin-5),'g',3));
    orbitalsPage_->setZmax(qv.setNum(2.0*(zmax+5),'g',3));

    fieldLinesPage_->setUmin(qv.setNum(2.0*(xmin-5),'g',3));
    fieldLinesPage_->setUmax(qv.setNum(2.0*(xmax+5),'g',3));
    fieldLinesPage_->setVmin(qv.setNum(2.0*(zmin-5),'g',3));
    fieldLinesPage_->setVmax(qv.setNum(2.0*(zmax+5),'g',3));
    fieldLinesPage_->setXmin(qv.setNum(2.0*(xmin-5),'g',3));
    fieldLinesPage_->setXmax(qv.setNum(2.0*(xmax+5),'g',3));
    fieldLinesPage_->setYmin(qv.setNum(2.0*(ymin-5),'g',3));
    fieldLinesPage_->setYmax(qv.setNum(2.0*(ymax+5),'g',3));
    fieldLinesPage_->setZmin(qv.setNum(2.0*(zmin-5),'g',3));
    fieldLinesPage_->setZmax(qv.setNum(2.0*(zmax+5),'g',3));

    orientedMultipolesPage_->setNumAtoms(get_natom());
}


void MainWindow::disable_pages(){
    atomicDensitiesPage_->setEnabled(false);
    orbitalsPage_->setEnabled(false);
    setPostDamPagesEnabled(false);
}

// Determines the highest value in an array
void MainWindow::dmax(QVector<double> &v,double &max)
{
    max=v[0];
    for (int i=0;i<v.size();i++){
        if (v[i]>max) max=v[i];
    }
}

// Determines the lowest value in an array
void MainWindow::dmin(QVector<double> &v,double &min)
{
    min=v[0];
    for (int i=0;i<v.size();i++){
        if (v[i]<min) min=v[i];
    }
}

// Determines the highest and the lowest values in an array
void MainWindow::dminmax(QVector<double> &v,double &min,double &max)
{
    min=v[0];
    max=v[0];
    for (int i=0;i<v.size();i++){
        if (v[i]<min) min=v[i];
        if (v[i]>max) max=v[i];
    }    
}

//    Checks whether a project file (.damproj) exist or not
bool MainWindow::existsinp(QString fullinputName, int def, bool pregunta)
{
    if (fullinputName.size()==0){
        QMessageBox::warning(this, tr("DAMQT"),tr("Introduce options file name")+" (*.damproj)");
        return false;
    }
    if (pregunta==true){
        if (!QFile::exists(fullinputName))  {
            loadDefault(def);
            saveOptions(fullinputName);
        }else{
            QMessageBox msgBox;
            msgBox.setInformativeText(QString(tr("Options file %1 exists")).arg(fullinputName)
                        +"\n"+tr("Do you want to overwrite?"));
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox:: Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
            msgBox.setButtonText(QMessageBox::No, tr("No"));
            msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel"));
            msgBox.setIcon(QMessageBox::Warning);
            int ret = msgBox.exec();
            if (ret == QMessageBox::Yes){
                saveOptions(fullinputName);
            }else if (ret == QMessageBox::No){
                readOptions(fullinputName);
            }else if (ret == QMessageBox::Cancel){
                return false;
            }
        }
    }else{
        saveOptions(fullinputName);
    }
    return true;
}

//    gets content of variable natom
int MainWindow::get_natom()
{
    return MainWindow::natom;
}

QString MainWindow::planesuffix(int planecase){
    switch (planecase){
    case 1:
        return QString("_XY0");
    case 2:
        return QString("_X0Z");
    case 3:
        return QString("_0YZ");
    case 4:
        return QString("_AB0");
    case 5:
        return QString("_A0C");
    case 6:
        return QString("_0BC");
    case 7:
        return QString("_ABC");
    default:
        return QString("");
    }
}


// Imports an output data file (*.out, *.log)
void MainWindow::importOUT()
{
    const QString initialDir =
        FileDialogUtils::initialDirectory(ProjectFolder);

    QFileDialog fileDialog(
        this,
        tr("Open output file ..."),
        initialDir
    );

    FileDialogUtils::configureLocalFileDialog(fileDialog);

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);

    fileDialog.setNameFilters({
        tr("Output files") + " (*.out *.log)",
        tr("All files") + " (*)"
    });

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QStringList selectedFiles = fileDialog.selectedFiles();

    if (selectedFiles.isEmpty())
        return;

    const QString fileName = selectedFiles.first();

    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(
            this,
            tr("Import output file"),
            tr("File %1 cannot be read:\n%2.")
                .arg(fileName, file.errorString())
        );

        return;
    }

    QTextStream input(&file);

    textEdit->setFont(QFont(QStringLiteral("Courier"), 10));
    textEdit->setPlainText(input.readAll());
}


//    Reads geometry
void MainWindow::readGeometry(int &nats,QVector<double> &x,QVector<double> &y,QVector<double> &z,QVector<int> &ncarga)
{
    rmax = 0.;
    QString suffix;
    if (lslater){
        suffix = ".sxyz";
        QString sxyzfilename = ProjectFolder + FileWithoutExt(ProjectName)+".sxyz";
        if (!(QFile::exists(sxyzfilename))){
            execsgbs2sxyz(sxyzfilename);
        }
        set_natom(read_natom(sxyzfilename));
    }
    else
        suffix = ".ggbs";
    QString filename=ProjectFolder+ProjectName+suffix;
    QFile file(filename);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("readGeometry"),tr("File %1 cannot be read")
                .arg(ProjectName+suffix)+
                QString(":\n%1.").arg(file.errorString()));
        nats = 0;
        return;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    if (line.size()==0)
        nats = 0;
    else
        nats = line.toInt();
    x.resize(nats);
    y.resize(nats);
    z.resize(nats);
    ncarga.resize(nats);
    double q;
    double raux;
    double xC = 0.;
    double yC = 0.;
    double zC = 0.;
    double qC = 0.;
    for (int i=0 ; i<nats ; i++){
        QString line = in.readLine();
        if (line.isEmpty()) continue;
#if QT_VERSION < 0x050E00
        QStringList fields = line.split(' ',QString::SkipEmptyParts);
#else
        QStringList fields = line.split(' ',Qt::SkipEmptyParts);
#endif
        x[i] = fields.takeFirst().toDouble();
        y[i] = fields.takeFirst().toDouble();
        z[i] = fields.takeFirst().toDouble();
        q = fields.takeLast().toDouble();
        ncarga[i] = (int)q;
        xC += q * x[i];
        yC += q * y[i];
        zC += q * z[i];
        qC += q;
        raux = x[i]*x[i] + y[i]*y[i] + z[i]*z[i];
        xC = xC / qC;
        yC = yC / qC;
        zC = zC / qC;
        if (raux > rmax) rmax = raux;
    }
    for (int i = 0 ; i < nats ; i++){
        x[i] = x[i] - xC;
        y[i] = y[i] - yC;
        z[i] = z[i] - zC;
    }
    rmax = sqrt(rmax);
    if (x.size() == 0 || y.size() == 0 || z.size() == 0 ){
        QMessageBox::warning(this, tr("DAMQT"),tr("Error reading geometry in file %1")
                             .arg(ProjectName+suffix));
        return;
    }
}

//    Reads the number of atoms from file fileName
int MainWindow::read_natom(QString fileName)
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


// Computes dlt for a given potential grid resolution
double MainWindow::set_delta(const char * c, double ini, double fin)
{
    double dlt = 1.0;
    if (potentialPage_->isLowResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/64; //2**6+1 (low 3D)
        }
        else{
            dlt=(fin-ini)/128; //2**7+1 (low 2D)
        }
    }
    else if (potentialPage_->isMediumResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/128; //2**7+1 (medium 3D)
        }
        else{
            dlt=(fin-ini)/256; //2**8+1 (medium 2D)
        }
    }
    else if (potentialPage_->isHighResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/256; //2**8+1 (high 3D)
        }
        else{
            dlt=(fin-ini)/512; //2**9+1 (high 2D)
        }
    }
    else if(potentialPage_->isCustomResolution()){
        if (QString(c).compare(QString("dltx")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsX();
        else if (QString(c).compare(QString("dlty")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsY();
        else if (QString(c).compare(QString("dltz")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsZ();
        else if (QString(c).compare(QString("dltu")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsUV();
        else if (QString(c).compare(QString("dltv")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsUV();
    }
    return dlt;
}

// Computes dlt for a given potential grid resolution
double MainWindow::set_deltaorb(const char * c, double ini, double fin)
{
    double dlt = 1.0;
    if (orbitalsPage_->isLowResolution()){
        if (orbitalsPage_->is3DGrid()){
            dlt=(fin-ini)/64; //2**6+1 (low 3D)
        }
        else{
            dlt=(fin-ini)/128; //2**7+1 (low 2D)
        }
    }
    else if (orbitalsPage_->isMediumResolution()){
        if (orbitalsPage_->is3DGrid()){
            dlt=(fin-ini)/128; //2**7+1 (medium 3D)
        }
        else{
            dlt=(fin-ini)/256; //2**8+1 (medium 2D)
        }
    }
    else if (orbitalsPage_->isHighResolution()){
        if (orbitalsPage_->is3DGrid()){
            dlt=(fin-ini)/256; //2**8+1 (high 3D)
        }
        else{
            dlt=(fin-ini)/512; //2**9+1 (high 2D)
        }
    }
    else if(orbitalsPage_->isCustomResolution()){
        if (QString(c).compare(QString("dltx")) == 0)
            dlt=(fin-ini) / orbitalsPage_->pointsX();
        else if (QString(c).compare(QString("dlty")) == 0)
            dlt=(fin-ini) / orbitalsPage_->pointsY();
        else if (QString(c).compare(QString("dltz")) == 0)
            dlt=(fin-ini) / orbitalsPage_->pointsZ();
        else if (QString(c).compare(QString("dltu")) == 0)
            dlt=(fin-ini) / orbitalsPage_->pointsUV();
        else if (QString(c).compare(QString("dltv")) == 0)
            dlt=(fin-ini) / orbitalsPage_->pointsUV();
    }
    return dlt;
}

// Computes dlt for a given potential grid resolution
double MainWindow::set_deltapot(const char * c, double ini, double fin)
{
    double dlt = 1.0;
    if (potentialPage_->isLowResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/64; //2**6+1 (low 3D)
        }
        else{
            dlt=(fin-ini)/128; //2**7+1 (low 2D)
        }
    }
    else if (potentialPage_->isMediumResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/128; //2**7+1 (medium 3D)
        }
        else{
            dlt=(fin-ini)/256; //2**8+1 (medium 2D)
        }
    }
    else if (potentialPage_->isHighResolution()){
        if (potentialPage_->is3DGrid()){
            dlt=(fin-ini)/256; //2**8+1 (high 3D)
        }
        else{
            dlt=(fin-ini)/512; //2**9+1 (high 2D)
        }
    }
    else if(potentialPage_->isCustomResolution()){
        if (QString(c).compare(QString("dltx")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsX();
        else if (QString(c).compare(QString("dlty")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsY();
        else if (QString(c).compare(QString("dltz")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsZ();
        else if (QString(c).compare(QString("dltu")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsUV();
        else if (QString(c).compare(QString("dltv")) == 0)
            dlt=(fin-ini) / potentialPage_->pointsUV();
    }
    return dlt;
}

// Computes dlt for a given Jacobi-Zernike density grid resolution

// Computes dlt for a given potential grid resolution
double MainWindow::set_deltaZJden(const char * c, double ini, double fin)
{
    double dlt = 1.0;
    if (zjDensityPage_->isLowResolution()){
        if (zjDensityPage_->is3DGrid()){
            dlt=(fin-ini)/64; //2**6+1 (low 3D)
        }
        else{
            dlt=(fin-ini)/128; //2**7+1 (low 2D)
        }
    }
    else if (zjDensityPage_->isMediumResolution()){
        if (zjDensityPage_->is3DGrid()){
            dlt=(fin-ini)/128; //2**7+1 (medium 3D)
        }
        else{
            dlt=(fin-ini)/256; //2**8+1 (medium 2D)
        }
    }
    else if (zjDensityPage_->isHighResolution()){
        if (zjDensityPage_->is3DGrid()){
            dlt=(fin-ini)/256; //2**8+1 (high 3D)
        }
        else{
            dlt=(fin-ini)/512; //2**9+1 (high 2D)
        }
    }
    else if(zjDensityPage_->isCustomResolution()){
        if (QString(c).compare(QString("dltx")) == 0)
            dlt=(fin-ini) / zjDensityPage_->pointsX();
        else if (QString(c).compare(QString("dlty")) == 0)
            dlt=(fin-ini) / zjDensityPage_->pointsY();
        else if (QString(c).compare(QString("dltz")) == 0)
            dlt=(fin-ini) / zjDensityPage_->pointsZ();
        else if (QString(c).compare(QString("dltu")) == 0)
            dlt=(fin-ini) / zjDensityPage_->pointsUV();
        else if (QString(c).compare(QString("dltv")) == 0)
            dlt=(fin-ini) / zjDensityPage_->pointsUV();
    }
    return dlt;
}

//    sets variable natom
void MainWindow::set_natom(int i)
{
    MainWindow::natom = i;
}

void MainWindow::start(){
FRMlanguage->close();
}


/*******************************************************************************************************/
/******************************** FILE NAME, PATH AND EXTENSION HANDLING *******************************/
/*******************************************************************************************************/


/* Returns the file extension */
QString MainWindow::Extension(const QString &fullFileName)
{
    return QFileInfo(fullFileName).suffix();
}

/* Returns the file name without any extension */
QString MainWindow::FileWithoutExt(const QString &fullFileName)
{
    return QFileInfo(fullFileName).completeBaseName();
}

/* Returns the file name without path */
QString MainWindow::FileWithoutPath(const QString &fullFileName)
{

    return QFileInfo(fullFileName).fileName();
}

/* Returns the path of a file */
QString MainWindow::Path(const QString &fullFileName)
{
    return QFileInfo(fullFileName).path();
}

void MainWindow::onExternalDamFinished(bool enabled)
{
    setPostDamPagesEnabled(enabled);
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onExternalDamStarted()
{
    setPostDamPagesEnabled(false);
    statusBar()->showMessage(tr("Computing..."));
}
emit
void MainWindow::onExternalProcessFinished()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::onExternalProcessStarted()
{
    statusBar()->showMessage(tr("Computing..."));
}

void MainWindow::setPostDamPagesEnabled(bool enabled)
{
    for (IExecutablePage *page : postDamPages_) {
        if (page) {
            page->setPageEnabled(enabled);
        }
    }
}

void MainWindow::setAllDamPagesEnabled(bool enabled)
{
    setPostDamPagesEnabled(enabled);
    orbitalsPage_->setEnabled(enabled);
}

/*******************************************************************************************************/
/*********************   2D AND 3D ViewerS SLOTS AND FUNCTIONS     *************************************/
/*******************************************************************************************************/

bool MainWindow::end_Viewer2DDialog(){
    if (!plots.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText(tr("2D Viewer: Delete Confirmation"));
        msgBox.setInformativeText(tr("Ending application will delete open 2D viewers. Do you want to exit?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setIcon(QMessageBox::Question);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
            return false;
        for (int i = 0 ; i < plots.length(); i++){
            delete plots.at(i);
        }
        plots.clear();
    }
    update_dockright();
    return true;
}

bool MainWindow::end_Viewer3DDialog(){
    if (!widgets.isEmpty()){
        QMessageBox msgBox;
        msgBox.setText(tr("3D Viewer: Delete Confirmation"));
        msgBox.setInformativeText(tr("Ending application will delete open 3D viewers. Do you want to exit?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
        msgBox.setButtonText(QMessageBox::No, tr("No"));
        msgBox.setIcon(QMessageBox::Question);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
            return false;

        qDeleteAll(widgets.begin(), widgets.end());
        widgets.clear();
    }
    update_dockright();
    return true;
}

void MainWindow::addviewer2D(){
    QString *name = new QString(tr("%1").arg(plotsknt++));
    Viewer2D *viewer = new Viewer2D(name);
    viewer->set_ProjectFolder(ProjectFolder);
    viewer->set_ProjectName(ProjectName);
    connect(viewer, SIGNAL(moveToTop(int)), this,SLOT(moveviewertotop(int)), Qt::UniqueConnection);
    plots.append(viewer);
    plots.last()->set_position(QPoint(75,75)*(widgets.length()+plots.length()-1));
    this->moveviewertotop(plotsknt-1);
    topindex = plots.length()-1;
    update_dockright();
}

void MainWindow::addglWidget(){
    QString *name = new QString(tr("%1").arg(widgetsknt++));
    glWidget *widget = new glWidget(name,this);
    connect(widget, SIGNAL(moveToTop(int)), this,SLOT(movewidgettotop(int)), Qt::UniqueConnection);
    widgets.append(widget);
    widgets.last()->set_position(QPoint(75,75)*(widgets.length()+plots.length()-1));
    widgets.last()->set_ProjectFolder(ProjectFolder);
    widgets.last()->set_ProjectName(ProjectName);
    this->movewidgettotop(widgetsknt-1);
    topindex = widgets.length()+plots.length()-1;
    update_dockright();
}

void MainWindow::exit(){

    this->close();
}

void MainWindow::deleteplot(int i){
    int number = plots.at(i)->getviewernumber();
    QMessageBox msgBox;
    msgBox.setInformativeText(QString(tr("Do you want to remove Plot %1")).arg(number)+"?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
    msgBox.setButtonText(QMessageBox::No, tr("No"));
    msgBox.setIcon(QMessageBox::Question);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No){
        plots.at(i)->raise_mainwindow();
        return;
    }
    delete plots.at(i);
    plots.removeAt(i);
    if (i == topindex)
        topindex = -1;
    update_dockright();
}

void MainWindow::deletewidget(int i){
    int number = widgets.at(i)->getwindownumber();
    QMessageBox msgBox;
    msgBox.setInformativeText(QString(tr("Do you want to remove 3D viewer %1")).arg(number)+"?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("Yes"));
    msgBox.setButtonText(QMessageBox::No, tr("No"));
    msgBox.setIcon(QMessageBox::Question);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No){
        widgets.at(i)->raise_mainwindow();
        return;
    }
    delete widgets.at(i);
    widgets.removeAt(i);
    if (i == topindex-plots.length())
        topindex = -1;
    update_dockright();
}

void MainWindow::moveviewertotop(int num){
    for (int i = 0 ; i < plots.length() ; i++){
        if (num == plots.at(i)->getviewernumber()){
            topindex = i;
            plots.at(topindex)->raise_mainwindow();
            return;
        }
    }
}

void MainWindow::movewidgettotop(int num){
    for (int i = 0 ; i < widgets.length() ; i++){
        if (num == widgets.at(i)->getwindownumber()){
            topindex = i + plots.length();
            widgets.at(i)->raise_mainwindow();
            return;
        }
    }
}

void MainWindow::raiseplot(int i){
    plots.at(i)->raise_mainwindow();
    topindex = i;
}

void MainWindow::raisewidget(int i){
    widgets.at(i)->raise_mainwindow();
    topindex = i + plots.length();
}

void MainWindow::showplot(int i){
    if (plots.at(i)->isvisible()){
        plots.at(i)->set_visible(false);
        BTNshowplotsslist.at(i)->setText(tr("Show"));
        QPoint position = plots.at(i)->get_position();
        QSize size = plots.at(i)->get_size();
        plots.at(i)->set_size(size);
        plots.at(i)->set_position(position);
        if (topindex < 0 )
            return;
        if (i != topindex){
            if (topindex < plots.length())
                plots.at(topindex)->raise_mainwindow();
            else if (topindex < plots.length() + widgets.length()) {
                widgets.at(topindex-plots.length())->raise_mainwindow();
            }
        }
    }
    else{
        plots.at(i)->set_visible(true);
        moveviewertotop(plots.at(i)->getviewernumber());
        BTNshowplotsslist.at(i)->setText(tr("Hide"));
    }
}

void MainWindow::showwidget(int i){
    if (widgets.at(i)->isvisible()){
        widgets.at(i)->set_visible(false);
        BTNshowwidgetslist.at(i)->setText(tr("Show"));
        QPoint position = widgets.at(i)->get_position();
        QSize size = widgets.at(i)->get_size();
        widgets.at(i)->set_size(size);
        widgets.at(i)->set_position(position);
        if (topindex < 0 )
            return;
        if (i != topindex-plots.length()){
            if (topindex < plots.length())
                plots.at(topindex)->raise_mainwindow();
            else if (topindex < plots.length() + widgets.length()) {
                widgets.at(topindex-plots.length())->raise_mainwindow();
            }
        }
    }
    else{
        widgets.at(i)->set_visible(true);
        movewidgettotop(widgets.at(i)->getwindownumber());
        BTNshowwidgetslist.at(i)->setText(tr("Hide"));
    }
}

void MainWindow::update_dockright(){
    if (BTNnewplot){
        delete BTNnewplot;
        BTNnewplot = nullpointer;
    }
    if (FRMplots){
        delete FRMplots;
        FRMplots = nullpointer;
    }
    if (QDLviewer2D){
        delete QDLviewer2D;
        QDLviewer2D = nullpointer;
    }

    if (BTNnewwidget){
        delete BTNnewwidget;
        BTNnewwidget = nullpointer;
    }
    if (FRMviewers){
        delete FRMviewers;
        FRMviewers = nullpointer;
    }
    if (QDLwidget3D){
        delete QDLwidget3D;
        QDLwidget3D = nullpointer;
    }

    if ( BTNraiseviewers){
        delete  BTNraiseviewers;
         BTNraiseviewers = nullpointer;
    }

    if (dockright){
        delete dockright;
        dockright = nullpointer;
    }
    dockright = new QDockWidget(tr(""),this);
    dockright->setAllowedAreas(Qt::RightDockWidgetArea);
    dockright->resize(QSize(500, this->height()));
    dockright->setFeatures(QDockWidget::DockWidgetMovable);
    dockright->setFeatures(QDockWidget::DockWidgetFloatable);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(1000);
    sizePolicy1.setHeightForWidth(dockright->sizePolicy().hasHeightForWidth());
    dockright->setSizePolicy(sizePolicy1);
    addDockWidget(Qt::RightDockWidgetArea,dockright);

    update_menu_viewer2D();
    update_menu_viewer3D();

    BTNraiseviewers = new QPushButton(tr("Raise all viewers"));
    BTNraiseviewers->setToolTip(tr("Move viewers to front"));
    BTNraiseviewers->setStyleSheet("QPushButton {background-color: darkGreen; color: white;}");
    connect(BTNraiseviewers, SIGNAL(clicked()), this, SLOT(updatewindowsoverlay()));

    QVBoxLayout *layout1 = new QVBoxLayout();
    layout1->addWidget(BTNraiseviewers);

    QWidget *btnwidget = new QWidget();
    btnwidget->setLayout(layout1);

    dockwidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(dockwidget);
    layout->addWidget(btnwidget);
    layout->addWidget(QDLviewer2D);
    layout->addWidget(QDLwidget3D);
    layout->addStretch();

    dockright->setWidget(dockwidget);
    updatewindowsoverlay();
}


void MainWindow::update_menu_viewer2D()
{
    for (int i = 0; i < connections2D.size(); i++) {
        QObject::disconnect(connections2D.at(i));
    }
    connections2D.clear();

    if (BTNnewplot) {
        delete BTNnewplot;
        BTNnewplot = nullptr;
    }

    QDLviewer2D = new ViewerDialog();
    QDLviewer2D->setWindowTitle(tr("2D Viewer: choose an option"));
    QDLviewer2D->resize(300, 80);

    FRMplots = new QGroupBox();
    FRMplots->setTitle(tr("Available plots"));
    FRMplots->setVisible(plots.count() > 0);

    QGridLayout *layout1 = new QGridLayout(FRMplots);

    BTNshowplotsslist.clear();
    BTNraiseplotslist.clear();

    for (int i = 0; i < plots.count(); i++) {
        QLabel *LBLplot = new QLabel(plots.at(i)->get_viewername());

        QPushButton *BTNdeleteplot = new QPushButton(tr("Delete"));

        connections2D << connect(BTNdeleteplot,&QPushButton::clicked,
            this,[this, i]() { deleteplot(i); });

        QPushButton *BTNraise = new QPushButton(tr("Raise"));

        BTNraiseplotslist.append(BTNraise);

        connections2D << connect(BTNraise,&QPushButton::clicked,
            this,[this, i]() { raiseplot(i); });

        QPushButton *BTNshow = new QPushButton();

        if (plots.at(i)->isvisible())
            BTNshow->setText(tr("Hide"));
        else
            BTNshow->setText(tr("Show"));

        BTNshowplotsslist.append(BTNshow);

        connections2D << connect(plots.at(i),&Viewer2D::hideplotter,
            BTNshow,&QPushButton::click);

        connections2D << connect(BTNshow,&QPushButton::clicked,
            this,[this, i]() { showplot(i); });

        layout1->addWidget(LBLplot,       i, 0);
        layout1->addWidget(BTNraise,      i, 1);
        layout1->addWidget(BTNshow,       i, 2);
        layout1->addWidget(BTNdeleteplot, i, 3);
    }

    BTNnewplot = new QPushButton(tr("New 2D Plotter"));
    BTNnewplot->setToolTip(
        tr("Creates a new window for 2D plotting")
    );

    connections2D << connect(BTNnewplot,&QPushButton::clicked,
        this,&MainWindow::addviewer2D);

    QLabel *label2D = new QLabel(tr("2D plotters"));
    label2D->setStyleSheet("QLabel { color : blue; }");

    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->addWidget(label2D, Qt::AlignCenter);

    QVBoxLayout *layout3 = new QVBoxLayout(QDLviewer2D);
    layout3->addLayout(layout2);
    layout3->addWidget(FRMplots);
    layout3->addWidget(BTNnewplot);
    layout3->addStretch();
}

void MainWindow::update_menu_viewer3D()
{
    for (int i = 0; i < connections3D.size(); i++) {
        QObject::disconnect(connections3D.at(i));
    }
    connections3D.clear();

    if (BTNnewwidget) {
        delete BTNnewwidget;
        BTNnewwidget = nullptr;
    }

    QDLwidget3D = new ViewerDialog();
    QDLwidget3D->setMinimumSize(250, 80);

    FRMviewers = new QGroupBox();
    FRMviewers->setTitle(tr("Available 3D viewers"));
    FRMviewers->setVisible(widgets.count() > 0);

    QGridLayout *layout1 = new QGridLayout(FRMviewers);

    BTNshowwidgetslist.clear();
    BTNraisewidgetslist.clear();

    for (int i = 0; i < widgets.count(); i++) {
        QLabel *LBLwidget = new QLabel(widgets.at(i)->getWindowName());

        QPushButton *BTNdeletewidget = new QPushButton(tr("Delete"));

        connections3D << connect(BTNdeletewidget,&QPushButton::clicked,
            this,[this, i]() { deletewidget(i); });

        QPushButton *BTNraise = new QPushButton(tr("Raise"));

        BTNraisewidgetslist.append(BTNraise);

        connections3D << connect(BTNraise,&QPushButton::clicked,
            this,[this, i]() { raisewidget(i); });

        QPushButton *BTNshow = new QPushButton();

        if (widgets.at(i)->isvisible())
            BTNshow->setText(tr("Hide"));
        else
            BTNshow->setText(tr("Show"));

        BTNshowwidgetslist.append(BTNshow);

        connections3D << connect(widgets.at(i),&glWidget::hideviewer,
            BTNshow,&QPushButton::click);

        connections3D << connect(BTNshow,&QPushButton::clicked,
            this,[this, i]() { showwidget(i); });

        layout1->addWidget(LBLwidget,       i, 0);
        layout1->addWidget(BTNraise,        i, 1);
        layout1->addWidget(BTNshow,         i, 2);
        layout1->addWidget(BTNdeletewidget, i, 3);
    }

    BTNnewwidget = new QPushButton(tr("New 3D Viewer"));
    BTNnewwidget->setToolTip(
        tr("Creates a new window for 3D display")
    );

    connections3D << connect(BTNnewwidget,&QPushButton::clicked,
        this,&MainWindow::addglWidget);

    QLabel *label3D = new QLabel(tr("3D viewers"));
    label3D->setStyleSheet("QLabel { color : red; }");

    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->addWidget(label3D, Qt::AlignCenter);

    QVBoxLayout *layout3 = new QVBoxLayout(QDLwidget3D);
    layout3->addLayout(layout2);
    layout3->addWidget(FRMviewers);
    layout3->addWidget(BTNnewwidget);
    layout3->addStretch();
}

void MainWindow::updatewindowsoverlay(){
    for (int i = 0 ; i < plots.length() ; i++){
        plots.at(i)->raise_mainwindow();
    }
    for (int i = 0 ; i < widgets.length() ; i++){
        widgets.at(i)->raise_mainwindow();
    }
    if (topindex >= 0){
        if (topindex < plots.length()){
            plots.at(topindex)->raise_mainwindow();
        }
        else if (topindex < widgets.length()+plots.length()){
                widgets.at(topindex-plots.length())->raise_mainwindow();
        }
    }
}

QString MainWindow::get_execName(QString processname, QString subdir){
#if defined(Q_WS_WIN) || defined(Q_OS_WIN)
    QString execName = processname;
    if (!QFileInfo::exists(execName)){
        execName = QCoreApplication::applicationDirPath()+"/"+processname;
    }
#else
    QString execName = QCoreApplication::applicationDirPath()+"/"+processname;
    if (!QFileInfo::exists(execName)){
        execName = processname;
    }
#endif
    if (!QFileInfo::exists(execName))
        execName = QCoreApplication::applicationDirPath()+"/../"+subdir+"/"+processname;
    if (!QFileInfo::exists(execName)){
        QString message1, message2, message3;
        QString direc = QString(QCoreApplication::applicationDirPath());
        direc.truncate(direc.lastIndexOf(QChar('/')));
        message1 = QString(tr("Executable file %1 does not exist\n\n").arg(processname));
        message2 = QString(tr("Check that the program is installed in any of the following directories: \n\n %1 \n %2 \n\n")
                        .arg(QCoreApplication::applicationDirPath()+"/")
                        .arg(direc+"/"+subdir+"/"));
        message3 = QString(tr("or in any other directory available in your $PATH"));
        int messagelen = qMax(qMax(message1.length(),message2.length()),message3.length());
        QMessageBox msg;
        msg.setText(message1+message2+message3);
        msg.setIcon(QMessageBox::Critical);
        QSpacerItem* horizontalSpacer = new QSpacerItem(messagelen * 4, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        QGridLayout* layout = (QGridLayout*)msg.layout();
        layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
        msg.exec();
        return QString("");
    }
    return execName;
}


QString MainWindow::get_python()
{
    const QStringList candidates = {
        QStringLiteral("python3"),
        QStringLiteral("python")
    };

    for (const QString& candidate : candidates) {
        const QString executable =
            QStandardPaths::findExecutable(candidate);

        if (executable.isEmpty()) {
            continue;
        }

        QProcess process;

        process.start(
            executable,
            {
                QStringLiteral("-c"),
                QStringLiteral(
                    "from lxml import etree; import numpy"
                )
            }
        );

        if (!process.waitForStarted(3000)) {
            continue;
        }

        process.waitForFinished(5000);

        if (process.exitStatus() == QProcess::NormalExit
            && process.exitCode() == 0) {
            return executable;
        }
    }

    QMessageBox::warning(
        this,
        tr("Python"),
        tr(
            "A suitable Python interpreter was not found.\n\n"
            "Python 3 with the lxml and numpy modules is required."
        )
    );

    return {};
}

/*******************************************************************************************************/
/********************************  Class mainmenu  implementation  *******************************/
/*******************************************************************************************************/

void mainmenu::resizeEvent(QResizeEvent *event){
    size = event->size();
}

/*******************************************************************************************************/
/********************************  New functions for DAMQT4  *******************************/
/*******************************************************************************************************/

void MainWindow::onFchkImportFinished()
{
    create_damproj(0, QProcess::NormalExit);
    updateOrbitalsPageState();
}

void MainWindow::onMolproImportFinished()
{
    defineRanges();
    updateOrbitalsPageState();
}

void MainWindow::onSinglePassImportFinished(
    const QString& outputFilePath)
{
    Q_UNUSED(outputFilePath);

    setExecutionControlsEnabled(true);
    statusBar()->showMessage(tr("End of calculation"));
    updateOrbitalsPageState();
}

void MainWindow::onSinglePassImportFailed(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    setExecutionControlsEnabled(true);

    if (exitStatus == QProcess::CrashExit) {
        statusBar()->showMessage(
            tr("Process crashed, exit code = %1").arg(exitCode));
    } else {
        statusBar()->showMessage(
            tr("Process failed, exit code = %1").arg(exitCode));
    }
}


void MainWindow::processStart()
{
    setExecutionControlsEnabled(false);
    statusBar()->showMessage(tr("Computing..."));
}

void MainWindow::setExecutionControlsEnabled(bool enabled)
{
    projectPage_->setExecEnabled(enabled);
    atomicDensitiesPage_->setExecEnabled(enabled);
    densityPage_->setExecEnabled(enabled);
    potentialPage_->setExecEnabled(enabled);
    hfForcesPage_->setExecEnabled(enabled);
    fieldLinesPage_->setExecEnabled(enabled);
    densityGradientPage_->setExecEnabled(enabled);
    radialFactorsPage_->setExecEnabled(enabled);
    orientedMultipolesPage_->setExecEnabled(enabled);
    orbitalsPage_->setExecEnabled(enabled);
    topographyPage_->setExecEnabled(enabled);
    zjExpansionPage_->setExecEnabled(enabled);
    zjDensityPage_->setExecEnabled(enabled);
    sigmaHolePage_->setExecEnabled(enabled);
}

void MainWindow::updateOrbitalsPageState()
{
    const QDir projectDir(ProjectFolder);

    const bool orbitalsAvailable =
        !projectDir.entryList(
            {"*.GAorb*", "*.SLorb*", "*.orb*"},
            QDir::Files | QDir::Readable
        ).isEmpty();

    orbitalsPage_->setEnabled(orbitalsAvailable);
}

void MainWindow::readMolproXml(const QString& importFile,
                               const QString& importFolder)
{
    QDir path;

    if (!path.exists(ProjectFolder)) {
        QMessageBox msgBox;

        msgBox.setInformativeText(
            tr("Project %1 not found").arg(ProjectFolder)
            + "\n"
            + tr("Do you wish to create?")
        );

        msgBox.setStandardButtons(
            QMessageBox::Yes |
            QMessageBox::No |
            QMessageBox::Cancel
        );

        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.setIcon(QMessageBox::Warning);

        if (msgBox.exec() != QMessageBox::Yes) {
            return;
        }

        createDir(ProjectFolder);
        statusBar()->showMessage(
            tr("Project successfully created"),
            2000
        );

        setPostDamPagesEnabled(false);
    }

    const QString sourceXmlFile = QDir::cleanPath(
        QDir(importFolder).filePath(importFile)
    );

    if (!QFileInfo::exists(sourceXmlFile)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("The MOLPRO XML file does not exist:\n%1")
                .arg(sourceXmlFile)
        );
        return;
    }

    const QString destinationXmlFile = QDir::cleanPath(
        QDir(ProjectFolder).filePath(
            QFileInfo(sourceXmlFile).fileName()
        )
    );

    /*
     * Keep a copy of the XML file in the project directory,
     * except when source and destination are the same file.
     */
    if (QFileInfo(sourceXmlFile).absoluteFilePath()
        != QFileInfo(destinationXmlFile).absoluteFilePath()) {

        if (QFile::exists(destinationXmlFile)
            && !QFile::remove(destinationXmlFile)) {

            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Cannot replace the existing XML file:\n%1")
                    .arg(destinationXmlFile)
            );
            return;
        }

        if (!QFile::copy(sourceXmlFile, destinationXmlFile)) {
            QMessageBox::warning(
                this,
                tr("DAMQT"),
                tr("Cannot copy the XML file:\n%1\n\nto:\n%2")
                    .arg(sourceXmlFile, destinationXmlFile)
            );
            return;
        }
    }

    const QString projectFile =
        QDir(ProjectFolder).filePath(ProjectName + ".damproj");

    existsinp(projectFile, 1, true);

    const QString scriptFile = QDir::cleanPath(
        QDir(QCoreApplication::applicationDirPath()).filePath(
            QStringLiteral("../%1/MOLPRO_xml_interface.py")
                .arg(QStringLiteral(DAMQT_INTERFACES_DIR))
        )
    );

    if (!QFileInfo::exists(scriptFile)) {
        QMessageBox::warning(
            this,
            tr("DAMQT"),
            tr("Python script not found:\n%1").arg(scriptFile)
        );
        return;
    }

    const QString pythonExecutable = get_python();

    if (pythonExecutable.isEmpty()) {
        return;
    }

    /*
     * The script reads the original XML from its original directory
     * and writes all generated files into ProjectFolder.
     */
    const QStringList arguments {
        scriptFile,
        QFileInfo(sourceXmlFile).fileName(),
        QFileInfo(sourceXmlFile).absolutePath(),
        ProjectFolder,
        ProjectName
    };

    QProcess* process = new QProcess(this);

    connect(
        process,
        &QProcess::started,
        this,
        &MainWindow::processStart
    );

    connect(
        process,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        [this, process](
            int exitCode,
            QProcess::ExitStatus exitStatus)
        {
            onMolproXmlFinished(exitCode, exitStatus);
            process->deleteLater();
        }
    );

    connect(
        process,
        &QProcess::errorOccurred,
        this,
        [this, process](QProcess::ProcessError error)
        {
            const QString processName =
                QStringLiteral("MOLPRO_xml_interface.py");

            if (error == QProcess::FailedToStart) {
                QMessageBox::critical(
                    this,
                    tr("Process error"),
                    tr("The process %1 could not be started.\n\n"
                       "Check that it is installed and accessible.\n\n%2")
                        .arg(processName, process->errorString())
                );

                return;
            }

            QMessageBox::critical(
                this,
                tr("Process error"),
                tr("Error when running %1:\n%2")
                    .arg(processName, process->errorString())
            );
        }
    );

    process->start(pythonExecutable, arguments);
}

void MainWindow::onMolproXmlFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus)
{
    projectPage_->setExecEnabled(true);
    QApplication::restoreOverrideCursor();

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        statusBar()->showMessage(
            tr("MOLPRO XML interface failed, exit code = %1")
                .arg(exitCode)
        );
        return;
    }

    const QString root =
        QDir(ProjectFolder).filePath(ProjectName);

    const bool filesGenerated =
        QFileInfo::exists(root + QStringLiteral(".ggbs"))
        && QFileInfo::exists(root + QStringLiteral(".den"));

    if (!filesGenerated) {
        QMessageBox::warning(
            this,
            tr("MOLPRO XML interface"),
            tr("The interface finished, but the basis-set or density "
               "files were not generated.")
        );

        return;
    }

    projectPage_->setExecEnabled(true);
    QApplication::restoreOverrideCursor();

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        statusBar()->showMessage(
            tr("MOLPRO XML interface failed, exit code = %1")
                .arg(exitCode)
        );

        return;
    }

    const QString outputFile =
        QDir(ProjectFolder).filePath(
            ProjectName
            + QStringLiteral("-MOLPRO_xml_interface.out")
        );

    QFile file(outputFile);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(
            this,
            tr("MOLPRO XML interface"),
            tr("File %1 cannot be read:\n%2")
                .arg(outputFile, file.errorString())
        );

        return;
    }

    QTextStream input(&file);

    textEdit->setFont(QFont(QStringLiteral("Courier"), 10));
    textEdit->setPlainText(input.readAll());

    atomicDensitiesPage_->setEnabled(true);
    orbitalsPage_->setEnabled(true);

    const QString projectFile =
        QDir(ProjectFolder).filePath(
            ProjectName + QStringLiteral(".damproj")
        );

    SetCurrentFile(projectFile, true, false);

    statusBar()->showMessage(
        tr("MOLPRO XML import completed")
    );
}

void MainWindow::updateMpiControls()
{
    const bool densityMpi =
        mpi && densityPage_->is3DGrid();

    densityPage_->setMpiVisible(densityMpi);
    densityPage_->setMpiControlsEnabled(densityMpi);

    const bool potentialMpi =
        mpi && potentialPage_->is3DGrid();

    potentialPage_->setMpiVisible(potentialMpi);
    potentialPage_->setMpiControlsEnabled(potentialMpi);

    const bool orbitalsMpi =
        mpi && orbitalsPage_->is3DGrid();

    orbitalsPage_->setMpiVisible(orbitalsMpi);
    orbitalsPage_->setMpiControlsEnabled(orbitalsMpi);

    const bool fieldLinesMpi =
        mpi && fieldLinesPage_->is3DPlot();

    fieldLinesPage_->setMpiVisible(fieldLinesMpi);
    fieldLinesPage_->setMpiControlsEnabled(fieldLinesMpi);

    const bool densityGradientMpi =
        mpi && densityGradientPage_->is3DPlot();

    densityGradientPage_->setMpiVisible(densityGradientMpi);
    densityGradientPage_->setMpiControlsEnabled(densityGradientMpi);

    const bool zjDensityMpi =
        mpi && zjDensityPage_->is3DGrid();

    zjDensityPage_->setMpiVisible(zjDensityMpi);
    zjDensityPage_->setMpiControlsEnabled(zjDensityMpi);
}
