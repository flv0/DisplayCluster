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

#include "PixelStreamWindowManager.h"

#include "configuration/Configuration.h"
#include "ContentWindow.h"
#include "ContentWindowController.h"
#include "ContentFactory.h"
#include "DisplayGroup.h"
#include "log.h"

#include <deflect/PixelStreamBuffer.h>
#include <deflect/PixelStreamFrame.h>

PixelStreamWindowManager::PixelStreamWindowManager( DisplayGroup& displayGroup )
    : QObject()
    , displayGroup_( displayGroup )
{
    connect( &displayGroup, SIGNAL( contentWindowRemoved( ContentWindowPtr )),
             this, SLOT( onContentWindowRemoved( ContentWindowPtr )));
}

ContentWindowPtr
PixelStreamWindowManager::createContentWindow( const QString& uri,
                                               const QPointF& pos,
                                               const QSizeF& size )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        put_flog( LOG_WARN, "Already have a window for stream: '%s'",
                  uri.toStdString().c_str( ));
    else
    {
        ContentPtr content = ContentFactory::getPixelStreamContent( uri );
        contentWindow.reset( new ContentWindow( content ));
        streamerWindows_[uri] = contentWindow;
    }

    ContentWindowController controller( *contentWindow, displayGroup_ );
    controller.resize( size );
    controller.moveCenterTo( pos.isNull() ?
                                displayGroup_.getCoordinates().center() : pos );

    return contentWindow;
}

void PixelStreamWindowManager::removeContentWindow( const QString& uri )
{
    streamerWindows_.erase( uri );
}

ContentWindowPtr
PixelStreamWindowManager::getContentWindow( const QString& uri ) const
{
    ContentWindowMap::const_iterator it = streamerWindows_.find( uri );
    return it != streamerWindows_.end() ? it->second : ContentWindowPtr();
}

void PixelStreamWindowManager::updateDimension( const QString& uri,
                                                const QSize& size )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( !contentWindow )
        return;

    contentWindow->getContent()->setDimensions( size );
}

void PixelStreamWindowManager::hideWindow( const QString& uri )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setState( ContentWindow::HIDDEN );
}

void PixelStreamWindowManager::showWindow( const QString& uri )
{
    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        contentWindow->setState( ContentWindow::SELECTED );
}

void PixelStreamWindowManager::openPixelStreamWindow( QString uri, QSize size )
{
    put_flog( LOG_DEBUG, "adding pixel stream: %s",
              uri.toLocal8Bit().constData( ));

    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( !contentWindow )
    {
        // external streamers have no window yet
        const QPointF center = displayGroup_.getCoordinates().center();
        contentWindow = createContentWindow( uri, center, size );
    }
    contentWindow->getContent()->setDimensions( size );
    contentWindow->setState( ContentWindow::SELECTED );
    displayGroup_.addContentWindow( contentWindow );
}

void PixelStreamWindowManager::closePixelStreamWindow( const QString uri )
{
    put_flog( LOG_DEBUG, "deleting pixel stream: %s",
              uri.toLocal8Bit().constData( ));

    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
        displayGroup_.removeContentWindow( contentWindow );
}

void PixelStreamWindowManager::registerEventReceiver( const QString uri,
                                                      const bool exclusive,
                                                      deflect::EventReceiver* receiver )
{
    bool success = false;

    ContentWindowPtr contentWindow = getContentWindow( uri );
    if( contentWindow )
    {
        put_flog( LOG_DEBUG, "found window: '%s'", uri.toStdString().c_str( ));

        // If a receiver is already registered, don't register this one if
        // "exclusive" was requested
        if( !exclusive || !contentWindow->hasEventReceivers( ))
        {
            success = contentWindow->registerEventReceiver( receiver );

            if( success )
                contentWindow->setState( ContentWindow::SELECTED );
        }
    }
    else
        put_flog( LOG_ERROR, "could not find window: '%s'",
                  uri.toStdString().c_str( ));

    emit eventRegistrationReply( uri, success );
}

void PixelStreamWindowManager::onContentWindowRemoved( ContentWindowPtr window )
{
    const QString& uri = window->getContent()->getURI();
    if( !getContentWindow( uri ))
        return;

    removeContentWindow( uri );
    emit pixelStreamWindowClosed( uri );
}

void PixelStreamWindowManager::onSendFrame( deflect::PixelStreamFramePtr frame )
{
    const QSize& size =
            deflect::PixelStreamBuffer::computeFrameDimensions( frame->segments );
    updateDimension( frame->uri, size );
}
