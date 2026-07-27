#ifndef IGRIDOPTIONSPAGE_H
#define IGRIDOPTIONSPAGE_H

#include <QString>

class IGridOptionsPage
{
public:
    virtual ~IGridOptionsPage() = default;

    virtual void setGrid2D(bool) = 0;
    virtual bool is2DGrid() const = 0;
    virtual bool is3DGrid() const = 0;

    virtual void setLowResolution(bool) = 0;
    virtual void setMediumResolution(bool) = 0;
    virtual void setHighResolution(bool) = 0;
    virtual void setCustomResolution(bool) = 0;

    virtual void setDeltaU(double) = 0;
    virtual void setDeltaV(double) = 0;
    virtual void setDeltaX(double) = 0;
    virtual void setDeltaY(double) = 0;
    virtual void setDeltaZ(double) = 0;

    virtual void setLowResolutionTip(const QString&) = 0;
    virtual void setMediumResolutionTip(const QString&) = 0;
    virtual void setHighResolutionTip(const QString&) = 0;

    virtual double deltaU() const = 0;
    virtual double deltaV() const = 0;
    virtual double deltaX() const = 0;
    virtual double deltaY() const = 0;
    virtual double deltaZ() const = 0;

    virtual QString uMinValue() const = 0;
    virtual QString uMaxValue() const = 0;
    virtual QString vMinValue() const = 0;
    virtual QString vMaxValue() const = 0;
    virtual QString xMinValue() const = 0;
    virtual QString xMaxValue() const = 0;
    virtual QString yMinValue() const = 0;
    virtual QString yMaxValue() const = 0;
    virtual QString zMinValue() const = 0;
    virtual QString zMaxValue() const = 0;

    virtual void setPointsX(int) = 0;
    virtual void setPointsY(int) = 0;
    virtual void setPointsZ(int) = 0;
    virtual void setPointsUV(int) = 0;

    virtual bool isLowResolution() const = 0;
    virtual bool isMediumResolution() const = 0;
    virtual bool isHighResolution() const = 0;
    virtual bool isCustomResolution() const = 0;

    virtual int pointsX() const = 0;
    virtual int pointsY() const = 0;
    virtual int pointsZ() const = 0;
    virtual int pointsUV() const = 0;
};

#endif
