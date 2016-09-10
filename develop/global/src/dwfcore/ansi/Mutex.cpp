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
#ifdef  _DWFCORE_FOR_ANSI_SYSTEM_ONLY

#include "dwfcore/Synchronization.h"
using namespace DWFCore;


_DWFCORE_API
DWFThreadMutex::DWFThreadMutex()
throw()
              : _bInit( false )
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
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Mutex already initialized" );
    }

        //
        // the default mutex attributes will provide a fast mutex (futex) implementation
        //
    if (pthread_mutex_init(&_tMutex, NULL) != 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to initialize mutex" );
    }

    _bInit = true;
}

_DWFCORE_API
void
DWFThreadMutex::destroy()
throw( DWFException )
{
    if (_bInit)
    {
        pthread_mutex_destroy( &_tMutex );
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
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Mutex not initialized" );
    }

    pthread_mutex_lock( &_tMutex );
}

_DWFCORE_API
bool
DWFThreadMutex::trylock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Mutex not initialized" );
    }

    return (pthread_mutex_trylock(&_tMutex) == 0);
}

_DWFCORE_API
void
DWFThreadMutex::unlock()
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Mutex not initialized" );
    }

    pthread_mutex_unlock( &_tMutex );
}

#endif

