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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

#include "VertexUtils.h"

/**
 * @brief Configuration dialog for performance settings
 *
 * This dialog allows users to configure memory and vertex limits
 * for the graphics system. Changes are saved to QSettings and
 * automatically invalidate the VertexUtils cache.
 */
class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Construct a new Config Dialog object
     * @param parent Parent widget
     */
    explicit ConfigDialog(QWidget *parent = nullptr);

private slots:
    void saveSettings();
    void updateLabels();
    void applyAutoConfiguration();

private:
    QSlider *memorySlider;
    QSlider *verticesSlider;
    QLabel *memoryLabel;
    QLabel *verticesLabel;
};

#endif // CONFIGDIALOG_H
