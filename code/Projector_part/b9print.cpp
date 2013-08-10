#include "b9print.h"
#include "ui_b9print.h"

B9Print::B9Print(B9Terminal *pTerm, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::B9Print)
{
    m_pTerminal = pTerm;
    ui->setupUi(this);

    testTimer = new QTimer(this);
    connect(testTimer, SIGNAL(timeout()), this, SLOT(testLoop()));
}

B9Print::~B9Print()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////////////////
void B9Print::print3D(CrushedPrintJob* pCPJ, int iXOff, int iYOff, int iTbase, int iTover, int iTattach, int iNumAttach, int iLastLayer, bool bPrintPreview, bool bUsePrimaryMonitor, bool bIsMirrored)
{
    // Note if, iLastLayer < 1, print ALL layers.
    // if bPrintPreview, run without turning on the projector

    m_iMinimumTintMS = m_vSettings.value("m_iMinimumTintMS",50).toInt(); // Grab the old value
    if(m_iMinimumTintMS>500)
        m_iMinimumTintMS=555; // Should never get this big, fix it.
    else if (m_iMinimumTintMS<50)
        m_iMinimumTintMS=56;  // Or this little
    m_vSettings.setValue("m_iMinimumTintMS",(int)((double)m_iMinimumTintMS*.9)); //Set it back to 90% of it last value, just to keep it close to the edge

    m_iPrintState = PRINT_NO;
    m_pTerminal->setEnabled(false);
    m_pCPJ = pCPJ;
    m_pTerminal->createNormalizedMask(m_pCPJ->getXYPixelmm());
    m_iTbase = iTbase; m_iTover = iTover; m_iTattach = iTattach; m_iNumAttach = iNumAttach;
    m_iXOff = iXOff; m_iYOff = iYOff;
    m_iCurLayerNumber = 0;
    m_iPaused = PAUSE_NO;
    m_bAbort = false;
    m_iLastLayer = iLastLayer;
    if(m_iLastLayer<1)m_iLastLayer = m_pCPJ->getTotalLayers();

    m_pTerminal->setUsePrimaryMonitor(bUsePrimaryMonitor);
    m_pTerminal->setPrintPreview(bPrintPreview);
    m_pTerminal->onScreenCountChanged();

    ui->lineEditJobName->setText(m_pCPJ->getName());
    ui->lineEditJobDescription->setText(m_pCPJ->getDescription());
    ui->progressBarPrintProgress->setMinimum(0);
    ui->progressBarPrintProgress->setMaximum(m_iLastLayer);
    ui->progressBarPrintProgress->setValue(0);
    ui->lineEditLayerCount->setText("Total Layers To Print: "+QString::number(m_iLastLayer)+"  Powering up the projector.");

    ui->lcdNumberTime->display(m_pTerminal->getEstCompleteTime(m_iCurLayerNumber,m_iLastLayer,m_pCPJ->getZLayermm(),m_iTbase+m_iTover).toString("hh:mm"));
    //QString sTimeUpdate = updateTimes();
    //setProjMessage("Total Layers to print: "+QString::number(m_iLastLayer)+"  "+sTimeUpdate);

    if(!bPrintPreview){
        // Turn on the projector and set the warm up time in ms
        ui->pushButtonPauseResume->setEnabled(false);
        ui->pushButtonAbort->setEnabled(false);
        m_pTerminal->rcIsMirrored(bIsMirrored);
        m_pTerminal->rcSetWarmUpDelay(20000);
        m_pTerminal->rcProjectorPwr(true);
    }
    else {
        ui->lineEditProjectorStatus->setText("OFF:  'Print Preview' Mode");
        ui->pushButtonPauseResume->setEnabled(true);
        ui->pushButtonAbort->setEnabled(true);
        m_iPrintState = PRINT_SETUP1;
        m_dLayerThickness = m_pCPJ->getZLayer().toDouble();
        m_pTerminal->rcBasePrint(-m_pTerminal->getHardZDownMM()); // Dynamic Z Zero, overshoot zero until we are down hard and motor 'skips'
    }
    qDebug() << "end of B9Print::print3D";
    count = 0;
    testTimer->start(2000);
}

void B9Print::testLoop() {
    testTimer->stop();
    if(count < 30) {
        exposeTBaseLayer();
        m_iCurLayerNumber++;
        count++;
        testTimer->start(2000);
    }

}

void B9Print::exposeTBaseLayer(){
    //Release & reposition cycle completed, time to expose the new layer
    if(m_iPrintState==PRINT_NO || m_iPrintState == PRINT_ABORT)return;

    if(m_iPrintState==PRINT_SETUP1){
        //We've used -  getHardZDownMM()to overshoot and get to here,
        // now reset current position to 0 and move up to + getZFlushMM
        //m_pTerminal->rcResetCurrentPositionPU(0);
        //m_pTerminal->rcBasePrint(m_pTerminal->getZFlushMM());
        m_iPrintState = PRINT_SETUP2;
        //return;
    }

    if(m_iPrintState==PRINT_SETUP2){
        //We should now be flush
        // reset current position 0 and continue
        //m_pTerminal->rcResetCurrentPositionPU(0);
        m_iPrintState = PRINT_RELEASING;
    }

    if(m_iPrintState == PRINT_DONE){
//        m_iPrintState=PRINT_NO;
//        m_pTerminal->setEnabled(true);
//        if(m_pTerminal->getPrintPreview()){
//            m_pTerminal->setPrintPreview(false);
//            m_pTerminal->setUsePrimaryMonitor(false);
//        }
//        m_pTerminal->onScreenCountChanged();
//        hide();
//        return;
    }

    if(m_bAbort){
        // We're done, release and raise
        m_pTerminal->rcSetCPJ(NULL); //blank
        ui->pushButtonAbort->setText("Abort");
         m_iPrintState = PRINT_ABORT;
        //on_signalAbortPrint();
        return;
    }

    //Start Tbase Print exposure
    ui->progressBarPrintProgress->setValue(m_iCurLayerNumber+1);
    ui->lineEditLayerCount->setText("Creating Layer "+QString::number(m_iCurLayerNumber+1)+" of "+QString::number(m_iLastLayer)+",  "+QString::number(100.0*(double)(m_iCurLayerNumber+1)/(double)m_iLastLayer,'f',1)+"% Complete");
    setSlice(m_iCurLayerNumber);
    //m_vClock.start(); // image is out there, start the clock running!
    //QString sTimeUpdate = updateTimes();
    //if(m_iPaused==PAUSE_WAIT){
        //ui->lineEditLayerCount->setText("Pausing...");
        //setProjMessage("Pausing...");
    //}
    //else{
        //setProjMessage("(Press'p' to pause, 'A' to ABORT)  " + sTimeUpdate+"  Creating Layer "+QString::number(m_iCurLayerNumber+1)+" of "+QString::number(m_iLastLayer));
    //}
    m_iPrintState = PRINT_EXPOSING;
    // set timer
    //int iAdjExposure = m_pTerminal->getLampAdjustedExposureTime(m_iTbase);
    //if(m_iCurLayerNumber<m_iNumAttach) iAdjExposure = m_pTerminal->getLampAdjustedExposureTime(m_iTattach);  //First layers may have different exposure timing
    //if(iAdjExposure>0){
        //QTimer::singleShot(iAdjExposure-m_vClock.elapsed(), this, SLOT(startExposeTOverLayers()));
        //return;
    //}
    //else
    //{
        //startExposeTOverLayers(); // If this is getting called, we're taking too long!
        //qDebug() << "EXPOSURE TIMING ERROR:  Tbase exposed for too long!, Tbase is set too small or computer too slow?" << iAdjExposure;
        //return;
    //}
}

void B9Print::setSlice(int iSlice)
{
    qDebug() << "m_iLastLayer = " << m_iLastLayer;
    qDebug() << "iSlice = " << iSlice;
    if(m_iLastLayer<1)
        m_pTerminal->rcSetCPJ(NULL);
    else {
        m_pCPJ->setCurrentSlice(iSlice);
        m_pTerminal->rcSetCPJ(m_pCPJ);
    }
}
