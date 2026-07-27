#ifndef FILEDIALOGUTILS_H
#define FILEDIALOGUTILS_H

#include <QString>

class QFileDialog;

namespace FileDialogUtils
{
    QString initialDirectory(const QString& preferredDirectory);

    void configureLocalFileDialog(QFileDialog& fileDialog);
}

#endif // FILEDIALOGUTILS_H
