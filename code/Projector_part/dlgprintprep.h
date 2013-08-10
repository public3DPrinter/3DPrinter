#ifndef DLGPRINTPREP_H
#define DLGPRINTPREP_H

#include <QDialog>
#include "b9terminal.h"

namespace Ui {
class DlgPrintPrep;
}

class DlgPrintPrep : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgPrintPrep(CrushedPrintJob* pCPJ, B9Terminal* pTerminal, QWidget *parent = 0);
    ~DlgPrintPrep();
    
private:
    Ui::DlgPrintPrep *ui;
    CrushedPrintJob *m_pCPJ;
    B9Terminal *m_pTerminal;
    bool m_bInitializing;

public:
    int m_iTattachMS;
    int m_iNumAttach;
    int m_iTbaseMS;
    int m_iToverMS;
    bool m_bMirrored;
    bool m_bDryRun;
    int m_iLastLayer;
};

#endif // DLGPRINTPREP_H
