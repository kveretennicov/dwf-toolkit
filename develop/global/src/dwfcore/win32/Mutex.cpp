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
#ifdef  _DWFCORE_WIN32_SYSTEM

#include "dwfcore/Synchronization.h"
using namespace DWFCore;


_DWFCORE_API
DWFThreadMutex::DWFThreadMutex()
throw()
              : _bInit( false )
              , _tSection()
{
    ;
}

_DWFCORE_API
DWFThreadMutex::~DWFThreadMutex()
throw()
{
    destroy();
}

_DWFCORE_API
void
DWFThreadMutex::init()
throw( DWFException )
{
    if (_bInit)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex already initialized" );
    }

    _bInit = true;
    ::InitializeCriticalSection( &_tSection );
}

_DWFCORE_API
void
DWFThreadMutex::destroy()
throw( DWFException )
{
    if (_bInit)
    {
        ::DeleteCriticalSection( &_tSection );
        _bInit = false;
    }
}

_DWFCORE_API
void
DWFThreadMutex::lock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    ::EnterCriticalSection( &_tSection );
}

_DWFCORE_API
bool
DWFThreadMutex::trylock()
throw( DWFException )
{
#if (_WIN32_WINNT < 0x0400)

    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This feature is not available on Win9x configurations" );

#else

    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    return (TRUE == ::TryEnterCriticalSection(&_tSection));

#endif
}

_DWFCORE_API
void
DWFThreadMutex::unlock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    ::LeaveCriticalSection( &_tSection );
}

_DWFCORE_API
DWFProcessMutex::DWFProcessMutex( const DWFString& zName )
throw()
               : _bInit( false )
               , _hMutex( NULL )
               , _zName( zName )
{
    ;
}

_DWFCORE_API
DWFProcessMutex::~DWFProcessMutex()
throw()
{
    destroy();
}

_DWFCORE_API
void
DWFProcessMutex::destroy()
throw( DWFException )
{
    if (_bInit && _hMutex)
    {
        ::CloseHandle( _hMutex );
    }
}

_DWFCORE_API
void
DWFProcessMutex::init()
throw( DWFException )
{
    if (_bInit)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex already initialized" );
    }

    if (_zName.bytes())
    {
        ::CreateMutexW( NULL, FALSE, _zName );
    }
    else
    {
        ::CreateMutex( NULL, FALSE, NULL );
    }

    _bInit = true;
    
}

void
_DWFCORE_API
DWFProcessMutex::lock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    ::WaitForSingleObject( _hMutex, INFINITE );
}

_DWFCORE_API
bool
DWFProcessMutex::trylock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    //
    // wait for one millisecond, return true if we got it
    //
    return (WAIT_OBJECT_0 == ::WaitForSingleObject(_hMutex, 1));
}

_DWFCORE_API
void
DWFProcessMutex::unlock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Mutex not initialized" );
    }

    ::ReleaseMutex( _hMutex );
}

_DWFCORE_API
const DWFString&
DWFProcessMutex::name()
const
throw()
{
    return _zName;
}


#endif


