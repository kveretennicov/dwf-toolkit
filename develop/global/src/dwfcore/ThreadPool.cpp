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

#include "dwfcore/ThreadPool.h"
#include "dwfcore/Synchronization.h"
using namespace DWFCore;


_DWFCORE_API
DWFThreadPool::DWFThreadPool()
throw()
             : _bInit( false )
             , _pQueueMutex( NULL )
             , _pQueueSemaphore( NULL )
             , _oMonitor()
             , _pMonitorThread( NULL )
{
    ;
}

_DWFCORE_API
DWFThreadPool::~DWFThreadPool()
throw()
{
        //
        // clean-up by ending and joining each thread.
        // don't use the queue since it's possible for a thread
        // to be missing from here if it's in use and we'd leak it.
        // also, important, never lock the queue mutex here or the
        // worker thread will deadlock on the _returnThread() call.
        //
    vector<DWFThread*>::iterator iThread = _oThreads.begin();
    for (; iThread != _oThreads.end(); iThread++)
    {
        (*iThread)->end();
        (*iThread)->_join();
    
        DWFCORE_FREE_OBJECT( (*iThread) );
    }

    //
    // the monitor
    //
    _oMonitor.finish();
    _pMonitorThread->end();
    _pMonitorThread->_join();

    DWFCORE_FREE_OBJECT( _pMonitorThread );

    if (_pQueueMutex)
    {
        _pQueueMutex->destroy();

        DWFCORE_FREE_OBJECT( _pQueueMutex );
    }

    if (_pQueueSemaphore)
    {
        _pQueueSemaphore->destroy();

        DWFCORE_FREE_OBJECT( _pQueueSemaphore );
    }
}

_DWFCORE_API
void
DWFThreadPool::init( unsigned int nThreads )
throw( DWFException )
{
    if (_bInit)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Thread pool already initialized" );
    }

    if (nThreads == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Thread count must be greater than zero" );
    }

    //
    // This mutex locks access to the queue itself
    //
    _pQueueMutex = DWFCORE_ALLOC_OBJECT( DWFThreadMutex );
    if (_pQueueMutex == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate mutex" );
    }

    _pQueueMutex->init();

    //
    // The semaphore provides the blocking mechanism for acquiring pooled threads
    //
    _pQueueSemaphore = DWFCORE_ALLOC_OBJECT( DWFSemaphore(nThreads) );
    if (_pQueueSemaphore == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate mutex" );
    }

    _pQueueSemaphore->init();

    size_t iThread = 0;
    for (; iThread < nThreads; iThread++)
    {
        DWFThread* pThread = DWFCORE_ALLOC_OBJECT( DWFThread(*this) );
        if (pThread == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate thread" );
        }

        //
        // all threads stored here for proper clean-up
        // never used by the pool runtime though.
        //
        _oThreads.push_back( pThread );

        //
        // available cache
        //
        _oThreadQueue.push( pThread );

        //
        // begin the thread and get it ready for use
        //
        pThread->_begin();
    }

    //
    // crate the monitor thread
    //
    _pMonitorThread = DWFCORE_ALLOC_OBJECT( DWFThread(*this) );
    if (_pMonitorThread == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate monitor thread" );
    }

    //
    // start up the monitor thread
    //
    _pMonitorThread->_begin();

    //
    // run the monitor
    //
    _pMonitorThread->_work( _oMonitor );

    //
    // state
    //
    _bInit = true;
}

_DWFCORE_API
DWFThreadPool::Controller*
DWFThreadPool::run( DWFThreadWorker& rWorker )
throw( DWFException )
{
    if (_bInit == false)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Thread pool not initialized" );
    }

    //
    // wait for and acquire the next available pooled thread
    //
    DWFThread* pThread = _acquireThread();

    //
    // create a controller
    //
    Controller* pController = DWFCORE_ALLOC_OBJECT( Controller(*pThread, *this) );

        //
        // return the thread to the pool before throwing an exception
        // although an out of memory exception probably is the beginning of the end anyhow
        //
    if (pController == NULL)
    {
        _returnThread( pThread );

        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate thread controller" );
    }

    //
    // run the worker in the thread
    //
    pThread->_work( rWorker );

    //
    // return the controller to the caller
    //
    return pController;
}

void
DWFThreadPool::_end( DWFThread* pThread )
throw( DWFException )
{
    //
    // request pooled thread the end itself gracefully taking the worker with it
    //
    _oMonitor.request( *pThread, DWFThread::eEnd, 1000 );

    //
    // request state after call
    //
    DWFThread::teState eResult = pThread->_requeststate();

        //
        // the state request wasn't cleared
        //
    if (eResult == DWFThread::eEnd)
    {
        //
        // now we have no choice but to hard kill this thread
        //
        _oMonitor.request( *pThread, DWFThread::eKill, 1000 );
    }

    //
    // when a thread is ended in this fashion, it must be replaced.
    // notice in the destructor (of this object) the thread itself
    // is stopped and joined. in the case that the control flow ends
    // up right here, the thread was ended through the controller interface.
    // in order to preserve the integrity of the thread pool, that is,
    // maintain the same number of threads as it is expected to have,
    // we must manage the destruction and replacement of the ended thread here.
    //
    // NOTE: This is going to invalidate the controller initiating the request...
    //       but that should hardly matter given the nature of what's happening here.
    //

    //
    // replace the thread
    //
    DWFThread* pNewThread = DWFCORE_ALLOC_OBJECT( DWFThread(*this) );
    if (pNewThread == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate thread" );
    }

    //
    // begin the thread and get it ready for use
    //
    pNewThread->_begin();

    //
    // safely add it into the queue
    // this will also match up the semaphore count
    // since the now-delete thread came out of an _acquireThread() call
    //
    _returnThread( pNewThread );

    //
    // Replace the thread in _oThreads.
    // The thread pool has two threads lists:
    // One is the _oThreadQueue, which is a producer-consumer queue of worker threads.
    //    The queue works when run/suspend/resume/end a thread.
    // The other is _oThreads, all threads stored here for proper clean-up
    //    never used by the pool runtime though.
    // So delete/new the DWFThread should add into _oThreads.
    //
    vector<DWFThread*>::iterator iThread = _oThreads.begin();
    for (; iThread != _oThreads.end(); iThread++)
    {
        if(*iThread == pThread)
        {
            *iThread = pNewThread;
            break;
        }
    }

    //
    // kill off the thread itself
    //
    DWFCORE_FREE_OBJECT( pThread );
}

DWFThread::teState
DWFThreadPool::_suspend( DWFThread& rThread )
throw()
{
    //
    // ask the monitor to suspend the worker
    //
    _oMonitor.request( rThread, DWFThread::eSuspend, 1000 );

    //
    // return the state of the thread
    //
    return rThread._state();
}

DWFThread::teState
DWFThreadPool::_resume( DWFThread& rThread )
throw()
{
    //
    // ask the monitor to resume the worker
    //
    _oMonitor.request( rThread, DWFThread::eRun, 1000 );

    //
    // return the state of the thread
    //
    return rThread._state();
}

DWFThread*
DWFThreadPool::_acquireThread()
throw( DWFException )
{
    //
    // wait for a thread to become available
    //
    _pQueueSemaphore->lock();

    //
    // lock access to the pool (obviously this must occur AFTER the semaphore wait returns)
    //
    _pQueueMutex->lock();

    //
    // get a thread
    //
    DWFThread* pThread = _oThreadQueue.front();

    //
    // remove it from the queue
    //
    _oThreadQueue.pop();

    //
    // open up access to the queue
    //
    _pQueueMutex->unlock();

    //
    // return the thread
    //
    return pThread;
}

void
DWFThreadPool::_returnThread( DWFThread* pThread )
throw( DWFException )
{
    //
    // lock access to the pool
    //
    _pQueueMutex->lock();

    //
    // add it back to the queue
    //
    _oThreadQueue.push( pThread );

    //
    // open up access to the queue
    //
    _pQueueMutex->unlock();

    //
    // "return" the semaphore
    //
    _pQueueSemaphore->unlock();
}


///
/// Controller
///

DWFThreadPool::Controller::Controller( DWFThread&     rThread,
                                       DWFThreadPool& rPool )
throw()
             : _rThread( rThread )
             , _rPool( rPool )
{
    ;
}

DWFThreadPool::Controller::~Controller()
throw()
{
    ;
}

void
DWFThreadPool::Controller::end()
throw( DWFException )
{
    return _rPool._end( &_rThread );
}

DWFThread::teState
DWFThreadPool::Controller::suspend()
throw( DWFException )
{
    return _rPool._suspend( _rThread );
}

DWFThread::teState
DWFThreadPool::Controller::resume()
throw( DWFException )
{
    return _rPool._resume( _rThread );
}


///
/// Monitor
///

void
DWFThreadPool::_Monitor::begin()
throw()
{
    while (_bRun)
    {
            //
            // wait for something to do
            //
        if (_oRequestSignal.wait())
        {
            try
            {
                switch (_eRequest)
                {
                        //
                        // request graceful shutdown of the thread
                        //
                    case DWFThread::eEnd:
                    {
                        _pThread->end();
                        break;
                    }

                        //
                        // hard kill
                        //
                    case DWFThread::eKill:
                    {
                        _pThread->kill();
                        break;
                    }

                        //
                        // suspend
                        //
                    case DWFThread::eSuspend:
                    {
                        _pThread->suspend();
                        break;
                    }

                        //
                        // resume
                        //
                    case DWFThread::eRun:
                    {
                        _pThread->resume();
                        break;
                    }

                    case DWFThread::eNone:
                    default:
                    {
                        ;
                    }
                }
            }
                //
                // just in case
                //
            catch (...)
            {
                ;
            }

            //
            // clear
            //
            _pThread = NULL;
            _eRequest = DWFThread::eNone;

            //
            // unblock request
            //
            _oResponseSignal.raise();
        }
    }
}


void
DWFThreadPool::_Monitor::finish()
throw()
{
    //
    // tell the main loop to stop running
    //
    _bRun = false;

    //
    // wake the main loop
    //
    _oRequestSignal.raise();

    //
    //
    // just in case someone is blocking infinitely
    // on a request, unblock that thread
    //
    _oResponseSignal.raise();
}

bool
DWFThreadPool::_Monitor::request( DWFThread&         rThread,
                                  DWFThread::teState eRequest,
                                  unsigned int       nMilliseconds )
throw()
{
    //
    // process on request at a time
    //
    _oRequestMutex.lock();

    //
    // set the request args
    //
    _pThread = &rThread;
    _eRequest = eRequest;

    //
    // issue the request
    //
    _oRequestSignal.raise();

    //
    // wait for the response
    //
    bool bWaitOK = _oResponseSignal.wait( nMilliseconds );

    //
    // unlock
    //
    _oRequestMutex.unlock();

    //
    // wait result
    //
    return bWaitOK;
}
