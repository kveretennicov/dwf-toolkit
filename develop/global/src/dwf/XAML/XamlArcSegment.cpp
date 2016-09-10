//
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

//This chops doubles to float quite simply
#define D2F(d) (float)(((long long)((d)*100000.0f))/100000.0f)

XamlArcSegment::XamlArcSegment(void)
: _bIsWriteStart(false)
, _nMajor(0)
, _nMinor(0)
, _fStart(.0f)
, _fEnd(.0f)
, _fRotate(.0f)
, _bIsFilled(false)
, _bIsClosed(false)
{
}

XamlArcSegment::XamlArcSegment(const XamlArcSegment& s)
: _bIsWriteStart(false)
{
    _bIsWriteStart = s._bIsWriteStart;
    _nMajor = s._nMajor;
    _nMinor = s._nMinor;
    _oPosition = s._oPosition;
    _fStart = s._fStart;
    _fEnd = s._fEnd;
    _fRotate = s._fRotate;
    _bIsFilled = s._bIsFilled;
    _bIsClosed = s._bIsClosed;
}

XamlArcSegment::XamlArcSegment(const WT_Filled_Ellipse& ellipse)
: _bIsWriteStart(false)
{
    _nMajor = ellipse.major();
    _nMinor = ellipse.minor();
    _oPosition = ellipse.position();
    _fStart = ellipse.start_radian();
    _fEnd = ellipse.end_radian();
    _fRotate = ellipse.tilt_radian();
    _bIsFilled = true;
    WT_Unsigned_Integer32 nEnd = ellipse.end();
    while (nEnd > 65535) nEnd -= 65536;
    _bIsClosed = ellipse.start()==nEnd;
}

XamlArcSegment::XamlArcSegment(const WT_Outline_Ellipse& ellipse)
: _bIsWriteStart(false)
{
    _nMajor = ellipse.major();
    _nMinor = ellipse.minor();
    _oPosition = ellipse.position();
    _fStart = ellipse.start_radian();
    _fEnd = ellipse.end_radian();
    _fRotate = ellipse.tilt_radian();
    _bIsFilled = false;
    WT_Unsigned_Integer32 nEnd = ellipse.end();
    while (nEnd > 65535) nEnd -= 65536;
    _bIsClosed = ellipse.start()==nEnd;
}

XamlArcSegment::~XamlArcSegment(void)
throw()
{
    _notifyDelete();
}

// a xr,yr rx fArc fSweep x,y
WT_Result XamlArcSegment::serializeAttributeValue( WT_XAML_File &rFile, tMemoryBuffer*& rpBuffer ) const
{
    //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    // use a temporary point to avoid modifying the original
    // coordinates (this must be a local to the serialization)
    //
    WT_Point2D center(_oPosition.m_x, _oPosition.m_y);
    WD_CHECK( rFile.flipPoint(center) );

    // These are needed since we flipped the y's.  We just mirror
    // the start, end, and rotation angles around the x axis.
    float fCalcStart = _fStart ? -_fStart : 0.0f;
    float fCalcEnd = _fEnd ? -_fEnd : 0.0f;
    float fCalcRotate = _fRotate ? -_fRotate : 0.0f;
 
    // But we use the original values here since... well... they work just fine!
    bool bIsLargeArc = (fabs(_fEnd - _fStart) > M_PI);

    if (_bIsClosed)
    {
        fCalcEnd = fCalcStart + D2F(M_PI);
    }

    teSweepDirection eSweep = fCalcEnd > fCalcStart ? Clockwise : Counterclockwise; //positive is clockwise, as per XAML spec.  Freaky.

    // set the start and end points of the ellipse based on major and minor radii
    WT_Point2D startPoint, endPoint;
    startPoint.m_x = cos(fCalcStart) * _nMajor;
    startPoint.m_y = sin(fCalcStart) * _nMinor;
    endPoint.m_x = cos(fCalcEnd) * _nMajor;
    endPoint.m_y = sin(fCalcEnd) * _nMinor;

    // run the points through a rotation transform.
    WT_Matrix2D oMatrix;
    oMatrix(0,0) *= cos( -fCalcRotate );
    oMatrix(0,1) = -sin( -fCalcRotate );
    oMatrix(1,0) =  sin( -fCalcRotate );
    oMatrix(1,1) =  cos( -fCalcRotate );
    oMatrix.transform( startPoint, startPoint );
    oMatrix.transform( endPoint, endPoint );

    // translate from to the center point so we have good values
    startPoint.m_x = startPoint.m_x + center.m_x;
    startPoint.m_y = startPoint.m_y + center.m_y;
    endPoint.m_x = endPoint.m_x + center.m_x;
    endPoint.m_y = endPoint.m_y + center.m_y;

    //allocating 1024 bytes for the arc is well more than we'll need (worst
    //case is around 512 bytes), but being conservative with 
    //our estimates helps us to skip doing all of the string appends 
    //(i.e. reallocations and memory copies).
    size_t nRequired = 2048;
    if ( rpBuffer->strlen() + nRequired > rpBuffer->size() )
    {
        //exponential growth, but these buffers are persistent for the 
        //session and can be reused over and over
        rpBuffer = rFile.exchangeBuffer( rpBuffer, max( rpBuffer->size()*2, rpBuffer->size()+nRequired ) );
    }

	char buf[256];

    // "relativize" endpoints
    WT_Point2D currPoint;
    currPoint = rFile.de_update_current_point(currPoint); // this effectively gives back the current coordinates

    WT_Point2D centerPt, startPt, endPt; 

    if (_bIsFilled && !_bIsClosed)
    {
        WT_Point2D centerPt = rFile.update_current_point( center );
        startPt = rFile.update_current_point( startPoint );
        endPt = rFile.update_current_point( endPoint );

        // If we're filled but not closed, we need additional segments from the center to the start, then we draw
        // the arc, and finally, we complete the closed "partially eaten pie" figure afterwards
        if (_bIsWriteStart)
        {
            // absolute
			rpBuffer->concatenate("M");
			rpBuffer->concatenate(center.toString(10));
			rpBuffer->concatenate("l");
			rpBuffer->concatenate(startPt.toString(10));
        }
        // But if we're a segment within a Path, we use relative coordinates.
        else
        {
            // relative
			rpBuffer->concatenate("m");
			rpBuffer->concatenate(centerPt.toString(10));
			rpBuffer->concatenate("l");
			rpBuffer->concatenate(startPt.toString(10));
        }
    }
    else
    {
        startPt = rFile.update_current_point( startPoint );
        endPt = rFile.update_current_point( endPoint );
        // We missed the need for moving the cursor every time.  If this segment is the start of
        // a new Path, we must use absolute coordinates.
        if (_bIsWriteStart)
        {
            // absolute
			rpBuffer->concatenate("M");
			rpBuffer->concatenate(startPoint.toString(10));
        }
        // But if we're a segment within a Path, we use relative coordinates.
        else
        {
            // relative
			rpBuffer->concatenate("m");
			rpBuffer->concatenate(startPt.toString(10));
        }
    }
    
    // Write out the attribute string
    char buf2[64];
    DWFString::DoubleToString(buf2, 64, fCalcRotate * (180.0f/M_PI), 17);

    _DWFCORE_SPRINTF(buf, 256, "a%d,%d %s %d %d %s", 
        (int)_nMajor, (int)_nMinor, buf2, 
        bIsLargeArc ? 1 : 0, (eSweep == Clockwise) ? 1 : 0, 
        endPt.toString(10));
    rpBuffer->concatenate(buf);

    if (_bIsFilled && !_bIsClosed)
    {
        // Complete the "partially eaten pie" figure by drawing a segment back to the center.
        _DWFCORE_SPRINTF(buf, 256, "z");
        rpBuffer->concatenate(buf);			
        rFile.update_current_point( center );
    }

    if (_bIsClosed)
    {
        // The XAML start and end points cannot be coexistant, i.e. the "0,0" as
        // a relative end point doesn't fly.  Microsoft says that we must serialize 
        // two arc segments.  To facilitate that, we've already reset the endpoint 
        // to be mid way in our ellipse. That happened above where "_bIsClosed" is 
        // set.   So, we translate back from the end point, and plug in the same 
        // values reversing the sweep and arc settings.  Anything else is madness!  jk

        // relative, always
		WT_Point2D endReverse( (-1*endPt.m_x), (-1*endPt.m_y) );
		rpBuffer->concatenate("m");
		rpBuffer->concatenate(endReverse.toString(10));

        DWFString::DoubleToString(buf2, 64, fCalcRotate * (180.0f/M_PI), 17);
        _DWFCORE_SPRINTF(buf, 256, "a%d,%d %s %d %d %s", 
            (int)_nMajor, (int)_nMinor, buf2, 
            bIsLargeArc ? 0 : 1, (eSweep == Clockwise) ? 0 : 1, 
			endPt.toString(10));
        rpBuffer->concatenate(buf);
    }

    return WT_Result::Success;
}
