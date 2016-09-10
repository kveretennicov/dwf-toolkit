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

#include <sys/time.h>

#include "dwfcore/ansi/Signal.h"
using namespace DWFCore;


_DWFCORE_API
DWFSignal::DWFSignal()
throw()
         : _bInit( false )
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

        //
        // the default mutex attributes will provide a fast mutex (futex) implementation
        //
    if (pthread_mutex_init(&_tMutex, NULL) != 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to initialize mutex" );
    }

        //
        // by default, condition variable attribute is ignored
        //
    if (pthread_cond_init(&_tCondition, NULL) != 0)
    {
        pthread_mutex_destroy( &_tMutex );

        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to initialize signal" );
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
        pthread_cond_destroy( &_tCondition );
        pthread_mutex_destroy( &_tMutex );
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

    bool bWaitOK = false;

    //
    // acquire the mutex - this is REQUIRED before waiting on the condition variable
    // the wait routine will unlock the mutex before entering wait state
    //
    pthread_mutex_lock( &_tMutex );

        //
        // timed wait
        //
    if (nMilliseconds > 0)
    {
        struct timeval tNow;
        gettimeofday( &tNow, NULL );

        struct timespec tDuration;
        tDuration.tv_sec = (nMilliseconds / 1000);
        tDuration.tv_nsec = (nMilliseconds - tDuration.tv_sec) * 1000;

        bWaitOK = (pthread_cond_timedwait(&_tCondition, &_tMutex, &tDuration) == 0);
    }
    else
    {
        bWaitOK = (pthread_cond_wait(&_tCondition, &_tMutex) == 0);
    }

    //
    // release the mutex - this is REQUIRED after returning from a wait routine
    // as the mutex is left in a locked state
    //
    pthread_mutex_unlock( &_tMutex );

    //
    // result of wait
    //
    return bWaitOK;
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

    //
    // acquire the mutex - this is REQUIRED before signalling the condition variable
    //
    pthread_mutex_lock( &_tMutex );

    //
    // when in doubt, call broadcast instead of signal
    //
    pthread_cond_broadcast( &_tCondition );

    //
    // release the mutex - this is REQUIRED after signalling the condition variable
    //
    pthread_mutex_unlock( &_tMutex );
}


#endif

