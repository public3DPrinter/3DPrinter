#include <QSettings>
#include "b9terminal.h"
#include "ui_b9terminal.h"

#define RAISE1 80
#define LOWER1 80
#define CLOSE1 100
#define OPEN1 50
#define BREATHE1 2.0
#define SETTLE1 3.0
#define OVERLIFT1 6.35

#define RAISE2 80
#define LOWER2 80
#define CLOSE2 50
#define OPEN2 50
#define BREATHE2 1.5
#define SETTLE2 3.5
#define OVERLIFT2 0.0

#define CUTOFF 0.5
#define HARDDOWN 0.0
#define UP2FLUSH 0.0

void PCycleSettings::loadSettings()
{
    QSettings settings;
    m_iRSpd1 = settings.value("RSpd1",RAISE1).toInt();
    m_iLSpd1 = settings.value("LSpd1",LOWER1).toInt();
    m_iCloseSpd1 = settings.value("CloseSpd1",CLOSE1).toInt();
    m_iOpenSpd1 = settings.value("OpenSpd1",OPEN1).toInt();
    m_dBreatheClosed1 = settings.value("BreatheClosed1",BREATHE1).toDouble();
    m_dSettleOpen1 = settings.value("SettleOpen1",SETTLE1).toDouble();
    m_dOverLift1 = settings.value("OverLift1",OVERLIFT1).toDouble();

    m_iRSpd2 = settings.value("RSpd2",RAISE2).toInt();
    m_iLSpd2 = settings.value("LSpd2",LOWER2).toInt();
    m_iCloseSpd2 = settings.value("CloseSpd2",CLOSE2).toInt();
    m_iOpenSpd2 = settings.value("OpenSpd2",OPEN2).toInt();
    m_dBreatheClosed2 = settings.value("BreatheClosed2",BREATHE2).toDouble();
    m_dSettleOpen2 = settings.value("SettleOpen2",SETTLE2).toDouble();
    m_dOverLift2 = settings.value("OverLift2",OVERLIFT2).toDouble();

    m_dBTClearInMM = settings.value("BTClearInMM",CUTOFF).toDouble();

    m_dHardZDownMM = settings.value("HardDownZMM",HARDDOWN).toDouble();
    m_dZFlushMM    = settings.value("ZFlushMM",   UP2FLUSH).toDouble();
}

B9Terminal::B9Terminal(QWidget *parent, Qt::WFlags flags) :
    QWidget(parent, flags),
    ui(new Ui::B9Terminal)
{
    setWindowFlags(Qt::WindowContextHelpButtonHint);
    m_bNeedsWarned = true;
    m_bPrimaryScreen = false;

    ui->setupUi(this);

    pSettings = new PCycleSettings;
    resetLastSentCycleSettings();

    // Always set up the B9Projector in the Terminal constructor
    m_pDesktop = QApplication::desktop();
    pProjector = new B9Projector();

    connect(m_pDesktop, SIGNAL(screenCountChanged(int)),this, SLOT(onScreenCountChanged(int)));
}

B9Terminal::~B9Terminal()
{
    delete ui;
    delete pProjector;
}

void B9Terminal::resetLastSentCycleSettings(){
    m_iD=m_iE=m_iJ=m_iK=m_iL=m_iW=m_iX = -1;
}

void B9Terminal::createNormalizedMask(double XYPS, double dZ, double dOhMM)
{
    //call when we show or resize
    pProjector->createNormalizedMask(XYPS, dZ, dOhMM);
}

void B9Terminal::onScreenCountChanged(int iCount){
    QString sVideo = "Disconnected or Primary Monitor";
    if(pProjector) {
        delete pProjector;
        pProjector = NULL;
        //if(pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_ON)
            //if(!isEnabled())emit signalAbortPrint("Print Aborted:  Connection to Projector Lost or Changed During Print Cycle");
    }
    pProjector = new B9Projector(true, 0,Qt::Window);
    makeProjectorConnections();
    int i=iCount;
    int screenCount = m_pDesktop->screenCount();
    QRect screenGeometry;

    if(m_bUsePrimaryMonitor)
    {
        screenGeometry = m_pDesktop->screenGeometry(0);
    }
    else{
        for(i=screenCount-1;i>= 0;i--) {
            screenGeometry = m_pDesktop->screenGeometry(i);
            //if(screenGeometry.width() == pPrinterComm->getNativeX() && screenGeometry.height() == pPrinterComm->getNativeY()) {
                //Found the projector!
                sVideo = "Connected to Monitor: " + QString::number(i+1);
                m_bNeedsWarned = true;
                break;
            //}
        }
    }
    if(i<=0||m_bUsePrimaryMonitor)m_bPrimaryScreen = true; else m_bPrimaryScreen = false;

    //emit updateProjectorOutput(sVideo);

    pProjector->setShowGrid(false);
    pProjector->setCPJ(NULL);

    //emit sendStatusMsg("B9Creator - Idle");
    pProjector->setGeometry(screenGeometry);
    if(!m_bPrimaryScreen){
        pProjector->showFullScreen(); // Only show it if it is a secondary monitor
        pProjector->hide();
        activateWindow(); // if not using primary monitor, take focus back to here.
    }
    else if(m_bPrintPreview) {
        pProjector->showFullScreen();
    }
    /*else if(m_bPrintPreview||(pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_OFF &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_COOLING &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_UNKNOWN)) {
        // if the projector is not turned off, we better put up the blank screen now!
        pProjector->showFullScreen();
    }*/
    //else warnSingleMonitor();
}

void B9Terminal::makeProjectorConnections()
{
    // should be called any time we create a new projector object
    if(pProjector==NULL)return;
    //connect(pProjector, SIGNAL(keyReleased(int)),this, SLOT(getKey(int)));
    connect(this, SIGNAL(sendStatusMsg(QString)),pProjector, SLOT(setStatusMsg(QString)));
    connect(this, SIGNAL(sendGrid(bool)),pProjector, SLOT(setShowGrid(bool)));
    connect(this, SIGNAL(sendCPJ(CrushedPrintJob*)),pProjector, SLOT(setCPJ(CrushedPrintJob*)));
    connect(this, SIGNAL(sendXoff(int)),pProjector, SLOT(setXoff(int)));
    connect(this, SIGNAL(sendYoff(int)),pProjector, SLOT(setYoff(int)));
}

QTime B9Terminal::getEstCompleteTime(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    return QTime::currentTime().addMSecs(getEstCompleteTimeMS(iCurLayer, iTotLayers, dLayerThicknessMM, iExposeMS));
}

int B9Terminal::getEstCompleteTimeMS(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    //return estimated completion time
    int iTransitionPointLayer = (int)(pSettings->m_dBTClearInMM/dLayerThicknessMM);

    int iLowerCount = (int)(pSettings->m_dBTClearInMM/dLayerThicknessMM);
    int iUpperCount = iTotLayers - iLowerCount;

    if(iLowerCount>iTotLayers)iLowerCount=iTotLayers;
    if(iUpperCount<0)iUpperCount=0;

    if(iCurLayer<iTransitionPointLayer)iLowerCount = iLowerCount-iCurLayer; else iLowerCount = 0;
    if(iCurLayer>=iTransitionPointLayer) iUpperCount = iTotLayers - iCurLayer;

    int iTotalTimeMS = iExposeMS*iLowerCount + iExposeMS*iUpperCount;

    /*iTotalTimeMS = getLampAdjustedExposureTime(iTotalTimeMS);

    // Add Breathe and Settle
    iTotalTimeMS += iLowerCount*(pSettings->m_dBreatheClosed1 + pSettings->m_dSettleOpen1)*1000;
    iTotalTimeMS += iUpperCount*(pSettings->m_dBreatheClosed2 + pSettings->m_dSettleOpen2)*1000;

    // Z Travel Time
    int iGap1 = iLowerCount*(int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU());
    int iGap2 = iUpperCount*(int)(pSettings->m_dOverLift2*100000.0/(double)pPrinterComm->getPU());

    int iZRaiseDistance1 = iGap1 + iLowerCount*(int)(dLayerThicknessMM*100000.0/(double)pPrinterComm->getPU());
    int iZLowerDistance1 = iGap1;

    int iZRaiseDistance2 = iGap2 + iUpperCount*(int)(dLayerThicknessMM*100000.0/(double)pPrinterComm->getPU());
    int iZLowerDistance2 = iGap2;

    iTotalTimeMS += getZMoveTime(iZRaiseDistance1,pSettings->m_iRSpd1);
    iTotalTimeMS += getZMoveTime(iZRaiseDistance2,pSettings->m_iRSpd2);
    iTotalTimeMS += getZMoveTime(iZLowerDistance1,pSettings->m_iLSpd1);
    iTotalTimeMS += getZMoveTime(iZLowerDistance2,pSettings->m_iLSpd2);

    // Vat movement Time
    iTotalTimeMS += iLowerCount*getVatMoveTime(pSettings->m_iOpenSpd1) + iLowerCount*getVatMoveTime(pSettings->m_iCloseSpd1);
    iTotalTimeMS += iUpperCount*getVatMoveTime(pSettings->m_iOpenSpd2) + iUpperCount*getVatMoveTime(pSettings->m_iCloseSpd2);
    */
    return iTotalTimeMS;
}

void B9Terminal::rcProjectorPwr(bool bPwrOn){
    //on_pushButtonProjPower_toggled(bPwrOn);
}

void B9Terminal::rcSetWarmUpDelay(int iDelayMS)
{
    //pPrinterComm->setWarmUpDelay(iDelayMS);
}

void B9Terminal::rcIsMirrored(bool bIsMirrored)
{
    //pPrinterComm->setMirrored(bIsMirrored);
}

void B9Terminal::rcResetCurrentPositionPU(int iCurPos){
    //pPrinterComm->SendCmd("O"+QString::number(iCurPos));
}

void B9Terminal::rcBasePrint(double dBaseMM)
{
    //setTgtAltitudeMM(dBaseMM);
    //on_pushButtonPrintBase_clicked();

}

void B9Terminal::rcSetCPJ(CrushedPrintJob *pCPJ)
{
    // Set the pointer to the CMB to be displayed, NULL if blank
    emit sendCPJ(pCPJ);
}
