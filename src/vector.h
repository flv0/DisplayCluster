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

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

template <class T> void vectorCrossProduct(const T a[3], const T b[3], T c[3])
{
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];

    return;
}

template <class T> T vectorMagnitude(const T a[3])
{
    return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

}

template <class T> void vectorNormalize(T a[3])
{
    T magnitude = vectorMagnitude(a);

    a[0] /= magnitude;
    a[1] /= magnitude;
    a[2] /= magnitude;

    return;
}

template <class T> T vectorDotProduct(const T a[3], const T b[3])
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

template <class T> T vectorDistance(const T a[3], const T b[3])
{
    return sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
}

#endif
