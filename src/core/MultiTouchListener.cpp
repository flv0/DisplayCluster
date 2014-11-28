/*********************************************************************/
/* Copyright (c) 2013, EPFL/Blue Brain Project                       */
/*                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>     */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "MultiTouchListener.h"

#include "log.h"

#include "gestures/DoubleTapGestureRecognizer.h"
#include "gestures/PanGestureRecognizer.h"
#include "gestures/PinchGestureRecognizer.h"

#include <QApplication>

MultiTouchListener::MultiTouchListener( QGraphicsView* graphicsView )
    : TUIO::TuioListener()
    , graphicsView_( graphicsView )
{
    DoubleTapGestureRecognizer::install();
    PanGestureRecognizer::install();
    PinchGestureRecognizer::install();

    client_.addTuioListener( this );
    client_.connect();
}

MultiTouchListener::~MultiTouchListener()
{
    client_.removeTuioListener( this );
    client_.disconnect();

    DoubleTapGestureRecognizer::uninstall();
    PanGestureRecognizer::uninstall();
    PinchGestureRecognizer::uninstall();
}

void MultiTouchListener::addTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::updateTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::removeTuioObject( TUIO::TuioObject* )
{
}

void MultiTouchListener::handleEvent( TUIO::TuioCursor* tcur,
                                      const QEvent::Type eventType )
{
    if( !graphicsView_ )
        return;

    const QPoint& viewPos = graphicsView_->mapToGlobal( graphicsView_->pos( ));
    const QRectF& sceneRect = graphicsView_->sceneRect();
    const QRectF& wallRect = graphicsView_->scene()->sceneRect();
    const int viewWidth = graphicsView_->geometry().width();
    const int viewHeight = graphicsView_->geometry().height();

    const qreal w = viewWidth / sceneRect.width();
    const qreal h = viewHeight / sceneRect.height();
    const int x = qreal( viewWidth - w ) * .5;
    const int y = qreal( viewHeight - h ) * .5;
    const QPoint sceneOffset( x, y );

    const QPointF normPos( tcur->getX(), tcur->getY( ));
    const QPoint pos( w * normPos.x(), h * normPos.y( ));
    const QPoint screenPos( viewPos + sceneOffset + pos );

    const QPoint wallPos( wallRect.width() * tcur->getX(),
                          wallRect.height() * tcur->getY( ));

    QTouchEvent::TouchPoint touchPoint( tcur->getCursorID( ));
    touchPoint.setPressure( 1.0 );
    touchPoint.setNormalizedPos( normPos );
    touchPoint.setPos( wallPos ); // need for pan gesture recognition
    touchPoint.setScenePos( normPos );
    touchPoint.setScreenPos( screenPos ); // need for hotspot & itemAt

    Qt::TouchPointStates touchPointStates = 0;
    if( tcur->getCursorID() == 0 )
        touchPointStates |= Qt::TouchPointPrimary;

    switch( eventType )
    {
    case QEvent::TouchBegin:
        touchPointStates = Qt::TouchPointPressed;

        touchPoint.setStartNormalizedPos( normPos );
        touchPoint.setStartPos( touchPoint.pos( ));
        touchPoint.setStartScreenPos( screenPos );
        touchPoint.setStartScenePos( touchPoint.scenePos( ));

        touchPoint.setLastNormalizedPos( normPos );
        touchPoint.setLastPos( touchPoint.pos( ));
        touchPoint.setLastScreenPos( screenPos );
        touchPoint.setLastScenePos( touchPoint.scenePos( ));
        break;

    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        if( eventType == QEvent::TouchUpdate )
            touchPointStates = tcur->isMoving() ? Qt::TouchPointMoved
                                                : Qt::TouchPointStationary;
        else
            touchPointStates = Qt::TouchPointReleased;

        const QTouchEvent::TouchPoint& prevPoint = touchPointMap_.value( tcur->getCursorID( ));
        touchPoint.setStartNormalizedPos( prevPoint.startNormalizedPos( ));
        touchPoint.setStartPos( prevPoint.startPos( ));
        touchPoint.setStartScreenPos( prevPoint.startScreenPos( ));
        touchPoint.setStartScenePos( prevPoint.startScenePos( ));

        touchPoint.setLastNormalizedPos( prevPoint.normalizedPos( ));
        touchPoint.setLastPos( prevPoint.pos( ));
        touchPoint.setLastScreenPos( prevPoint.screenPos( ));
        touchPoint.setLastScenePos( prevPoint.scenePos( ));
        break;
    }

    default:
        put_flog( LOG_ERROR, "Got wrong touch event type %i", eventType );
        return;
    }

    touchPoint.setState( touchPointStates );
    touchPointMap_.insert( tcur->getCursorID(), touchPoint );

    QEvent* touchEvent = new QTouchEvent( eventType, QTouchEvent::TouchScreen,
                                          Qt::NoModifier, touchPointStates,
                                          touchPointMap_.values( ));
    QApplication::postEvent( graphicsView_->viewport(), touchEvent );

    if( eventType == QEvent::TouchEnd )
        touchPointMap_.remove( tcur->getCursorID( ));
}

void MultiTouchListener::addTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchBegin );
    const QPointF position( tcur->getX(), tcur->getY( ));
    emit touchPointAdded( tcur->getCursorID(), position );
}

void MultiTouchListener::updateTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchUpdate );
    const QPointF position( tcur->getX(), tcur->getY( ));
    emit touchPointUpdated( tcur->getCursorID(), position );
}

void MultiTouchListener::removeTuioCursor( TUIO::TuioCursor* tcur )
{
    handleEvent( tcur, QEvent::TouchEnd );
    emit touchPointRemoved( tcur->getCursorID( ));
}

void MultiTouchListener::refresh( TUIO::TuioTime )
{
}
