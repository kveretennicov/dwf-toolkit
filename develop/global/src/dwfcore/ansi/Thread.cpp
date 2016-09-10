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
#ifdef  _DWFCORE_FOR_ANSI_SYSTEM_ONLY

#include "dwfcore/ThreadPool.h"
#include "dwfcore/Synchronization.h"
using namespace DWFCore;


static void* RunThread( void* pThread )
{
    ((DWFThread*)pThread)->run();

    pthread_exit( NULL );
}

DWFThread::DWFThread( DWFThreadPool& rPool )
throw()
         : _rPool( rPool )
         , _pWorker( NULL )
         , _eRunState( eNone )
         , _eRequestState( eNone )
{
    ;
}

DWFThread::~DWFThread()
throw()
{
    _oWorkSignal.destroy();

    pthread_mutex_destroy( &_tStateMutex );
    pthread_attr_destroy( &_tThreadAttributes );
}

void
DWFThread::_begin()
throw( DWFException )
{
    //
    // initialization
    //
    _oWorkSignal.init();

    if (pthread_mutex_init(&_tStateMutex, NULL) != 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to initialize mutex" );
    }

    //
    // not all pthread implementations may create threads in a joinable intial state
    // for maximum portability, we should explicitly do it
    //
    pthread_attr_init( &_tThreadAttributes );
    pthread_attr_setdetachstate( &_tThreadAttributes, PTHREAD_CREATE_JOINABLE );

        //
        // create a launch the thread
        //
    if (pthread_create(&_tThread, &_tThreadAttributes, RunThread, (void *)this) != 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to create new thread" );
    }
}

void
DWFThread::_join()
throw( DWFException )
{
    pthread_join( _tThread, NULL );
}
void
DWFThread::run()
throw()
{
    //
    // now the thread is actually running
    //
    _setstate( eRun );

    try
    {
            //
            // Run until we are requested to end
            //
        while (_requeststate() != eEnd)
        {
                //
                // wait for some work to do
                //
            if (_oWorkSignal.wait())
            {
                    //
                    // before proceeding check the current state request
                    //
                if (_requeststate() == eNone)
                {
                    //
                    // change run state
                    //
                    _setstate( eWork );

                    //
                    // tell the worker to do his thing
                    //
                    _pWorker->begin();

                    //
                    // clear - this MUST occure before returning the thread
                    // to the pool since there is no guarantee we can beat
                    // the next request from setting his worker
                    //
                    _pWorker = NULL;

                        //
                        // check the thread state again
                        // and only return the thread if
                        // all systems are go
                        //
                    if (_requeststate() == eNone)
                    {
                        //
                        // return to the pool
                        //
                        _rPool._returnThread( this );
                    }

                    //
                    // change run state
                    //
                    _setstate( eRun );

                    //
                    // release anyone waiting to use this thread for more work
                    //
                    _oWorkSignal.raise();
                }
            }
        }

        //
        // clear the request
        //
        _setrequest( eNone );
    }
    catch (...)
    {
        ;
    }

    //
    // now the thread is actually ended
    //
    _setstate( eEnd );
}

void
DWFThread::_work( DWFThreadWorker& rWorker )
throw( DWFException )
{
    //
    // set the next worker
    //
    _pWorker = &rWorker;

    if (_state() == eWork)
    {
        _oWorkSignal.wait();
        usleep( 10 );
    }

    //
    // wake up the thread
    //
    _oWorkSignal.raise();
}

void
DWFThread::end()
throw( DWFException )
{
    //
    // change request state (prevents re-pooling)
    //
    _setrequest( eEnd );

    //
    // just in case, wake him up
    //
    _oWorkSignal.raise();
}

void
DWFThread::kill()
throw( DWFException )
{
    //
    // change request state (prevents re-pooling)
    //
    _setrequest( eKill );

    //
    // just in case, wake him up
    //
    _oWorkSignal.raise();

    //
    // hard kill
    //
    pthread_cancel( _tThread );
}

void
DWFThread::suspend()
throw( DWFException )
{
    //
    // not supported
    //
}

void
DWFThread::resume()
throw( DWFException )
{
    //
    // not supported
    //
}

DWFThread::teState
DWFThread::_state()
throw( DWFException )
{
    pthread_mutex_lock( &_tStateMutex );

    teState eCurrent = _eRunState;

    pthread_mutex_unlock( &_tStateMutex );

    return eCurrent;
}

void
DWFThread::_setstate( teState eState )
throw( DWFException )
{
    pthread_mutex_lock( &_tStateMutex );

    _eRunState = eState;

    pthread_mutex_unlock( &_tStateMutex );
}

DWFThread::teState
DWFThread::_requeststate()
throw( DWFException )
{
    pthread_mutex_lock( &_tStateMutex );

    teState eCurrent = _eRequestState;

    pthread_mutex_unlock( &_tStateMutex );

    return eCurrent;
}

void 
DWFThread::_setrequest( teState eState )
throw( DWFException )
{
    pthread_mutex_lock( &_tStateMutex );

    _eRequestState = eState;

    pthread_mutex_unlock( &_tStateMutex );
}

#endif

