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

#include "dwfcore/ThreadPool.h"
#include "dwfcore/Synchronization.h"
using namespace DWFCore;



static DWORD WINAPI RunThread( LPVOID pThread )
{
    ((DWFThread*)pThread)->run();

    ::ExitThread( 0 );
}

DWFThread::DWFThread( DWFThreadPool& rPool )
throw()
         : _rPool( rPool )
         , _hThread( NULL )
         , _oStateLock()
         , _oWorkSignal()
         , _pWorker( NULL )
         , _eRunState( eNone )
         , _eRequestState( eNone )
         , _eLastSuspendedState( eNone )
{
    ;
}

DWFThread::~DWFThread()
throw()
{
    _oWorkSignal.destroy();

    ::DeleteCriticalSection( &_oStateLock );
    ::CloseHandle( _hThread );
}

void
DWFThread::_begin()
throw( DWFException )
{
    //
    // initialization
    //
    _oWorkSignal.init();
    ::InitializeCriticalSection( &_oStateLock );

#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

        //
        // On Win9x systems a valid DWORD pointer must be passed in to receive the
        // the thread id
        //
    if (IsWindows9x())
    {
        DWORD dwThreadId;
        _hThread = ::CreateThread( NULL, 0, RunThread, (LPVOID)this, 0, &dwThreadId );
    }
    else
#endif
    {
        _hThread = ::CreateThread( NULL, 0, RunThread, (LPVOID)this, 0, NULL );
    }

    if (_hThread == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to create new thread" );
    }  
}

void
DWFThread::_join()
throw( DWFException )
{
    //
    // 
    //
    ::WaitForSingleObject( _hThread, INFINITE );
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
                    try
                    {
                        //
                        // tell the worker to do his thing
                        //
                        _pWorker->begin();
                    }
                        //
                        // they shouldn't be throwing any exceptions out of here
                        // but just in case...
                        //
                    catch (...)
                    {
                        ;
                    }

                    //
                    // clear - this MUST occur before returning the thread
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
    ::TerminateThread( _hThread, 0 );
}

void
DWFThread::suspend()
throw( DWFException )
{
    ::SuspendThread( _hThread );
    
    _eLastSuspendedState = _state();
    _setstate( eSuspend );
}

void
DWFThread::resume()
throw( DWFException )
{
        //
        // reset previous run state
        // unless something happened in the meantime
        //
    if (_state() == eSuspend)
    {
        _setstate( _eLastSuspendedState );
    }

    ::ResumeThread( _hThread );
}

DWFThread::teState
DWFThread::_state()
throw( DWFException )
{
    ::EnterCriticalSection( &_oStateLock );

    teState eCurrent = _eRunState;

    ::LeaveCriticalSection( &_oStateLock );

    return eCurrent;
}

void
DWFThread::_setstate( teState eState )
throw( DWFException )
{
    ::EnterCriticalSection( &_oStateLock );

    _eRunState = eState;

    ::LeaveCriticalSection( &_oStateLock );
}

DWFThread::teState
DWFThread::_requeststate()
throw( DWFException )
{
    ::EnterCriticalSection( &_oStateLock );

    teState eCurrent = _eRequestState;

    ::LeaveCriticalSection( &_oStateLock );

    return eCurrent;
}

void 
DWFThread::_setrequest( teState eState )
throw( DWFException )
{
    ::EnterCriticalSection( &_oStateLock );

    _eRequestState = eState;

    ::LeaveCriticalSection( &_oStateLock );
}

#endif

