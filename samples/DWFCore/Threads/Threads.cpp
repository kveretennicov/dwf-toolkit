//
//  Copyright (c) 2006 by Autodesk, Inc.
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

#include "stdafx.h"


using namespace std;
using namespace DWFCore;

#ifdef  _DWFCORE_WIN32_SYSTEM   
#define SLEEP( ms ) ( Sleep(ms) )
#else
#define SLEEP( ms ) ( usleep(ms) )
#endif

DWFCore::DWFThreadMutex g_oMutex;


class worker : public DWFThreadWorker
{
public:

    int nSEQ;

public:

    worker() throw() {;}
    virtual ~worker() throw() {;}

    void begin() throw()
    {

#ifdef  _DWFCORE_WIN32_SYSTEM
        DWORD nTID = ::GetCurrentThreadId();
#else
        pthread_t nTID = pthread_self();
#endif

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  START    ";
        cout << "Sequence [" << nSEQ << "] TID [" << nTID << "]" << "\n";
        cout.flush();
#ifdef  __linux__
		SLEEP( 150 );
        g_oMutex.unlock();
#else
		g_oMutex.unlock();
		SLEEP( 150 );
#endif

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  FINISH   ";
        cout << "Sequence [" << nSEQ << "] TID [" << nTID << "]" << "\n";
        cout.flush();
        g_oMutex.unlock();

    }
};

class lazyworker : public DWFThreadWorker
{

public:

    lazyworker() throw() {;}
    virtual ~lazyworker() throw() {;}

    void begin() throw()
    {

#ifdef  _DWFCORE_WIN32_SYSTEM
        DWORD nTID = ::GetCurrentThreadId();
#else
        pthread_t nTID = pthread_self();
#endif

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  START LAZYWORKER   ";
        cout << "TID [" << nTID << "]" << "\n";
        cout.flush();
#ifdef __linux__
		SLEEP( 1000000 );
		g_oMutex.unlock();
#else
		g_oMutex.unlock();
		SLEEP( 1000000 );
#endif

        g_oMutex.lock();
        cout << "LAZYWORKER Tick (" << DWFTimer::Tick64() << ")  FINISH   ";
        cout << "TID [" << nTID << "]" << "\n";
        cout.flush();
        g_oMutex.unlock();

    }
};



#define POOL    20
#define THREADS 10

int main()
{
        g_oMutex.init();

        DWFCore::DWFThreadPool oPool;
        oPool.init( POOL );
	
	//SLEEP(1000);

        cout << "Thread pool initialized with " << POOL << " threads\n\n\n";

        worker w[THREADS];

        unsigned long i = 0;
        for (; i<THREADS; i++)
        {
            w[i].nSEQ = i;
            oPool.run( w[i] );
        }

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  STARTED " << i << " workers\n\n";
        cout.flush();
        g_oMutex.unlock();

        lazyworker lz;
        DWFCore::DWFThreadPool::Controller* pLazyController = oPool.run( lz );

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  waiting for lazy worker...\n";
        cout.flush();
        g_oMutex.unlock();

        for (i=0; i<THREADS; i++)
        {
            w[i].nSEQ = i;
            oPool.run( w[i] );

            if ((i == (THREADS/2)) && pLazyController)
            {
                g_oMutex.lock();
                cout << "Tick (" << DWFTimer::Tick64() << ")  ending lazy worker...\n";
                cout.flush();
                g_oMutex.unlock();

                // add lock when end the lazy thread.
                // If lazy thread lock the g_oMutex; and the main thread kill the lazy thread;
                // then the g_oMutex cannot unlock; and lead the process into deadlock.
                g_oMutex.lock();
                pLazyController->end();
                g_oMutex.unlock();

                g_oMutex.lock();
                cout << "Tick (" << DWFTimer::Tick64() << ")  ended lazy worker...\n";
                cout << "Tick (" << DWFTimer::Tick64() << ")  deleting lazy worker controller...\n";
                cout.flush();
                g_oMutex.unlock();

                DWFCORE_FREE_OBJECT( pLazyController );
                pLazyController = NULL;

                g_oMutex.lock();
                cout << "Tick (" << DWFTimer::Tick64() << ")  lazy worker controller deleted.\n";
                cout.flush();
                g_oMutex.unlock();
            }
        }

        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  STARTED " << i << " workers\n\n";
        cout.flush();
        g_oMutex.unlock();

        SLEEP( 2000 );
        
        g_oMutex.lock();
        cout << "Tick (" << DWFTimer::Tick64() << ")  END APP\n\n";
        cout.flush();
        g_oMutex.unlock();


        g_oMutex.lock();
        cout << "OK\n";
        cout.flush();
        g_oMutex.unlock();

        g_oMutex.destroy();

    return 0;
}
