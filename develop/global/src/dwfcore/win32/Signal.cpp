//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
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
#ifdef  _DWFCORE_WIN32_SYSTEM


#include "dwfcore/win32/Signal.h"
using namespace DWFCore;


_DWFCORE_API
DWFSignal::DWFSignal()
throw()
         : _bInit( false )
         , _hEvent( NULL )
{
    ;
}

_DWFCORE_API
DWFSignal::~DWFSignal()
throw()
{
    destroy();
}

_DWFCORE_API
void
DWFSignal::init()
throw( DWFException )
{
    if (_bInit)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Signal already initialized" );
    }

#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

    //
    // use the ASCII version on Win9x systems
    //
    if (IsWindows9x())
    {
        _hEvent = ::CreateEventA( NULL, FALSE, FALSE, NULL );
    }
    else
#endif
    {
        _hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    }

    if (_hEvent == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to create event" );
    }

    _bInit = true;
}

_DWFCORE_API
void
DWFSignal::destroy()
throw( DWFException )
{
    if (_bInit)
    {
        ::CloseHandle( _hEvent );
        _bInit = false;
    }
}

_DWFCORE_API
bool
DWFSignal::wait( unsigned long nMilliseconds )
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Signal not initialized" );
    }

    return (::WaitForSingleObject(_hEvent, (nMilliseconds == 0) ? INFINITE : nMilliseconds) == WAIT_OBJECT_0);
}

_DWFCORE_API
void
DWFSignal::raise()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Signal not initialized" );
    }

    ::SetEvent( _hEvent );
}


#endif

