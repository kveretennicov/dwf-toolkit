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
DWFSemaphore::DWFSemaphore( unsigned int nCount )
throw()
              : _bInit( false )
              , _nCount( nCount )
              , _hSemaphore( NULL )
{
    ;
}

_DWFCORE_API
DWFSemaphore::~DWFSemaphore()
throw()
{
    destroy();
}

_DWFCORE_API
void
DWFSemaphore::init()
throw( DWFException )
{
    if (_bInit)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Semaphore already initialized" );
    }

#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

        //
        // use the ASCII version on Win9x systems
        //
    if (IsWindows9x())
    {
        _hSemaphore = ::CreateSemaphoreA( NULL, _nCount, _nCount, NULL );
    }
    else
#endif
    {
        _hSemaphore = ::CreateSemaphore( NULL, _nCount, _nCount, NULL );
    }

    if (_hSemaphore == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to initialize semaphore" );
    }

    _bInit = true;
}

_DWFCORE_API
void
DWFSemaphore::destroy()
throw( DWFException )
{
    if (_bInit)
    {
        ::CloseHandle( _hSemaphore );
        _bInit = false;
    }
}

_DWFCORE_API
void
DWFSemaphore::lock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Semaphore not initialized" );
    }

    ::WaitForSingleObject( _hSemaphore, INFINITE );
}

_DWFCORE_API
bool
DWFSemaphore::trylock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Semaphore not initialized" );
    }

    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This feature is not available on Win32 systems" );
}

_DWFCORE_API
void
DWFSemaphore::unlock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Semaphore not initialized" );
    }

    ::ReleaseSemaphore( _hSemaphore, 1, NULL );
}

#endif

