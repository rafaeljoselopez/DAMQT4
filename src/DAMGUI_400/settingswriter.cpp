#include "settingswriter.h"

#include <QFile>
#include <QTextStream>

SettingsWriter::SettingsWriter(const QString& fileName)
    : fileName_(fileName)
{
}

void SettingsWriter::beginSection(const QString& section)
{
    buffer_.append('[');
    buffer_.append(section.toUtf8());
    buffer_.append("]\n");
}

void SettingsWriter::writeText(const QString& key,
                               const QString& value)
{
    buffer_.append(key.toUtf8());
    buffer_.append('=');
    buffer_.append(value.toUtf8());
    buffer_.append('\n');
}

void SettingsWriter::writeInt(const QString& key,
                              int value)
{
    writeText(key, QString::number(value));
}

void SettingsWriter::writeFloat(const QString& key,
                                double value)
{
    writeText(key, QString::number(value, 'g', 16));
}

void SettingsWriter::writeBool(const QString& key,
                               bool value)
{
    writeText(key, value ? QStringLiteral("1")
                         : QStringLiteral("0"));
}

void SettingsWriter::writeColor(const QString& key,
                                const QColor& color)
{
    writeText(
        key,
        QStringLiteral("%1,%2,%3")
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue())
    );
}

void SettingsWriter::writeVector(const QString& key,
                                 const QVector3D& vector)
{
    writeText(
        key,
        QStringLiteral("%1,%2,%3")
            .arg(vector.x(), 0, 'g', 16)
            .arg(vector.y(), 0, 'g', 16)
            .arg(vector.z(), 0, 'g', 16)
    );
}

void SettingsWriter::writeQuaternion(
    const QString& key,
    const QQuaternion& quaternion)
{
    writeText(
        key,
        QStringLiteral("%1,%2,%3,%4")
            .arg(quaternion.scalar(), 0, 'g', 16)
            .arg(quaternion.x(), 0, 'g', 16)
            .arg(quaternion.y(), 0, 'g', 16)
            .arg(quaternion.z(), 0, 'g', 16)
    );
}

void SettingsWriter::writeList(
    const QString& key,
    const QStringList& values)
{
    writeText(key, values.join(','));
}

bool SettingsWriter::save(QString* errorMessage)
{
    QFile file(fileName_);

    if (!file.open(QIODevice::WriteOnly |
                   QIODevice::Text |
                   QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage =
                QObject::tr("File %1 cannot be opened:\n%2.")
                    .arg(fileName_, file.errorString());
        }

        return false;
    }

    QTextStream stream(&file);
    stream << buffer_;

    if (stream.status() != QTextStream::Ok) {
        if (errorMessage) {
            *errorMessage =
                QObject::tr("An error occurred while writing file %1.")
                    .arg(fileName_);
        }

        return false;
    }

    return true;
}
