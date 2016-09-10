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


#include "dwfcore/MonitoredInputStream.h"
using namespace DWFCore;

_DWFCORE_API
DWFMonitoredInputStream::DWFMonitoredInputStream( DWFInputStream* pStream,
                                                  bool            bOwnStream )
throw()
                       : _pStream( pStream )
                       , _pMonitor( NULL )
                       , _pMonitorStream( NULL )
                       , _bOwnStream( bOwnStream )
                       , _bOwnMonitor( false )
                       , _bOwnMonitorStream( false )
{
    ;
}

_DWFCORE_API
DWFMonitoredInputStream::~DWFMonitoredInputStream()
throw()
{
    detach();

    if (_bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pStream );
    }
}

_DWFCORE_API
void
DWFMonitoredInputStream::attach( DWFMonitoredInputStream::Monitor* pMonitor, bool bOwnMonitor )
throw( DWFException )
{
    if (_pMonitor && _bOwnMonitor)
    {
        DWFCORE_FREE_OBJECT( _pMonitor );
    }

    _pMonitor = pMonitor;
    _bOwnMonitor = bOwnMonitor;
}

_DWFCORE_API
void
DWFMonitoredInputStream::attach( DWFOutputStream* pMonitor, bool bOwnMonitor )
throw( DWFException )
{
    if (_pMonitorStream && _bOwnMonitorStream)
    {
        DWFCORE_FREE_OBJECT( _pMonitorStream );
    }

    _pMonitorStream = pMonitor;
    _bOwnMonitorStream = bOwnMonitor;
}

_DWFCORE_API
void
DWFMonitoredInputStream::detach()
throw( DWFException )
{
    if (_pMonitor && _bOwnMonitor)
    {
        DWFCORE_FREE_OBJECT( _pMonitor );
    }

    if (_pMonitorStream && _bOwnMonitorStream)
    {
        DWFCORE_FREE_OBJECT( _pMonitorStream );
    }
}

_DWFCORE_API
size_t
DWFMonitoredInputStream::available() const
throw( DWFException )
{
    return _pStream->available();
}

_DWFCORE_API
size_t
DWFMonitoredInputStream::read( void*  pBuffer,
                               size_t nBytesToRead )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No underlying input stream" );
    }

    size_t nBytesRead = _pStream->read( pBuffer, nBytesToRead );

    if (_pMonitor)
    {
        _pMonitor->notify( pBuffer, nBytesToRead, nBytesRead );
    }

    if (_pMonitorStream)
    {
        _pMonitorStream->write( pBuffer, nBytesRead );
        _pMonitorStream->flush();
    }

    return nBytesRead;
}

_DWFCORE_API
off_t
DWFMonitoredInputStream::seek( int      eOrigin,
                               off_t    nOffset )
throw( DWFException )
{
    return _pStream->seek( eOrigin, nOffset );
}

