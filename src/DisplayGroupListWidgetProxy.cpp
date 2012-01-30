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

#include "DisplayGroupListWidgetProxy.h"
#include "ContentWindowManager.h"
#include "Content.h"

DisplayGroupListWidgetProxy::DisplayGroupListWidgetProxy(boost::shared_ptr<DisplayGroupManager> displayGroupManager) : DisplayGroupInterface(displayGroupManager)
{
    // create actual list widget
    listWidget_ = new QListWidget();
}

DisplayGroupListWidgetProxy::~DisplayGroupListWidgetProxy()
{
    delete listWidget_;
}

QListWidget * DisplayGroupListWidgetProxy::getListWidget()
{
    return listWidget_;
}

void DisplayGroupListWidgetProxy::addContentWindowManager(boost::shared_ptr<ContentWindowManager> contentWindowManager, DisplayGroupInterface * source)
{
    DisplayGroupInterface::addContentWindowManager(contentWindowManager, source);

    if(source != this)
    {
        // for now, just clear and refresh the entire list, since this is just a read-only interface
        // later this could be modeled after DisplayGroupGraphicsViewProxy if we want to expand the interface
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::removeContentWindowManager(boost::shared_ptr<ContentWindowManager> contentWindowManager, DisplayGroupInterface * source)
{
    DisplayGroupInterface::removeContentWindowManager(contentWindowManager, source);

    if(source != this)
    {
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::moveContentWindowManagerToFront(boost::shared_ptr<ContentWindowManager> contentWindowManager, DisplayGroupInterface * source)
{
    DisplayGroupInterface::moveContentWindowManagerToFront(contentWindowManager, source);

    if(source != this)
    {
        refreshListWidget();
    }
}

void DisplayGroupListWidgetProxy::refreshListWidget()
{
    // clear list
    listWidget_->clear();

    for(unsigned int i=0; i<contentWindowManagers_.size(); i++)
    {
        // add to list view
        QListWidgetItem * newItem = new QListWidgetItem(listWidget_);
        newItem->setText(contentWindowManagers_[i]->getContent()->getURI().c_str());
    }
}
