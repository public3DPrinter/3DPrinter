#ifndef B9PRINT_H
#define B9PRINT_H

#include <QDialog>
#include <QDateTime>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include "b9terminal.h"

namespace Ui {
class B9Print;
}

class B9Print : public QDialog
{
    Q_OBJECT
    
public:
    explicit B9Print(B9Terminal *pTerm, QWidget *parent = 0);
    ~B9Print();
    // If PrintPreview we do not power up the projector.  If UsePrimaryMonitor we force the output to the primary monitor
    void print3D(CrushedPrintJob *pCPJ, int iXOff, int iYOff, int iTbase, int iTover, int iTattach, int iNumAttach = 1, int iLastLayer = 0, bool bPrintPreview = false, bool bUsePrimaryMonitor = false, bool bIsMirrored = false);
    
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize

private:
    Ui::B9Print *ui;
    enum {PRINT_NO, PRINT_SETUP1, PRINT_SETUP2, PRINT_RELEASING, PRINT_EXPOSING, PRINT_ABORT, PRINT_DONE};
    enum {PAUSE_NO, PAUSE_WAIT, PAUSE_YES};
    int m_iMinimumTintMS;
    QSettings m_vSettings;
    B9Terminal* m_pTerminal;
    CrushedPrintJob* m_pCPJ;
    int m_iTbase,  m_iTover, m_iTattach, m_iNumAttach;
    int m_iXOff, m_iYOff;
    int m_iPrintState;
    int m_iCurLayerNumber;
    int m_iLastLayer;
    int m_iPaused;
    bool m_bAbort;
    QTime m_vClock;
    double m_dLayerThickness;

    void setSlice(int iSlice);

    QTimer *testTimer;
    int count;

private slots:
    //void testLoop();
    void exposeTBaseLayer();
    void startExposeTOverLayers();
    void exposureOfCurTintLayerFinished();
    void exposureOfTOverLayersFinished();
};

#endif // B9PRINT_H
