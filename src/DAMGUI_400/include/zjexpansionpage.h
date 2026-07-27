#ifndef ZJEXPANSIONPAGE_H
#define ZJEXPANSIONPAGE_H

#include <QWidget>
#include <QValidator>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QRegExp>
#include <QSignalBlocker>
#include <QButtonGroup>

#include "externalprogramrunner.h"
#include "iexecutablepage.h"
#include "readwriteoptions.h"

#define MAX_LEXPZJ 22
#define MAX_KEXPZJ 40

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;

class ZJExpansionPage : public QWidget, public IExecutablePage
{
    Q_OBJECT

public:
    explicit ZJExpansionPage(bool mpiAvailable, QWidget* parent = nullptr);

    // ---- basic API used from MainWindow ----

    int distributionsCutoff() const;
    int kMax() const;
    int kMaxTop() const;
    int lMax() const;
    int lMaxTop() const;
    int mpiProcessors() const;
    int multipoleCutoff() const;
    int nquadPoints() const;

    bool isAbsoluteRadio() const;
    bool isEchelon() const;
    bool isInputOnly() const;
    bool isJacobi() const;
    bool isMpiChecked() const;
    bool isRelativeRadio() const;
    bool isZernike() const;
    
    void execDamZJ();
    void loadDefault();
    void readFromFile(const std::string& file);
    void setAbsoluteRadio(bool checked);
    void setBallSize(const QString& value);
    void setDistributionsCutoff(int value);
    void setDistributionsCutoffMinimum(int value);
    void setEchelon(bool checked);
    void setExecEnabled(bool enabled);
    void setIsWindows(bool windows);
    void setJacobi(bool checked);
    void setKmax(int lmax);
    void setKmaxTop(int ltop);
    void setLmax(int lmax);
    void setLmaxTop(int ltop);
    void setSlater(bool checked);
    void setMpiCheckEnabled(bool enabled);
    void setMpiChecked(bool checked);
    void setMpiControlsEnabled(bool enabled);
    void setMpiEnabled(bool enabled);
    void setMpiProcessors(int value);
    void setMpiVisible(bool visible);
    void setMpiSettings(const QString &mpiCommand,
                            const QString &mpiFlags);
    void setMultipoleCutoff(int value);
    void setMultipoleCutoffMinimum(int value);
    void setPageEnabled(bool enabled);
    void setProjectData(const QString &projectFolder,
                                const QString &projectName);
    void setProjectFolder(const QString& value);
    void setQuadratureLength(int value);
    void setQuadratureMaximum(int value);
    void setQuadratureMinimum(int value);
    void setRelativeRadio(bool checked);
    void setStopEnabled(bool enabled);
    void setZernike(bool checked);
    void stopCurrentProcess();
//    void writeToFile(const std::string& file,
//                     bool isWindows,
//                     bool* printwarns,
//                     QString* warns);
    void writeToFile(const std::string& file,
                     bool* printwarns,
                     QString* warns);
    

signals:
    void execRequested();
    void externalProcessStarted();
    void externalProcessFinished(bool ok);
    void externalProcessMessage(const QString &message);
    void openOutputRequested();
    void outputTextReady(const QString &text);
    void statusMessageRequested(const QString &message);
    void stopRequested();
    void showInputFileRequested(const QString &text);
    

private:
    void buildUi();
    void connectSignals();
    void handleNormalProcessExit();
    void inputOnlyStateChanged(int state);
    void mpiStateChanged(int state);
    
    enum BallSizes {
        ABSOLUTE = 0,
        RELATIVE = 1
    };
    
    enum ExpansionType {
        ZERNIKE = 0,
        JACOBI = 1
    };

    bool mpiAvailable_ = false;
    bool iswindows_ = false;
    bool lslater_ = false;

    ExternalProgramRunner *runner_ = nullptr;

    QString lastOutputFileName_;
    QString mpiCommand_;
    QString mpiFlags_;
    QString projectFolder_;
    QString projectName_;

    // ---- Ball size ----
    QGroupBox* ballSizeGroup_ = nullptr;
    QLineEdit* ballSizeEdit_ = nullptr;
    
    QGroupBox* ballRadioTypeGroup_ = nullptr;
    QRadioButton* ballAbsoluteRadio_ = nullptr;
    QRadioButton* ballRelativeRadio_ = nullptr;
    QButtonGroup* ballRadioGroup_ = nullptr;
    
    // ---- Expansion length ----
    QGroupBox* expansionLengthGroup_ = nullptr;
    QLabel* lMaxLabel_ = nullptr;
    QSpinBox* lMaxSpin_ = nullptr;
    QLabel* kMaxLabel_ = nullptr;
    QSpinBox* kMaxSpin_ = nullptr;
    QRadioButton* echelonRadio_ = nullptr;
    
    // ---- Type of fitting ----
    QGroupBox* fittingTypeGroup_ = nullptr;
    QRadioButton* zernike3DRadio_ = nullptr;
    QRadioButton* jacobiRadio_ = nullptr;
    QButtonGroup* fittingGroup_ = nullptr;
        
    // ---- Quadrature length ----
    QGroupBox* quadratureGroup_ = nullptr;
    QLabel* quadratureLabel_ = nullptr;
    QSpinBox* quadratureSpin_ = nullptr;
    
    // ---- Thresholds ----
    QGroupBox* thresholdsGroup_ = nullptr;
    QLabel* multipoleCutoffLabel_ = nullptr;
    QSpinBox* multipoleCutoffSpin_ = nullptr;
    QLabel* distributionsCutoffLabel_ = nullptr;
    QSpinBox* distributionsCutoffSpin_ = nullptr;
    
    // ---- Input only ----
    QGroupBox* inputOnlyGroup_ = nullptr;
    QCheckBox* inputOnlyCheck_ = nullptr;

    // ---- MPI ----
    QGroupBox* mpiGroup_ = nullptr;
    QCheckBox* mpiCheck_ = nullptr;
    QLabel* mpiProcessorsLabel_ = nullptr;
    QSpinBox* mpiProcessorsSpin_ = nullptr;

    // ---- Buttons ----
    QPushButton* execButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* outputButton_ = nullptr;
};

#endif
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
