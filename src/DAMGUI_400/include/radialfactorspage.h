#ifndef RADIALFACTORSPAGE_H
#define RADIALFACTORSPAGE_H

#include <QWidget>
#include <QValidator>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QSignalBlocker>
#include <QButtonGroup>

#include "Sheet.h"
#include <functional>
#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "igridoptionspage.h"
#include "ixyztabulationpage.h"
#include "readwriteoptions.h"


class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class Sheet;


class RadialFactorsPage : public QWidget, public IXYZTabulationPage, public IExecutablePage
{
    Q_OBJECT

public:
    explicit RadialFactorsPage(QWidget* parent = nullptr);
    
    // ---- basic API used from MainWindow ----

    int lMax() const;
    int mMax() const;
    int xyzTableRows() const;
    
    bool isExtraValues() const;
    bool isGradient() const;
    bool isInputOnly() const;
    bool isSecondDerivatives() const;
    
    void clearSheet();
    void execDamFrad();
    void insertRow(int i);
    void loadDefault();
    void readFromFile(const std::string& file);
    void readSelectedAtoms(const std::string& file, const char* section);
    void readTable(const std::string& file, const char* section);
    void resizeSheet(int i);
    void setCellValue(const QString& value, int i, int j);
    void setCentersList(const QStringList& value);
    void setExecEnabled(bool enabled);
    void setExtraValues(bool checked);
    void setGradient(bool checked);
    void setIsWindows(bool windows);
    void setLmax(int lmax);
    void setlTabulation(int value);
    void setMmax(int lmax);
    void setmTabulation(int value);
    void setOutputPrefix(const QString& value);
    void setPageEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                        const QString &projectName);
    void setProjectFolder(const QString& value);
    void setRadiiTableEnabled(bool enabled);
    void setRadiiTableVisible(bool visible);
    void setSecondDerivatives(bool checked);
    void setStopEnabled(bool enabled);
    void setTabulationInitial(const QString& value);
    void setTabulationFinal(const QString& value);
    void setTabulationStep(const QString& value);
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool* printwarns,
//                     QString* warns);
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);
    
    QString getCellValue(int i, int j) const;
    QString outputPrefix() const;
    QString numtabular() const;
    QString tabularkey() const;
    
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
    void extraValuesChanged(int state);
    void gradientStateChanged(int state);
    void handleNormalProcessExit();
//    void inputOnlyStateChanged(int state);
    void lTabulationChanged(int lvalue);
    void secondDerivativesStateChanged(int state);
    void writeSelectedAtoms(const std::string& file,
                            int maxAtomCount,
                            bool* printwarns,
                            QString* warns) const;

    bool iswindows_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString projectFolder_;
    QString projectName_;

    int lmax_;
    int mmax_;
                            
    QDoubleValidator* myDoubleValidator_;
    QRegExpValidator* myRegExpValidator_;
                            
    // ---- Output prefix ----
    QGroupBox* outputGroup_ = nullptr;
    QLineEdit* outputPrefixEdit_ = nullptr;
    
    
    // ---- Tabulation points ----
    QGroupBox* tabulationGroup_ = nullptr;
    QLabel* tabulationInitialLabel_ = nullptr;
    QLineEdit* tabulationInitialEdit_ = nullptr;
    QLabel* tabulationFinalLabel_ = nullptr;
    QLineEdit* tabulationFinalEdit_ = nullptr;
    QLabel* tabulationStepLabel_ = nullptr;
    QLineEdit* tabulationStepEdit_ = nullptr;
    QCheckBox* extraValuesCheck_ = nullptr;
    QWidget* radiiTable_ = nullptr;
    Sheet* radiiSheet_ = nullptr;
    
    // ---- Radial factors ----
    QGroupBox* radialFactorsGroup_ = nullptr;
    QLabel* lTabulationLabel_ = nullptr;
    QSpinBox* lTabulationSpin_ = nullptr;
    QLabel* mTabulationLabel_ = nullptr;
    QSpinBox* mTabulationSpin_ = nullptr;
    
    // ---- Centers ----
    QGroupBox* centersGroup_ = nullptr;
    QLabel* centersLabel_ = nullptr;
    QLineEdit* centersEdit_ = nullptr;
    QRegExpValidator* centersValidator_ = nullptr;
    QStringList* centersList_ = nullptr;
    
    // ---- Derivatives ----
    QGroupBox* derivativesGroup_ = nullptr;
    QCheckBox* gradientCheck_ = nullptr;
    QCheckBox* secondDerivativesCheck_ = nullptr;
    
    
    // ---- Input only ----
    QGroupBox* inputOnlyGroup_ = nullptr;
    QCheckBox* inputOnlyCheck_ = nullptr;

    // ---- Buttons ----
    QPushButton* execButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* outputButton_ = nullptr;
    };

#endif
    
    
    
    
