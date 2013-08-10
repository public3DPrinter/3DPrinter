#include "dlgprintprep.h"
#include "ui_dlgprintprep.h"

DlgPrintPrep::DlgPrintPrep(CrushedPrintJob* pCPJ, B9Terminal* pTerminal, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPrintPrep)
{
    ui->setupUi(this);
    m_bInitializing = true;
    m_pTerminal = pTerminal;
    m_pCPJ = pCPJ;
    m_iTattachMS=0;
    m_iNumAttach = 1;
    m_iTbaseMS=0;
    m_iToverMS=0;
    m_bDryRun = false;
    m_bMirrored = false;

    m_iLastLayer = 0;
}

DlgPrintPrep::~DlgPrintPrep()
{
    delete ui;
}
