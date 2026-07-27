#ifndef HFFORCESPAGE_H
#define HFFORCESPAGE_H

#include <QWidget>
#include <QValidator>
#include <QRegExpValidator>
#include <QRegExp>

#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "readwriteoptions.h"

class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;


class HFForcesPage : public QWidget, public IExecutablePage
{
    Q_OBJECT

public:

    explicit HFForcesPage(QWidget* parent = nullptr);

    bool isAtomicFragments() const;
    bool isInputOnly() const;
    
    void execDamForces();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setCentersEnabled(bool enabled);
    void setAtomicFragments(bool checked);
    void setExecEnabled(bool enabled);
    void setIsWindows(bool windows);
    void setIsValence(bool valence);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                        const QString &projectName);
    void setProjectFolder(const QString& value);
    void setStopEnabled(bool enabled);
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool lvalence,
//                     bool* printwarns,
//                     QString* warns);
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);
    
    QString outputPrefix() const;

signals:

    void execRequested();
    void openOutputRequested();
//    void stopRequested();

    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void outputTextReady(const QString &text);
    void showInputFileRequested(const QString &text);
    void statusMessageRequested(const QString &message);


private:    
    void atomicFragmentsChanged(int state);
    void buildUi();
    void centersChanged();
    void connectSignals();
    void handleNormalProcessExit();
    
    // ---- basic API used from MainWindow ----

    bool iswindows_ = false;
    bool lvalence_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString projectFolder_;
    QString projectName_;

    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;

    // ---- Atomic fragments selection ----
    QGroupBox* atomicFragmentsGroup_ = nullptr;
    QCheckBox* atomicFragmentsCheck_ = nullptr;
    QGroupBox* centersGroup_ = nullptr;
    QLabel* centersLabel_ = nullptr;
    QLineEdit* centersEdit_ = nullptr;
    QRegExpValidator* centersValidator_ = nullptr;
    QStringList* centersList_ = nullptr;

    // ---- Input only ----
    QGroupBox* inputOnlyGroup_ = nullptr;
    QCheckBox* inputOnlyCheck_ = nullptr;

    // ---- Buttons ----
    QPushButton* execButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* outputButton_ = nullptr;
};

#endif
