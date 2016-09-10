//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
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
// Mac OSX implementation
//
#ifdef  _DWFCORE_MAC_SYSTEM

//
// Carbon API with atomic operations
//
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4) 
#include <libkern/OSAtomic.h>
#define OTAtomicAdd32 OSAtomicAdd32
typedef int32_t _SInt32;
#else
#include <Carbon/Carbon.h>
typedef SInt32 _SInt32;
#endif

namespace DWFCore
{


int
AtomicIncrement( volatile int* pValue )
throw()
{
    //
    // /usr/include/ppc/types.h defines int32_t with int
    // so this cast feels safe
    //
    return ::OTAtomicAdd32( 1, (_SInt32*)pValue );
}

int
AtomicDecrement( volatile int* pValue )
throw()
{
    //
    // /usr/include/ppc/types.h defines int32_t with int
    // so this cast feels safe
    //
    return ::OTAtomicAdd32( -1, (_SInt32*)pValue );
}

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


}


#endif


