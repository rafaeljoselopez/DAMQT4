//  Copyright 2008-2026, Rafael Lopez, Alfredo Aguado, Octavio Roncero
//  This file is part of the package VIEWER
//
//  Author: Rafael Lopez
//  rafael.lopez@uam.es
//
//  Universidad Autonoma de Madrid, November 2025
//
//  class ConfigDialog
//
#include "configdialog.h"
#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QHBoxLayout>
#include <QIcon>
#include <QSysInfo>
// #include <QDebug>

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Performance Configuration"));
    setMinimumSize(500, 400);

    QSettings settings("DAMQT", "Densidades");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    // Title
    QLabel *titleLabel = new QLabel(tr("Performance Limits Configuration"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Memory configuration group
    QGroupBox *memoryGroup = new QGroupBox(tr("Memory Limits"));
    memoryGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *memoryLayout = new QFormLayout(memoryGroup);
    memoryLayout->setSpacing(10);

    memorySlider = new QSlider(Qt::Horizontal);
    // Use SAFE limits instead of theoretical maximums
    memorySlider->setRange(VertexConstants::MIN_MEMORY_MB,
                          VertexConstants::SAFE_MAX_MEMORY_MB);  // Safe max
    memorySlider->setValue(settings.value("Graphics/maxMemoryMB",
                                         VertexConstants::DEFAULT_MAX_MEMORY_MB).toInt());
    memorySlider->setTickPosition(QSlider::TicksBelow);
    memorySlider->setTickInterval(512);  // Every 512MB
    memorySlider->setPageStep(256);
    memorySlider->setToolTip(tr("Maximum memory allocated for vertex data (safe limit: %1 MB)")
                            .arg(VertexConstants::SAFE_MAX_MEMORY_MB));

    memoryLabel = new QLabel();
    memoryLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");

    // Memory information with safe limits
    QLabel *memoryInfo = new QLabel();
    memoryInfo->setText(tr("Recommended: %1MB - %2MB\nSafe maximum for QVector: %3MB")
                       .arg(VertexConstants::MIN_MEMORY_MB)
                       .arg(VertexConstants::SAFE_MAX_MEMORY_MB)
                       .arg(VertexConstants::SAFE_MAX_MEMORY_MB));
    memoryInfo->setStyleSheet("color: #7f8c8d; font-size: 11px;");

    memoryLayout->addRow(tr("Maximum memory (MB):"), memorySlider);
    memoryLayout->addRow(tr("Current value:"), memoryLabel);
    memoryLayout->addRow("", memoryInfo);

    // Vertex configuration group
    QGroupBox *verticesGroup = new QGroupBox(tr("Vertex Limits"));
    verticesGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QFormLayout *verticesLayout = new QFormLayout(verticesGroup);
    verticesLayout->setSpacing(10);

    verticesSlider = new QSlider(Qt::Horizontal);
    // Use SAFE limits instead of theoretical maximums
    verticesSlider->setRange(VertexConstants::MIN_VERTICES_K,
                            VertexConstants::MAX_VERT_STRUCT_K);  // Already safe (50M)

    // Get value in thousands from QSettings
    qint64 savedVertices = settings.value("Graphics/maxVertices",
                                         VertexConstants::DEFAULT_MAX_VERTICES).toLongLong();
    int savedVerticesK = VertexConstants::verticesToK(savedVertices);
    verticesSlider->setValue(savedVerticesK);
    verticesSlider->setTickPosition(QSlider::TicksBelow);
    verticesSlider->setTickInterval(5000);  // Every 5M vertices
    verticesSlider->setPageStep(1000);
    verticesSlider->setToolTip(tr("Maximum number of vertices (in thousands)\nSafe limit: %1k vertices")
                              .arg(VertexConstants::MAX_VERT_STRUCT_K));

    verticesLabel = new QLabel();
    verticesLabel->setStyleSheet("font-weight: bold; color: #2c3e50;");

    // Vertex information with safe limits
    QLabel *verticesInfo = new QLabel();
    verticesInfo->setText(tr("Minimum: %1k, Safe maximum: %2k vertices\nEach vertex uses ~%3 bytes")
                         .arg(VertexConstants::MIN_VERTICES_K)
                         .arg(VertexConstants::MAX_VERT_STRUCT_K)
                         .arg(VertexConstants::BYTES_PER_VERTEX));
    verticesInfo->setStyleSheet("color: #7f8c8d; font-size: 11px;");

    verticesLayout->addRow(tr("Maximum vertices (thousands):"), verticesSlider);
    verticesLayout->addRow(tr("Current value:"), verticesLabel);
    verticesLayout->addRow("", verticesInfo);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *safeDefaultsButton = new QPushButton(tr("Safe Defaults"));
    safeDefaultsButton->setToolTip(tr("Set safe default values for QVector compatibility"));

    QPushButton *defaultButton = new QPushButton(tr("Default Values"));
    defaultButton->setToolTip(tr("Restore default values"));

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    QPushButton *saveButton = new QPushButton(tr("Save Configuration"));
    saveButton->setDefault(true);
    saveButton->setStyleSheet("QPushButton { font-weight: bold; }");

    buttonLayout->addWidget(safeDefaultsButton);
    buttonLayout->addWidget(defaultButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(saveButton);

    // Add widgets to main layout
    mainLayout->addWidget(memoryGroup);
    mainLayout->addWidget(verticesGroup);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(memorySlider, &QSlider::valueChanged, this, &ConfigDialog::updateLabels);
    connect(verticesSlider, &QSlider::valueChanged, this, &ConfigDialog::updateLabels);

    // Connection for synchronizing vertices when memory changes
    connect(memorySlider, &QSlider::valueChanged, this, [this](int value) {
        int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;
        qint64 theoreticalVertices = (static_cast<qint64>(value) * 1024LL * 1024LL) / bytesPerVertex;
        int theoreticalKVertices = VertexConstants::verticesToK(theoreticalVertices);

        // Cap at safe maximum
        theoreticalKVertices = qMin(theoreticalKVertices, VertexConstants::MAX_VERT_STRUCT_K);

        // Only adjust if current vertices exceed the new limit
        if (verticesSlider->value() > theoreticalKVertices) {
            verticesSlider->setValue(theoreticalKVertices);
        }

        updateLabels();
    });

    // Connection for synchronizing memory when vertices change
    connect(verticesSlider, &QSlider::valueChanged, this, [this](int value) {
        int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;
        qint64 actualVertices = VertexConstants::kToVertices(value);
        int requiredMemory = (actualVertices * bytesPerVertex) / (1024 * 1024);

        // Add 10% margin
        requiredMemory += requiredMemory * 0.1;

        // Cap at safe maximum
        requiredMemory = qMin(requiredMemory, VertexConstants::SAFE_MAX_MEMORY_MB);

        // Only adjust if current memory is insufficient
        if (memorySlider->value() < requiredMemory) {
            memorySlider->setValue(requiredMemory);
        }

        updateLabels();
    });

    connect(saveButton, &QPushButton::clicked, this, &ConfigDialog::saveSettings);

    // Safe defaults: 512MB, 12M vertices (in thousands: 12000)
    connect(safeDefaultsButton, &QPushButton::clicked, this, [this]() {
        memorySlider->setValue(VertexConstants::DEFAULT_MAX_MEMORY_MB);
        verticesSlider->setValue(VertexConstants::verticesToK(12000000));  // 12M vertices
    });

    // Default values: 2GB, 1M vertices (original defaults)
    connect(defaultButton, &QPushButton::clicked, this, [this]() {
        memorySlider->setValue(2000);
        verticesSlider->setValue(1000);  // 1M vertices in thousands
    });

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Update initial labels
    updateLabels();
}

void ConfigDialog::updateLabels() {
    int memoryValue = memorySlider->value();
    int kVerticesValue = verticesSlider->value(); // in thousands
    qint64 actualVertices = VertexConstants::kToVertices(kVerticesValue);

    int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;

    // Compute theoretical vertices for this memory
    qint64 theoreticalVertices = (static_cast<qint64>(memoryValue) * 1024LL * 1024LL) / bytesPerVertex;
    int theoreticalKVertices = VertexConstants::verticesToK(theoreticalVertices);

    // Formatting memory text
    QString memoryText;
    if (memoryValue >= 1024) {
        memoryText = tr("%1 MB (%2 GB)").arg(memoryValue).arg(memoryValue / 1024.0, 0, 'f', 1);
    } else {
        memoryText = tr("%1 MB").arg(memoryValue);
    }

    // Formatting vertices text
    QString verticesText = tr("%1k").arg(kVerticesValue);

    memoryLabel->setText(memoryText);
    verticesLabel->setText(verticesText);

    // Visual feedback for safe limits
    if (kVerticesValue > VertexConstants::MAX_VERT_STRUCT_K) {
        verticesLabel->setStyleSheet("font-weight: bold; color: #e74c3c;");
        verticesLabel->setToolTip(tr("Warning: Exceeds safe QVector limit (%1k)\nMay cause allocation errors")
                                 .arg(VertexConstants::MAX_VERT_STRUCT_K));
    }
    else if (kVerticesValue > theoreticalKVertices) {
        verticesLabel->setStyleSheet("font-weight: bold; color: #e74c3c;");
        qint64 requiredMemory = (actualVertices * bytesPerVertex) / (1024LL * 1024LL);
        verticesLabel->setToolTip(tr("Warning: %1k vertices require approximately %2 MB (current limit: %3 MB)")
                                 .arg(kVerticesValue)
                                 .arg(requiredMemory)
                                 .arg(memoryValue));
    }
    else if (kVerticesValue > theoreticalKVertices * 0.9) {
        verticesLabel->setStyleSheet("font-weight: bold; color: #f39c12;");
        verticesLabel->setToolTip(tr("Caution: %1k vertices use %2% of allocated memory")
                                 .arg(kVerticesValue)
                                 .arg((kVerticesValue * 100) / theoreticalKVertices));
    }
    else if (memoryValue > VertexConstants::SAFE_MAX_MEMORY_MB) {
        memoryLabel->setStyleSheet("font-weight: bold; color: #f39c12;");
        memoryLabel->setToolTip(tr("Caution: Exceeds recommended QVector safe limit (%1 MB)")
                               .arg(VertexConstants::SAFE_MAX_MEMORY_MB));
    }
    else {
        verticesLabel->setStyleSheet("font-weight: bold; color: #27ae60;");
        memoryLabel->setStyleSheet("font-weight: bold; color: #27ae60;");
        verticesLabel->setToolTip(tr("Within safe QVector limits"));
    }
}

void ConfigDialog::applyAutoConfiguration() {
    // Simplified system detection
    qint64 totalMemory = 0;

    // For now, use safe defaults
    totalMemory = 8192; // Assume 8GB system

    if (totalMemory > 0) {
        // Use 25% of system memory for graphics, maximum 2GB (safe limit)
        qint64 recommendedMemory = qMin(static_cast<qint64>(totalMemory * 0.25),
                                       static_cast<qint64>(VertexConstants::SAFE_MAX_MEMORY_MB));

        // Ensure within safe slider range
        recommendedMemory = qBound(static_cast<qint64>(VertexConstants::MIN_MEMORY_MB),
                                  recommendedMemory,
                                  static_cast<qint64>(VertexConstants::SAFE_MAX_MEMORY_MB));

        memorySlider->setValue(static_cast<int>(recommendedMemory));

        // Compute vertices from memory and structure size
        int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;
        qint64 recommendedVertices = (recommendedMemory * 1024LL * 1024LL) / bytesPerVertex;

        // Convert to thousands and cap at safe maximum
        int recommendedKVertices = VertexConstants::verticesToK(recommendedVertices);
        recommendedKVertices = qMin(recommendedKVertices, VertexConstants::MAX_VERT_STRUCT_K);

        // Ensure within safe limits
        recommendedKVertices = qBound(VertexConstants::MIN_VERTICES_K,
                                     recommendedKVertices,
                                     VertexConstants::MAX_VERT_STRUCT_K);

        verticesSlider->setValue(recommendedKVertices);

        QMessageBox::information(this, tr("Auto Configuration"),
            tr("Configured with safe QVector limits:\n"
               "• Allocated graphics memory: %1 MB\n"
               "• Maximum vertices: %2k\n"
               "• Vertex structure size: %3 bytes\n"
               "• Based on assumed %4 MB system memory")
            .arg(recommendedMemory)
            .arg(recommendedKVertices)
            .arg(bytesPerVertex)
            .arg(totalMemory));
    }
    else {
        // Use safe default values
        memorySlider->setValue(VertexConstants::DEFAULT_MAX_MEMORY_MB);
        verticesSlider->setValue(VertexConstants::verticesToK(12000000));  // 12M vertices

        QMessageBox::information(this, tr("Auto Configuration"),
            tr("Safe default configuration applied:\n"
               "• Memory: %1 MB (safe QVector limit)\n"
               "• Vertices: %2k (safe QVector limit)\n"
               "• Vertex structure size: %3 bytes")
               .arg(VertexConstants::DEFAULT_MAX_MEMORY_MB)
               .arg(VertexConstants::verticesToK(12000000))
               .arg(VertexConstants::BYTES_PER_VERTEX));
    }
}

void ConfigDialog::saveSettings() {
    int memoryValue = memorySlider->value();
    int kVerticesValue = verticesSlider->value();  // in thousands
    qint64 actualVertices = VertexConstants::kToVertices(kVerticesValue);
    int bytesPerVertex = VertexConstants::BYTES_PER_VERTEX;

    // Calculate consistency with safe limits
    qint64 requiredMemory = (actualVertices * bytesPerVertex) / (1024LL * 1024LL);
    qint64 maxVertices = (static_cast<qint64>(memoryValue) * 1024LL * 1024LL) / bytesPerVertex;
    int maxKVertices = VertexConstants::verticesToK(maxVertices);

    // Apply safe limits
    memoryValue = qMin(memoryValue, VertexConstants::SAFE_MAX_MEMORY_MB);
    kVerticesValue = qMin(kVerticesValue, VertexConstants::MAX_VERT_STRUCT_K);
    actualVertices = VertexConstants::kToVertices(kVerticesValue);

    // Verify consistency
    if (kVerticesValue > maxKVertices) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            tr("Inconsistent Configuration"),
            tr("The vertex limit exceeds available memory:\n\n"
               "• Memory limit: %1 MB\n"
               "• Vertex limit: %2k vertices\n"
               "• Vertex size: %3 bytes\n\n"
               "Calculation:\n"
               "• Memory supports: %4k vertices\n"
               "• Vertices require: %5 MB\n\n"
               "Adjust vertices to %4k?")
               .arg(memoryValue)
               .arg(kVerticesValue)
               .arg(bytesPerVertex)
               .arg(maxKVertices)
               .arg(requiredMemory),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);

        if (reply == QMessageBox::Yes) {
            verticesSlider->setValue(maxKVertices);
            kVerticesValue = maxKVertices;
            actualVertices = VertexConstants::kToVertices(kVerticesValue);
        }
    }

    // Warn about QVector limits
    if (kVerticesValue >= VertexConstants::MAX_VERT_STRUCT_K * 0.9) {
        QMessageBox::warning(this, tr("QVector Compatibility Warning"),
            tr("You are approaching the safe QVector limit:\n\n"
               "• Configured: %1k vertices\n"
               "• Safe maximum: %2k vertices\n\n"
               "Values above %3k may cause std::bad_alloc errors.\n"
               "Consider reducing vertex count for stability.")
               .arg(kVerticesValue)
               .arg(VertexConstants::MAX_VERT_STRUCT_K)
               .arg(VertexConstants::MAX_VERT_STRUCT_K));
    }

    // Save to QSettings
    QSettings settings("DAMQT", "Densidades");
    settings.setValue("Graphics/maxMemoryMB", memoryValue);
    settings.setValue("Graphics/maxVertices", actualVertices);

    // Invalidate VertexUtils cache
    VertexUtils::invalidateCache();

    QMessageBox::information(this, tr("Configuration Saved"),
        tr("Configuration saved successfully with safe QVector limits:\n\n"
           "• Memory: %1 MB\n"
           "• Vertices: %2k\n"
           "• Vertex structure size: %3 bytes")
           .arg(memoryValue)
           .arg(kVerticesValue)
           .arg(bytesPerVertex));

    accept();
}
