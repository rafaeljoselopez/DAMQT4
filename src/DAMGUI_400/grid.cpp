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
//	Implementation of class grid
//
//	File:   grid.cpp
//
//	Author: Rafael Lopez    (rafael.lopez@uam.es)
//
//	Last version: April 2018
//
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>#include <QList>
#include <QVBoxLayout>

#include <QFile>
#include <QTextStream>

#include <QtDebug>

#include <float.h>     // For FLT_MAX and FLT_MIN  (highest and lowest float numbers
#include <cmath>       // std::abs

#include "grid.h"

grid::grid(QWidget *parent) : QWidget(parent)
{
    initialposition = QPoint(100,500);
    compatderiv = false;
    fun   = nullptr;
    dxfun = nullptr;
    dyfun = nullptr;
    dzfun = nullptr;
//    cisosurface = nullptr;
    sup_corner_max = QVector3D(0,0,0);
    sup_corner_min = QVector3D(0,0,0);
    nameindex = 0;
    surfcolors << QColor(245,0,0) << QColor(0,0,245) << QColor(0,175,0) <<
                QColor(255,128,0) << QColor(128,0,128) << QColor(0,128,128) <<
                QColor(255,255,0) << QColor(128,128,0) << QColor(255,0,255) <<
                QColor(0,255,255) << QColor(0,170,0) << QColor(0,0,170) <<
                QColor(170,0,170) << QColor(255,220,168) << QColor(192,192,192) <<
                QColor(187,5,27) << QColor(128,0,0) << QColor(0,128,0) <<
                QColor(0,128,128) << QColor(128,0,128) << QColor(128,128,0) <<
                QColor(128,128,128) << QColor(255,255,255) << QColor(0,0,0);
}

grid::~grid(){
    delete fun;
    delete dxfun;
    delete dyfun;
    delete dzfun;

    qDeleteAll(surfaces);
}

QString grid::getfullname(){
    return fullname;
}

QString grid::getname(){
    return name;
}

void grid::addisosurf(){
    surfaces.append(new isosurface());
    surfaces.last()->set_ProjectFolder(ProjectFolder);
    surfaces.last()->set_ProjectName(ProjectName);
    surfaces.last()->setname(name.remove(".plt")+QString(tr("_surf_%1")).arg(nameindex));
    surfaces.last()->setfullname(fullname+QString(tr("_surf_%1")).arg(nameindex++));
    surfaces.last()->setmaxcontourvalue(maxcontourvalue);
    surfaces.last()->setmincontourvalue(mincontourvalue);
    surfaces.last()->setinitialposition(QPoint(getinitialposition())+QPoint(20,100)*(surfaces.count()-1)+QPoint(0,10));
    surfaces.last()->setsurfcolor(this->surfcolors.at((nameindex-1)%surfcolors.count()));
    surfaces.last()->setcompatderiv(compatderiv);
    emit surfaceadded();
}

void grid::deletesurf(int i){
    // All surfaces editors must be closed to prevent crash
    for (int j = 0 ; j < surfaces.length() ; j++){
        surfaces.at(j)->closeeditor();
    }
    delete surfaces.at(i);
    surfaces.removeAt(i);
    emit surfacedeleted();
}

void grid::copyMeshToSurface(const CIsoSurface<float>& iso,
                             isosurface* surface)
{
    if (!surface)
        return;

//    surface->allvertices.clear();
//    surface->allvertices.reserve(static_cast<int>(iso.m_nVertices));

    surface->clearGeometry();
    surface->reserveVertices(static_cast<int>(iso.m_nVertices));

    const QColor surfaceColor = surface->getsurfcolor();
    const float opacity = surface->getopacity();

    const QVector4D vertexColor(
        surfaceColor.redF(),
        surfaceColor.greenF(),
        surfaceColor.blueF(),
        opacity
    );

    const QVector3D displacement = sup_corner_min;

    for (unsigned int i = 0; i < iso.m_nVertices; ++i) {
        VertexNormalData vertex;

        vertex.position = QVector3D(
            iso.m_ppt3dVertices[i][0],
            iso.m_ppt3dVertices[i][1],
            iso.m_ppt3dVertices[i][2]
        ) + displacement;

        vertex.normal = QVector3D(
            iso.m_pvec3dNormals[i][0],
            iso.m_pvec3dNormals[i][1],
            iso.m_pvec3dNormals[i][2]
        );

        vertex.color = vertexColor;

//        surface->allvertices.append(vertex);

        surface->addVertex(vertex);
    }

    const unsigned int indexCount = 3 * iso.m_nTriangles;

//    surface->allindices.clear();
//    surface->allindices.reserve(static_cast<int>(indexCount));

    surface->reserveIndices(static_cast<int>(indexCount));

    for (unsigned int i = 0; i < indexCount; ++i)
//        surface->allindices.append(iso.m_piTriangleIndices[i]);

        surface->addIndex(iso.m_piTriangleIndices[i]);
}


void grid::generatesurf(int i)
{
    auto* surface = surfaces.at(i);

    CIsoSurface<float> iso;

    const float cellSizes[3] = {
        fun->voxel_x() / (fun->x() - 1),
        fun->voxel_y() / (fun->y() - 1),
        fun->voxel_z() / (fun->z() - 1)
    };

    if (compatderiv && surface->getnormalgrad()) {
        iso.GenerateSurfacewithgrad(
            fun->data(),
            dxfun->data(),
            dyfun->data(),
            dzfun->data(),
            surface->getcontourvalue(),
            fun->x() - 1,
            fun->y() - 1,
            fun->z() - 1,
            cellSizes[0],
            cellSizes[1],
            cellSizes[2]
        );
    } else {
        iso.GenerateSurface(
            fun->data(),
            surface->getcontourvalue(),
            fun->x() - 1,
            fun->y() - 1,
            fun->z() - 1,
            cellSizes[0],
            cellSizes[1],
            cellSizes[2]
        );
    }

    if (!iso.IsSurfaceValid()) {
        QMessageBox::warning(
            this,
            tr("generatesurf"),
            tr("Invalid surface")
        );
        return;
    }

    if (iso.m_nVertices != iso.m_nNormals) {
        QMessageBox::warning(
            this,
            tr("generatesurf"),
            tr("Number of vertices (%1)\n"
               "does not coincide with the number of normals (%2).")
                .arg(iso.m_nVertices)
                .arg(iso.m_nNormals)
        );
        return;
    }

    copyMeshToSurface(iso, surface);

    const float bounds[6] = {
        sup_corner_min.x(),
        sup_corner_max.x(),
        sup_corner_min.y(),
        sup_corner_max.y(),
        sup_corner_min.z(),
        sup_corner_max.z()
    };

    surface->generategridbounds(bounds);
}

QPoint grid::getinitialposition(){
    return initialposition;
}

float grid::getmaxcontourvalue(){
    return maxcontourvalue;
}

float grid::getmincontourvalue(){
    return mincontourvalue;
}

bool grid::loadnormals(FILE *f1, FILE *f2, FILE *f3, VVBuffer*v1, VVBuffer*v2, VVBuffer*v3,
        int *iref, float *vref, int kntbar, QProgressBar *bar){
    bool fdouble;
    int iaux[3];
//    Reads header of file f1 and checks compatibility
    fread( iaux , sizeof(int) , 2 , f1);     // reads two integer values
    if (iaux[0] == 0){   // if the first one is zero: grid data in double precision
        fdouble = true;
    }
    else{                // elsel grid data: float
        fdouble = false;
    }
    fread( iaux , sizeof(int) , 3 , f1);     // reads nz, ny , nx (in this order)
    if (iaux[0] != iref[0] || iaux[1] != iref[1] || iaux[2] != iref[2])
        return false;
//    Reads header of file f2 and checks compatibility
    fread( iaux , sizeof(int) , 2 , f2);     // reads two integer values
    if ((iaux[0] == 0 && !fdouble) || (iaux[0] != 0 && fdouble)){   // if the first one is zero: grid data in double precision
        return false;
    }
    fread( iaux , sizeof(int) , 3 , f2);     // reads nz, ny , nx (in this order)
    if ((iaux[0] != iref[0]) || (iaux[1] != iref[1]) || (iaux[2] != iref[2]))
        return false;
//    Reads header of file f3 and checks compatibility
    fread( iaux , sizeof(int) , 2 , f3);     // reads two integer values
    if ((iaux[0] == 0 && !fdouble) || (iaux[0] != 0 && fdouble)){   // if the first one is zero: grid data in double precision
        return false;
    }
    fread( iaux , sizeof(int) , 3 , f3);     // reads nz, ny , nx (in this order)
    if ((iaux[0] != iref[0]) || (iaux[1] != iref[1]) || (iaux[2] != iref[2]))
        return false;

    float vaux[6];
    if (fdouble){
        double dvaux[6];
//      Reads grid dimensions from file f1 and checks compatibility
        fread(dvaux , sizeof(double) , 6 , f1);
        for(int i = 0 ; i < 6 ; i++){
            vaux[i] = (float)dvaux[i];
        }
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads grid dimensions from file f2 and checks compatibility
        fread(dvaux , sizeof(double) , 6 , f2);
        for(int i = 0 ; i < 6 ; i++){
            vaux[i] = (float)dvaux[i];
        }
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads grid dimensions from file f3 and checks compatibility
        fread(dvaux , sizeof(double) , 6 , f3);
        for(int i = 0 ; i < 6 ; i++){
            vaux[i] = (float)dvaux[i];
        }
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads gradient components, normalizes gradient and stores normalized components
        float *dp1 = v1->data();
        float *dp2 = v2->data();
        float *dp3 = v3->data();
        double *buff1 = new double[nx];
        double *buff2 = new double[nx];
        double *buff3 = new double[nx];
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                fread(buff1, sizeof(double), nx, f1);
                fread(buff2, sizeof(double), nx, f2);
                fread(buff3, sizeof(double), nx, f3);
                for (int k = 0; k < nx; k++) {
                    double normalization = sqrt(buff1[k]*buff1[k]+buff2[k]*buff2[k]+buff3[k]*buff3[k]);
                    if (normalization > 0.)
                        normalization = 1. / normalization;
                    *dp1++ = (float)(buff1[k] * normalization);
                    *dp2++ = (float)(buff2[k] * normalization);
                    *dp3++ = (float)(buff3[k] * normalization);
                }
                bar->setValue(i*nx+j*nx*ny + kntbar*nx*ny*nz);
            }
        }
        return true;
    }
    else{
//      Reads grid dimensions from file f1 and checks compatibility
        fread(vaux , sizeof(float) , 6 , f1);
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads grid dimensions from file f2 and checks compatibility
        fread(vaux , sizeof(float) , 6 , f2);
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads grid dimensions from file f3 and checks compatibility
        fread(vaux , sizeof(float) , 6 , f3);
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
//      Reads gradient components, normalizes gradient and stores normalized components         ;
        float *dp1 = v1->data();
        float *dp2 = v2->data();
        float *dp3 = v3->data();
        float *buff1 = new float[nx];
        float *buff2 = new float[nx];
        float *buff3 = new float[nx];
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                fread(buff1, sizeof(float), nx, f1);
                fread(buff2, sizeof(float), nx, f2);
                fread(buff3, sizeof(float), nx, f3);
                for (int k = 0; k < nx; k++) {
                    double normalization = sqrt(buff1[k]*buff1[k]+buff2[k]*buff2[k]+buff3[k]*buff3[k]);
                    if (normalization > 0.)
                        normalization = 1. / normalization;
                    *dp1++ = (float)(buff1[k] * normalization);
                    *dp2++ = (float)(buff2[k] * normalization);
                    *dp3++ = (float)(buff3[k] * normalization);
                }
                bar->setValue(i*nx+j*nx*ny + kntbar*nx*ny*nz);
            }
        }
        return true;
    }
}

bool grid::loadderivnew(QFile *inputfile, VVBuffer*v, int *iref, float *vref, int kntbar, QProgressBar *bar){
    bool fdouble;
    int iaux[3];
    QDataStream data(inputfile);
    for (int i = 0 ; i < 2 ; i++){
        QByteArray bar;
        bar = inputfile->read(sizeof(int));
        memcpy(&iaux[i], bar.constData(), sizeof(int));
//        qDebug() << "en loadderivnew: iaux[" << i << "] = " << iaux[i];
    }
    if (iaux[0] == 0){   // if the first one is zero: grid data in double precision
        fdouble = true;
    }
    else{                // elsel grid data: float
        fdouble = false;
    }
    for (int i = 0 ; i < 3 ; i++){
        QByteArray bar;
        bar = inputfile->read(sizeof(int));
        memcpy(&iaux[i], bar.constData(), sizeof(int));
//        qDebug() << "iaux[" << i << "] = " << iaux[i];
    }
    if (iaux[0] != iref[0] || iaux[1] != iref[1] || iaux[2] != iref[2])
        return false;
    float vaux[6];
    QByteArray bytar;
    if (fdouble){
        double dvaux[6];
        for (int i = 0 ; i < 6 ; i++){
            bytar = inputfile->read(sizeof(double));
            memcpy(&dvaux[i], bytar.constData(), sizeof(double));
            vaux[i] = (float)dvaux[i];
//            qDebug() << "dvaux[" << i << "] = " << dvaux[i];
        }
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
        float *dp = v->data();
        double val;
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                for (int k = 0; k < nx; k++){
                    bytar = inputfile->read(sizeof(double));
                    memcpy(&val, bytar.constData(), sizeof(double));
                    *dp++ = (float)val;
                }
                bar->setValue(i*nx+j*nx*ny + kntbar*nx*ny*nz);
            }
        }
        return true;
    }
    else{
        for (int i = 0 ; i < 6 ; i++){
            bytar = inputfile->read(sizeof(float));
            memcpy(&vaux[i], bytar.constData(), sizeof(float));
//            qDebug() << "dvaux[" << i << "] = " << dvaux[i];
        }
        if (std::abs(vaux[0]-vref[0]) + std::abs(vaux[1]-vref[1]) + std::abs(vaux[2]-vref[2]) + std::abs(vaux[3]-vref[3])
                + std::abs(vaux[4]-vref[4]) + std::abs(vaux[5]-vref[5]) > 1.e-5){
            return false;
        }
        float *dp = v->data();
        float val;
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                for (int k = 0; k < nx; k++){
                    bytar = inputfile->read(sizeof(float));
                    memcpy(&val, bytar.constData(), sizeof(float));
                    *dp++ = val;
                }
                bar->setValue(i*nx+j*nx*ny + kntbar*nx*ny*nz);
            }
        }
        return true;
    }
}


bool grid::readpltnew(QString fileName){
    const double factor=0.529177249; // units conversion factor

    int  iaux[3]; //iaux[0]=z, iaux[1]=y, iaux[2]=x
    bool fdouble;
    bool existderivs;
    QString filename = fileName;
    QFile inputfile(filename);
    QFile inputfiledx(filename.remove(".plt")+"-dx.pltd");
    QFile inputfiledy(filename+"-dy.pltd");
    QFile inputfiledz(filename+"-dz.pltd");
//    qDebug() << "inputfile = " << filename;
//    qDebug() << "inputfiledx = " << filename+"-dx.pltd";
//    qDebug() << "inputfiledy = " << filename+"-dy.pltd";
//    qDebug() << "inputfiledz = " << filename+"-dz.pltd";

    compatderiv = true;

    if (!inputfile.open(QIODevice::ReadOnly)){
//        qDebug() << "No puede abrir " << filename+".plt";
        return false;
    }
//    qDebug() << filename+".plt" << "opened";
    existderivs = true;
    if (!inputfiledx.open(QIODevice::ReadOnly)){
//        qDebug() << "No puede abrir " << filename+"-dx.pltd";
        existderivs = false;
    }
//    qDebug() << filename+"-dx.pltd" << "opened";
    if (!inputfiledy.open(QIODevice::ReadOnly)){
//        qDebug() << "No puede abrir " << filename+"-dy.pltd";
        existderivs = false;
    }
//    qDebug() << filename+"-dy.pltd" << "opened";
    if (!inputfiledz.open(QIODevice::ReadOnly)){
//        qDebug() << "No puede abrir " << filename+"-dz.pltd";
        existderivs = false;
    }
//    qDebug() << filename+"-dz.pltd" << "opened";

//    Read file with function

    QDataStream data(&inputfile);
    for (int i = 0 ; i < 2 ; i++){
        QByteArray bar;
        bar = inputfile.read(sizeof(int));
        memcpy(&iaux[i], bar.constData(), sizeof(int));
//        qDebug() << "iaux[" << i << "] = " << iaux[i];
    }
    if (iaux[0] == 0)   // if the first one is zero: grid data in double precision
        fdouble = true;
    else                // elsel grid data: float
        fdouble = false;

    for (int i = 0 ; i < 3 ; i++){
        QByteArray bar;
        bar = inputfile.read(sizeof(int));
        memcpy(&iaux[i], bar.constData(), sizeof(int));
//        qDebug() << "iaux[" << i << "] = " << iaux[i];
    }
    nx=iaux[2]; ny=iaux[1]; nz=iaux[0];
    if ( nx < 0 || ny < 0 || nz < 0) {
        QMessageBox msgBox;
        msgBox.setText(tr("readplt"));
        msgBox.setInformativeText(tr("Error: wrong dimensions"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return false;
    }
//    qDebug() << "nx = " << nx << "ny = " << ny << "nz = " << nz;

    QWidget *win;
    win = new QWidget(this);
    win->setAutoFillBackground(true);
    win->setFixedSize(320,50);
    win->setWindowTitle(tr("Loading files"));
    win->raise();
    QVBoxLayout* layout;
    layout = new QVBoxLayout(this);
    QLabel *label;
    label = new QLabel(tr("Loading files"), win);
    QProgressBar *bar;
    bar = new QProgressBar(win);
    bar->resize(300,25);
    bar->setOrientation(Qt::Horizontal);	//Orientation can be vertical too
    bar->setMinimumWidth(300);
    bar->setMaximumWidth(300);
    bar->setMinimum(0);
    if (existderivs)
        bar->setMaximum(4*nx*ny*nz);
    else
        bar->setMaximum(nx*ny*nz);
    layout->addWidget(label,Qt::AlignCenter);
    layout->addWidget(bar,Qt::AlignCenter);
    win->setLayout(layout);
    win->show();
    QByteArray fileNameUtf8 = (filename + ".plt").toUtf8();
    const char *file = fileNameUtf8.constData();
    if (!fun)   fun   = new VVBuffer(file, nx, ny, nz);
    float *dp = fun->data();
    float min = FLT_MAX, max = FLT_MIN;
    float vaux[6];
    QByteArray bytar;
    if (fdouble){
        double dvaux[6];
        for (int i = 0 ; i < 6 ; i++){
            bytar = inputfile.read(sizeof(double));
            memcpy(&dvaux[i], bytar.constData(), sizeof(double));
            vaux[i] = (float)dvaux[i];
//            qDebug() << "dvaux[" << i << "] = " << dvaux[i];
        }
    }else{
        QByteArray bytar;
        for (int i = 0 ; i < 6 ; i++){
            bytar = inputfile.read(sizeof(float));
            memcpy(&vaux[i], bytar.constData(), sizeof(float));
//            qDebug() << "dvaux[" << i << "] = " << dvaux[i];
        }
    }
    fun->set_voxel_size((vaux[5]-vaux[4])/factor, (vaux[3]-vaux[2])/factor, (vaux[1]-vaux[0])/factor);
    if (fdouble){
        double val;
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                for (int k = 0; k < nx; k++){
                    bytar = inputfile.read(sizeof(double));
                    memcpy(&val, bytar.constData(), sizeof(double));
                    *dp++ = (float)val;
                    if (max < val) max = val;
                    if (min > val) min = val;
                }
                bar->setValue(i*nx+j*nx*ny);
            }
        }
    }
    else{
        float val;
        for (int j = 0; j < nz; j++){
            for (int i = 0; i < ny; i++) {
                for (int k = 0; k < nx; k++){
                    bytar = inputfile.read(sizeof(float));
                    memcpy(&val, bytar.constData(), sizeof(float));
                    *dp++ = val;
                    if (max < val) max = val;
                    if (min > val) min = val;
                }
                bar->setValue(i*nx+j*nx*ny);
            }
        }
    }

    sup_corner_min.setX(vaux[4]/factor);
    sup_corner_min.setY(vaux[2]/factor);
    sup_corner_min.setZ(vaux[0]/factor);
    sup_corner_max.setX(vaux[5]/factor);
    sup_corner_max.setY(vaux[3]/factor);
    sup_corner_max.setZ(vaux[1]/factor);
    setmaxcontourvalue(max);
    setmincontourvalue(min);

    compatderiv = existderivs;

//    Read files with function derivatives (gradient), and computes and stores them for normals interpolation
    if (existderivs){
        QByteArray fileNameUtf8 = (filename + "-dx.pltd").toUtf8();
        const char *filedx = fileNameUtf8.constData();
        if (!dxfun)   dxfun   = new VVBuffer(filedx, nx, ny, nz);
        compatderiv = loadderivnew(&inputfiledx, dxfun, iaux, vaux, 1, bar);
        if (compatderiv){
            QByteArray fileNameUtf8 = (filename + "-dy.pltd").toUtf8();
            const char *filedy = fileNameUtf8.constData();
            if (!dyfun)   dyfun   = new VVBuffer(filedy, nx, ny, nz);
            compatderiv = loadderivnew(&inputfiledy, dyfun, iaux, vaux, 1, bar);
        }
        if (compatderiv){
            QByteArray fileNameUtf8 = (filename + "-dz.pltd").toUtf8();
            const char *filedz = fileNameUtf8.constData();
            if (!dzfun)   dzfun   = new VVBuffer(filedz, nx, ny, nz);
            compatderiv = loadderivnew(&inputfiledz, dzfun, iaux, vaux, 1, bar);
        }
    }

    return true;
}

void grid::setinitialposition(QPoint a){
    initialposition = a;
}

void grid::setmaxcontourvalue(float a){
    maxcontourvalue = a;
}

void grid::setmincontourvalue(float a){
    mincontourvalue = a;
}

void grid::setfullname(QString a){
    fullname = a;
}

void grid::setname(QString a){
    name = a;
}

void grid::set_ProjectFolder(QString name){
    ProjectFolder = name;
}

void grid::set_ProjectName(QString name){
    ProjectName = name;
}

void grid::toggleshowsurf(int i){
    surfaces.at(i)->toggleshowsurf();
}
