#ifndef SETTINGSREADER_H
#define SETTINGSREADER_H

#include <QColor>
#include <QQuaternion>
#include <QString>
#include <QStringList>
#include <QVector3D>

#include <string>

class SettingsReader
{
public:
    explicit SettingsReader(const QString &fileName);

    QString value(const QString &section,
                  const QString &key,
                  const QString &defaultValue = QString()) const;

    int intValue(const QString &section,
                 const QString &key,
                 int defaultValue = 0) const;

    float floatValue(const QString &section,
                     const QString &key,
                     float defaultValue = 0.0f) const;

    bool boolValue(const QString &section,
                   const QString &key,
                   bool defaultValue = false) const;

    QStringList listValue(const QString &section,
                          const QString &key) const;

    bool colorValue(const QString &section,
                    const QString &key,
                    QColor &color) const;

    bool vectorValue(const QString &section,
                     const QString &key,
                     QVector3D &vector) const;

    bool quaternionValue(const QString &section,
                         const QString &key,
                         QQuaternion &quaternion) const;

private:
    std::string fileName_;
};

#endif
