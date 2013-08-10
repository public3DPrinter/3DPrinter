#include <QtGui>
#include "b9projector.h"

B9Projector::B9Projector(bool bPrintWindow, QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    hideCursor();
    setMouseTracking(true);
    m_bGrid = true;
    mStatusMsg = "B9Creator - www.b9creator.com";
    mpCPJ = NULL;
    m_xOffset = m_yOffset = 0;
    m_bIsPrintWindow = bPrintWindow;
    m_iLevel = -1;
}

B9Projector::B9Projector()
{
    mpCPJ = NULL;
}

B9Projector::~B9Projector()
{

}

void B9Projector::setStatusMsg(QString status)
{
    if(mStatusMsg == status) return;
    mStatusMsg = status;
    drawAll();
}

void B9Projector::setShowGrid(bool bShow)
{
    if(m_bGrid == bShow) return;
    m_bGrid = bShow;
    drawAll();
}

void B9Projector::setCPJ(CrushedPrintJob * pCPJ)
{
    qDebug() << "calling b9projector setCPJ";
    m_iLevel = -1;
    mpCPJ = pCPJ;
    drawAll();
}

void B9Projector::drawAll()
{
    blankProjector();
    drawGrid();
    drawStatusMsg();
    drawCBM();
    update();
}

void B9Projector::blankProjector()
{
    mImage.fill(qRgb(0,0,0));
}

void B9Projector::drawGrid()
{
    if(!m_bGrid) return;
    QPainter painter(&mImage);
    QColor color;
    color.setRgb(127,0,0);

    painter.setPen(QPen(color,1,Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));

    for(int x=0; x < width(); x+=100)
        painter.drawLine(x,0,x,height());
    painter.drawLine(width()-1,0,width()-1,height());
    for(int y=0; y < height(); y+=100)
        painter.drawLine(0,y,width(),y);
    painter.drawLine(0,height()-1,width(),height()-1);
}

void B9Projector::drawStatusMsg()
{

}

void B9Projector::createNormalizedMask( double XYPS, double dZ, double dOhMM)
{
    if(mpCPJ!=NULL)
        XYPS = mpCPJ->getXYPixelmm();
    double xsqrmm, ysqrmm;
    double zsqrmm = dZ*dZ;
    double Oh = dOhMM;
    double Ol = width()*XYPS*0.5;
    double dDimRg = 255.0/sqrt(Ol*Ol + Oh*Oh + zsqrmm);

    //Here we create a gray scale mask to normalize the projector's output
    m_NormalizedMask = QImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    m_NormalizedMask.fill(qRgba(0,0,0,0));
    int igs;
    for (quint32 y = 0; y < (quint32)m_NormalizedMask.height(); ++y) {
        QRgb *scanLine = (QRgb *)m_NormalizedMask.scanLine(y);
        for (quint32 x = 0; x < (quint32)m_NormalizedMask.width(); ++x){
            xsqrmm = pow((-Ol + (double)x*0.1),2.0);
            ysqrmm = pow(( Oh - (double)y*0.1),2.0);
            igs = sqrt(xsqrmm+ysqrmm+zsqrmm)*dDimRg;
            scanLine[x] = qRgba(igs,igs,igs,255);
        }
    }
}

void B9Projector::drawCBM()
{
    if(mpCPJ==NULL) return;
    if(m_iLevel<0)  // Only inflate and normalize the slice if we've not started clearing it
    {
        // Here we inflate the slice

        mCurSliceImage = QImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
        mCurSliceImage.fill(qRgba(0,0,0,0));
        mpCPJ->inflateCurrentSlice(&mCurSliceImage, m_xOffset, m_yOffset);
        createToverMap(3);  //calculate effect of pixels up to a radius of 3 pixel's away.

        // Here we copy the gray scale over using the slice as a mask
        QPainter mPainter(&mCurSliceImage);
        mPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        mPainter.drawImage(0,0,m_NormalizedMask);
    }

    // Here we copy the resulting normalized slice to the mImage
    QPainter mPainter2(&mImage);
    mPainter2.setCompositionMode(QPainter::CompositionMode_SourceOver);
    mPainter2.drawImage(0,0,mCurSliceImage);
}

void B9Projector::paintEvent (QPaintEvent * pEvent)
{
    QPainter painter(this);
    QRect dirtyRect = pEvent->rect();
    painter.drawImage(dirtyRect, mImage, dirtyRect);
}

void B9Projector::resizeEvent ( QResizeEvent * pEvent )
{
    pEvent->accept();
    QImage newImage(width(),height(),QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0,0), mImage);
    mImage = newImage;
    createNormalizedMask();
    drawAll();
    QDesktopWidget dt;
    emit newGeometry (dt.screenNumber(), geometry());
}

void B9Projector::createToverMap(int iRadius)
{
    switch (iRadius){
    case 0:
        createToverMap0();
        break;
    case 1:
        createToverMap1();
        break;
    case 2:
        createToverMap2();
        break;
    case 3:
        createToverMap3();
        break;
    default:
        break;
    }
}

void B9Projector::createToverMap3()
{
    int width = mCurSliceImage.width();
    int height = mCurSliceImage.height();
    QRgb *pixels = (QRgb *)mCurSliceImage.scanLine(0);
    double dBlobVal;
    m_vToverMap.resize(width*height);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            dBlobVal = 0;
            if(qAlpha(pixels[x + y*width])==255)
            { //we only care about set pixels...
                ///////////////////
                // R1 pattern
                //(-1,-1)
                if(!(x<1       ||        y<1)) if(qAlpha(pixels[(x - 1) + (y - 1)*width])==0) dBlobVal += 4.285122492;
                //( 0,-1)
                if(!(                    y<1)) if(qAlpha(pixels[(x    ) + (y - 1)*width])==0) dBlobVal += 3.030039172;
                //( 1,-1)
                if(!(x>=width-1||        y<1)) if(qAlpha(pixels[(x + 1) + (y - 1)*width])==0) dBlobVal += 4.285122492;

                //(-1,0)
                if(!(x<1                    )) if(qAlpha(pixels[(x - 1) + (y    )*width])==0) dBlobVal += 3.030039172;
                //( 1,0)
                if(!(x>=width-1             )) if(qAlpha(pixels[(x + 1) + (y    )*width])==0) dBlobVal += 3.030039172;

                //(-1, 1)
                if(!(x<1       ||y>=height-1)) if(qAlpha(pixels[(x - 1) + (y + 1)*width])==0) dBlobVal += 4.285122492;
                //( 0, 1)
                if(!(            y>=height-1)) if(qAlpha(pixels[(x    ) + (y + 1)*width])==0) dBlobVal += 3.030039172;
                //( 1, 1)
                if(!(x>=width-1||y>=height-1)) if(qAlpha(pixels[(x + 1) + (y + 1)*width])==0) dBlobVal += 4.285122492;

                ///////////////////////
                // R2
                //(-1,-2)
                if(!(x<1       ||        y<2)) if(qAlpha(pixels[(x - 1) + (y - 2)*width])==0) dBlobVal += 6.775373563;
                //( 0,-2)
                if(!(                    y<2)) if(qAlpha(pixels[(x    ) + (y - 2)*width])==0) dBlobVal += 6.060078344;
                //( 1,-2)
                if(!(x>=width-1||        y<2)) if(qAlpha(pixels[(x + 1) + (y - 2)*width])==0) dBlobVal += 6.775373563;


                //(-2,-1)
                if(!(x<2       ||       y <1)) if(qAlpha(pixels[(x - 2) + (y - 1)*width])==0) dBlobVal += 6.775373563;
                //(-2,0)
                if(!(x<2                    )) if(qAlpha(pixels[(x - 2) + (y    )*width])==0) dBlobVal += 6.060078344;
                //(-2,1)
                if(!(x<2       ||y>=height-1)) if(qAlpha(pixels[(x - 2) + (y + 1)*width])==0) dBlobVal += 6.775373563;

                //(2,-1)
                if(!(x>=width-2||       y <1)) if(qAlpha(pixels[(x + 2) + (y - 1)*width])==0) dBlobVal += 6.775373563;
                //(2,0)
                if(!(x>=width-2             )) if(qAlpha(pixels[(x + 2) + (y    )*width])==0) dBlobVal += 6.060078344;
                //(2,1)
                if(!(x>=width-2||y>=height-1)) if(qAlpha(pixels[(x + 2) + (y + 1)*width])==0) dBlobVal += 6.775373563;


                //(-1, 2)
                if(!(x<1       ||y>=height-2)) if(qAlpha(pixels[(x - 1) + (y + 2)*width])==0) dBlobVal += 6.775373563;
                //( 0, 2)
                if(!(            y>=height-2)) if(qAlpha(pixels[(x    ) + (y + 2)*width])==0) dBlobVal += 6.060078344;
                //( 1, 2)
                if(!(x>=width-1||y>=height-2)) if(qAlpha(pixels[(x + 1) + (y + 2)*width])==0) dBlobVal += 6.775373563;

                //(-2,-2)
                if(!(x<2       ||        y<2)) if(qAlpha(pixels[(x - 2) + (y - 2)*width])==0) dBlobVal += 8.570244983;
                //( 2,-2)
                if(!(x>=width-2||        y<2)) if(qAlpha(pixels[(x + 2) + (y - 2)*width])==0) dBlobVal += 8.570244983;
                //(-2, 2)
                if(!(x<2       ||y>=height-2)) if(qAlpha(pixels[(x - 2) + (y + 2)*width])==0) dBlobVal += 8.570244983;
                //( 2, 2)
                if(!(x>=width-2||y>=height-2)) if(qAlpha(pixels[(x + 2) + (y + 2)*width])==0) dBlobVal += 8.570244983;

                ////////////////////////////////
                // R3
                //(-1,-3)
                if(!(x<1       ||        y<3)) if(qAlpha(pixels[(x - 1) + (y - 3)*width])==0) dBlobVal += 9.581825183;
                //( 0,-3)
                if(!(                    y<3)) if(qAlpha(pixels[(x    ) + (y - 3)*width])==0) dBlobVal += 9.090117516;
                //( 1,-3)
                if(!(x>=width-1||        y<3)) if(qAlpha(pixels[(x + 1) + (y - 3)*width])==0) dBlobVal += 9.581825183;


                //(-3,-1)
                if(!(x<3       ||       y <1)) if(qAlpha(pixels[(x - 3) + (y - 1)*width])==0) dBlobVal += 9.581825183;
                //(-3,0)
                if(!(x<3                    )) if(qAlpha(pixels[(x - 3) + (y    )*width])==0) dBlobVal += 9.090117516;
                //(-3,1)
                if(!(x<3       ||y>=height-1)) if(qAlpha(pixels[(x - 3) + (y + 1)*width])==0) dBlobVal += 9.581825183;

                //(3,-1)
                if(!(x>=width-3||       y <1)) if(qAlpha(pixels[(x + 3) + (y - 1)*width])==0) dBlobVal += 9.581825183;
                //(3,0)
                if(!(x>=width-3             )) if(qAlpha(pixels[(x + 3) + (y    )*width])==0) dBlobVal += 9.090117516;
                //(3,1)
                if(!(x>=width-3||y>=height-1)) if(qAlpha(pixels[(x + 3) + (y + 1)*width])==0) dBlobVal += 9.581825183;


                //(-1, 3)
                if(!(x<1       ||y>=height-3)) if(qAlpha(pixels[(x - 1) + (y + 3)*width])==0) dBlobVal += 9.581825183;
                //( 0, 2)
                if(!(            y>=height-3)) if(qAlpha(pixels[(x    ) + (y + 3)*width])==0) dBlobVal += 9.090117516;
                //( 1, 2)
                if(!(x>=width-1||y>=height-3)) if(qAlpha(pixels[(x + 1) + (y + 3)*width])==0) dBlobVal += 9.581825183;

                m_vToverMap[x + y*width] = (uchar)dBlobVal;
            }
            else
            {
                m_vToverMap[x + y*width] = (uchar)0;
            }
        }
    }
}

void B9Projector::createToverMap2()
{
    int width = mCurSliceImage.width();
    int height = mCurSliceImage.height();
    QRgb *pixels = (QRgb *)mCurSliceImage.scanLine(0);
    double dBlobVal;
    m_vToverMap.resize(width*height);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            dBlobVal = 0;
            if(qAlpha(pixels[x + y*width])==255)
            { //we only care about set pixels...
                //(-1,-1)
                if(!(x<1       ||        y<1)) if(qAlpha(pixels[(x - 1) + (y - 1)*width])==0) dBlobVal += 10.14546124;
                //( 0,-1)
                if(!(                    y<1)) if(qAlpha(pixels[(x    ) + (y - 1)*width])==0) dBlobVal += 7.173924442;
                //( 1,-1)
                if(!(x>=width-1||        y<1)) if(qAlpha(pixels[(x + 1) + (y - 1)*width])==0) dBlobVal += 10.14546124;

                //(-1,0)
                if(!(x<1                    )) if(qAlpha(pixels[(x - 1) + (y    )*width])==0) dBlobVal += 7.173924442;
                //( 1,0)
                if(!(x>=width-1             )) if(qAlpha(pixels[(x + 1) + (y    )*width])==0) dBlobVal += 7.173924442;

                //(-1, 1)
                if(!(x<1       ||y>=height-1)) if(qAlpha(pixels[(x - 1) + (y + 1)*width])==0) dBlobVal += 10.14546124;
                //( 0, 1)
                if(!(            y>=height-1)) if(qAlpha(pixels[(x    ) + (y + 1)*width])==0) dBlobVal += 7.173924442;
                //( 1, 1)
                if(!(x>=width-1||y>=height-1)) if(qAlpha(pixels[(x + 1) + (y + 1)*width])==0) dBlobVal += 10.14546124;

                ///////////////////////

                //(-1,-2)
                if(!(x<1       ||        y<2)) if(qAlpha(pixels[(x - 1) + (y - 2)*width])==0) dBlobVal += 16.04138272;
                //( 0,-2)
                if(!(                    y<2)) if(qAlpha(pixels[(x    ) + (y - 2)*width])==0) dBlobVal += 14.34784888;
                //( 1,-2)
                if(!(x>=width-1||        y<2)) if(qAlpha(pixels[(x + 1) + (y - 2)*width])==0) dBlobVal += 16.04138272;


                //(-2,-1)
                if(!(x<2       ||       y <1)) if(qAlpha(pixels[(x - 2) + (y - 1)*width])==0) dBlobVal += 16.04138272;
                //(-2,0)
                if(!(x<2                    )) if(qAlpha(pixels[(x - 2) + (y    )*width])==0) dBlobVal += 14.34784888;
                //(-2,1)
                if(!(x<2       ||y>=height-1)) if(qAlpha(pixels[(x - 2) + (y + 1)*width])==0) dBlobVal += 16.04138272;

                //(2,-1)
                if(!(x>=width-2||       y <1)) if(qAlpha(pixels[(x + 2) + (y - 1)*width])==0) dBlobVal += 16.04138272;
                //(2,0)
                if(!(x>=width-2             )) if(qAlpha(pixels[(x + 2) + (y    )*width])==0) dBlobVal += 14.34784888;
                //(2,1)
                if(!(x>=width-2||y>=height-1)) if(qAlpha(pixels[(x + 2) + (y + 1)*width])==0) dBlobVal += 16.04138272;


                //(-1, 2)
                if(!(x<1       ||y>=height-2)) if(qAlpha(pixels[(x - 1) + (y + 2)*width])==0) dBlobVal += 16.04138272;
                //( 0, 2)
                if(!(            y>=height-2)) if(qAlpha(pixels[(x    ) + (y + 2)*width])==0) dBlobVal += 14.34784888;
                //( 1, 2)
                if(!(x>=width-1||y>=height-2)) if(qAlpha(pixels[(x + 1) + (y + 2)*width])==0) dBlobVal += 16.04138272;

                m_vToverMap[x + y*width] = (uchar)dBlobVal;
            }
            else
            {
                m_vToverMap[x + y*width] = (uchar)0;
            }
        }
    }
}

void B9Projector::createToverMap1()
{
    int width = mCurSliceImage.width();
    int height = mCurSliceImage.height();
    QRgb *pixels = (QRgb *)mCurSliceImage.scanLine(0);

    double dBlobVal;

    m_vToverMap.resize(width*height);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            dBlobVal = 0;
            if(qAlpha(pixels[x + y*width])>0)
            { //we only care about set pixels...
                //(-1,-1)
                if(!(x<1       ||        y<1)) if(qAlpha(pixels[(x - 1) + (y - 1)*width])==0) dBlobVal += 37.3138854;
                //( 0,-1)
                if(!(                    y<1)) if(qAlpha(pixels[(x    ) + (y - 1)*width])==0) dBlobVal += 26.4061146;
                //( 1,-1)
                if(!(x>=width-1||        y<1)) if(qAlpha(pixels[(x + 1) + (y - 1)*width])==0) dBlobVal += 37.3138854;

                //(-1,0)
                if(!(x<1                    )) if(qAlpha(pixels[(x - 1) + (y    )*width])==0) dBlobVal += 26.4061146;
                //( 1,0)
                if(!(x>=width-1             )) if(qAlpha(pixels[(x + 1) + (y    )*width])==0) dBlobVal += 26.4061146;

                //(-1, 1)
                if(!(x<1       ||y>=height-1)) if(qAlpha(pixels[(x - 1) + (y + 1)*width])==0) dBlobVal += 37.3138854;
                //( 0, 1)
                if(!(            y>=height-1)) if(qAlpha(pixels[(x    ) + (y + 1)*width])==0) dBlobVal += 26.4061146;
                //( 1, 1)
                if(!(x>=width-1||y>=height-1)) if(qAlpha(pixels[(x + 1) + (y + 1)*width])==0) dBlobVal += 37.3138854;

                m_vToverMap[x + y*width] = (uchar)dBlobVal;

            }
            else
            {
                m_vToverMap[x + y*width] = (uchar)0;
            }
        }
    }
}

void B9Projector::createToverMap0()
{
    //Simple perimeter/empty center (255/0)
    int width = mCurSliceImage.width();
    int height = mCurSliceImage.height();
    QRgb *pixels = (QRgb *)mCurSliceImage.scanLine(0);


    //Simple edge detection, set pixel to 255 if "next to"(above, below, left, right) a black pixel
    m_vToverMap.resize(width*height);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if(qBlue(pixels[x + y*width])>0)
            { //we only care about set pixels...

                if(y==0 || y==height-1 || x==0 || x==width-1) // first check if we're a border pixel, always set these
                {
                    m_vToverMap[x + y*width] = (uchar)255;
                }
                else if(    qBlue(pixels[(x + 1) + (y    )*width])==0 ||
                            qBlue(pixels[(x - 1) + (y    )*width])==0 ||
                            qBlue(pixels[(x    ) + (y + 1)*width])==0 ||
                            qBlue(pixels[(x    ) + (y - 1)*width])==0   )
                {
                    m_vToverMap[x + y*width] = (uchar)255;
                    //qDebug() << x << ", " << y << " Value" << (uchar)m_vToverMap[x + y*width];
                }
                else
                {
                    m_vToverMap[x + y*width] = (uchar)0;
                    //qDebug() << x << ", " << y << " Value" << (uchar)m_vToverMap[x + y*width];
                }
            }
            else
            {
                m_vToverMap[x + y*width] = (uchar)0;
            }
        }
    }
}
