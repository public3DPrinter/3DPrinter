#ifndef B9TERMINAL_H
#define B9TERMINAL_H

#include <QDesktopWidget>
#include <QWidget>
#include <QTime>
#include "b9printercomm.h"
#include "b9projector.h"

class PCycleSettings {
public:
    PCycleSettings(){loadSettings();}
    ~PCycleSettings(){}
    void updateValues(); // Opens a dialog and allows user to change settings

    void loadSettings();
    void saveSettings();
    void setFactorySettings();
    int m_iRSpd1, m_iLSpd1, m_iRSpd2, m_iLSpd2;
    int m_iOpenSpd1, m_iCloseSpd1, m_iOpenSpd2, m_iCloseSpd2;
    double m_dBreatheClosed1, m_dSettleOpen1, m_dBreatheClosed2, m_dSettleOpen2;
    double m_dOverLift1, m_dOverLift2;
    double m_dBTClearInMM;
    double m_dHardZDownMM;
    double m_dZFlushMM;
};

namespace Ui {
class B9Terminal;
}

class B9Terminal : public QWidget
{
    Q_OBJECT
    
public:
    explicit B9Terminal(QWidget *parent = 0, Qt::WFlags flags = Qt::Widget);
    ~B9Terminal();
    
    //bool isConnected(){return pPrinterComm->isConnected();}
    bool isConnected(){return true;}

    double getHardZDownMM(){return pSettings->m_dHardZDownMM;}

    QTime getEstCompleteTime(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS);
    int getEstCompleteTimeMS(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS);

    void setUsePrimaryMonitor(bool bFlag){m_bUsePrimaryMonitor=bFlag; }
    void setPrintPreview(bool bFlag){m_bPrintPreview=bFlag; }
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize

    //int getXYPixelSize(){return pPrinterComm->getXYPixelSize();}
    int getXYPixelSize(){return 1;}
    void setIsPrinting(bool bFlag){}
            //pPrinterComm->m_bIsPrinting = bFlag;}
private:
    PCycleSettings *pSettings;
    int m_iD, m_iE, m_iJ, m_iK, m_iL, m_iW, m_iX;
    void resetLastSentCycleSettings();

    Ui::B9Terminal *ui;
    B9Projector *pProjector;
    B9PrinterComm *pPrinterComm;
    QDesktopWidget* m_pDesktop;

    int m_iFillLevel;

    bool m_bUsePrimaryMonitor;
    bool m_bPrintPreview;
    bool m_bPrimaryScreen;
    bool m_bNeedsWarned;

public slots:
    void rcResetHomePos();

    void rcBasePrint(double dBaseMM); // Position for Base Layer Exposure.
    void rcNextPrint(double dNextMM); // Position for Next Layer Exposure.
    void rcSetWarmUpDelay(int iDelayMS);
    void rcIsMirrored(bool bIsMirrored);
    void rcResetCurrentPositionPU(int iCurPos);

    void rcProjectorPwr(bool bPwrOn);
    void rcSetCPJ(CrushedPrintJob *pCPJ); // Set the pointer to the CMB to be displayed, NULL if blank
    void rcGotoFillAfterReset(int iFillLevel);

    void onScreenCountChanged(int iCount = 0);  // Signal that the number of monitors has changed

    void on_pushButtonPrintBase_clicked();
    void on_pushButtonPrintNext_clicked();

signals:
    void PrintReleaseCycleFinished();

    void sendStatusMsg(QString text);					// signal to the Projector window to change the status msg
    void sendGrid(bool bshow);							// signal to the Projector window to update the grid display
    void sendCPJ(CrushedPrintJob * pCPJ);				// signal to the Projector window to show a crushed bit map image
    void sendXoff(int xOff);							// signal to the Projector window to update the X offset
    void sendYoff(int yOff);							// signal to the Projector window to update the Y offset

private slots:
    void makeProjectorConnections();

    void onBC_PrintReleaseCycleFinished();

    void on_pushButtonCmdReset_clicked(); // Remote slot for commanding Reset (find home) motion
};

#endif // B9TERMINAL_H
