#ifndef IXYZTABULATIONPAGE_H
#define IXYZTABULATIONPAGE_H

#include <QString>


class ILinesTabulationPage
{
public:
    virtual ~ILinesTabulationPage() = default;

    virtual int linesUVTableRows() const = 0;
    virtual int linesXYZTableRows() const = 0;
    virtual QString getUVCellValue(int i, int j) const = 0;
    virtual QString getXYZCellValue(int i, int j) const = 0;
    virtual QString tabularkey() const = 0;
    virtual QString numtabular() const = 0;
};

class IXYZTabulationPage
{
public:
    virtual ~IXYZTabulationPage() = default;

    virtual int xyzTableRows() const = 0;
    virtual QString getCellValue(int i, int j) const = 0;
    virtual QString tabularkey() const = 0;
    virtual QString numtabular() const = 0;
};

#endif
