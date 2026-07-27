#include "filedialogutils.h"

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QUrl>

QString FileDialogUtils::initialDirectory(
    const QString& preferredDirectory)
{
    QString directory = preferredDirectory.trimmed();

    if (directory.startsWith(
            QStringLiteral("file:"),
            Qt::CaseInsensitive)) {
        const QUrl url(directory);

        if (url.isLocalFile())
            directory = url.toLocalFile();
    }

    directory = QDir::cleanPath(directory);

    const QFileInfo info(directory);

    if (info.isFile())
        directory = info.absolutePath();

    if (directory.isEmpty() || !QDir(directory).exists())
        directory = QDir::homePath();

    return directory;
}

void FileDialogUtils::configureLocalFileDialog(
    QFileDialog& fileDialog)
{
    fileDialog.setWindowFlags(
        fileDialog.windowFlags() | Qt::WindowStaysOnTopHint
    );

    QList<QUrl> sidebarUrls;

    sidebarUrls
        << QUrl::fromLocalFile(QDir::homePath())
        << QUrl::fromLocalFile(QStringLiteral("/"));

    fileDialog.setSidebarUrls(sidebarUrls);
}
