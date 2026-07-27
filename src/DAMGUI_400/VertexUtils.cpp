//  Copyright 2008-2026, Jaime Fernandez Rico, Rafael Lopez, Ignacio Ema,
//  Guillermo Ramirez, David Zorrilla, Anmol Kumar, Sachin D. Yeole, Shridhar R. Gadre
//
//  This file is part of DAMQT.
//
//
//  Author: Rafael Lopez
//  rafael.lopez@uam.es
//
//  Universidad Autonoma de Madrid, November 2025
//
//  Class VertexUtils
//

// Cache definition

#include "VertexUtils.h"
// #include <QDebug>

VertexUtils::Cache& VertexUtils::getCache() {
    static Cache instance;
    return instance;
}

void VertexUtils::updateCache() {
    Cache& cache = getCache();

    if (!cache.valid) {
        QSettings settings("DAMQT", "Densidades");

        // Get vertex limit from QSettings or use default
        qint64 requestedVertices = settings.value("Graphics/maxVertices",
                                                 VertexConstants::DEFAULT_MAX_VERTICES).toLongLong();

        // Get memory limit from QSettings or use default
        qint64 requestedMemory = settings.value("Graphics/maxMemoryMB",
                                               VertexConstants::DEFAULT_MAX_MEMORY_MB).toLongLong();

        // APPLY SAFE LIMITS to prevent QVector std::bad_alloc
        // Cap vertices at safe maximum (50M for QVector compatibility)
        cache.maxVertices = qMin(requestedVertices, static_cast<qint64>(VertexConstants::SAFE_MAX_VERTICES));

        // Cap memory at safe maximum (2GB for QVector compatibility)
        cache.maxMemoryMB = qMin(requestedMemory, static_cast<qint64>(VertexConstants::SAFE_MAX_MEMORY_MB));

        // Log if limits were reduced (commented out as requested)
        /*
        if (requestedVertices > VertexConstants::SAFE_MAX_VERTICES) {
            qDebug() << "Vertex limit reduced from" << requestedVertices
                     << "to safe maximum" << VertexConstants::SAFE_MAX_VERTICES
                     << "for QVector compatibility";
        }

        if (requestedMemory > VertexConstants::SAFE_MAX_MEMORY_MB) {
            qDebug() << "Memory limit reduced from" << requestedMemory
                     << "MB to safe maximum" << VertexConstants::SAFE_MAX_MEMORY_MB
                     << "MB for QVector compatibility";
        }
        */

        cache.vertexStructSize = sizeof(VertexNormalData);
        cache.valid = true;

        // Debug output (commented out as requested)
        // qDebug() << "Cache updated: maxVertices =" << cache.maxVertices
        //          << ", maxMemoryMB =" << cache.maxMemoryMB
        //          << ", structSize =" << cache.vertexStructSize;
    }
}

void VertexUtils::invalidateCache() {
    Cache& cache = getCache();
    cache.valid = false;

    // Debug output (commented out as requested)
    // qDebug() << "Cache invalidated";
}

bool VertexUtils::canAddMoreVertices(const QVector<VertexNormalData>& currentVector,
                                     int verticesToAdd) {
    updateCache();  // Ensures cache is updated
    const Cache& cache = getCache();

    // 1. Verify the highest number of vertices
    if (currentVector.size() + verticesToAdd > cache.maxVertices) {
        // Only show warning if significantly over limit (commented out as requested)
        /*
        qWarning() << "Maximum number of vertices reached:"
                   << currentVector.size() + verticesToAdd
                   << ">" << cache.maxVertices;
        */
        return false;
    }

    // 2. Verify estimated memory
    qint64 estimatedMemoryMB = getEstimatedMemoryUsageMB(currentVector, verticesToAdd);
    if (estimatedMemoryMB > cache.maxMemoryMB) {
        // Only show warning if significantly over limit (commented out as requested)
        /*
        qWarning() << "Maximum memory reached:"
                   << estimatedMemoryMB << "MB >"
                   << cache.maxMemoryMB << "MB";
        */
        return false;
    }

    return true;
}

qsizetype VertexUtils::getMaxVerticesLimit() {
    updateCache();
    return getCache().maxVertices;
}

qint64 VertexUtils::getMaxMemoryLimitMB() {
    updateCache();
    return getCache().maxMemoryMB;
}

qsizetype VertexUtils::getVertexStructSize() {
    updateCache();
    return getCache().vertexStructSize;
}

qint64 VertexUtils::getEstimatedMemoryUsageMB(const QVector<VertexNormalData>& currentVector,
                                              int verticesToAdd) {
    updateCache();
    const Cache& cache = getCache();

    // Compute total memory in bytes
    qint64 totalVertices = currentVector.size() + verticesToAdd;
    qint64 totalBytes = totalVertices * cache.vertexStructSize;

    // Convert to MB (1 MB = 1024 * 1024 bytes)
    qint64 memoryMB = totalBytes / (1024 * 1024);

    // Round up to be conservative
    if (totalBytes % (1024 * 1024) != 0) {
        memoryMB++;
    }

    // Debug output (commented out as requested)
    // qDebug() << "Estimated memory:" << memoryMB << "MB for" << totalVertices << "vertices";

    return memoryMB;
}
