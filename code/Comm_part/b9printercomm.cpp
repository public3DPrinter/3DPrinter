#include <QtGui/QApplication>
#include <QProcess>
#include <QTimer>
#include "b9printercomm.h"
#include "qextserialport.h"
#include "qextserialenumerator.h"
#include <termios.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
B9PrinterComm::B9PrinterComm()
{
    m_bIsPrinting = false;
    pPorts = new QList<QextPortInfo>;
    pEnumerator = new QextSerialEnumerator();
    m_serialDevice = NULL;
    //m_bCloneBlanks = false;
    //m_Status.reset();
    //m_iWarmUpDelayMS = 15000;
    qDebug() << "B9Creator COMM Start";
    QTimer::singleShot(500, this, SLOT(RefreshCommPortItems())); // Check in .5 seconds
}

B9PrinterComm::~B9PrinterComm()
{
    if(pPorts)delete pPorts;
    if(pEnumerator) pEnumerator->deleteLater();
    if(m_serialDevice) delete m_serialDevice;
    qDebug() << "B9Creator COMM End";
}

void B9PrinterComm::RefreshCommPortItems()
{
    if(m_bIsPrinting)return; // We assume we stay connected during the print proccess.  If we are disconnected, the watchdog timer will fire
    QString sCommPortStatus = MSG_SEARCHING;
    QString sCommPortDetailedStatus = MSG_SEARCHING;
    QString sPortName;
    // Load the current enumerated available ports
    *pPorts = pEnumerator->getPorts();

    if(m_serialDevice){
        // We've previously located the printer, are we still connected?
        for (int i = 0; i < pPorts->size(); i++) {
        // Check each existing port to see if our's still exists
        #ifdef Q_OS_LINUX
            if(pPorts->at(i).physName == m_serialDevice->portName()){
        #else
            if(pPorts->at(i).portName == m_serialDevice->portName()){
        #endif
                // We're still connected, set a timer to check again in 5 seconds and then exit
                QTimer::singleShot(5000, this, SLOT(RefreshCommPortItems()));
                return;
            }
        }
        // We lost the previous connection and should delete this port connection
        if (m_serialDevice->isOpen())
            m_serialDevice->close();
        delete m_serialDevice;
        m_serialDevice = NULL;
        //m_Status.reset();
        qDebug() << sCommPortStatus;
        //emit updateConnectionStatus(sCommPortStatus);
        sCommPortDetailedStatus = "Lost Comm on previous port. Searching...";
        //emit BC_ConnectionStatusDetailed("Lost Comm on previous port. Searching...");
        handleLostComm();
    }

    // Now we search for a B9Creator
    int eTime = 1000;  // 1 second search cylce time for while not connected

    sNoFirmwareAurdinoPort = "";  // Reset to null string before scanning ports
    if(pPorts->size()>0){
        // Some ports are available, are they the B9Creator?
        //qDebug() << "Scanning For Serial Port Devices (" << pPorts->size() << "found )";
        for (int i = 0; i < pPorts->size(); i++) {
            //qDebug() << "  port name   " << pPorts->at(i).portName;
            //qDebug() << "  locationInfo" << pPorts->at(i).physName;
         #ifndef Q_OS_LINUX
            //Note: We only trust friendName, vendorID and productID with Windows and OS_X
            qDebug() << "  description " << pPorts->at(i).friendName;
            qDebug() << "  vendorID    " << pPorts->at(i).vendorID;
            qDebug() << "  productID   " << pPorts->at(i).productID;
         #endif
         #ifdef Q_OS_LINUX
            // linux ID's ports by physName
            sPortName = pPorts->at(i).physName;
            // We filter ports by requiring the portName to begin with "ttyA"
            if(pPorts->at(i).portName.left(4) == "ttyA" && OpenB9CreatorCommPort(sPortName)){
         #else
            // Windows and OSX use portName to ID ports
            sPortName = pPorts->at(i).portName;


            // We filter ports by requiring vendorID value of 9025 (Arduino)
            if(pPorts->at(i).vendorID==9025 && OpenB9CreatorCommPort(sPortName)){
         #endif
                // Connected!
                sCommPortStatus = MSG_CONNECTED;
                sCommPortDetailedStatus = "Connected on Port: "+m_serialDevice->portName();
                eTime = 5000;  // Re-check connection again in 5 seconds
                //if(m_serialDevice && m_Status.isCurrentVersion())startWatchDogTimer();  // Start B9Creator "crash" watchDog
                break;
            }
        }
        /*bool bUpdateFirmware = false;
        if( m_serialDevice==NULL && sNoFirmwareAurdinoPort!=""){
            // We did not find a B9Creator with valid firmware, but we did find an Arduino
            // We assume this is a new B9Creator and needs firmware!
            // However, we will not upload firmware unless m_bCloneBlanks is true!
            if(m_bCloneBlanks){
                qDebug() << "\"Clone Firmware\" Option is enabled.  Attempting Firmware Upload to possible B9Creator found on port: "<< sNoFirmwareAurdinoPort;
                bUpdateFirmware = true;
                sPortName = sNoFirmwareAurdinoPort;
            }
            else {
                qDebug() << "\"Clone Firmware\" Option is disabled.  No Firmware upload attempted on possible B9Creator found on port: " << sNoFirmwareAurdinoPort;
                bUpdateFirmware = false;
            }
        }
        else if (m_serialDevice!=NULL && !m_Status.isCurrentVersion()){
            // We found a B9Creator with the wrong firmware version, update it!
            qDebug() << "Incorrect Firmware version found on connected B9Creator"<< sPortName << "  Attempting B9Creator Firmware Update";
            bUpdateFirmware = true;
            if(m_serialDevice!=NULL) {
                m_serialDevice->flush();
                m_serialDevice->close();

                delete m_serialDevice;

            }
            m_serialDevice = NULL;
            m_Status.reset();
        }
        if(bUpdateFirmware){
            // Update the firmware on device on sPortName
            emit updateConnectionStatus(MSG_FIRMUPDATE);
            emit BC_ConnectionStatusDetailed("Updating Firmware on port: "+sPortName);
            B9FirmwareUpdate Firmware;
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            Firmware.UploadHex(sPortName);
            QApplication::restoreOverrideCursor();
            emit updateConnectionStatus(MSG_SEARCHING);
            emit BC_ConnectionStatusDetailed("Firmware Update Complete.  Searching...");
        }
        emit updateConnectionStatus(sCommPortStatus);
        emit BC_ConnectionStatusDetailed(sCommPortDetailedStatus);*/
    }
    QTimer::singleShot(eTime, this, SLOT(RefreshCommPortItems())); // Check again in 5 seconds if connected, 1 secs if not
}

bool B9PrinterComm::OpenB9CreatorCommPort(QString sPortName)
{
    if(m_serialDevice!=NULL) qFatal("Error:  We found an open port handle that should have been deleted!");
    //char* device = "/dev/ttyACM0";

    //qDebug() << open(device,O_RDWR | O_NOCTTY | O_NDELAY);;

    // Attempt to establish a serial connection with the B9Creator
    m_serialDevice = new QextSerialPort(sPortName, QextSerialPort::EventDriven, this);
    if (m_serialDevice->open(QIODevice::ReadWrite) == true) {
        m_serialDevice->setBaudRate(BAUD115200);
        m_serialDevice->setDataBits(DATA_8);
        m_serialDevice->setParity(PAR_NONE);
        m_serialDevice->setStopBits(STOP_1);
        m_serialDevice->setFlowControl(FLOW_OFF);
        m_serialDevice->setDtr(true);   // Reset the Aurduino
        m_serialDevice->setDtr(false);

        connect(m_serialDevice, SIGNAL(readyRead()), this, SLOT(ReadAvailable()));
        qDebug() << "Opened Comm port:" << sPortName;
    }
    else {
        // device failed to open
        if(m_serialDevice!=NULL) delete m_serialDevice;
        m_serialDevice = NULL;
        //m_Status.reset();
        qDebug() << "Failed to open Comm port:" << sPortName;
        return false;
    }

    // Delay for up to 5 seconds while we wait for response from printer
    /*QTime delayTime = QTime::currentTime().addSecs(5);
    while( QTime::currentTime() < delayTime && !m_Status.isValidVersion() )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    if(!m_Status.isValidVersion()){
        if(m_serialDevice!=NULL) {
            m_serialDevice->flush();
            m_serialDevice->close();
            delete m_serialDevice;
        }
        m_serialDevice = NULL;
        m_Status.reset();
        qDebug() << "Found a possible Arduino Device, perhaps a B9Creator without firmware loaded, on Port: " << sPortName;
        sNoFirmwareAurdinoPort = sPortName;
        return false;
    }*/
    return true;
}

void B9PrinterComm::handleLostComm(){
//    emit BC_LostCOMM();
    qDebug() << "lost comm";
}

void B9PrinterComm::ReadAvailable() {
    if(m_serialDevice==NULL) qFatal("Error:  slot 'ReadAvailable()' but NULL Port Handle");

    //m_Status.resetLastMsgTime();  //update for watchdog

    //if(m_Status.getHomeStatus() == B9PrinterStatus::HS_SEEKING) {
        //m_Status.setHomeStatus(B9PrinterStatus::HS_UNKNOWN); // if we are receiving data, we must no longer be seeking Home
        // we'll set the status to HS_FOUND once we recieve a 'X' diff broadcast
    //}
    QByteArray ba = m_serialDevice->readAll();  // read block of available raw data
    qDebug() << "receive " << ba.data();
    // We process the raw data one line at a time, keeping in mind they may be spread across multiple blocks.
    /*int iCurPos = 0;
    int iLampHrs = -1;
    int iInput = -1;
    char c;
    while(iCurPos<ba.size()){
        c = ba.at(iCurPos);
        iCurPos++;
        if(c!='\r') m_sSerialString+=QString(c);
        if(c=='\n'){
            // Line Read Complete, process data

           if(m_sSerialString.left(1) != "P" && m_sSerialString.left(1) != "L" && m_sSerialString.length()>0){
                // We only emit this data for display & log purposes
                if(m_sSerialString.left(1) == "C"){
                    qDebug() << m_sSerialString.right(m_sSerialString.length()-1) << "\n";
                }
                else{
                    //emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
            }

            int iCmdID = m_sSerialString.left(1).toUpper().toAscii().at(0);
            qDebug() << "receive " << iCmdID;
            switch (iCmdID){
            case 'F':
            case 'f':
                qDebug() << "receive F";
                break;
            /*case 'U':  // Mechanical failure of encoder?
                qDebug() << "WARNING:  Printer has sent 'U' report, runaway X Motor indication: " + m_sSerialString << "\n";
                break;
            case 'Q':  // Printer got tired of waiting for command and shut down projectors
                       // We will likely never see this as something bad has happened
                       // (like we have crashed or been shut off during a print process
                       // So if we get it, we'll just post it to the log and wait for
                       // timeouts to correct things.
                qDebug() << "WARNING:  Printer has sent 'Q' report, lost comm with host." << "\n";
                break;
            case 'P':  // take care of projector status
                iInput = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(iInput!=1)iInput = 0;
                if(handleProjectorBC(iInput)){
                    //projector status changed
                    emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
                break;
            case 'L':  // take care of projector Lamp hours update
                iLampHrs = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(m_Status.getLampHrs()!= iLampHrs){
                    m_Status.setLampHrs(iLampHrs);
                    emit BC_ProjectorStatusChanged();
                    emit BC_RawData(m_sSerialString);
                    qDebug() << m_sSerialString << "\n";
                }
                break;

            case 'X':  // Found Home with this Difference Offset
                m_Status.setHomeStatus(B9PrinterStatus::HS_FOUND);
                m_Status.setLastHomeDiff(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HomeFound();
                break;

            case 'R':  // Needs Reset?
                iInput = m_sSerialString.right(m_sSerialString.length()-1).toInt();
                if(iInput==0) m_Status.setHomeStatus(B9PrinterStatus::HS_FOUND);
                else m_Status.setHomeStatus(B9PrinterStatus::HS_UNKNOWN);
                break;

            case 'K':  // Current Lamp 1/2 life
                m_Status.setHalfLife(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HalfLife(m_Status.getHalfLife());
                break;

            case 'D':  // Current Native X Projector resolution
                m_Status.setNativeX(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_NativeX(m_Status.getNativeX());
                break;

            case 'E':  // Current Native Y Projector resolution
                m_Status.setNativeY(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_NativeY(m_Status.getNativeY());
                break;

            case 'H':  // Current XY Pixel Size
                m_Status.setXYPixelSize(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_XYPixelSize(m_Status.getXYPixelSize());
                break;

            case 'I':  // Current PU
                m_Status.setPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_PU(m_Status.getPU());
                break;

            case 'A':  // Projector Control capability
                m_Status.setProjectorRemoteCapable(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_ProjectorRemoteCapable(m_Status.isProjectorRemoteCapable());
                break;

            case 'J':  // Projector Shutter capability
                m_Status.setHasShutter(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_HasShutter(m_Status.hasShutter());
                break;

            case 'M':  // Current Z Upper Limit in PUs
                m_Status.setUpperZLimPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_UpperZLimPU(m_Status.getUpperZLimPU());
                break;

            case 'Z':  // Current Z Position Update
                m_Status.setCurZPosInPU(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_CurrentZPosInPU(m_Status.getCurZPosInPU());
                break;

            case 'S':  // Current Vat(Shutter) Percent Open Position Update
                m_Status.setCurVatPercentOpen(m_sSerialString.right(m_sSerialString.length()-1).toInt());
                emit BC_CurrentVatPercentOpen(m_Status.getCurVatPercentOpen());
                break;

            case 'C':  // Comment
                emit BC_Comment(m_sSerialString.right(m_sSerialString.length()-1));
                break;

            case 'F':  // Print release cycle finished
                emit BC_PrintReleaseCycleFinished();
                break;

            case 'V':  // Version
                m_Status.setVersion(m_sSerialString.right(m_sSerialString.length()-1).trimmed());
                emit BC_FirmVersion(m_Status.getVersion());
                break;

            case 'W':  // Model
                m_Status.setModel(m_sSerialString.right(m_sSerialString.length()-1).trimmed());
                emit BC_ModelInfo(m_Status.getModel());
                break;

            case 'Y':  // Current Z Home position in PU's
                // ignored
                break;

            default:
                qDebug() <<"WARNING:  IGNORED UNKNOWN CMD:  " << m_sSerialString << "\n";
                break;
            }
            m_sSerialString=""; // Line processed, clear it for next line
        }
    }*/
}

void B9PrinterComm::SendCmd(QString sCmd)
{
    if(m_serialDevice)m_serialDevice->write(sCmd.toAscii()+'\n');
    //if(sCmd == "r" || sCmd == "R") m_Status.setHomeStatus(B9PrinterStatus::HS_SEEKING);
    qDebug() << "SendCmd->" << sCmd;
}
