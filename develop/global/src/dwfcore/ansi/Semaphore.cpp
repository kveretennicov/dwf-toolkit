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
DWFSemaphore::DWFSemaphore( unsigned int nCount )
throw()
              : _bInit( false )
              , _nCount( nCount )
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


#ifdef _DWFCORE_MAC_SYSTEM
	if (semaphore_create(mach_task_self(), &_tSemaphore, SYNC_POLICY_FIFO, _nCount) != 0)
#else
    if (sem_init(&_tSemaphore, 0, _nCount) != 0)
#endif
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
#ifdef _DWFCORE_MAC_SYSTEM
		semaphore_destroy(mach_task_self(), _tSemaphore);
#else
        sem_destroy( &_tSemaphore );
#endif
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

#ifdef _DWFCORE_MAC_SYSTEM
	semaphore_wait( _tSemaphore );
#else
    sem_wait( &_tSemaphore );
#endif
	
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

#ifdef _DWFCORE_MAC_SYSTEM
	mach_timespec_t delay = { 0, 0};
	return (semaphore_timedwait(_tSemaphore, delay) == 0);
#else
    return (sem_trywait(&_tSemaphore) == 0);
#endif
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

#ifdef _DWFCORE_MAC_SYSTEM
	semaphore_signal( _tSemaphore );
#else
    sem_post( &_tSemaphore );
#endif
}

#endif

