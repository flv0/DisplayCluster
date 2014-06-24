/*********************************************************************/
/* Copyright (c) 2011 - 2012, The University of Texas at Austin.     */
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

#include "GLWindow.h"
#include "globals.h"
#include "Options.h"
#include "MPIChannel.h"
#include "Marker.h"
#include "configuration/WallConfiguration.h"
#include "ContentWindowManager.h"
#include "DisplayGroupManager.h"
#include "Renderable.h"
#include "RenderContext.h"
#include "log.h"
#include <QtOpenGL>
#include <boost/shared_ptr.hpp>

#ifdef __APPLE__
    #include <OpenGL/glu.h>
#else
    #include <GL/glu.h>
#endif

GLWindow::GLWindow(const int tileIndex, QRect windowRect, QGLWidget* shareWidget)
  : QGLWidget(0, shareWidget)
  , configuration_(static_cast<WallConfiguration*>(g_configuration))
  , tileIndex_(tileIndex)
  , left_(0)
  , right_(0)
  , bottom_(0)
  , top_(0)
{
    setGeometry(windowRect);

    if(shareWidget && !isSharing())
    {
        put_flog(LOG_FATAL, "failed to share OpenGL context");
        exit(-1);
    }

    setAutoBufferSwap(false);
}

GLWindow::~GLWindow()
{
}

int GLWindow::getTileIndex() const
{
    return tileIndex_;
}

void GLWindow::insertPurgeTextureId(GLuint textureId)
{
    QMutexLocker locker(&purgeTexturesMutex_);

    purgeTextureIds_.push_back(textureId);
}

void GLWindow::purgeTextures()
{
    QMutexLocker locker(&purgeTexturesMutex_);

    for(size_t i=0; i<purgeTextureIds_.size(); ++i)
        deleteTexture(purgeTextureIds_[i]);

    purgeTextureIds_.clear();
}

void GLWindow::addRenderable(RenderablePtr renderable)
{
    renderables_.append(renderable);
}

void GLWindow::setTestPattern(RenderablePtr testPattern)
{
    testPattern_ = testPattern;
}

void GLWindow::initializeGL()
{
    // enable depth testing; disable lighting
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void GLWindow::paintGL()
{
    OptionsPtr options = configuration_->getOptions();

    clear(options->getBackgroundColor());
    setOrthographicView();

    if(options->getShowTestPattern())
    {
        testPattern_->render();
        return;
    }

    foreach (RenderablePtr renderable, renderables_) {
        renderable->render();
    }

    if (options->getShowStreamingStatistics())
        drawFps();

#if ENABLE_SKELETON_SUPPORT
    if(options->getShowSkeletons())
        renderSkeletons(displayGroup_->getSkeletons());
#endif
}

void GLWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    update();
}

void GLWindow::clear(const QColor& clearColor)
{
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alpha());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLWindow::setOrthographicView()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // invert y-axis to put origin at lower-left corner
    glScalef(1.,-1.,1.);

    // tiled display parameters
    double tileI = (double)configuration_->getGlobalScreenIndex(tileIndex_).x();
    double numTilesX = (double)configuration_->getTotalScreenCountX();
    double screenWidth = (double)configuration_->getScreenWidth();
    double mullionWidth = (double)configuration_->getMullionWidth();

    double tileJ = (double)configuration_->getGlobalScreenIndex(tileIndex_).y();
    double numTilesY = (double)configuration_->getTotalScreenCountY();
    double screenHeight = (double)configuration_->getScreenHeight();
    double mullionHeight = (double)configuration_->getMullionHeight();

    // border calculations
    left_ = tileI / numTilesX * ( numTilesX * screenWidth ) + tileI * mullionWidth;
    right_ = left_ + screenWidth;
    bottom_ = tileJ / numTilesY * ( numTilesY * screenHeight ) + tileJ * mullionHeight;
    top_ = bottom_ + screenHeight;

    // normalize to 0->1
    double totalWidth = (double)configuration_->getTotalWidth();
    double totalHeight = (double)configuration_->getTotalHeight();

    left_ /= totalWidth;
    right_ /= totalWidth;
    bottom_ /= totalHeight;
    top_ /= totalHeight;

    gluOrtho2D(left_, right_, bottom_, top_);
    glPushMatrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

#if ENABLE_SKELETON_SUPPORT
bool GLWindow::setPerspectiveView(double x, double y, double w, double h)
{
    // we want a perspective view for an area over the entire display bounded by (x,y,w,h)
    // this windows area is produced by intersection((left_,right_,bottom_,top_), (x,y,w,h))
    // in the current coordinate system, bottom is at the top of the screen, top at the bottom...
    QRectF screenRect = QRectF(left_, bottom_, right_-left_, top_-bottom_);
    QRectF windowRect = QRectF(x, y, w, h);
    QRectF boundRect = screenRect.intersected(windowRect);

    // if bounding rectangle is empty, return false to indicate no rendering should be done
    if(boundRect.isEmpty() == true)
    {
        return false;
    }

    if(boundRect != screenRect)
    {
        // x,y for viewport is lower-left corner
        // the y coordinate needs to be shifted from the top of the screen to the bottom, and y-direction inverted
        int viewPortX = (int)((boundRect.x() - screenRect.x()) / screenRect.width() * width());
        int viewPortY = (int)((screenRect.height() - (boundRect.y() + boundRect.height() - screenRect.y())) / screenRect.height() * height());
        int viewPortW = (int)(boundRect.width() / screenRect.width() * width());
        int viewPortH = (int)(boundRect.height() / screenRect.height() * height());

        glViewport(viewPortX, viewPortY, viewPortW, viewPortH);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double near = 0.001;
    double far = 100.;

    double aspect = (double)configuration_->getTotalHeight() / (double)configuration_->getTotalWidth() * windowRect.height() / windowRect.width();

    double winFovY = 45.0 * aspect;

    double top = tan(0.5 * winFovY * M_PI/180.) * near;
    double bottom = -top;
    double left = 1./aspect * bottom;
    double right = 1./aspect * top;

    // this window's portion of the entire view above is bounded by (left_, right_) and (bottom_, top_)
    // the full frustum would be for this screen:
    // glFrustum(left + left_ * (right-left), left + right_ * (right-left), top + top_ * (bottom-top), top + bottom_ * (bottom-top), near, far);
    double fLeft = left + (boundRect.x() - windowRect.x()) / windowRect.width() * (right-left);
    double fRight = fLeft + boundRect.width() / windowRect.width() * (right-left);
    double fBottom = top + (boundRect.y() - windowRect.y()) / windowRect.height() * (bottom-top);
    double fTop = fBottom + boundRect.height() / windowRect.height() * (bottom-top);

    glFrustum(fLeft, fRight, fTop, fBottom, near, far);

    glPushMatrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // don't clear the GL_COLOR_BUFFER_BIT since this may be an overlay. only clear depth
    glClear(GL_DEPTH_BUFFER_BIT);

    // new lookat matrix
    glLoadIdentity();

    gluLookAt(0,0,1, 0,0,0, 0,1,0);

    // setup lighting
    GLfloat LightAmbient[] = { 0.01, 0.01, 0.01, 1.0 };
    GLfloat LightDiffuse[] = { .5, .5, .5, 1.0 };
    GLfloat LightSpecular[] = { .9,.9,.9, 1.0 };

    GLfloat LightPosition[] = { 0,0,1000000, 1.0 };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);

    glEnable(GL_LIGHT1);

    // glEnable(GL_LIGHTING) needs to be called to actually use lighting. ditto for depth testing.
    // let other code enable / disable such settings so glPushAttrib() and glPopAttrib() can be used appropriately

    return true;
}

void GLWindow::renderSkeletons(const std::vector< boost::shared_ptr<SkeletonState> >& skeletons)
{
    // render perspective overlay for skeletons

    // setPersectiveView() may change the viewport!
    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

    // set the height of the skeleton view to a fraction of the total display height
    // set the width to maintain a 16/9 aspect ratio
    double skeletonViewHeight = 0.4;
    double skeletonViewWidth = 16./9. / configuration_->getAspectRatio() * skeletonViewHeight;

    // view at the center bottom
    if(setPerspectiveView(0.5 * (1. - skeletonViewWidth), 1. - skeletonViewHeight, skeletonViewWidth, skeletonViewHeight))
    {
        // enable depth testing, lighting, color tracking, and normal normalization (since we're scaling)
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glEnable(GL_NORMALIZE);

        for(unsigned int i = 0; i < skeletons.size(); i++)
            skeletons[i]->render();
    }

    glPopAttrib();
}

#endif

bool GLWindow::isRegionVisible(const QRectF& region) const
{
    // screen rectangle
    const QRectF screenRect(left_, bottom_, right_-left_, top_-bottom_);

    return screenRect.intersects(region);
}

void GLWindow::drawRectangle(double x, double y, double w, double h)
{
    glBegin(GL_QUADS);

    glVertex2d(x,y);
    glVertex2d(x+w,y);
    glVertex2d(x+w,y+h);
    glVertex2d(x,y+h);

    glEnd();
}

void GLWindow::drawFps()
{
    fpsCounter_.tick();

    const int fontSize = 32;
    QFont textFont;
    textFont.setPixelSize(fontSize);

    glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);

    glDisable(GL_DEPTH_TEST);
    glColor4f(0.,0.,1.,1.);

    renderText(10, fontSize, fpsCounter_.toString(), textFont);

    glPopAttrib();
}

QRectF GLWindow::getProjectedPixelRect(const bool clampToWindowArea) const
{
    // get four corners in object space (recall we're in normalized 0->1 dimensions)
    const double corners[4][3] =
    {
        {0.,0.,0.},
        {1.,0.,0.},
        {1.,1.,0.},
        {0.,1.,0.}
    };

    // get four corners in screen space
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    GLdouble projection[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble xWin[4][3];

    for(size_t i=0; i<4; i++)
    {
        gluProject(corners[i][0], corners[i][1], corners[i][2], modelview, projection, viewport, &xWin[i][0], &xWin[i][1], &xWin[i][2]);

        if( clampToWindowArea )
        {
            // clamp to on-screen portion
            if(xWin[i][0] < 0.)
                xWin[i][0] = 0.;

            if(xWin[i][0] > (double)width())
                xWin[i][0] = (double)width();

            if(xWin[i][1] < 0.)
                xWin[i][1] = 0.;

            if(xWin[i][1] > (double)height())
                xWin[i][1] = (double)height();
        }
    }
    const QPointF topleft( xWin[0][0], (double)height() - xWin[0][1] );
    const QPointF bottomright( xWin[2][0], (double)height() - xWin[2][1] );

    return QRectF( topleft, bottomright );
}
