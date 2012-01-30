/*********************************************************************/
/* Copyright 2011 - 2012  The University of Texas at Austin.         */
/* All rights reserved.                                              */
/*                                                                   */
/* This is a pre-release version of DisplayCluster. All rights are   */
/* reserved by the University of Texas at Austin. You may not modify */
/* or distribute this software without permission from the authors.  */
/* Refer to the LICENSE file distributed with the software for       */
/* details.                                                          */
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

#include "Content.h"
#include "ContentWindowManager.h"
#include "TextureContent.h"
#include "DynamicTextureContent.h"
#include "MovieContent.h"
#include <QGLWidget>

Content::Content(std::string uri)
{
    uri_ = uri;
    width_ = 0;
    height_ = 0;
}

std::string Content::getURI()
{
    return uri_;
}

void Content::getDimensions(int &width, int &height)
{
    width = width_;
    height = height_;
}

void Content::setDimensions(int width, int height)
{
    width_ = width;
    height_ = height;

    emit(dimensionsChanged(width_, height_));
}

void Content::render(boost::shared_ptr<ContentWindowManager> window)
{
    // get parameters from window
    double x, y, w, h;
    window->getCoordinates(x, y, w, h);

    double centerX, centerY;
    window->getCenter(centerX, centerY);

    double zoom = window->getZoom();

    // calculate texture coordinates
    float tX = centerX - 0.5 / zoom;
    float tY = centerY - 0.5 / zoom;
    float tW = 1./zoom;
    float tH = 1./zoom;

    // transform to a normalize coordinate system so the content can be rendered at (x,y,w,h) = (0,0,1,1)
    glPushMatrix();

    glTranslatef(x, y, 0.);
    glScalef(w, h, 1.);

    // render the factory object
    renderFactoryObject(tX, tY, tW, tH);

    glPopMatrix();
}

boost::shared_ptr<Content> Content::getContent(std::string uri)
{
    // convert to lower case for case-insensitivity in determining file type
    QString fileTypeString = QString::fromStdString(uri).toLower();

    // see if this is an image
    QImageReader imageReader(uri.c_str());

    if(imageReader.canRead() == true)
    {
        // get its size
        QSize size = imageReader.size();

        // small images will use Texture; larger images will use DynamicTexture
        boost::shared_ptr<Content> c;

        if(size.width() <= 4096 && size.height() <= 4096)
        {
            boost::shared_ptr<Content> temp(new TextureContent(uri));
            c = temp;
        }
        else
        {
            boost::shared_ptr<Content> temp(new DynamicTextureContent(uri));
            c = temp;
        }

        return c;
    }
    // see if this is a movie
    // todo: need a better way to determine file type
    else if(fileTypeString.endsWith(".mov") || fileTypeString.endsWith(".avi") || fileTypeString.endsWith(".mp4") || fileTypeString.endsWith(".mkv") || fileTypeString.endsWith(".mpg") || fileTypeString.endsWith(".flv"))
    {
        boost::shared_ptr<Content> c(new MovieContent(uri));

        return c;
    }
    // see if this is an image pyramid
    else if(fileTypeString.endsWith(".pyr"))
    {
        boost::shared_ptr<Content> c(new DynamicTextureContent(uri));

        return c;
    }

    // otherwise, return NULL
    return boost::shared_ptr<Content>();
}
