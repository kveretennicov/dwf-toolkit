//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#include "dwf/w3dtk/W3DCamera.h"


W3DCamera::W3DCamera()
throw()
         : _nPositionX( 0.0 )
         , _nPositionY( 0.0 )
         , _nPositionZ( 0.0 )
         , _nTargetX( 0.0 )
         , _nTargetY( 0.0 )
         , _nTargetZ( 0.0 )
         , _nUpVectorX( 0.0 )
         , _nUpVectorY( 0.0 )
         , _nUpVectorZ( 0.0 )
         , _nFieldWidth( 0.0 )
         , _nFieldHeight( 0.0 )
         , _eProjection( ePerspective )
{
    ;
}

W3DCamera::W3DCamera( float nPositionX,    float nPositionY,   float nPositionZ,
                      float nTargetX,      float nTargetY,     float nTargetZ,
                      float nUpVectorX,    float nUpVectorY,   float nUpVectorZ,
                      float nFieldWidth,   float nFieldHeight,
                      teProjection eProjection )
throw()
         : _nPositionX( nPositionX )
         , _nPositionY( nPositionY )
         , _nPositionZ( nPositionZ )
         , _nTargetX( nTargetX )
         , _nTargetY( nTargetY )
         , _nTargetZ( nTargetZ )
         , _nUpVectorX( nUpVectorX )
         , _nUpVectorY( nUpVectorY )
         , _nUpVectorZ( nUpVectorZ )
         , _nFieldWidth( nFieldWidth )
         , _nFieldHeight( nFieldHeight )
         , _eProjection( eProjection )
{
    ;
}

W3DCamera::~W3DCamera()
throw()
{
    ;
}

W3DCamera::W3DCamera( const W3DCamera& rCamera )
throw()
         : _nPositionX( rCamera._nPositionX )
         , _nPositionY( rCamera._nPositionY )
         , _nPositionZ( rCamera._nPositionZ )
         , _nTargetX( rCamera._nTargetX )
         , _nTargetY( rCamera._nTargetY )
         , _nTargetZ( rCamera._nTargetZ)
         , _nUpVectorX( rCamera._nUpVectorX )
         , _nUpVectorY( rCamera._nUpVectorY )
         , _nUpVectorZ( rCamera._nUpVectorZ )
         , _nFieldWidth( rCamera._nFieldWidth )
         , _nFieldHeight( rCamera._nFieldHeight )
         , _eProjection( rCamera._eProjection )
{
    ;
}

W3DCamera&
W3DCamera::operator=( const W3DCamera& rCamera )
throw()
{
    _nPositionX = rCamera._nPositionX;
    _nPositionY = rCamera._nPositionY;
    _nPositionZ = rCamera._nPositionZ;
    _nTargetX = rCamera._nTargetX;
    _nTargetY = rCamera._nTargetY;
    _nTargetZ = rCamera._nTargetZ;
    _nUpVectorX = rCamera._nUpVectorX;
    _nUpVectorY = rCamera._nUpVectorY;
    _nUpVectorZ = rCamera._nUpVectorZ;
    _nFieldWidth = rCamera._nFieldWidth;
    _nFieldHeight = rCamera._nFieldHeight;
    _eProjection = rCamera._eProjection;

    return *this;
}

void
W3DCamera::setPosition( float nPositionX,
                        float nPositionY,
                        float nPositionZ )
throw()
{
    _nPositionX = nPositionX;
    _nPositionY = nPositionY;
    _nPositionZ = nPositionZ;
}

const float*
W3DCamera::getPosition( float anPosition[3] )
const
throw()
{
    anPosition[0] = _nPositionX;
    anPosition[1] = _nPositionY;
    anPosition[2] = _nPositionZ;

    return anPosition;
}

void
W3DCamera::setTarget( float nTargetX,
                      float nTargetY,
                      float nTargetZ )
throw()
{
    _nTargetX = nTargetX;
    _nTargetY = nTargetY;
    _nTargetZ = nTargetZ;
}

const float*
W3DCamera::getTarget( float anTarget[3] )
const
throw()
{
    anTarget[0] = _nTargetX;
    anTarget[1] = _nTargetY;
    anTarget[2] = _nTargetZ;

    return anTarget;
}

void
W3DCamera::setUpVector( float nUpVectorX,
                        float nUpVectorY,
                        float nUpVectorZ )
throw()
{
    _nUpVectorX = nUpVectorX;
    _nUpVectorY = nUpVectorY;
    _nUpVectorZ = nUpVectorZ;
}

const float*
W3DCamera::getUpVector( float anUpVector[3] )
const
throw()
{
    anUpVector[0] = _nUpVectorX;
    anUpVector[1] = _nUpVectorY;
    anUpVector[2] = _nUpVectorZ;

    return anUpVector;
}

void
W3DCamera::setField( float nFieldWidth,
                     float nFieldHeight )
throw()
{
    _nFieldWidth = nFieldWidth;
    _nFieldHeight = nFieldHeight;
}

const float*
W3DCamera::getField( float anField[2] )
const
throw()
{
    anField[0] = _nFieldWidth;
    anField[1] = _nFieldHeight;

    return anField;
}

void
W3DCamera::setProjection( teProjection eProjection )
throw()
{
    _eProjection = eProjection;
}

W3DCamera::teProjection
W3DCamera::getProjection()
const
throw()
{
    return _eProjection;
}
