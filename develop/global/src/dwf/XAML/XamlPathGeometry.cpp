//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "XAML/pch.h"

const char XamlPathGeometry::kcAbsolute_Move             = 'M';
const char XamlPathGeometry::kcRelative_Move             = 'm';
const char XamlPathGeometry::kcAbsolute_Line             = 'L';
const char XamlPathGeometry::kcRelative_Line             = 'l';
const char XamlPathGeometry::kcAbsolute_Close            = 'Z';
const char XamlPathGeometry::kcRelative_Close            = 'z';
const char XamlPathGeometry::kcAbsolute_HorizontalLine   = 'H';
const char XamlPathGeometry::kcRelative_HorizontalLine   = 'h';
const char XamlPathGeometry::kcAbsolute_VerticalLine     = 'V';
const char XamlPathGeometry::kcRelative_VerticalLine     = 'v';
const char XamlPathGeometry::kcAbsolute_EllipticalArc    = 'A';
const char XamlPathGeometry::kcRelative_EllipticalArc    = 'a';

namespace XamlPathGeometry_Cpp
{
    static char *pNull = "";
}

using namespace XamlPathGeometry_Cpp;

XamlPathGeometry::XamlPathGeometry(void)
: _oPathFigure()
{
}

XamlPathGeometry::XamlPathGeometry(const XamlPathGeometry& s)
: _oPathFigure()
{
    operator= (s);
}

XamlPathGeometry::~XamlPathGeometry(void) 
{
}

XamlPathGeometry& XamlPathGeometry::operator=(const XamlPathGeometry& s)
{ 
    if (&s != this)
    {
        _oPathFigure = s._oPathFigure;
    }
    return * this;
}

bool XamlPathGeometry::operator==(const XamlPathGeometry& s) const
{
    return _oPathFigure == s._oPathFigure;
}

WT_Result XamlPathGeometry::materializeAttribute( WT_XAML_File &rFile, const char* pAttribute )
{
    return _oPathFigure.materializeAttribute( rFile, pAttribute );
}

WT_Result XamlPathGeometry::serializeAttributeValue( WT_XAML_File &rFile, tMemoryBuffer*& rpBuffer ) const
{
    return _oPathFigure.serializeAttributeValue( rFile, rpBuffer );
}

XamlPathGeometry::XamlPathFigure::~XamlPathFigure()
throw()
{
    vector<XamlGraphicsObject*>::const_iterator it = _voGraphicsObjects.begin();
    for(; it != _voGraphicsObjects.end(); it++)
    {
        XamlGraphicsObject *p = *it;
        if (p->owner() == this)
        {
            DWFCORE_FREE_OBJECT(p);
        }
        else
        {
            p->unobserve( *this );
        }
    }
}

WT_Result XamlPathGeometry::XamlPathFigure::serializeAttributeValue( WT_XAML_File &rFile, tMemoryBuffer*& rpBuffer ) const
{
    bool bIsFirstStart = true;
    WT_Point2D newStartPoint;
    vector<XamlGraphicsObject*>::const_iterator it = _voGraphicsObjects.begin();

    for(; it != _voGraphicsObjects.end(); it++)
    {
        XamlGraphicsObject* pGraphicsObject = *it;

        pGraphicsObject->setWriteStart(bIsFirstStart);
        WD_CHECK( pGraphicsObject->serializeAttributeValue( rFile, rpBuffer ) );
        bIsFirstStart = false;
    }

    return WT_Result::Success;
}

WT_Result 
XamlPathGeometry::XamlPathFigure::materializeAttribute( WT_XAML_File &rFile, const char* pAttribute )
{
    if (pAttribute == NULL)
    {
        return WT_Result::Internal_Error;
    }
    char* pRead = const_cast< char* >( pAttribute );
    char cCmd = *pRead;
    
    WT_Point2D current_point;

    while( *pRead != '\0' )
    {
        if (cCmd == XamlPathGeometry::kcAbsolute_Move )
        {
            _createPathGeometry( rFile, pRead );
            WD_CHECK( getPoint( &rFile, pRead, current_point, false ) );
            rFile.update_current_point( current_point );
        }
        else if (cCmd == XamlPathGeometry::kcRelative_Move )
        {
            _createPathGeometry( rFile, pRead );
            WD_CHECK( getPoint( &rFile, pRead, current_point, true ) );
            rFile.update_current_point( current_point );
        }
        else if (cCmd == XamlPathGeometry::kcAbsolute_Line ||
                 cCmd == XamlPathGeometry::kcRelative_Line ||
                 cCmd == XamlPathGeometry::kcAbsolute_HorizontalLine ||
                 cCmd == XamlPathGeometry::kcRelative_HorizontalLine ||
                 cCmd == XamlPathGeometry::kcAbsolute_VerticalLine ||
                 cCmd == XamlPathGeometry::kcRelative_VerticalLine ||
                 cCmd == XamlPathGeometry::kcAbsolute_EllipticalArc ||
                 cCmd == XamlPathGeometry::kcRelative_EllipticalArc)
        {
            WD_CHECK( _createPathGeometry( rFile, pRead ) );
        }
        else if ( (cCmd == XamlPathGeometry::kcAbsolute_Close) || (cCmd == XamlPathGeometry::kcRelative_Close))
        {
            WD_CHECK( _createPathGeometry( rFile, pRead ) );
            if (_voGraphicsObjects.size() == 0)
                return WT_Result::Corrupt_File_Error;
            pRead++;
        }
        else
        {
            WD_Assert( false ); //where are we? 
            pRead++; // ignore for the new feature
        }

        cCmd = _getNextCommand( pRead );
    }

    if (_oPointVector.size() > 0 )
    {
        WD_CHECK( _createPathGeometry( rFile, pNull ) );
    }

    return WT_Result::Success;
}

char 
XamlPathGeometry::XamlPathFigure::_getNextCommand( char *& rpRead ) const
{
    char chCmd = 0; 
    while(*rpRead == ' ')
        rpRead++;

    while(*rpRead != '\0')
    {
        if((*rpRead>= 'a' && *rpRead <= 'z' && *rpRead != 'e') ||
            (*rpRead>= 'A' && *rpRead <= 'Z' && *rpRead != 'E') )
        {            
            chCmd = *rpRead;
            break;
        }
        rpRead++;
    }
    return chCmd;

}

WT_Result
XamlPathGeometry::XamlPathFigure::getPoints(WT_XAML_File *pFile, char *&rpRead, vector<WT_Point2D> &points, bool bIsRelative, bool bAddFirstPoint)
{
    WT_Point2D oPt;
    bool bFirstPoint = false;

    while(*rpRead &&
        ((*rpRead >= 'a' && *rpRead <= 'z') ||
        (*rpRead >= 'A' && *rpRead <= 'Z')))
        rpRead++;

    while(*rpRead && 
        !(*rpRead >= 'a' && *rpRead <= 'z') && 
        !(*rpRead >= 'A' && *rpRead <= 'Z'))
    {
        if (pFile)
        {
            if (!bFirstPoint)
            {
                oPt = pFile->de_update_current_point( WT_Point2D() );
                if (bAddFirstPoint)
                {
                    points.push_back( oPt );
                }
                bFirstPoint = true;
            }
        }
        WD_CHECK( getPoint(pFile, rpRead, oPt, bIsRelative ) );
        points.push_back( oPt );
    }

    return WT_Result::Success;
}

WT_Result 
XamlPathGeometry::XamlPathFigure::getScalar( char* &rpRead, double& rScalar )
{
    char* pCoords = NULL;

    while(*rpRead == ' ' || (*rpRead >= 'a' && *rpRead <= 'z') ||
        (*rpRead >= 'A' && *rpRead <= 'Z'))
        rpRead++;
    pCoords = rpRead;
    while((*rpRead != ' ') && 
        (!(*rpRead >= 'a' && *rpRead <= 'z') || *rpRead == 'e') && 
        (!(*rpRead >= 'A' && *rpRead <= 'Z') || *rpRead == 'E') && 
        (*rpRead != '\0'))
    {
        rpRead++;
    }

    char cHold = *rpRead;

    *rpRead = '\0';
    WD_CHECK( getScalarFromString( pCoords, rScalar) );
    *rpRead = cHold;

    return WT_Result::Success;
}

WT_Result 
XamlPathGeometry::XamlPathFigure::getPoint(WT_XAML_File *pFile, char* &rpRead, WT_Point2D& rPoint, bool bIsRelative)
{
    char* pCoords = NULL;

    while(*rpRead == ' ' || (*rpRead >= 'a' && *rpRead <= 'z') ||
        (*rpRead >= 'A' && *rpRead <= 'Z'))
        rpRead++;
    pCoords = rpRead;
    while((*rpRead != ' ') && 
        (!(*rpRead >= 'a' && *rpRead <= 'z') || *rpRead == 'e') && 
        (!(*rpRead >= 'A' && *rpRead <= 'Z') || *rpRead == 'E') && 
        (*rpRead != '\0'))
    {
        rpRead++;
    }

    char cHold = *rpRead;

    *rpRead = '\0';
    WD_CHECK( getPointFromString(pFile, pCoords, rPoint, bIsRelative) );
    *rpRead = cHold;

    return WT_Result::Success;
}

//String should have x,y kind of format to create logical point
WT_Result 
XamlPathGeometry::XamlPathFigure::getPointFromString(WT_XAML_File *pFile, char *pCoords, WT_Point2D& rPoint, bool bIsRelative)
{
    char *pCoords2 = strchr(pCoords, ',');
    if (pCoords2 != NULL)
    {
        char cHold = *(pCoords2);
        *(pCoords2++) = '\0';

        rPoint.m_x = DWFString::StringToDouble(pCoords);
        rPoint.m_y = DWFString::StringToDouble(pCoords2);

        *(--pCoords2) = cHold; //restore all to its former glory
    }
    else
    {
        WD_Assert( false ); //not okay!
        return WT_Result::Internal_Error;
    }

    if (pFile)
    {
        if(bIsRelative)
            rPoint = pFile->de_update_current_point(rPoint);
        else
            pFile->update_current_point(rPoint);
    }

    return WT_Result::Success;
}

WT_Result 
XamlPathGeometry::XamlPathFigure::getScalarFromString( char *pCoords, double& rScalar )
{
    rScalar = atof(pCoords);
    return WT_Result::Success;
}

WT_Result 
XamlPathGeometry::XamlPathFigure::_createPathGeometry( WT_XAML_File& rFile, char *& rpRead )
{
    bool bRelative = false;
    bool bClosed = false;
    switch( *rpRead )
    {
    case 'z':
    case 'Z':
        bClosed = true;
    case '\0':
    case 'M':
    case 'm':
        {
            if ( _oPointVector.size() )
            {

                WT_Point2D oFirst = _oPointVector[0];

                //
                // y-flip the coordinates (the page layout must have been
                // set beforehand on the file)
                //
                unsigned int i;
                for(i = 0; i < _oPointVector.size(); i++)
                {
                    rFile.unflipPoint(_oPointVector[ i ]);
                }

                WT_XAML_Point_Set_Data oPointSet( _oPointVector );
                XamlPolylineSegment *pSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment( oPointSet ) );
                if (pSegment == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }

                if (bClosed)
                {
                    pSegment->closed() = bClosed;
                    rFile.update_current_point( oFirst );
                }

                addDrawable( pSegment );
                _oPointVector.clear();
            }
        }
        break;

    case 'l':
        bRelative = true;
    case 'L':
        {
            //Polyline segment

            //
            // if _oPointVector is already populated, do not duplicate vertices
            // (the 1st entry in oPoints is then already at the end of _oPointVector)
            //
            WD_CHECK( getPoints( &rFile, rpRead, _oPointVector, bRelative, _oPointVector.empty() ) );

            break;
        }

    case 'h':
        bRelative = true;
    case 'H':
        {
            //Polyline segment
            WT_Point2D oPoints[2];
            oPoints[0] = rFile.de_update_current_point( WT_Point2D() ); //hold the current point

            //not really a full point, but we use it to parse
            WD_CHECK( getScalar( rpRead, oPoints[1].m_x ) );
            oPoints[1].m_y = oPoints[0].m_y;
            if (bRelative)
                oPoints[1].m_x += oPoints[0].m_x;

            rFile.update_current_point( oPoints[1] );

            if(_oPointVector.size() == 0)
            {
                _oPointVector.push_back( oPoints[0] );
            }    
            _oPointVector.push_back( oPoints[1] );
            break;
        }

    case 'v':
        bRelative = true;
    case 'V':
        {
            //Polyline segment
            //Polyline segment
            WT_Point2D oPoints[2];
            oPoints[0] = rFile.de_update_current_point( WT_Point2D() ); //hold the current point

            //not really a full point, but we use it to parse
            WD_CHECK( getScalar( rpRead, oPoints[1].m_y ) );
            oPoints[1].m_x = oPoints[0].m_x;
            if (bRelative)
                oPoints[1].m_y += oPoints[0].m_y;

            rFile.update_current_point( oPoints[1] );

            if(_oPointVector.size() == 0)
            {
                _oPointVector.push_back( oPoints[0] );
            }
            _oPointVector.push_back( oPoints[1] );
            break;
        }

    case 'a':
        bRelative = true;
    case 'A':
        {
            WD_CHECK( _createPathGeometry( rFile, pNull ) );
            //Arc segment
            WT_Point2D oStartPoint = rFile.de_update_current_point( WT_Point2D() ); //hold the current point
            WT_Point2D oEndPoint;
            WT_Point2D oTemp;

            //Looking for the following:
            //major, minor rotation isLarge sweepDirection endpoint

            //not really a point, but we use it to parse
            WD_CHECK( getPoint( NULL, rpRead, oTemp, false ) ); //send NULL file so as not to update current point
            float fMajor = (float)oTemp.m_x;
            float fMinor = (float)oTemp.m_y;

            double fDouble;
            WD_CHECK( getScalar( rpRead, fDouble ) );
            float fRotation = (float) fDouble; //degrees

            //Note: At this point as we have most information coming from w2x,
            //we dont need these flags (bIsLargeArc and eSweepDirection) and oEndPoint
            //to materialize WT_Ellipse from ArcSegment. However, commenting them out 
            //messes up our current point tracking, so although these are discarded, the
            //parser needs to deal with them anyway.  We comment out the arc/sweep setting
            //since we really don't need them until we are a full-blown XAML reader.

            WD_CHECK( getScalar( rpRead, fDouble ) );
            //bool bIsLargeArc = floor(fDouble + 0.5) == 1.0; 
            
            WD_CHECK( getScalar( rpRead, fDouble ) );
            //XamlArcSegment::teSweepDirection eSweepDirection = (floor(fDouble + 0.5) == 0.0) ? XamlArcSegment::Clockwise : XamlArcSegment::Counterclockwise;
          
            WD_CHECK( getPoint( &rFile, rpRead, oEndPoint, bRelative ) );
                                              
            XamlArcSegment *pSegment = DWFCORE_ALLOC_OBJECT( XamlArcSegment() );
            if (pSegment == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }

            pSegment->major() = (WT_Integer32) floor(fMajor + 0.5); //round for fuzz/noise
            pSegment->minor() = (WT_Integer32) floor(fMinor + 0.5); //round for fuzz/noise
            pSegment->tilt_radian() = (float) (-fRotation * (M_PI/180.));

            addDrawable( pSegment );
            break;
        }
    }

    return WT_Result::Success;
}


XamlPathGeometry::XamlPathFigure::XamlPathFigure(const XamlPathGeometry::XamlPathFigure& s)
{
    XamlPathGeometry::XamlPathFigure::operator =(s);
}


XamlPathGeometry::XamlPathFigure& XamlPathGeometry::XamlPathFigure::operator=(const XamlPathGeometry::XamlPathFigure& s)
{
    if (&s != this) 
    {
        reset();
        appendFigure(s);
    }
    return *this;
}

bool XamlPathGeometry::XamlPathFigure::operator==(const XamlPathGeometry::XamlPathFigure& s) const
{
    if (&s == this)
    {
        return true;
    }

    return _voGraphicsObjects == s._voGraphicsObjects;
}

void XamlPathGeometry::XamlPathFigure::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    vector<XamlGraphicsObject*>::iterator it = _voGraphicsObjects.begin();
    for(; it != _voGraphicsObjects.end(); it++)
    {
        if ( &rOwnable == *it )
        {
            _voGraphicsObjects.erase( it );
            return;
        }
    }
}


void XamlPathGeometry::XamlPathFigure::addDrawable(XamlGraphicsObject* p)
{
    _voGraphicsObjects.push_back(p); 
    p->own( *this );
}

void XamlPathGeometry::XamlPathFigure::appendFigure(const XamlPathFigure& s)
{
    vector<XamlGraphicsObject*>::const_iterator it = s._voGraphicsObjects.begin();
    for(; it != s._voGraphicsObjects.end(); it++)
    {
        addDrawable( *it );
    }
}

void XamlPathGeometry::XamlPathFigure::reset()
{
    vector<XamlGraphicsObject*>::const_iterator it = _voGraphicsObjects.begin();
    for(; it != _voGraphicsObjects.end(); it++)
    {
        XamlGraphicsObject *p = *it;
        p->unobserve( *this );
    }

    _voGraphicsObjects.clear();
}
