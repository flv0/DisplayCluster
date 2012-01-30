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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// increment this whenever when serialized state information changes
#define CONTENTS_FILE_VERSION_NUMBER 1

#include "GLWindow.h"
#include <QtGui>
#include <QGLWidget>
#include <boost/shared_ptr.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:

        MainWindow();

        bool getConstrainAspectRatio();

        boost::shared_ptr<GLWindow> getGLWindow(int index=0);
        std::vector<boost::shared_ptr<GLWindow> > getGLWindows();

    public slots:

        void openContent();
        void openContentsDirectory();
        void clearContents();
        void saveState();
        void loadState();
        void computeImagePyramid();
        void constrainAspectRatio(bool set);

        void updateGLWindows();

    signals:

        void updateGLWindowsFinished();

    private:

        std::vector<boost::shared_ptr<GLWindow> > glWindows_;

        bool constrainAspectRatio_;
};

#endif
