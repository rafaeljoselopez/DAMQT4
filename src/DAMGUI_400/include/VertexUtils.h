//  Copyright 2008-2026, Rafael Lopez, Alfredo Aguado, Octavio Roncero
//  This file is part of the package VIEWER
//
//  Author: Rafael Lopez
//  rafael.lopez@uam.es
//
//  Universidad Autonoma de Madrid, November 2025
//
//  Class VertexUtils
//

// The following subclassing for checking memory when adding members to a QVector <VertexNormalData>
#ifndef VERTEXUTILS_H
#define VERTEXUTILS_H

#include <QVector>
#include <QSettings>
#include "VertexNormalData.h"

namespace VertexConstants {
    // Bytes per vertex
    constexpr int BYTES_PER_VERTEX = sizeof(VertexNormalData);

    // SAFE MEMORY LIMITS (QVector compatible)
    // Reduced for QVector compatibility - std::bad_alloc occurs around 3.8GB contiguous memory
    constexpr int SAFE_MAX_MEMORY_MB = 2048;        // 2 GB safe limit for QVector
    constexpr int SAFE_MAX_VERTICES = 50000000;     // 50M vertices safe limit

    // Memory limits
    constexpr int MAX_MEMORY_STRUCT_MB = 16384LL;     // 16 GB (theoretical)
    constexpr int DEFAULT_MAX_MEMORY_MB = 512LL;      // 512 MB default (conservative)
    constexpr int MIN_MEMORY_MB = 256LL;              // 256 MB minimum

    // Core calculation function
    constexpr qint64 calculateMaxVertices(int memoryMB) {
        return (static_cast<qint64>(memoryMB) * 1024LL * 1024LL) / BYTES_PER_VERTEX;
    }

    // Helper: convert vertices to thousands (rounding up)
    constexpr int verticesToK(qint64 vertices) {
        return static_cast<int>((vertices + 999LL) / 1000LL);
    }

    // Helper: convert thousands to vertices
    constexpr qint64 kToVertices(int thousands) {
        return static_cast<qint64>(thousands) * 1000LL;
    }

    // Absolute values (in vertices) - USING SAFE LIMITS
    constexpr qint64 DEFAULT_MAX_VERTICES = calculateMaxVertices(DEFAULT_MAX_MEMORY_MB);
    constexpr qint64 MAX_VERT_STRUCT = SAFE_MAX_VERTICES;  // Use safe limit instead of calculated
    constexpr qint64 MIN_VERTICES = calculateMaxVertices(MIN_MEMORY_MB);

    // Values in thousands (k-vertices)
    constexpr int DEFAULT_MAX_VERTICES_K = verticesToK(DEFAULT_MAX_VERTICES);
    constexpr int MAX_VERT_STRUCT_K = verticesToK(MAX_VERT_STRUCT);
    constexpr int MIN_VERTICES_K = verticesToK(MIN_VERTICES);

    // Alias for backward compatibility
    constexpr qint64 DEFAULT_MAX_VERTICES_ABS = DEFAULT_MAX_VERTICES;
    constexpr qint64 MAX_VERT_STRUCT_ABS = MAX_VERT_STRUCT;
    constexpr qint64 MIN_VERTICES_ABS = MIN_VERTICES;

    inline int getBytesPerVertex() {
        return BYTES_PER_VERTEX;
    }
}

class VertexUtils {
private:
    // Cache for limits
    struct Cache {
        qsizetype maxVertices;
        qint64 maxMemoryMB;
        qsizetype vertexStructSize;
        bool valid;

        Cache() : maxVertices(0), maxMemoryMB(0), vertexStructSize(0), valid(false) {}
    };

    static Cache& getCache();
    static void updateCache();

public:
    // Force cache update when configuration changes
    static void invalidateCache();

    // Main functions
    static bool canAddMoreVertices(const QVector<VertexNormalData>& currentVector,
                                   int verticesToAdd);

    static qsizetype getMaxVerticesLimit();
    static qint64 getMaxMemoryLimitMB();
    static qsizetype getVertexStructSize();
    static qint64 getEstimatedMemoryUsageMB(const QVector<VertexNormalData>& currentVector,
                                            int verticesToAdd = 0);

    // Safe limits for QVector compatibility
    static constexpr qint64 SAFE_MAX_VERTICES = VertexConstants::SAFE_MAX_VERTICES;
    static constexpr int SAFE_MAX_MEMORY_MB = VertexConstants::SAFE_MAX_MEMORY_MB;
};

#endif // VERTEXUTILS_H
