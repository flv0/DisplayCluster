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

#include "MovieContent.h"
#include "main.h"
#include "Movie.h"
#include "ContentWindowManager.h"

BOOST_CLASS_EXPORT_GUID(MovieContent, "MovieContent")

void MovieContent::getFactoryObjectDimensions(int &width, int &height)
{
    g_mainWindow->getGLWindow()->getMovieFactory().getObject(getURI())->getDimensions(width, height);
}

void MovieContent::advance(boost::shared_ptr<ContentWindowManager> window)
{
    // skip a frame if the Content rectangle is not visible in ANY windows; otherwise decode normally
    bool skip = true;

    // window parameters
    double x, y, w, h;
    window->getCoordinates(x, y, w, h);

    std::vector<boost::shared_ptr<GLWindow> > glWindows = g_mainWindow->getGLWindows();

    for(unsigned int i=0; i<glWindows.size(); i++)
    {
        if(glWindows[i]->isScreenRectangleVisible(x, y, w, h) == true)
        {
            skip = false;
            break;
        }
    }

    g_mainWindow->getGLWindow()->getMovieFactory().getObject(getURI())->nextFrame(skip);
}

void MovieContent::renderFactoryObject(float tX, float tY, float tW, float tH)
{
    g_mainWindow->getGLWindow()->getMovieFactory().getObject(getURI())->render(tX, tY, tW, tH);
}
