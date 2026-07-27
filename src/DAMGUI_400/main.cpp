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
//  File:   main.cpp
//
//      Last version: September 2018
//
#include <QtGlobal>

#include <QApplication>
#include <QStyleFactory>
#include <QSplashScreen>
#include <QTranslator>
#include <QLibraryInfo>

#include <QtDebug>

#include "mainwindow.h"
#include "configdialog.h"
#include "VertexUtils.h"
#include "configdialog.h" // Include ConfigDialog header

#include <stdio.h>
#include <stdlib.h>

#if QT_VERSION >= 0x050000
    void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString &msg)
    {
        // In this function, you can write the message to any stream!
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stderr, "MESSAGE (%s:%u %s): %s\n",
                context.file, context.line, context.function, localMsg.constData());
        fflush(stderr);
    }
#else
    void myMessageOutput(QtMsgType type, const char *msg)
    {
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s\n", msg);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", msg);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", msg);
            abort();
        }
    }
#endif

/**
 * @brief Set up application settings on first run
 *
 * Shows configuration dialog on first run and logs current settings.
 * The configuration is stored in QSettings and cached by VertexUtils.
 */
void setupApplicationSettings() {
    QSettings settings;

    // Only show configuration dialog on first run
    if (!settings.contains("Graphics/firstRunCompleted")) {
        ConfigDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            settings.setValue("Graphics/firstRunCompleted", true);
            qDebug() << "First run configuration completed.";
        } else {
            // User canceled first-run configuration
            // Set default values if not already set
            if (!settings.contains("Graphics/maxMemoryMB")) {
                settings.setValue("Graphics/maxMemoryMB", 2000);
            }
            if (!settings.contains("Graphics/maxVertices")) {
                settings.setValue("Graphics/maxVertices", 1000000);
            }
            settings.setValue("Graphics/firstRunCompleted", true);
            VertexUtils::invalidateCache(); // Ensure cache is updated
        }
    }

    // Log current configuration
    qDebug() << "Current configuration:";
    qDebug() << "  Memory limit:" << VertexUtils::getMaxMemoryLimitMB() << "MB";
    qDebug() << "  Vertex limit:" << VertexUtils::getMaxVerticesLimit();
    qDebug() << "Configuration file:" << settings.fileName();

    // Log vertex structure size for debugging
    qDebug() << "Vertex structure size: " << sizeof(VertexNormalData) << " bytes";
}

/**
 * @brief Main application entry point
 *
 * Initializes the Qt application, sets up message handling,
 * loads translations, and starts the main window.
 */
int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(damqt);

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(myMessageOutput); // Install custom message handler
#else
    qInstallMsgHandler(myMessageOutput);
#endif


    QApplication app(argc, argv);

    // Set application metadata for QSettings organization
    app.setOrganizationName("UAM");
    app.setApplicationName("DAMQT");
    app.setApplicationVersion("1.0.0");

    // Set application style
#if QT_VERSION >= 0x050000
    app.setStyle(QStyleFactory::create("Fusion")); // Using Fusion instead of plastique for better cross-platform
#else
    qDebug() << "QT_VERSION < 0x050000";
    QApplication::setStyle(QStyleFactory::create("plastique")); // windowsxp, macintosh
#endif

    // Setup application settings (first-run configuration)
//    setupApplicationSettings();

    // Pon esto en main() después de setupApplicationSettings()
    qDebug() << "=== QUICK DIAGNOSIS ===";

    // Check 1: What is qsizetype?
    qDebug() << "qsizetype max:" << std::numeric_limits<qsizetype>::max();

    // Check 2: What does VertexUtils return?
    qDebug() << "VertexUtils::getMaxVerticesLimit():" << VertexUtils::getMaxVerticesLimit();

    // Check 3: What's in QSettings?
    QSettings settings;
    QVariant v = settings.value("Graphics/maxVertices");
    qDebug() << "QSettings value:" << v << "type:" << v.typeName();

    // Check 4: Direct calculation
    int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;
    qint64 directCalc = (16384LL * 1024LL * 1024LL) / bytesPerVertex;
    qDebug() << "Direct calc (16GB /" << bytesPerVertex << "bytes):" << directCalc;

    qDebug() << "=== END DIAGNOSIS ===";

    // Load Qt translations
    QTranslator qtTranslator;
    if (qtTranslator.load("qt_" + QLocale::system().name(),
                         QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
        qDebug() << "Loaded Qt translations for locale: " << QLocale::system().name();
    } else {
        qDebug() << "Could not load Qt translations for locale: " << QLocale::system().name();
    }

    // Load application translations
    QTranslator appTranslator;
    if (appTranslator.load(":/translations/damqt_" + QLocale::system().name())) {
        app.installTranslator(&appTranslator);
        qDebug() << "Loaded application translations for locale: " << QLocale::system().name();
    }

    // Process any pending events
    app.processEvents();

    // Create and show main window
    MainWindow mainWin;
    qDebug() << "MainWindow constructed";
    mainWin.show();
    qDebug() << "MainWindow shown";

    // Finish splash screen if applicable
    mainWin.finishsplash();

    qDebug() << "Application started successfully";

    return app.exec();
}
