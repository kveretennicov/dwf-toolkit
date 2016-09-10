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

#include "XAML/pch.h"

XamlPolylineSegment::XamlPolylineSegment(const WT_XAML_Point_Set_Data& pointSet, bool bIsClosed)
: _oPointSet(pointSet.count(), pointSet.points(), true)
, _bIsWriteStart(false)
, _bIsClosed(bIsClosed)
{
}

XamlPolylineSegment::~XamlPolylineSegment(void)
throw()
{
    _notifyDelete();
}

WT_Result XamlPolylineSegment::serializeAttributeValue( WT_XAML_File &rFile, tMemoryBuffer*& rpBuffer ) const
{
    WT_Point2D * points = _oPointSet.points();
    if (!points)
    {
        WD_Assert(false && "No points for polyline!");
        return WT_Result::Internal_Error;
    }

    int count = _oPointSet.count();
    
    //requiring 48 characters per point (plus overhead for the first move)
    //is more than we'll need at the extreme, but being conservative with 
    //our estimates helps us to skip doing all of the string appends 
    //(i.e. reallocations and memory copies).
    size_t nRequired = (count+1) * sizeof(wchar_t) * 48;
    if ( rpBuffer->strlen() + nRequired > rpBuffer->size() )
    {
        //exponential growth, but these buffers are persistent for the 
        //session and can be reused over and over
        rpBuffer = rFile.exchangeBuffer( rpBuffer, max( rpBuffer->size()*2, rpBuffer->size()+nRequired ) );
    }

    char buf[64];

    size_t pointCopiesSize = _oPointSet.count() * sizeof(WT_Point2D);
    tMemoryBuffer* localPointBuffer = rFile.getBuffer( pointCopiesSize );
    if (localPointBuffer == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }
    WT_Point2D *pointCopies = (WT_Point2D *) localPointBuffer->buffer();
    DWFCORE_COPY_MEMORY( pointCopies, points, pointCopiesSize );

    WT_XAML_Point_Set_Data localPointSet( _oPointSet.count(), pointCopies, false );
    localPointSet.relativize(rFile);

    bool bLineStart = true;
    double x0 = _oPointSet.points()[0].m_x;
    double y0 = _oPointSet.points()[0].m_y;

    if (_bIsWriteStart)
    {
        // absolute
        buf[0] = 'M';
        DWFCORE_ASCII_STRING_COPY( buf+1, _oPointSet.points()[0].toString(10));
		rpBuffer->concatenate( buf );
    }
    else
    {
        // relative
        buf[0] = 'm';
        DWFCORE_ASCII_STRING_COPY( buf+1, localPointSet.points()[0].toString(10));
		rpBuffer->concatenate( buf );
    }

    bool bSetClosed = false;
    for (int loop = 1; loop < count; loop++)
    {
        double x = localPointSet.points()[loop].m_x;
        double y = localPointSet.points()[loop].m_y;
        double x1 = _oPointSet.points()[loop].m_x;
        double y1 = _oPointSet.points()[loop].m_y;

        if (x == 0 && y == 0 && loop == count - 1)
        {
            DWFCORE_ASCII_STRING_COPY( buf, "h0" );
        }
        else if (x1 == x0 && y1 == y0 && loop > 1 && loop == count - 1) 
        {
            DWFCORE_ASCII_STRING_COPY( buf, "z" );
            bSetClosed = true;
        }
        else if (x == 0 && y != 0)
        {
            DWFCORE_ASCII_STRING_COPY( buf, "v" );
            DWFString::DoubleToString( buf+1, 63, y, 10 );
            bLineStart = true;
        }
        else if (y == 0 && x != 0)
        {
            DWFCORE_ASCII_STRING_COPY( buf, "h" );
            DWFString::DoubleToString( buf+1, 63, x, 10 );
            bLineStart = true;
        }
        else 
        {
			if (bLineStart)
            {
                DWFCORE_ASCII_STRING_COPY( buf, "l" );
                bLineStart = false;
            }
            else
            {
                DWFCORE_ASCII_STRING_COPY( buf, " " );
            }
            DWFCORE_ASCII_STRING_COPY( buf+1, localPointSet.points()[loop].toString(10));
        }

		rpBuffer->concatenate( buf );
    }

    if (_bIsClosed && !bSetClosed)
    {
		rpBuffer->concatenate( "z" );
        rFile.update_current_point( _oPointSet.points()[0] );
    }
    else if (count>0)
    {
        rFile.update_current_point( _oPointSet.points()[count-1] );
    }

    rFile.releaseBuffer( localPointBuffer );

    return WT_Result::Success;
}
