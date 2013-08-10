#ifndef B9PROJECTOR_H
#define B9PROJECTOR_H

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QByteArray>
#include "crushbitmap.h"

class B9Projector : public QWidget
{
    Q_OBJECT
public:
    B9Projector(bool bPrintWindow, QWidget *parent = 0, Qt::WFlags flags = Qt::WindowMinMaxButtonsHint|Qt::Window|Qt::WindowCloseButtonHint);
    B9Projector();
    ~B9Projector();

public slots:
    void hideCursor(){setCursor(Qt::BlankCursor);}	// Hide the mouse cursor when signaled

    void setShowGrid(bool bShow);					// Set bShow to true if Grid is to be drawn
    void setStatusMsg(QString status);				// Set status to the message to be displayed
    void setCPJ(CrushedPrintJob *pCPJ);				// Set the pointer to the CMB to be displayed, NULL if blank
    void setXoff(int xOff){m_xOffset = xOff;drawAll();} // x offset for layer image
    void setYoff(int yOff){m_yOffset = yOff;drawAll();} // y offset for layer image
    void createToverMap(int iRadius);
    void createNormalizedMask(double XYPS=0.1, double dZ = 257.0, double dOhMM = 91.088); //call when we show or resize

signals:
    void hideProjector();			// signal to the parent requesting we be hidden

    void newGeometry (int iScreenNumber, QRect geoRect);

private:
    void paintEvent (QPaintEvent * pEvent);			// Handle paint events
    void resizeEvent ( QResizeEvent * event );      // Handle resize events

    void drawAll();			// refresh the entire screen from scratch
    void blankProjector();	// fills the background in with black, overwrites previous image
    void drawGrid();		// draws a grid pattern using mGridColor
    void drawStatusMsg();	// draws the current status msg on the projector screen
    void drawCBM();			// draws the current CBM pointed to by mpCBM, returns if mpCBM is null

    void createToverMap0();
    void createToverMap1();
    void createToverMap2();
    void createToverMap3();

    bool m_bIsPrintWindow;  // set to true if we lock the window to full screen when shown
    bool m_bGrid;			// if true, grid is to be drawn
    CrushedPrintJob* mpCPJ;	// CPJ to inflate CBM from
    QImage mImage;
    QImage mCurSliceImage;  // Current Normalized slice, possible that has some or all pixels cleared
    int m_iLevel;
    QImage m_NormalizedMask;
    QString mStatusMsg;
    int m_xOffset, m_yOffset;
    QByteArray m_vToverMap;
};

#endif // B9PROJECTOR_H
