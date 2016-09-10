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
#include "dwfcore/Timer.h"
using namespace DWFCore;


//
// Mac OSX implementation
//
#ifdef  _DWFCORE_MAC_SYSTEM

//
// Carbon API with absolute timer operations
//
//#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

DWFTimer::DWFTimer()
throw()
     : _nTick( 0LL )
{
    ;
}

DWFTimer::DWFTimer( const DWFTimer& rTimer )
throw()
     : _nTick( rTimer._nTick )
{
    ;
}

DWFTimer&
DWFTimer::operator=( const DWFTimer& rTimer )
throw()
{
    _nTick = rTimer._nTick;
    return *this;
}

DWFTimer::~DWFTimer()
throw()
{
}

unsigned long
DWFTimer::Tick32()
throw()
{
    AbsoluteTime tTime = ::UpTime();
    return *(unsigned long*)&tTime;
}

unsigned long long
DWFTimer::Tick64()
throw()
{
    AbsoluteTime tTime = ::UpTime();
    return *(unsigned long long *)&tTime;
}

void
DWFTimer::start()
throw()
{
    _nTick = Tick64();
}

unsigned long
DWFTimer::tick32()
throw()
{
    return (Tick32() - (unsigned long)(0x00000000ffffffff & _nTick));
}

unsigned long long
DWFTimer::tick64()
throw()
{
    return (Tick64() - _nTick);
}

void
DWFTimer::stop()
throw()
{
    _nTick = (Tick64() - _nTick);
}

unsigned long
DWFTimer::timer32()
throw()
{
    return (unsigned long)(0x00000000ffffffff & _nTick);
}

unsigned long long
DWFTimer::timer64()
throw()
{
    return _nTick;
}



#endif
