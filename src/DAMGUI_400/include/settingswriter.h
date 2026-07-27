#ifndef SETTINGSWRITER_H
#define SETTINGSWRITER_H

#include <QByteArray>
#include <QColor>
#include <QQuaternion>
#include <QString>
#include <QStringList>
#include <QVector3D>

class SettingsWriter
{
public:
    explicit SettingsWriter(const QString& fileName);

    void beginSection(const QString& section);

    void writeText(const QString& key,
                   const QString& value);

    void writeInt(const QString& key,
                  int value);

    void writeFloat(const QString& key,
                    double value);

    void writeBool(const QString& key,
                   bool value);

    void writeColor(const QString& key,
                    const QColor& color);

    void writeVector(const QString& key,
                     const QVector3D& vector);

    void writeQuaternion(const QString& key,
                         const QQuaternion& quaternion);

    void writeList(const QString& key,
                   const QStringList& values);

    bool save(QString* errorMessage = nullptr);

private:
    QString fileName_;
    QByteArray buffer_;
};

#endif // SETTINGSWRITER_H
