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
#include "dwfcore/Timer.h"
using namespace DWFCore;

//
// Intel and AMD x86 architecture implementation
//
#if defined(_DWFCORE_X86_SYSTEM) && !defined(_DWFCORE_MAC_SYSTEM)

_DWFCORE_API
DWFTimer::DWFTimer()
throw()
        : _nTick( 0 )
{
    ;
}

_DWFCORE_API
DWFTimer::DWFTimer( const DWFTimer& rTimer )
throw()
        : _nTick( rTimer._nTick )
{
    ;
}

_DWFCORE_API
DWFTimer&
DWFTimer::operator=( const DWFTimer& rTimer )
throw()
{
    _nTick = rTimer._nTick;
    return *this;
}

_DWFCORE_API
DWFTimer::~DWFTimer()
throw()
{
}

_DWFCORE_API
unsigned long
DWFTimer::Tick32()
throw()
{
#ifdef    _DWFCORE_WIN32_SYSTEM
    __asm
    {
        __emit  0fh
        __emit  31h
    }
#else
#ifdef    _DWFCORE_LINUX_SYSTEM
    unsigned long eax = 0L;
    
    __asm__ __volatile__( ".byte 15, 49" : : : "eax", "edx" );
    __asm__ __volatile__( "movl %%eax, %0" : "=r"(eax) );
    
    //
    // gcc is checking no return
    //
    return eax;
#else
#error    Cannot continue - implementation not provided for this hardware configuration
#endif
#endif

    //
    // lower 32-bits in eax
    //
}

_DWFCORE_API
uint64_t
DWFTimer::Tick64()
throw()
{
#ifdef    _DWFCORE_WIN32_SYSTEM
    __asm
    {
        __emit  0fh
        __emit  31h
    }
#else
#ifdef    _DWFCORE_LINUX_SYSTEM
    unsigned long eax = 0L;
    unsigned long edx = 0L;
    
    __asm__ __volatile__( ".byte 15, 49" : : : "eax", "edx" );
    __asm__ __volatile__( "movl %%eax, %0; movl %%edx, %1": "=r"(eax), "=r"(edx) );
    
    //
    // gcc is checking no return
    //
    uint64_t nTick64 = edx;
    nTick64 <<= 32;
    nTick64 |= eax;
    return nTick64;
#else
#error    Cannot continue - implementation not provided for this hardware configuration
#endif
#endif

    //
    // lower 32-bits in eax
    // upper 32-bits in edx
    //
}

_DWFCORE_API
void
DWFTimer::start()
throw()
{
    _nTick = Tick64();
}

_DWFCORE_API
unsigned long
DWFTimer::tick32()
throw()
{
    return (Tick32() - (unsigned long)(0x00000000ffffffff & _nTick));
}

_DWFCORE_API
uint64_t
DWFTimer::tick64()
throw()
{
    return (Tick64() - _nTick);
}

_DWFCORE_API
void
DWFTimer::stop()
throw()
{
    _nTick = (Tick64() - _nTick);
}

_DWFCORE_API
unsigned long
DWFTimer::timer32()
throw()
{
    return (unsigned long)(0x00000000ffffffff & _nTick);
}

_DWFCORE_API
uint64_t
DWFTimer::timer64()
throw()
{
    return _nTick;
}

#endif

