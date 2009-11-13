/*
 * imgLabel.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: carlos
 */

#include <QtGui>

#include "imgLabel.h"
#include "qcorr.h"

ImgLabel::ImgLabel(Qcorr *parentWindow, QWidget *parent) :
   QLabel(parent)
{
   m_imgLabel = new QLabel;
   m_parentWindow = parentWindow; // Point to the Qcorr MainWindow

   m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
   m_rubberBand->setCursor(Qt::SizeAllCursor);
   m_rubberBand->setMouseTracking(true);

   m_bStartedTemplateSelection = false;
   m_bMouseIsPressed = false;

   setTemplateFlags(false);

//   this->stackUnder(m_rubberBand);  //Places the widget under the rubberBand in the parent widget's stack.
}

ImgLabel::~ImgLabel()
{
   // TODO Auto-generated destructor stub
}

void
ImgLabel::mousePressEvent(QMouseEvent *event)
{
   m_bMouseIsPressed = true;

   m_currentPressedPoint = event->pos();

   if (m_rubberBand->isHidden() && (m_bStartedTemplateSelection == false))
      {
         m_originPoint = event->pos();
         m_rubberBand->setGeometry(QRect(m_originPoint, QSize()));
      }
   else
      {
         m_rubberBand->hide();
      }

   update();
}

void
ImgLabel::mouseMoveEvent(QMouseEvent *event)
{
   if (m_bMouseIsPressed)
      {
         m_rubberBand->show();

         m_bStartedTemplateSelection = true;

         QPoint newOriginPoint = QPoint(0,0);

         if(m_bMouseInTemplateRegion)
            {
            newOriginPoint = m_currentPressedPoint - (m_finalPoint - event->pos());
            m_rubberBand->move(newOriginPoint);

            QPoint movedFinalPoint;
            movedFinalPoint.setX(newOriginPoint.x() + m_rubberBand->width());
            movedFinalPoint.setY(newOriginPoint.y() + m_rubberBand->height());

            displayCoordinatesOnStatusLabel(newOriginPoint, movedFinalPoint);
            }
         else if(m_bMouseAtTemplateTopEdge)
            {
            newOriginPoint = QPoint(m_originPoint.x(), event->pos().y());
            m_rubberBand->setGeometry(QRect(newOriginPoint, m_finalPoint).normalized());
            displayCoordinatesOnStatusLabel(newOriginPoint, m_finalPoint);
            }
         else if(m_bMouseAtTemplateBottomEdge)
            {
            newOriginPoint = QPoint(m_finalPoint.x(), event->pos().y());
            m_rubberBand->setGeometry(QRect(m_originPoint, newOriginPoint).normalized());
            displayCoordinatesOnStatusLabel(m_originPoint, newOriginPoint);
            }
         else if(m_bMouseAtTemplateLeftEdge)
            {
            newOriginPoint = QPoint(event->pos().x(), m_originPoint.y());
            m_rubberBand->setGeometry(QRect(newOriginPoint, m_finalPoint).normalized());
            displayCoordinatesOnStatusLabel(newOriginPoint, m_finalPoint);
            }
         else if(m_bMouseAtTemplateRightEdge)
            {
            newOriginPoint = QPoint(event->pos().x(), m_finalPoint.y());
            m_rubberBand->setGeometry(QRect(m_originPoint, newOriginPoint).normalized());
            displayCoordinatesOnStatusLabel(m_originPoint, newOriginPoint);
            }
         else
            {
            m_finalPoint = event->pos();
            m_rubberBand->setGeometry(
                           QRect(m_currentPressedPoint, m_finalPoint).normalized());
            setTemplateFlags(false);
            displayCoordinatesOnStatusLabel(m_currentPressedPoint, m_finalPoint);
            }

      }
   else
      {
         if(m_rubberBand->isVisible())
         {
         checkTemplateRegions(event->pos().x(), event->pos().y());
         }
         else
         {
            m_bStartedTemplateSelection = false;
            m_currentPressedPoint = event->pos();
            displayCoordinatesOnStatusLabel(m_currentPressedPoint, m_currentPressedPoint); // Show mouse moving position only
            setTemplateFlags(false);
         }
      }
}

void
ImgLabel::mouseReleaseEvent(QMouseEvent *event)
{
   m_bMouseIsPressed = false;
   m_bStartedTemplateSelection = false;

   if (m_rubberBand->isVisible())
      {
      // Obtain actual origin and final Points
      m_originPoint = m_rubberBand->pos(); // Set new position of the Points
      m_finalPoint.setX(m_originPoint.x() + m_rubberBand->width());
      m_finalPoint.setY(m_originPoint.y() + m_rubberBand->height());

      checkTemplateRegions(event->pos().x(), event->pos().y());
      // ------------------------------------------------------------
      //To process the rectangular regions, use something like:
//      QImage::copy ( const QRect & rectangle = QRect() ) const
      // for further processing
      // It's missing QImage member function that was set as QPixmap on the MainWindow app (qcorr.cpp)
      //----------------------------------------------------------
         // determine selection, for example using QRect::intersects()
         // and QRect::contains().

      }
}

void ImgLabel::setTemplateFlags(bool status)
{
   m_bMouseInTemplateRegion = status;
   m_bMouseAtTemplateTopEdge = status;
   m_bMouseAtTemplateBottomEdge = status;
   m_bMouseAtTemplateLeftEdge = status;
   m_bMouseAtTemplateRightEdge = status;
}

void ImgLabel::checkTemplateRegions(int mouseX, int mouseY)
{
   m_bMouseAtTemplateTopEdge = (mouseY == m_originPoint.y());
   m_bMouseAtTemplateBottomEdge = (mouseY == m_finalPoint.y());
   m_bMouseAtTemplateLeftEdge = (mouseX == m_originPoint.x());
   m_bMouseAtTemplateRightEdge = (mouseX == m_finalPoint.x());

   // Hackish way of changing the mouse cursor when inside the QRubberBand region:
   if(((mouseX > m_originPoint.x()) && (mouseY > m_originPoint.y())
         && (mouseX < m_finalPoint.x()) && (mouseY < m_finalPoint.y()) )
         || // When the region was selected from bottom to top
         ((mouseX < m_originPoint.x()) && (mouseY < m_originPoint.y())
         && (mouseX > m_finalPoint.x()) && (mouseY > m_finalPoint.y()) )
        )
      {
      this->setCursor(Qt::SizeAllCursor);
      m_bMouseInTemplateRegion = true;
      }
   // Change the cursor at the edges:
   else if(m_bMouseAtTemplateLeftEdge || m_bMouseAtTemplateRightEdge )
      {
      this->setCursor(Qt::SizeHorCursor);
      m_bMouseInTemplateRegion = false;
      // TODO: Implement resize on the horizontal sides
      }
//      else if((m_bMouseAtTemplateTopEdge = mouseY == m_originPoint.y()) || (mouseY == m_finalPoint.y()) )
   else if(m_bMouseAtTemplateTopEdge || m_bMouseAtTemplateBottomEdge )
      {
      this->setCursor(Qt::SizeVerCursor);
      m_bMouseInTemplateRegion = false;
      // TODO: Implement resize on the horizontal sides
      }
   else
      {
      this->setCursor(Qt::CrossCursor);
      setTemplateFlags(false);
      }
}

void ImgLabel::displayCoordinatesOnStatusLabel(QPoint &point0, QPoint &point1)
{
   m_parentWindow->m_status_label->setText(
                  "[X0:" + QString::number(point0.x()) + "px, " + "Y0:" +
                  QString::number(point0.y()) + "px]" + "<b> to </b>" +
                  "[X1:" + QString::number(point1.x()) + "px, " + "Y1:"
                  + QString::number(point1.y()) + "px]");
}
