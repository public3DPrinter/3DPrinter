#ifndef TEMP_H
#define TEMP_H

#include <QObject>
#include <QTime>
#include <QtDebug>

#define FIRMWAREUPDATESPEED 115200
#define FIRMWAREHEXFILE "B9Firmware.hex"
#define MSG_SEARCHING "Searching..."
#define MSG_CONNECTED "Connected"
#define MSG_FIRMUPDATE "Updating Firmware..."

class  QextSerialPort;
class  QextSerialEnumerator;
struct QextPortInfo;

class B9PrinterComm : public QObject
{
    Q_OBJECT
public:
    B9PrinterComm();
    ~B9PrinterComm();

signals:
    void BC_PrintReleaseCycleFinished();  // Broadcast when we receive the "F" notice from the printer

public slots:
    void SendCmd(QString sCmd);

private slots:
    void ReadAvailable();
    void RefreshCommPortItems();

public:
    bool m_bIsPrinting;

private:
    QextSerialPort *m_serialDevice;
    QextSerialEnumerator *pEnumerator;		// enumerator for finding available comm ports
    QList<QextPortInfo> *pPorts;			// list of available comm ports
    QString sNoFirmwareAurdinoPort;         // if we locate an arduino without firmware, set this to the portname as a flag

    QString m_sSerialString; // Used by ReadAvailable to store current broadcast line

    bool OpenB9CreatorCommPort(QString sPortName);
    void handleLostComm();

};

#endif // TEMP_H
