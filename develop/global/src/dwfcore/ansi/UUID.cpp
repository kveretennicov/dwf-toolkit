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

#include "dwfcore/Timer.h"
#include "dwfcore/ansi/UUID.h"
using namespace DWFCore;

#ifdef  HAVE_NETINET_ETHER_H
#include <netinet/ether.h>
#endif


DWFUUIDImpl_ANSI::DWFUUIDImpl_ANSI()
throw()
{
    srand( DWFTimer::Tick32() );
    _generate();
}

DWFUUIDImpl_ANSI::DWFUUIDImpl_ANSI( const DWFUUIDImpl_ANSI& zuuid )
throw()
                : _zUUID( zuuid._zUUID )
{
    DWFCORE_COPY_MEMORY( _tUUID, zuuid._tUUID, sizeof(_tUUID) );
}

DWFUUIDImpl_ANSI&
DWFUUIDImpl_ANSI::operator=( const DWFUUIDImpl_ANSI& zuuid )
throw()
{
    _zUUID = zuuid._zUUID;

    DWFCORE_COPY_MEMORY( _tUUID, zuuid._tUUID, sizeof(_tUUID) );

    return *this;
}

DWFUUIDImpl_ANSI::~DWFUUIDImpl_ANSI()
throw()
{
    ;
}

DWFUUIDImpl*
DWFUUIDImpl_ANSI::clone() const
throw( DWFException )
{
    DWFUUIDImpl_ANSI* pNew = DWFCORE_ALLOC_OBJECT( DWFUUIDImpl_ANSI(*this) );
    if (pNew == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate clone" );
    }

    return pNew;
}

const DWFString&
DWFUUIDImpl_ANSI::uuid( bool bSquash )
throw( DWFException )
{
    char str[64] = {0};

    if (bSquash)
    {
        DWFString::EncodeBase64( _tUUID, sizeof(_tUUID), str, 64, false );
    }
    else
    {
        ::sprintf( str, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", _tUUID[0], _tUUID[1], _tUUID[2], _tUUID[3], _tUUID[4], _tUUID[5], _tUUID[6], _tUUID[7], _tUUID[8], _tUUID[9], _tUUID[10], _tUUID[11], _tUUID[12], _tUUID[13], _tUUID[14], _tUUID[15] );
    }

    _zUUID.assign( str );
    return _zUUID;
}

const DWFString&
DWFUUIDImpl_ANSI::next( bool bSquash )
throw( DWFException )
{
        //
        // increment the "timestamp" portion
        //
    if (_tUUID[3] < 0xff)
    {
        _tUUID[3]++;
    }
    else if (_tUUID[2] < 0xff)
    {
        _tUUID[2]++;
    }
    else if (_tUUID[1] < 0xff)
    {
        _tUUID[1]++;
    }
    else if (_tUUID[0] < 0xfe)
    {
        _tUUID[0]++;
    }
    else
    {
        _generate();
    }

    return uuid( bSquash );
}

void
DWFUUIDImpl_ANSI::_generate()
throw()
{
#ifdef  HAVE_LIBUUID

    //
    // by default, uuid_generate() will attempt to use a
    // algorithm with a highly random number generator
    // instead of using the MAC address of the network adapter.
    // the reason given is to enhance the privacy of the uuid source.
    // failing the availability of such a generator, the standard
    // timestamp and MAC address algorithm is used.
    // to change this behavior, use the uuid_generate_time() function
    //
    uuid_generate( _tUUID );

#else

    //
    // generate our own uuid
    // first, the timestamp portion, get 64 bits from the CPU
    //
    uint64_t nTimestamp = DWFTimer::Tick64();

    //
    // xaaabbbbcccccccc gets arranged like so:
    // cccccccc-bbbb-1aaa
    // note the 1 indicates a time-based uuid with a known or random host id
    //
    nTimestamp = ((nTimestamp << 32) & _DWFCORE_LARGE_CONSTANT(0xffffffff00000000)) |
                 ((nTimestamp >> 16) & _DWFCORE_LARGE_CONSTANT(0x00000000ffff0000)) |
                 ((nTimestamp >> 48) & _DWFCORE_LARGE_CONSTANT(0x0000000000000fff)) |
                                       _DWFCORE_LARGE_CONSTANT(0x0000000000001000);

    //
    // 
    //
    DWFCORE_COPY_MEMORY( &_tUUID[0], &nTimestamp, 8 );

    //
    // second, get a clock id, which can just be a random number with at least 14-bits
    //
    int nClock = rand();

    //
    // 0xmmmmNnnn gets arranged like so:
    // 0x80 + 2 bits of N + nnn
    // the leading 10 indicates the current uuid variant,
    // others are 0 (NCS) and 110 (Microsoft)
    //
    nClock = (nClock & 0x00003fff) | 
                       0x00008000;

    //
    // 
    //
    DWFCORE_COPY_MEMORY( &_tUUID[8], &nClock, 2 );

#ifdef  HAVE_NETINET_ETHER_H
    //
    // if this system header is available, 
    // we can get the MAC address of the network card
    //

    struct ether_addr tAddr;
    ether_hostton( NULL, &tAddr );

    //
    // the host id goes into the uuid at the end:
    // cccccccc-bbbb-1aaa-Vnnn-hhhhhhhhhhhh
    //
    DWFCORE_COPY_MEMORY( &_tUUID[10], &tAddr, 6 );

#else
    //
    // we have to make up a host id
    //
    nTimestamp   = rand();
    nTimestamp <<= 16;
    nTimestamp  |= rand();
    nTimestamp <<= 16;
    nTimestamp  |= rand();

    //
    //
    //
    DWFCORE_COPY_MEMORY( &_tUUID[10], &nTimestamp, 6 );
    
#endif

#endif
}
