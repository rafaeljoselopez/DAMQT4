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
//    Class defining the functions for reading/writing options from/to .damproj files.
//
//  File:   readwriteoptions.cpp
//
//      Last version: March 2026
//
#include "externalprogramrunner.h"
#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>
#include <QFileInfo>
#include <QSizePolicy>

#include <string>

ExternalProgramRunner::ExternalProgramRunner(QObject *parent)
    : QObject(parent)
{
}

bool ExternalProgramRunner::start(const RunRequest &request)
{
    if (process_ && process_->state() != QProcess::NotRunning) {
//        emit messageChanged(tr("A process is already running."));
        QMessageBox::warning(nullptr, tr("DAMQT"),tr("Process %1 already running").arg(request.rootName));
        return false;
    }

    QString program;
    QStringList args;

    const QString suffix = request.runMpi ? "_mpi" : "";
    const QString processName = request.rootName + suffix + ".exe";
    const QString execName =
        getExecutableName(processName, request.subdir + suffix);

    if (execName.isEmpty()) {
        return false;
    }

    currentProgramName_ = request.rootName + suffix;

    if (request.runMpi) {
        if (request.isWindows) {
            const QString wrapperScript =
                QDir(QCoreApplication::applicationDirPath()).filePath("run_mpi.sh");

            program = "bash";

            args << wrapperScript
                 << QString::number(request.nprocs)
                 << execName
                 << request.stdInput;
        } else {
            QStringList mpiCommandParts = QProcess::splitCommand(mpiCommand_);

            if (mpiCommandParts.isEmpty()) {
                return false;
            }

            program = mpiCommandParts.takeFirst();
            args << mpiCommandParts;

            args << "-np"
                 << QString::number(request.nprocs);

            if (!mpiFlags_.trimmed().isEmpty()) {
                args << QProcess::splitCommand(mpiFlags_);
            }

            args << execName;
        }
    } else {
        program = execName;
    }

    const QString stdOutput = buildOutputFileName(request);

    process_ = new QProcess(this);

    process_->setStandardInputFile(request.stdInput);
    process_->setStandardOutputFile(stdOutput, QIODevice::Truncate);
    process_->setStandardErrorFile(stdOutput, QIODevice::Append);

    connect(process_, &QProcess::started,
            this, [this]() {
                emit started();
            });

    connect(process_, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                emit errorOccurred(error);
            });

    connect(process_,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                emit finished(exitCode, exitStatus);

                if (process_) {
                    process_->deleteLater();
                    process_ = nullptr;
                }

                processKilled_ = false;
                currentProgramName_.clear();
            });

    process_->start(program, args);

    return true;
}

void ExternalProgramRunner::stop()
{
    if (!process_) {
        return;
    }

#if defined(Q_OS_WIN)
    process_->kill();
    processKilled_ = true;
#else
    if (currentProgramName_.contains("_mpi")) {
        killMpiProcessesIfNeeded();
    }
    if (!process_) {
        return;
    }
    process_->kill();
    processKilled_ = true;
#endif
}


QString ExternalProgramRunner::getExecutableName(const QString &processname, const QString &subdir) const
{
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

QString ExternalProgramRunner::buildOutputFileName(const RunRequest &request) const
{
    if (!request.stdOutput.isEmpty()) {
        return request.stdOutput;
    }

//    const QString suffix = request.runMpi ? "_mpi" : "";

//    const QString fileName =
//        request.outputPrefix + "-" + request.rootName + suffix + request.suffix + ".out";
    const QString fileName =
        request.outputPrefix + "-" + request.rootName + request.suffix + ".out";

    return QDir(projectFolder_).filePath(fileName);
}

void ExternalProgramRunner::killMpiProcessesIfNeeded()
{
#if !defined(Q_OS_WIN)

    if (currentProgramName_.isEmpty()) {
        return;
    }

    QProcess getProcesses;
    getProcesses.start("pgrep", QStringList() << "-f" << currentProgramName_);
    getProcesses.waitForFinished();

    const QString procString =
        QString::fromLocal8Bit(getProcesses.readAllStandardOutput());

    const QStringList procList =
        procString.split('\n', Qt::SkipEmptyParts);

    if (procList.isEmpty()) {
        return;
    }

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("DAMQT"));

    msgBox.setInformativeText(
        tr("Do you want to kill all processes named %1?\n"
           "The following processes will be killed: %2")
            .arg(currentProgramName_,
                 procString.simplified())
    );

    msgBox.setStandardButtons(QMessageBox::Yes |
                              QMessageBox::No |
                              QMessageBox::Cancel);

    msgBox.setDefaultButton(QMessageBox::Cancel);

    const int ret = msgBox.exec();

    if (ret != QMessageBox::Yes) {
        return;
    }

    for (const QString &pid : procList) {
        QProcess killProcess;
        killProcess.start("kill", QStringList() << "-9" << pid);
        killProcess.waitForFinished();
    }

#endif
}


//    Creates a suitable input file from options file (*.damproj)
//void ExternalProgramRunner::inputdatafile(const QString &suffix,
//                                          const QString &section,
//                                          const QString &fullFileName,
//                                          const QString &projectDir,
//                                          const QString &projectName)
//{
//    QString fileoutstr = projectDir+projectName+"-"+suffix;
//    std::string v;
//    // Opens file .damproj for reading options
//    QFile fileinp(fullFileName);
//    if (!fileinp.isOpen()){
//        fileinp.open(QFile::ReadOnly);
//    }
////      Opens file to be filled with suitable input file pointed by suffix
//    QFile fileout(fileoutstr);

//    if (!fileout.isOpen()){
//        fileout.open(QFile::Text | QFile::WriteOnly);
//    }
//    QTextStream outfile(&fileout); // Buffer for writing to fileout
//    QByteArray buff = ReadSectionOptions(section, &fileinp);
//    outfile << buff;
//    std::string vproj = "\"" + projectDir.toStdString() + projectName.toStdString() + "\"" ;
//    outfile << vproj.c_str();
//#if QT_VERSION < 0x050E00
//    outfile << endl;
//#else
//    outfile << Qt::endl;
//#endif
//    fileout.close();
//}

// Creates a suitable input file from options file (*.damproj)
void ExternalProgramRunner::inputdatafile(const QString &suffix,
                                          const QString &section,
                                          const QString &fullFileName,
                                          const QString &projectDir,
                                          const QString &projectName)
{
    const QString fileoutstr = projectDir + projectName + "-" + suffix;

//qDebug() << "en inputdatafile:";
//qDebug() << "suffix = " << suffix;
//qDebug() << "section = " << section;
//qDebug() << "fullFileName = " << fullFileName;
//qDebug() << "projectDir = " << projectDir;
//qDebug() << "projectName = " << projectName;

    QFile fileinp(fullFileName);

    if (!fileinp.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }

    QFile fileout(fileoutstr);

    if (!fileout.open(QFile::Text | QFile::WriteOnly)) {
        return;
    }

    QTextStream outfile(&fileout);

    const QByteArray buff = ReadSectionOptions(section, &fileinp);

    outfile << buff;

    const QString vproj =
        "\"" + projectDir + projectName + "\"";

    outfile << vproj;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    outfile << endl;
#else
    outfile << Qt::endl;
#endif

    fileout.close();
}

// Reads the content of a section in options .damproj file

QByteArray ExternalProgramRunner::ReadSectionOptions(
    const QString& sectionName,
    QFile* file)
{
    QByteArray result;

    if (!file || !file->isOpen())
        return result;

    file->seek(0);

    const QByteArray expectedHeader =
        "[" + sectionName.toUtf8() + "]";

    bool sectionFound = false;

    while (!file->atEnd()) {
        const QByteArray line = file->readLine();

        if (line.trimmed() == expectedHeader) {
            sectionFound = true;
            break;
        }
    }

    if (!sectionFound)
        return result;

    result.append("&OPTIONS\n");
    while (!file->atEnd()) {
        const QByteArray line = file->readLine();
        const QByteArray trimmedLine = line.trimmed();

        // Comienzo de la siguiente sección
        if (trimmedLine.startsWith('[') &&
            trimmedLine.endsWith(']')) {
            break;
        }

        result.append(line);
    }
    result.append("&END\n");

    return result;
}




//QByteArray ExternalProgramRunner::ReadSectionOptions(const QString &sectionName,
//                                                     QFile *fileName)
//{
//    QByteArray buff;
//    QByteArray line;

//    bool lreadend = false;

//    buff.append("&OPTIONS\n");

//qDebug() << "en ReadSectionOptions: sectionName = " << sectionName << "  fileName = " << fileName;
//    while (!fileName->atEnd() && !lreadend) {
//        line = fileName->readLine();

//        if (QString::fromUtf8(line).contains(sectionName)) {
//            while (!fileName->atEnd()) {
//                line = fileName->readLine();
//qDebug() << "line: = " << line;result.append("&OPTIONS\n");

//                if (line.contains("[")) {
//                    lreadend = true;
//                    break;
//                }

//                buff.append(line);
//            }
//        }
//    }

//    buff.append("&END\n");

//    return buff;
//}

//// Reads the content of a section in options .damproj file
//QByteArray ExternalProgramRunner::ReadSectionOptions(const char *SectionName, QFile *FileName)
//{
//    QByteArray buff, line;
//    bool lreadend = false;    // True when reading section ended
//    buff.append("&OPTIONS\n");
//    while(!(*FileName).atEnd() && !lreadend){
//        line = (*FileName).readLine(50);
//        if( line.contains(SectionName) ) {
//            while(!(*FileName).atEnd()){
//                line = (*FileName).readLine(50);
//                if( line.contains("[") ) {
//                    lreadend = true;
//                    break;
//                }
//                buff.append(line);
//            }
//        }
//    }
//    buff.append("&END\n");
//    return buff;
//}

void ExternalProgramRunner::setProjectFolder(const QString &projectFolder)
{
    projectFolder_ = projectFolder;
}

void ExternalProgramRunner::setMpiCommand(const QString &mpiCommand)
{
    mpiCommand_ = mpiCommand;
}

void ExternalProgramRunner::setMpiFlags(const QString &mpiFlags)
{
    mpiFlags_ = mpiFlags;
}

bool ExternalProgramRunner::isRunning() const
{
    return process_ && process_->state() != QProcess::NotRunning;
}
