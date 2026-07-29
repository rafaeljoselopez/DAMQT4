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
#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QPoint>
#include <QProcess>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QToolButton>
#include <QVector>
#include <QVector3D>
#include <QVector4D>

#include "ColorButton.h"
#include "widgetsubclasses.h"
#include "VertexNormalData.h"

#ifndef pi
#define pi 3.14159265358979323846  // pi
#endif

#ifndef LOGBASE
#define LOGBASE 1.08                      // base for logaritmics scale
#endif


class editIsoSurfaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit editIsoSurfaceDialog(QWidget *parent = nullptr);
    ~editIsoSurfaceDialog() override = default;
signals:
    void closed();
protected:
    void closeEvent(QCloseEvent *event) override;
    virtual void reject() override;
};

// Class surface
//
class isosurface : public QWidget
{
    friend class geomProcessor;
    Q_OBJECT
public:
    explicit isosurface(QWidget *parent = nullptr);
    ~isosurface() override = default;

    void clearGeometry();

   void reserveVertices(int count);
   void reserveIndices(int count);

   void shiftVertexColors(const QVector4D &shift);

   void addVertex(const VertexNormalData &vertex);
   void addIndex(GLuint index);

   const QVector<VertexNormalData> &vertices() const;
   const QVector<GLuint> &indices() const;

    bool isvisible();
    bool getnormalgrad();
    bool getshowgridbound();
    bool gettranslucence();
    bool getsolidsurf();

    int getscalevalueInt(float, int, int, float, float, bool);

    float getcontourvalue();
    float getmaxcontourvalue();
    float getmincontourvalue();
    float getopacity();
    float getscalevalueFloat(int, int, int, float, float, bool);

    QColor getsurfcolor();

    QPoint getinitialposition();

    QString get_basename();
    QString get_execName(QString processname, QString subdir);
    QString getfullname();
    QString getname();

    QVector3D gettranslation();
    QVector3D getrotationAxis();

    QVector <GLuint> getallindices();
    QVector <VertexNormalData> getallvertices();
    QVector <GLuint> gridindices;             // indices of vertices in grid boundaries
    QVector <GLuint> gridindicesoffset;       // Offsets of indices in grid boundaries
    QVector <VertexNormalData> gridvertices;  // vertices of triangles in grid boundaries (position, normal, color)

    void generategridbounds(const float *);
    void setcompatderiv(bool);
    void setcontourvalue(float);
    void setinitialposition(QPoint);
    void setmaxcontourvalue(float);
    void setmincontourvalue(float);
    void setfullname(QString);
    void setname(QString);
    void setopacity(float);
    void set_ProjectFolder(QString);
    void set_ProjectName(QString);
    void setsolidsurf(bool);
    void setsurfcolor(QColor);
    void settranslucence(bool);
    void setvisible(bool);

signals:
    void generatesurface();
    void opendialog();
    void updatedisplay();
    void updatelabelcolor(QString);
    void updateRightMenu();
    void updatetext(QString);

public slots:
//    bool QDLeditSurface_isVisible();

    togglingGroupBox * editisosurface();

    void toggleshowsurf();
    void BTNexec_clicked();
    void BTNsurfcolor_clicked();
    void CHKmpi_changed(int state);
    void CHKnormalgrad_changed();
    void CHKshowgrid_changed();
    void CHKtranslucence_changed();
    void closeeditor();
    void emitupdateRightMenu();
    void processError(QProcess::ProcessError error);
    void processOutput(int exitCode, QProcess::ExitStatus exitStatus);
    void processStart();
    void processStop();
    void RBTscale_changed();
    void RBTsurftype_changed();
    void SLDcontourvalue_changed(int);
    void SLDcontourvalue_released();
    void SLDopacity_changed(int);
    void SLDopacity_released();
    void SPBmpi_changed(int nprocessors);
    void SPBsensitive_changed(int);
    void TXTcontourvalue_changed();

private:
    bool active;                        // If true, the surface is active for transformations
    bool compatderiv;
    bool editoropen;
    bool isdensity;
    bool normalgrad;                    // If ture, computes normals from inerpolated gradient
    bool solidsurf;                     // If true, surface display is solid, if false, surface display is wire frame
    bool showgridbound;                 // If true displays grid boundaries
    bool translucence;                  // If true, translucence correction is applied
    bool visible;                       // If true the surface is displayed

    int logdlt = 3;
    int nprocessors = 1;

    QVector <GLuint> allindices;             // Indices of vertices in surface
    QVector <VertexNormalData> allvertices;  // Vertices of triangles in surface (position, normal, color)

    QVector<float> griddimensions;      // Original grid dimensions: xmin, xmax, ymin, ymax, zmin, zmax
    QVector<int> gridnxyz;              // Original grid number of points (nx, ny, nz)


    float contourvalue = 0.0f;                   // Function value for isosurface
    float maxcontourvalue = 1.0f;                // Highest contourvalue available
    float mincontourvalue = -1.0f;               // Lowest contourvalue available
    float opacity = 1.0f;                        // Opacity: 1 (opaque) 0 (transparent)

    QColor surfcolor;

    QPoint initialposition;

    ColorButton *BTNsurfcolor = nullptr;            // Opens dialog for surface color

    LineEdit *TXTcontourvalue = nullptr;

    QCheckBox *CHKmpi = nullptr;
    QCheckBox *CHKnormalgrad = nullptr;
    QCheckBox *CHKshowgrid = nullptr;
    QCheckBox *CHKtranslucence = nullptr;

    QDoubleSpinBox *SPBopacity = nullptr;           // Surface opacity/transparency

    QDoubleValidator *myDoubleValidator = nullptr;

    QGroupBox *FRMhighquality = nullptr;
    QGroupBox *FRMsurfcolor = nullptr;
    QGroupBox *FRMsurftype = nullptr;

    togglingGroupBox *FRMisosurface = nullptr;

    QLabel *LBLalpha = nullptr;
    QLabel *LBLcontourvalue = nullptr;
    QLabel *LBLfilename = nullptr;
    QLabel *LBLmpi = nullptr;
    QLabel *LBLopacity = nullptr;
    QLabel *LBLscale = nullptr;
    QLabel *LBLsensitive = nullptr;
    QLabel *LBLstatus = nullptr;

    QLineEdit *TXTisosurffilename = nullptr;

    QProcess *myProcess = nullptr;

    QPushButton *BTNexec = nullptr;
    QPushButton *BTNstop = nullptr;

    QRadioButton *RBTscalelin = nullptr;
    QRadioButton *RBTscalelog = nullptr;
    QRadioButton *RBTsolidsurf = nullptr;           // Solid surface
    QRadioButton *RBTwiresurf = nullptr;

    QSlider *SLDcontourvalue = nullptr;
    QSlider *SLDopacity = nullptr;

    QSpinBox *SPBmpi = nullptr;
    QSpinBox *SPBsensitive = nullptr;

    QString basename = nullptr;
    QString fullname = nullptr;                     // Full name for surface including path
    QString name = nullptr;                        // Name for surface
    QString processname = nullptr;
    QString ProjectFolder = nullptr;
    QString ProjectName = nullptr;
};

#endif // ISOSURFACE_H
