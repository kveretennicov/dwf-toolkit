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



    //
    // ANSI only
    //
#ifdef  _DWFCORE_FOR_ANSI_SYSTEM_ONLY


//
//
// Atomic operations
//
//
#if defined( _DWFCORE_LINUX_SYSTEM ) && defined( HAVE_CONFIG_H )
#include "dwfcore/config.h"

#if     defined( _DWFCORE_USE_ASM_ATOMIC_H )
#include <asm/atomic.h>
#elif   defined( _DWFCORE_USE_ALSA_IATOMIC_H )
#include <alsa/iatomic.h>
#else
#define _DWFCORE_CORE_CPP_SKIP_ATOMIC
#endif

#ifndef _DWFCORE_CORE_CPP_SKIP_ATOMIC
namespace DWFCore
{

_DWFCORE_API
int
AtomicIncrement( volatile int* pValue )
throw()
{
    atomic_t tValue;
    atomic_set( &tValue, *pValue );
    atomic_inc( &tValue );

    *pValue = atomic_read( &tValue );
    return *pValue;
}

_DWFCORE_API
int
AtomicDecrement( volatile int* pValue )
throw()
{
    atomic_t tValue;
    atomic_set( &tValue, *pValue );
    atomic_dec( &tValue );

    *pValue = atomic_read( &tValue );
    return *pValue;
}

}
#endif


    //
    // Define default global allocation/deallocation operators
    //
#if defined( DWFCORE_ENABLE_MEMORY_BASE ) && defined( DWFCORE_USE_DEFAULT_MEMORY_BASE_IMPL )

void* DWFCore::DWFCoreMemory::operator new( size_t s )
{   
    return ::operator new( s );
}

void* DWFCore::DWFCoreMemory::operator new( size_t s, void* p )
{   
    p = ::operator new( s );
    return p;
}

void DWFCore::DWFCoreMemory::operator delete( void* p )
{ 
    ::operator delete( p ); 
}

void DWFCore::DWFCoreMemory::operator delete( void* p, void* )
{ 
    ::operator delete( p ); 
}
#endif

#endif
#endif




