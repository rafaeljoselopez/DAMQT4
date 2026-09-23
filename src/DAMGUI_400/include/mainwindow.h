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
//  File:   mainwindow.h
//
//      Last version: March 2026
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QProcess>
#include <QSettings>
#include <QSplashScreen>
#include <QStatusBar>
#include <QString>
#include <QTextEdit>
#include <QTextStream>
#include <QTableWidget>
#include <QtGlobal>
#include <QToolBar>
#include <QTranslator>
#include <QVector3D>
#include <stdlib.h>
#include <string>

#include <QtDebug>

#include "configdialog.h"
#include "dialog.h"
#include "externals.h"
#include "IniFile.h"
#include "math.h"
#include "viewer2D.h"
#include "viewerdialog.h"
#include "Sheet.h"
#include "glWidget.h"
#include "readwriteoptions.h"
#include "iexecutablepage.h"

using namespace std;

#define MAX_LEXP 25
#define MAX_LEXPZJ 22
#define MAX_KEXPZJ 40
#define MAX_ARCHIVOS_RECIENTES  20
#define MAX_NUM_PROCESSORS 64
#define NFORTRANPROCS 13

bool checkMpiCommand(QString& mpiCommand);

class QAction;
class QActionGroup;
class QComboBox;
class QDir;
class QMenu;
class QTextEdit;
class QToolBox;
class QWidget;
class QLabel;
class QLineEdit;
class QGroupBox;
class QToolButton;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QProcess;
class QTabWidget;
class QStringList;
class QPrinter;
class QPainter;
class Sheet;
class Surface2D;
class QDoubleValidator;

class ProjectPage;
class AtomicDensitiesPage;
class DensityPage;
class PotentialPage;
class OrbitalsPage;
class TopographyPage;
class SigmaHolePage;
class FieldLinesPage;
class DensityGradientPage;
class HFForcesPage;
class RadialFactorsPage;
class OrientedMultipolesPage;
class ZJExpansionPage;
class ZJDensityPage;

class FchkImporter;
class MolproImporter;
class SinglePassImporter;

#if __cplusplus <= 199711L
    #define nullpointer NULL
#else
//  C++11 compliant compiler
    #define nullpointer nullptr
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    void finishsplash();
    
    
protected:
    void closeEvent(QCloseEvent *event);       
    
signals:
    void initproject();

public slots:

    void deleteplot(int); 
    void deletewidget(int);
    void external_package();
    void raiseplot(int);
    void raisewidget(int);
    void showplot(int);
    void showwidget(int);
    void exit();

private slots:      // Alphabetically sorted (including type in sort)
    bool end_Viewer2DDialog();
    bool end_Viewer3DDialog();
//    bool GAUSS_two_pass_case(QString);
    bool saveProject();
//    bool SaveProjectAs();
    bool saveProjectAs();

    void about();
    void addglWidget();
    void addviewer2D();

    void CHKatdensinput_changed(int state);

    void chooseLanguage(QAction *action);

    void create_damproj(int exitCode, QProcess::ExitStatus exitStatus);
    void createLanguageMenu();

    void disable_pages();

    void execDam();
    void execDamden();
    void execDamdengrad();
    void execDamdenZJ();
    void execDamfield();
    void execDamforces();
    void execDamfrad();
    void execDamSGhole();
    void execDamZJ();
    void execGgbsDen();
    void execDammultrot();
    void execDamorb();
    void execDampot();
    void execDamTopography();
    void execImport();
    void execsgbs2sxyz(const QString &qstring);
    void execSxyzDen();
    void Help();

    void importFile();
    void importOUT();

    void menu_viewer2D();
    void menu_viewer3D();
    void moveviewertotop(int);
    void movewidgettotop(int);

    void newProject();

    void onFchkImportFinished();
    void onMolproImportFinished();
    void onSinglePassImportFinished(const QString& outputFilePath);
    void onSinglePassImportFailed(
       int exitCode,
       QProcess::ExitStatus exitStatus);
    void openProject();
    void openRecentProjects();

    void PrintFile();
    void PrintFilePdf();
    void processStart();

    void readFchk();
    void readMolpro();
    void readMopac();
    void readNWChem();
    void readTurbom();
    void readMOLEKEL();

    void setExecutionControlsEnabled(bool enabled);

    void showPerformanceSettings();

    void atdenslmaxexp_changed();
        
    void start();

    void tabChanged(int index);
    void TXTImport_changed();
    void TXTmpicommand_changed();
    void TXTmpiflags_changed();
    void TXTProjectFolder_changed(const QString &cad);
    void TXTProjectName_changed(const QString &cad);

    void update_dockright();
    void update_menu_viewer2D();
    void update_menu_viewer3D();
    void update_statusbar(QString);
    void update_textedit(QString);
    void updateOrbitalsPageState();
    void updatewindowsoverlay();

    void showOutputText(const QString &text);


    QString get_execName(QString, QString);
    QString get_python();

    void showRecentProjectContextMenu(const QPoint& pos);
    
private:
    bool saveProjectToFile(const QString& fullFileName);

    template <typename Page, typename ExecSlot>
    void connectExecutablePage(Page *page, ExecSlot execSlot)
    {
        executablePages_ << page;

        connect(page, &Page::outputTextReady,
                this, &MainWindow::showOutputText);

        connect(page, &Page::showInputFileRequested,
                this, &MainWindow::showOutputText);

        connect(page, &Page::externalProcessStarted,
                this, &MainWindow::onExternalProcessStarted);

        connect(page, &Page::externalProcessFinished,
                this, &MainWindow::onExternalProcessFinished);

        connect(page, &Page::openOutputRequested,
                this, &MainWindow::importOUT);

        connect(page, &Page::execRequested,
                this, execSlot);
    }

    void initializeNames();
    void readMolproXml(const QString& importFile,
                       const QString& importFolder);
    void onMolproXmlFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus
    );

    void updateMpiControls();

    void setOutputPrefixes(const QString& prefix);
    void updateProjectAvailability();
    void setFileDialogProjectFolders(const QString& folder);

    ProjectPage* projectPage_ = nullptr;
    AtomicDensitiesPage* atomicDensitiesPage_ = nullptr;
    DensityPage* densityPage_ = nullptr;
    PotentialPage* potentialPage_ = nullptr;
    OrbitalsPage* orbitalsPage_ = nullptr;
    TopographyPage* topographyPage_ = nullptr;
    SigmaHolePage* sigmaHolePage_ = nullptr;
    FieldLinesPage* fieldLinesPage_ = nullptr;
    DensityGradientPage* densityGradientPage_ = nullptr;
    HFForcesPage* hfForcesPage_ = nullptr;
    RadialFactorsPage* radialFactorsPage_ = nullptr;
    OrientedMultipolesPage* orientedMultipolesPage_ = nullptr;
    ZJExpansionPage* zjExpansionPage_ = nullptr;
    ZJDensityPage* zjDensityPage_ = nullptr;

    FchkImporter* fchkImporter_ = nullptr;
    MolproImporter* molproImporter_;
    SinglePassImporter* singlePassImporter_;

    int mden;    // type of density matrix: 0: SCF density, 1: SCF spin density, 2: CI density, 3: CI spin 
    int densplanecase;
    int dZJplanecase;
    int MOplanecase;
    int plotsknt;
    int potplanecase;
    int topindex;
    int widgetsknt;

    double rmax;
    double rstar;
    double xmax;
    double xmin;
    double ymax;
    double ymin;
    double zmax;
    double zmin;
    
    QSplashScreen *splash = nullptr;
    
    QTranslator appTranslator;
    QTranslator qtTranslator;
    
//    Actions
    
    QAction *Acc2Dplot = nullptr;
    QAction *Acc3Dview = nullptr;
    QAction *AccAbout = nullptr;
    QAction *AccAboutQt = nullptr;
    QAction *AccExit = nullptr;
    QAction *AccExternal = nullptr;
    QAction *AccHelp = nullptr;
    QAction *AccNew = nullptr;
    QAction *AccOpen = nullptr;
    QAction *AccPdf = nullptr;
    QAction *AccPerformance = nullptr;
    QAction *AccPrint = nullptr;
    QAction *AccRecentFiles[MAX_ARCHIVOS_RECIENTES] = { nullptr };
    QAction *AccSave = nullptr;
    QAction *AccSaveAs = nullptr;

    QActionGroup *languageActionGroup = nullptr;

//    Dialogs

    ViewerDialog *QDLviewer2D = nullptr;
    ViewerDialog *QDLwidget3D = nullptr;

//    Labels

    QLabel *LBLlanguage = nullptr;

//    Lists

    QList<QPushButton*> BTNraiseplotslist;             // Stores buttons for rising 2D plots
    QList<QPushButton*> BTNshowplotsslist;              // Stores buttons for hide/show 2D plots
    QList<QPushButton*> BTNraisewidgetslist;            // Stores buttons for rising 3D plots
    QList<QPushButton*> BTNshowwidgetslist;             // Stores buttons for hide/show 3D viewers

    QList<glWidget*> widgets;
    QList<Viewer2D*> plots;

    QList<IExecutablePage *> executablePages_;
    QList<IExecutablePage *> postDamPages_;

//    Menus

    QMenu *languageMenu = nullptr;
    QMenu *FileMenu = nullptr;
    QMenu *GraphicsMenu = nullptr;
    QMenu *HelpMenu = nullptr;
    QMenu *RecentProjectsMenu = nullptr;

//   Push Buttons

    QPushButton *BTNlangstart = nullptr;
    QPushButton *BTNexit2D = nullptr;
    QPushButton *BTNnewplot = nullptr;
    QPushButton *BTNnewwidget = nullptr;
        
//    Directories and files
    QString ImportFolder;    // path to the folder where the original input data of the project reside
    QString ImportFile;    // name of the file for data import (.fchk for Gaussian, .basis or .coor or .mos for TURBMOOLE, .mkl for MOLEKEL, .out for MOLPRO))
    QString ProjectFolder;    // path to the project folder 
    QString ProjectName;    // common name for all the files of the project
    QString DataFile; // Data input filenames for FORTRAN programs

//    Toolbars
    
    QToolBar *ToolBarFile = nullptr;
    QToolBar *ToolBarHelp = nullptr;
    
//    General purpose variables
    
    QString ProjectFilePath;
    QStringList ArchivosRecientes;
    
    bool changes;
    bool activebeware;
    bool lzdo;
    bool lvalence;

    Dialog *dialog = nullptr;
        
    QDialog *FRMlanguage = nullptr;
    
    QString LanguagePath;
    bool lslater;        // true if slater calculation, false if gaussian calculation
    
    int natom;

    QPushButton *BTNraiseviewers = nullptr;
    
    QWidget *dockwidget = nullptr;
    
    QString System;

    QTabWidget *TAWprincipal = nullptr;
    QTextEdit *textEdit = nullptr;

    QToolBox *toolBox = nullptr;
    QWidget *rightBox = nullptr;
    QVBoxLayout *rightBoxLayout = nullptr;
    
//   page_project: Project
//   ---------------------

    QGroupBox *FRMplots = nullptr;
    QGroupBox *FRMviewers = nullptr;
    
//   page_atdens: Atomic densities
//   -----------------------------
    
    QStringList *denslist = nullptr;
    
    QValidator *densvalidator = nullptr;


    QDockWidget *dockright = nullptr;
    

//  Functions (alphabetically sorted including type in sort)
//  --------------------------------------------------------

    bool Open(const QString &fileName);
    bool compareIntegers(const QString& s1, const QString& s2);
    bool mustSave();
    bool existsinp(QString fullinputName,int def, bool pregunta);
    bool Save(const QString &fileName);

    void UpdateRecentFiles();
    void CreateActions();
    void loadDefault(int all);
    void CreateLeftMenu();
    void CreateRightMenu();
    void CreateMenus();
    void CreateStatusBar();
    void CreateToolBars();
    void DAMDENdatafile(const QString &fullFileName, const QString projectDir, const QString projectName);
    void defineRanges();

    void GDAMdatafile(const QString &fullinputName, const QString projectDir, const QString projectName);
    void import(const QString &fileName);
    void inputdatafile(const char *suffix, const char *section, const QString &fullFileName,
        const QString &projectDir, const QString &projectName);
    void onExternalDamFinished(bool enabled);
    void onExternalDamStarted();
    void onExternalProcessFinished();
    void onExternalProcessStarted();
    void saveOptions(const QString &fullFileName);
    void readGeometry(int &natom,QVector<double> &x,QVector<double> &y,QVector<double> &z,QVector<int> &ncarga);
    void readOptions(const QString &fullFileName);
    void readSettings();
    void rename_density_cntfile();
    void rename_pot_cntfile();
    void setAllDamPagesEnabled(bool enabled);
    void SetCurrentFile(const QString &fileName,bool usar,bool modificado);
    void SetDir(const QString &carpeta,const QString &nombre);
    void setPostDamPagesEnabled(bool enabled);
    void setuvxyz();

    void writeSettings();

    int read_natom(QString fileName);

    QByteArray ReadSectionOptions(const char *SectionName, QFile *FileName);

    QString FileWithoutExt(const QString &fullFileName);
    QString planesuffix(int);

    QVector3D wu;
    QVector3D wv;

    PlaneResult planeResult;

    string toString(QString qv);

};

#endif

/* 
 * Estracted from File:   mainmenu.h
 * Author: rafa
 *
 * Created on 13 de febrero de 2013, 8:51
 */

#ifndef MAINMENU_H
#define    MAINMENU_H

#include <QToolBox>

class QToolBox;

class mainmenu : public QToolBox
{
    Q_OBJECT

public:
    mainmenu(QWidget *parent) : QToolBox(parent){    
    };
    QSize sizeHint() const
    {          
        return size;
    } 
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    QSize size = QSize(400,2000);
};

#endif    /* MAINMENU_H */
