#include "settingsreader.h"

#include "IniFile.h"

SettingsReader::SettingsReader(const QString &fileName)
    : fileName_(fileName.toStdString())
{
}


QString SettingsReader::value(const QString &section,
                              const QString &key,
                              const QString &defaultValue) const
{
    const QString result = QString::fromStdString(
        CIniFile::GetValue(key.toStdString(),
                           section.toStdString(),
                           fileName_)
    );

    return result.isEmpty() ? defaultValue : result;
}


int SettingsReader::intValue(const QString &section,
                             const QString &key,
                             int defaultValue) const
{
    bool ok = false;
    const int result = value(section,key).toInt(&ok);

    return ok ? result : defaultValue;
}


float SettingsReader::floatValue(const QString &section,
                                 const QString &key,
                                 float defaultValue) const
{
    bool ok = false;
    const float result = value(section,key).toFloat(&ok);

    return ok ? result : defaultValue;
}


bool SettingsReader::boolValue(const QString &section,
                               const QString &key,
                               bool defaultValue) const
{
    const QString text = value(section,key).trimmed().toLower();

    if (text.isEmpty())
        return defaultValue;

    if (text == "true" || text == "yes" || text == "on")
        return true;

    if (text == "false" || text == "no" || text == "off")
        return false;

    bool ok = false;
    const int result = text.toInt(&ok);

    return ok ? result != 0 : defaultValue;
}


QStringList SettingsReader::listValue(const QString &section,
                                      const QString &key) const
{
    return value(section,key).split(',',Qt::SkipEmptyParts);
}


bool SettingsReader::colorValue(const QString &section,
                                const QString &key,
                                QColor &color) const
{
    const QStringList values = listValue(section,key);

    if (values.size() != 3)
        return false;

    bool okRed = false;
    bool okGreen = false;
    bool okBlue = false;

    const int red = values.at(0).trimmed().toInt(&okRed);
    const int green = values.at(1).trimmed().toInt(&okGreen);
    const int blue = values.at(2).trimmed().toInt(&okBlue);

    if (!okRed || !okGreen || !okBlue)
        return false;

    if (red < 0 || red > 255
            || green < 0 || green > 255
            || blue < 0 || blue > 255) {
        return false;
    }

    color = QColor(red,green,blue);
    return true;
}


bool SettingsReader::vectorValue(const QString &section,
                                 const QString &key,
                                 QVector3D &vector) const
{
    const QStringList values = listValue(section,key);

    if (values.size() != 3)
        return false;

    bool okX = false;
    bool okY = false;
    bool okZ = false;

    const float x = values.at(0).trimmed().toFloat(&okX);
    const float y = values.at(1).trimmed().toFloat(&okY);
    const float z = values.at(2).trimmed().toFloat(&okZ);

    if (!okX || !okY || !okZ)
        return false;

    vector = QVector3D(x,y,z);
    return true;
}


bool SettingsReader::quaternionValue(const QString &section,
                                     const QString &key,
                                     QQuaternion &quaternion) const
{
    const QStringList values = listValue(section,key);

    if (values.size() != 4)
        return false;

    bool okScalar = false;
    bool okX = false;
    bool okY = false;
    bool okZ = false;

    const float scalar = values.at(0).trimmed().toFloat(&okScalar);
    const float x = values.at(1).trimmed().toFloat(&okX);
    const float y = values.at(2).trimmed().toFloat(&okY);
    const float z = values.at(3).trimmed().toFloat(&okZ);

    if (!okScalar || !okX || !okY || !okZ)
        return false;

    quaternion = QQuaternion(scalar,x,y,z);
    return true;
}
