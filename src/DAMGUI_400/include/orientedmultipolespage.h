#ifndef ORIENTEDMULTIPOLESPAGE_H
#define ORIENTEDMULTIPOLESPAGE_H

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
class QSpinBox;


class OrientedMultipolesPage : public QWidget, public IExecutablePage
{
    Q_OBJECT

public:

    explicit OrientedMultipolesPage(QWidget* parent = nullptr);

    bool isAtomicFragments() const;
    bool isInputOnly() const;
    
    int leftIndex() const;
    int lMax() const;
    int lMin() const;
    int middleIndex() const;
    int rightIndex() const;

    void execDamMultRot();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setExecEnabled(bool enabled);
    void setIsWindows(bool windows);
    void setLeftSpinMax(int nmax);
    void setLeftSpinValue(int nmax);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setLmin(int lmin);
    void setLminTop(int ltop);
    void setMiddleSpinMax(int nmax);
    void setMiddleSpinValue(int nmax);
    void setNumAtoms(int value);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                        const QString &projectName);
    // void setProjectFolder(const QString& value);
    void setRightSpinMax(int nmax);
    void setRightSpinValue(int nmax);
    void setStopEnabled(bool enabled);
//    void writeToFile(const std::string& file,
//                     bool isWindows,
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
    void buildUi();
    void centersChanged();
    void connectSignals();
    void handleNormalProcessExit();
    void lMaxChanged();
    void lMinChanged();
    void leftCenterChanged();
    void middleCenterChanged();
    void rightCenterChanged();
    
    int numatoms_;      // Number of atoms
    int rotleft_;    // Used to store the previous value of SPBmrotleft (see SLOT SPBmrotleft_changed)
    int rotmiddle_;    // Used to store the previous value of SPBmrotmiddle (see SLOT SPBmrotmiddle_changed)
    int rotright_;    // Used to store the previous value of SPBmrotright (see SLOT SPBmrotright_changed)
    
    // ---- basic API used from MainWindow ----
    bool iswindows_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString projectFolder_;
    QString projectName_;

    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;
    
    // ---- Multipoles ----
    QGroupBox* multipolesGroup_ = nullptr;
    QLabel* lMaxLabel_ = nullptr;
    QSpinBox* lMaxSpin_ = nullptr;
    QLabel* lMinLabel_ = nullptr;
    QSpinBox* lMinSpin_ = nullptr;
    
    // ---- Centers defining plane ----
    QGroupBox* centersPlaneGroup_ = nullptr;
    QLabel* leftLabel_ = nullptr;
    QSpinBox* leftSpin_ = nullptr;
    QLabel* middleLabel_ = nullptr;
    QSpinBox* middleSpin_ = nullptr;
    QLabel* rightLabel_ = nullptr;
    QSpinBox* rightSpin_ = nullptr;
    
    
    // ---- Atomic fragments selection ----
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
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
