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
//	Header file of molecule class
//  Description: molecule defines a molecule object with geometry, surfaces, critical points, etc
//
//	File:   molecule.h
//
//	Author: Rafael Lopez    (rafael.lopez@uam.es)
//
//	Last version: May 2021
//
#ifndef MOLECULE_H
#define MOLECULE_H

#include <QApplication>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDoubleSpinBox>
#include <QFont>
#include <QLabel>
#include <QGridLayout>
#include <QObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QtGlobal>
#include <QTimer>
#include <QVector>
#include <QVector3D>
#include <QtCore/qmath.h>

#include "ColorButton.h"

#include "grid.h"
#include "criticalpoints.h"
#include "elements.h"
#include "fieldlines.h"
#include "forces.h"
#include "surface.h"
#include "widgetsubclasses.h"


#include <string>
#include <regex>
#include <cstdlib> // para strtod

#ifndef pi
#define pi 3.14159265358979323846  // pi
#endif

#ifndef ANGSTROM_TO_BOHR
#define ANGSTROM_TO_BOHR 1.88971616463
#endif

#ifndef BOHR_TO_ANGSTROM
#define BOHR_TO_ANGSTROM 0.529177248882
#endif

#ifndef INIT_BOND_THRESHOLD
#define INIT_BOND_THRESHOLD  1.2
#endif

#ifndef INTERVAL_INI
#define INTERVAL_INI  100
#endif

#ifndef INTERVAL_SCALE
#define INTERVAL_SCALE  200
#endif

#ifndef MAX_CPS
#define MAX_CPS  4   // Number of types of critical points
#endif

#ifndef MAX_FORCES
#define MAX_FORCES  5		 // Number of types of force types
#endif

#ifndef MAX_INTERVAL
#define MAX_INTERVAL  220.
#endif

#ifndef MIN_INTERVAL
#define MIN_INTERVAL  20.
#endif

#ifndef MOLSCALEHEIGHT
#define MOLSCALEHEIGHT  20.
#endif

#ifndef MOLSCALEARROWSHEIGHT
#define MOLSCALEARROWSHEIGHT  2.
#endif

#ifndef SCALE
#define SCALE  0.01
#endif

#ifndef Z_TRANS_INI
#define Z_TRANS_INI  -20.
#endif

class editMoleculeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit editMoleculeDialog(QWidget *parent = 0);
    ~editMoleculeDialog();
signals:
    void closed();
protected:
    void closeEvent(QCloseEvent *event);
    virtual void reject();
};

class molecule : public QWidget
{
    friend class geomProcessor;
    Q_OBJECT
public:
    explicit molecule(QWidget *parent = 0);

    ~molecule();

    bool getangstromcoor();
    bool getangstromcp();
    bool getdrawatomcoords();
    bool getdrawatomindices();
    bool getdrawatomsymbols();
    bool gethideatoms();
    bool gethidebonds();
    bool gethidehydrogens();
    bool getiscluster();
    bool getonlyatomactive();
    bool isactive();
    bool isatomactive(int);
    bool isatomvisible(int);
    bool isaxeslabelsvisible();
    bool isaxesvisible();
    bool isvisible();
    bool loadgrid();
    bool loadsurf();
    bool retrievegrid(const QString &filename);
    bool retrievesurf(const QString &filename);

    double getz_trans_ini();

    qreal getballradius();

    float getstepwheel();

    int  getcoordprecision();
    int  getlabelsvshift();
    int  getnumatoms();
    int  getnumcps(int);
    int  getznuc(int);

    void setlabelsvshift(int);
    void set_ProjectFolder(QString);
    void set_ProjectName(QString);

    fieldlines *flines = nullptr;
    forces *hfforces = nullptr;
    criticalpoints *cps = nullptr;

    QColor getfontcolor();
    QColor getXaxis_color();
    QColor getYaxis_color();
    QColor getZaxis_color();

    QFont getfont();
    QFont getfontaxeslabels();

    QList<grid*> grids;
    QList<surface*> surfaces;
    QList<surface*> sortedSurfaces;

    QPoint getparentposition();

    QQuaternion getrotation();

    QString getfullname();
    QString getname();
    QString getpath();

    QVector <QVector3D> getpositionaxeslabels();
    QVector3D gettranslation();
    QVector3D getrotationAxis();

    QVector <GLuint> getallindices();
    QVector <GLuint> getallindicesoffset();
    QVector <VertexNormalData> getallvertices();
    QVector <GLuint> allaxesindices;           // Indices of axes vertices
    QVector <GLuint> allaxesindicesoffset;     // Offsets of axes indices in objects
    QVector <VertexNormalData> allaxesvertices;  // Vertices of axes triangles(position, normal, color)
    QVector <GLuint> allindices;           // Indices of vertices in structure
    QVector <GLuint> allindicesoffset;     // Offsets of indices in objects (cylinders and spheres) in structure
    QVector <VertexNormalData> allvertices;  // Vertices of triangles in structure (position, normal, color)
    QVector <int> getcharges();
    QVector <QVector3D> getxyz();

    void createeditMoleculeDialog();
    void darken();
    void initatomactive(bool);
    void initatomvisible(bool);
    void lighten();
    void loadallindices(QVector<GLuint>);
    void loadallindicesoffset(QVector<GLuint>);
    void loadallvertices(QVector<VertexNormalData>);
    void loadcharges(QVector <int>);
    void loadxyz(QVector <QVector3D>);
    void setactive(bool);
    void setallatomactive(bool);
    void setatomactive(int, bool);
    void setdrawatomcoords(bool);
    void setdrawatomindices(bool);
    void setdrawatomsymbols(bool);
    void setfont(QFont);
    void setfontaxeslabels(QFont);
    void setfontcolor(QColor);
    void setfullname(QString);
    void setiscluster(bool);
    void setname(QString);
    void setparentposition(QPoint);
    void setonlyatomactive(bool);
    void setpath(QString);
    void setrotation(QQuaternion);
    void setrotationAxis(QVector3D);
    void setrotationButtons();
    void setscaleradii(bool);
    void setstepwheel(float);
    void settranslationButtons();
    void settranslation(QVector3D);
    void setvisible(bool);
signals:
    void animate(bool);
    void updatedisplay();
    void updateGL();
    void updateRightMenu();

public slots:
    bool QDLeditMolecule_isVisible();
    bool startanimate();
    bool stopanimate();

    togglingGroupBox *surfaces_editor(surface *);

    void addcriticalpoints();
    void addgrid();
    void addfield();
    void addforces();
    void addsurface();
    void animaterotation();
    void BTNaddaxes_clicked();
    void BTNaddcriticalpoints_clicked();
    void BTNaddfieldlines_clicked();
    void BTNaddforces_clicked();
    void BTNanimation_clicked();
    void BTNcpcolor_change();
    void BTNcpcolorfont_clicked();
    void BTNcpeigcolor_change();
    void BTNcplblfont_clicked();
    void BTNcpselectall_clicked();
    void BTNcpselectnone_clicked();
    void BTNflinescolor_clicked();
    void BTNfontcolor_clicked();
    void BTNfont_clicked();
    void BTNfontaxeslabels_clicked();
    void BTNforcescolor_changed();
    void BTNrotation_clicked();
    void BTNselectall_clicked();
    void BTNselectnone_clicked();
    void BTNtranslation_clicked();
    void BTNskeleton_clicked();
    void BTNsymbols_clicked();
    void BTNXaxiscolor_clicked();
    void BTNYaxiscolor_clicked();
    void BTNZaxiscolor_clicked();
    void CHKactiveonly_changed(int);
    void CHKcpactiveonly_changed(int);
    void CHKcpcoords_changed(int);
    void CHKcpeigvec_changed(int);
    void CHKcpindices_changed(int);
    void CHKcps_changed();
    void CHKcpsymbols_changed(int);
    void CHKcpvalues_changed(int);
    void CHKflines_changed();
    void CHKflinesarrows_changed();
    void CHKforces_changed();
    void CHKhideatoms_changed();
    void CHKhidebonds_changed();
    void CHKhidehydrogens_changed();
    void CHKrotate_changed();
    void CHKshowaxes_changed(int);
    void CHKshowaxeslabels_changed(int);
    void CHKshowcoords_changed(int);
    void CHKshowindices_changed(int);
    void CHKshowsymbols_changed(int);
    void closeisosurfeditors();
    void deletegrid(int);
    void deletesurf(int);
    void editmolecule();
    void emitupdatedisplay();
    void emitupdateGL();
    void emitupdateRightMenu();
    void makeStructureBondsandSticks();
    void QDLeditMolecule_close();
    void QDLeditMolecule_delete();
    void QDLeditMolecule_raise();
    void RBTbohr_changed();
    void RBTbohrcoor_changed();
    void RBTbohrcp_changed();
    void readcpsfiles_dialog();
    void readfieldlines_dialog();
    void readforcefiles_dialog();
    void resetinterval();
    void reset_rotation();
    void reset_translation();
    void resizeQDLeditMolecule();
    void rotation_changed();
    void setatomhidden(int);
    void setatomvisible(int);
    void setballradius(qreal);
    void setcylradius(qreal);
    void setdisthressq(qreal);
    void setworld_rotation(QQuaternion);
    void SPBaxesarrowssize_changed(int);
    void SPBaxesarrowswidth_changed(int);
    void SPBaxeslength_changed(int);
    void SPBaxesthickness_changed(int);
    void SPBcoordprecision_changed(int);
    void SPBcpballradius_changed(int);
    void SPBcpcoordprecision_changed(int);
    void SPBcpeigarrowsize_changed(int);
    void SPBcpeigarrowwidth_changed(int);
    void SPBcpeiglength_change(int);
    void SPBcpeigthickness_changed(int);
    void SPBcpprecision_changed(int);
    void SPBcpvshift_changed(int);
    void SPBflinesarrowssep_changed(int);
    void SPBflinesarrowssize_changed(int);
    void SPBflinesarrowswidth_changed(int);
    void SPBflineslinewidth_changed(int);
    void SPBforcesarrowlength_changed(int);
    void SPBforcesarrowwidth_changed(int);
    void SPBforceslength_changed(int);
    void SPBforcesthickness_changed(int);
    void SPBlabelsvshift_changed(int);
    void SPBstepwheel_changed();
    void toggleactive();
    void toggleshowsurf(int);
    void translation_changed();
    void TXTcps_changed();
    void TXTfieldlines_changed();
    void TXTforces_changed();
    void updateeditMoleculeDialog();
    void updateQDLeditMolecule(bool);

private slots:
    void create_axes_widgets_and_layouts();
    void create_critical_points_widgets_and_layouts();
    void create_field_lines_widgets_and_layouts();
    void create_forces_widgets_and_layouts();
    void create_grids_widgets_and_layouts();
    void create_molecular_skeleton_widgets_and_layouts();
    void create_rotation_widgets_and_layouts();
    void create_surfaces_widgets_and_layouts();
    void create_symbols_indices_widgets_and_layouts();
    void create_translation_widgets_and_layouts();
    void make_axes();
    void makeAxesCone(int slices, int stacks, float height);    // Creates a triangle strip for a cone of width 1 and given height
    void makeAxesCylinder(int slices, int stacks);              // Creates a triangle strip for cylinder of radius 1 and height 1
    void makeCylinder(int slices, int stacks, double radius, double height);
    void makeCylinder(int slices, int stacks, double radius, double height, QVector3D center);
    void makeCylinder(int slices, int stacks, double radius, double height, QVector3D center, QVector3D axis);
    void makeCylinder(int slices, int stacks, double radius, double height, QVector3D center, QVector3D axis, QColor color);
    void makeSphere(int slices, int stacks, double radius);
    void makeSphere(int slices, int stacks, double radius, QVector3D center);
    void makeSphere(int slices, int stacks, double radius, QColor color);
    void makeSphere(int slices, int stacks, double radius, QVector3D center, QColor color);


private:
    int getscalevalueInt(float, int, int, float, float);
    double extractContourFromSghName(const std::string& filename);

    QVector <GLuint> coneindices;
    QVector <GLuint> cylinderindices;
    QVector <QVector3D> conevertices;
    QVector <QVector3D> conenormals;
    QVector <QVector3D> cylindervertices;

    bool active = true;                           // If true, the molecule is active for transformations
    bool angstrom = false;                        // If true translation distances displayed in angstrom
    bool angstromcoor = false;                    // If true atoms coordinates displayed in angstrom
    bool angstromcp = false;                      // If true CPs coordinates displayed in angstrom
    bool axes_visible = false;
    bool axeslabels_visible = false;
    bool cpschecked[MAX_CPS] = {};
    bool drawatomcoords = false;                  // If true displays atom coordinates
    bool drawatomindices = false;                 // If true displays atom indices
    bool drawatomsymbols = false;                 // If true displays atom symbols
    bool hideatoms = false;                       // If true, does not display atoms
    bool hidebonds = false;                       // If true, does not display bonds
    bool hidehydrogens = false;                   // If true, does not display hydrogen atoms
    bool iscluster = false;
    bool maxnumver = false;                       // If true, cannot add more vertices in structures
    bool startanimation = false;                  // If true animates rotation
    bool onlyatomactive = false;                  // If true atom labels displayed only for active atoms
    bool rotatex = false;
    bool rotatey = false;
    bool rotatez = false;
    bool scaleradii = true;
    bool visible = true;                          // If true molecule is displayed

    Elements *elem = nullptr;

    float deltaAngles = 4.f;
    float dltinterval = (MAX_INTERVAL-MIN_INTERVAL)/float(INTERVAL_SCALE);
    float interval = MAX_INTERVAL - dltinterval * INTERVAL_INI;
    float stepwheel = 0.1;

    int axesarrowssize = 9;
    int axesarrowswidth = 4;
    int axeslength = 10;
    int axesthickness = 2;
    int coordprecision = 2;
    int labelsvshift = 0;

    struct Sphere {
        QVector3D position;
        float radius;
    };

    ColorButton *BTNcpcolor[MAX_CPS];
    ColorButton *BTNcpcolorfont = nullptr;
    ColorButton *BTNcpeigcolor[3] { nullptr, nullptr, nullptr };
    ColorButton *BTNcpselectall = nullptr;
    ColorButton *BTNcpselectnone = nullptr;
    ColorButton *BTNflinescolor = nullptr;          // Opens dialog for lines color
    ColorButton *BTNfontcolor = nullptr;            // Opens dialog for font color
    ColorButton *BTNforcecolors[MAX_FORCES];
    ColorButton *BTNselectall = nullptr;            // Marks all centers as active for indices display
    ColorButton *BTNselectnone = nullptr;           // Marks all centers as nonactive for indices display
    ColorButton *BTNXaxiscolor = nullptr;           // Dialog for X axis color
    ColorButton *BTNYaxiscolor = nullptr;           // Dialog for Y axis color
    ColorButton *BTNZaxiscolor = nullptr;           // Dialog for Z axis color

    DoubleSpinBox *SPBrot_angle = nullptr;         // Rotation angle
    DoubleSpinBox *SPBrot_x = nullptr;             // x component of rotation axis
    DoubleSpinBox *SPBrot_y = nullptr;             // y component of rotation axis
    DoubleSpinBox *SPBrot_z = nullptr;             // z component of rotation axis
    DoubleSpinBox *SPBstepwheel = nullptr;         // stride for translation with mouse wheel
    DoubleSpinBox *SPBtras_x = nullptr;            // x component of translation vector
    DoubleSpinBox *SPBtras_y = nullptr;            // y component of translation vector
    DoubleSpinBox *SPBtras_z = nullptr;            // z component of translation vector

    editMoleculeDialog *QDLeditMolecule = nullptr;

    myScrollArea *scrollArea = nullptr;

    QCheckBox *CHKactiveonly = nullptr;
    QCheckBox *CHKcpactiveonly = nullptr;
    QCheckBox *CHKcpcoords = nullptr;
    QCheckBox *CHKcpeigvec = nullptr;
    QCheckBox *CHKcpindices = nullptr;
    QCheckBox *CHKcps[MAX_CPS] = {};
    QCheckBox *CHKcpsymbols = nullptr;
    QCheckBox *CHKcpvalues = nullptr;
    QCheckBox *CHKflines = nullptr;
    QCheckBox *CHKflinesarrows = nullptr;
    QCheckBox *CHKforces[MAX_FORCES] = {};
    QCheckBox *CHKhideatoms = nullptr;
    QCheckBox *CHKhidebonds = nullptr;
    QCheckBox *CHKhidehydrogens = nullptr;
    QCheckBox *CHKrotatex = nullptr;
    QCheckBox *CHKrotatey = nullptr;
    QCheckBox *CHKrotatez = nullptr;
    QCheckBox *CHKshowaxes = nullptr;
    QCheckBox *CHKshowaxeslabels = nullptr;
    QCheckBox *CHKshowcoords = nullptr;
    QCheckBox *CHKshowindices = nullptr;
    QCheckBox *CHKshowsymbols = nullptr;

    QColor fontcolor = QColor(255, 172, 0, 255);
    QColor Xaxis_color = QColor(0,255,0);
    QColor Yaxis_color = QColor(0,0,255);
    QColor Zaxis_color = QColor(255,0,0);


    QFont font = QFont("Helvetica", 20, QFont::Bold);
    QFont fontaxeslabels = QFont("Noto Sans", 20, QFont::Bold);

    QGridLayout *layoutgrids = nullptr;

    QGroupBox *FRMaxes = nullptr;
    QGroupBox *FRMcoorunits = nullptr;
    QGroupBox *FRMcps = nullptr;
    QGroupBox *FRMcpeigvec = nullptr;
    QGroupBox *FRMcpunits = nullptr;
    QGroupBox *FRMcriticalpoints = nullptr;
    QGroupBox *FRMfield = nullptr;
    QGroupBox *FRMflinesarrows = nullptr;
    QGroupBox *FRMforces = nullptr;
    QGroupBox *FRMrotation = nullptr;
    QGroupBox *FRMskeleton = nullptr;
    QGroupBox *FRMsymbols = nullptr;
    QGroupBox *FRMtranslation = nullptr;
    QGroupBox *FRMtranslationunits = nullptr;

    QLabel *LBLcoordprecision = nullptr;
    QLabel *LBLcpcoordprecision = nullptr;
    QLabel *LBLcpprecision = nullptr;
    QLabel *LBLcpselect = nullptr;
    QLabel *LBLloadinggrid = nullptr;
    QLabel *LBLselect = nullptr;

    QLineEdit *TXTcps = nullptr;
    QLineEdit *TXTfieldlines = nullptr;
    QLineEdit *TXTforces = nullptr;

    QPoint parentposition = QPoint(200,200);
    QPoint scrollAreaposition = QPoint(200,200);

    QPushButton *BTNaddaxes = nullptr;
    QPushButton *BTNaddcriticalpoints = nullptr;
    QPushButton *BTNaddfieldlines = nullptr;
    QPushButton *BTNaddforces = nullptr;
    QPushButton *BTNaddgrid = nullptr;
    QPushButton *BTNaddsurface = nullptr;
    QPushButton *BTNanimation = nullptr;
    QPushButton *BTNcplblfont = nullptr;
    QPushButton *BTNfont = nullptr;
    QPushButton *BTNfontaxeslabels = nullptr;
    QPushButton *BTNhide = nullptr;
    QPushButton *BTNrotation = nullptr;
    QPushButton *BTNskeleton = nullptr;
    QPushButton *BTNsymbols = nullptr;
    QPushButton *BTNtranslation = nullptr;

    QQuaternion rotation;                 // Quaternion for rotation
    QQuaternion world_rotation;

    QRadioButton *RBTangstrom = nullptr;
    QRadioButton *RBTangstromcoor = nullptr;
    QRadioButton *RBTangstromcp = nullptr;
    QRadioButton *RBTbohr = nullptr;
    QRadioButton *RBTbohrcoor = nullptr;
    QRadioButton *RBTbohrcp = nullptr;

    QSlider *SLDspeed;                    // Speed of rotation animation

    QSpinBox *SPBaxesarrowsize = nullptr;
    QSpinBox *SPBaxesarrowwidth = nullptr;
    QSpinBox *SPBaxeslength = nullptr;
    QSpinBox *SPBaxesthickness = nullptr;
    QSpinBox *SPBcoordprecision = nullptr;
    QSpinBox *SPBcpballradius = nullptr;
    QSpinBox *SPBcpcoordprecision = nullptr;
    QSpinBox *SPBcpeigarrowsize = nullptr;
    QSpinBox *SPBcpeigarrowwidth = nullptr;
    QSpinBox *SPBcpeigthickness = nullptr;
    QSpinBox *SPBcpeiglength = nullptr;
    QSpinBox *SPBcpprecision = nullptr;
    QSpinBox *SPBcpvshift = nullptr;
    QSpinBox *SPBflinesarrowssep = nullptr;             // Arrows separation
    QSpinBox *SPBflinesarrowssize = nullptr;            // Arrows size
    QSpinBox *SPBflinesarrowswidth = nullptr;            // Arrows width
    QSpinBox *SPBflineslinewidth = nullptr;             // Lines width
    QSpinBox *SPBforcesarrowlength = nullptr;
    QSpinBox *SPBforcesarrowwidth = nullptr;
    QSpinBox *SPBforceslength = nullptr;
    QSpinBox *SPBforcesthickness = nullptr;
    QSpinBox *SPBlabelsvshift = nullptr;

    QString fullname;                     // Full geometry file name including path
    QString name;                         // Name for window
    QString path = QString(".");          // Path to molecule home directory (that which contains the file with geometry)
    QString ProjectFolder;
    QString ProjectName;

    QTimer *timer = nullptr;

    QToolButton *BTNcps = nullptr;;
    QToolButton *BTNfieldlines = nullptr;
    QToolButton *BTNforces;

    QVBoxLayout *layoutsurfs = nullptr;

    QVector <QVector3D> positionaxeslabels;
    QVector3D rotationAxis;               // Rotation axis
    QVector3D translation = QVector3D(0,0,0);                // Translation vector

    QVector4D darkenshift = QVector4D(0.3f,0.3f,0.3f,0.f);

    QVector<bool> atomactive;   // Atom active for visualization
    QVector<bool> atomvisible;   // Atom not hidden in the scene
    QVector<int> znuc;          // Atomic number of centers
    QVector<QVector3D > xyz;    // Cartesian coordinates
    QVector<Sphere> atomspheres;

    qreal ballradius = 0.2;                       // Radius of atom spheres
    qreal cylradius = 0.05;                       // Radius of bond cylinders
    qreal disthressq = pow((INIT_BOND_THRESHOLD * ANGSTROM_TO_BOHR),2); // Threshold for bonding

};

#endif // MOLECULE_H
