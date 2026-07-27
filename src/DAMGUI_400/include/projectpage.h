#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include <QWidget>
#include "readwriteoptions.h"
#include "IniFile.h"
#include "igridoptionspage.h"
#include "ixyztabulationpage.h"

class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

class ProjectPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectPage(bool mpiAvailable,
                         const QString& mpiCommand = QString(),
                         QWidget* parent = nullptr);

    QString importFile() const;
    QString projectFolder() const;
    QString projectName() const;
    QString mpiCommand() const;
    QString mpiFlags() const;

    void loadDefault();
    void readFromFile(const std::string& file, const QString ImportFolder);
    void setBrowseEnabled(bool enabled);
    void setImportEnabled(bool enabled);
    void setImportFile(const QString& value);
    void setProjectFolder(const QString& value);
    void setProjectFolderEnabled(bool enabled);
    void setProjectName(const QString& value);
    void setProjectNameEnabled(bool enabled);
    void setExecEnabled(bool enabled);
    void writeToFile(const std::string& file,
                     bool isWindows,
                     bool* printwarns,
                     QString* warns);

signals:
    void importFileChanged(const QString& text);
    void projectFolderChanged(const QString& text);
    void projectNameChanged(const QString& text);
    void mpiCommandChanged(const QString& text);
    void mpiFlagsChanged(const QString& text);

    void browseImportRequested();
    void execImportRequested();

private:
    void buildUi();
    void connectSignals();

    bool mpiAvailable_;
    QString mpiCommandDefault_;

    QGroupBox* projectGroup_ = nullptr;
    QLabel* importLabel_ = nullptr;
    QLineEdit* importEdit_ = nullptr;
    QToolButton* importBrowseButton_ = nullptr;

    QLabel* projectFolderLabel_ = nullptr;
    QLineEdit* projectFolderEdit_ = nullptr;

    QLabel* projectNameLabel_ = nullptr;
    QLineEdit* projectNameEdit_ = nullptr;

    QPushButton* execImportButton_ = nullptr;

    QGroupBox* mpiGroup_ = nullptr;
    QLabel* mpiCommandLabel_ = nullptr;
    QLineEdit* mpiCommandEdit_ = nullptr;
    QLabel* mpiFlagsLabel_ = nullptr;
    QLineEdit* mpiFlagsEdit_ = nullptr;
};

#endif
