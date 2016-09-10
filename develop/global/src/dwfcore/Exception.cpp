//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
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


#include "dwfcore/Core.h"
#include "dwfcore/Exception.h"
using namespace DWFCore;



_DWFCORE_API
DWFException::DWFException( const wchar_t* zMessage,
                            const char*    zFunction,
                            const wchar_t* zFile,
                            unsigned int   nLine )
throw()
            : _zFunction( zFunction )
            , _zFile( zFile )
            , _nLine( nLine )
{
    _zMessage[0] = 0;

    if (zMessage)
    {
        size_t nBytes = min( 511*sizeof(wchar_t), DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(zMessage) );

        DWFCORE_ZERO_MEMORY( _zMessage, nBytes + 2 );
        DWFCORE_COPY_MEMORY( _zMessage, zMessage, nBytes );
    }
}

_DWFCORE_API
DWFException::~DWFException()
throw()
{
    ;
}

_DWFCORE_API
DWFException::DWFException( const DWFException& rEx )
throw()
            : _zFunction( rEx._zFunction )
            , _zFile( rEx._zFile )
            , _nLine( rEx._nLine )
{
    _zMessage[0] = 0;

    if (rEx._zMessage)
    {
        size_t nBytes = min( 511*sizeof(wchar_t), DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(rEx._zMessage) );

        DWFCORE_ZERO_MEMORY( _zMessage, nBytes + 2 );
        DWFCORE_COPY_MEMORY( _zMessage, rEx._zMessage, nBytes );
    }
}

_DWFCORE_API
DWFException&
DWFException::operator=( const DWFException& rEx )
throw()
{
    _zFunction = rEx._zFunction;
    _zFile = rEx._zFile;
    _nLine = rEx._nLine;

    _zMessage[0] = 0;

    if (rEx._zMessage)
    {
        size_t nBytes = min( 511*sizeof(wchar_t), DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(rEx._zMessage) );

        DWFCORE_ZERO_MEMORY( _zMessage, nBytes + 2 );
        DWFCORE_COPY_MEMORY( _zMessage, rEx._zMessage, nBytes );
    }

    return *this;
}

_DWFCORE_API
const wchar_t*
DWFException::message() const
throw()
{
    return _zMessage;
}

_DWFCORE_API
const char*
DWFException::function() const
throw()
{
    return _zFunction;
}

_DWFCORE_API
const wchar_t*
DWFException::file() const
throw()
{
    return _zFile;
}

_DWFCORE_API
unsigned int
DWFException::line() const
throw()
{
    return _nLine;
}

