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
#include "dwfcore/InputStream.h"
#include "dwfcore/OutputStream.h"
#include "dwfcore/StreamOutputBufferDescriptor.h"
using namespace DWFCore;

_DWFCORE_API 
DWFStreamOutputBufferDescriptor::DWFStreamOutputBufferDescriptor( size_t nInitialBytes )
throw()
                     : DWFFileDescriptor( DWFFile(/*NOXLATE*/L"") )
                     , _pBuffer( NULL )
                     , _pOutPtr( NULL )
                     , _pEndPtr( NULL )
                     , _nBufferBytes( 0 )
                     , _nInitialBytes( nInitialBytes )
{;}


_DWFCORE_API
DWFStreamOutputBufferDescriptor::~DWFStreamOutputBufferDescriptor()
throw()
{
    reset();
}

_DWFCORE_API
void
DWFStreamOutputBufferDescriptor::open()
throw( DWFException )
{
    reset();

    _pBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, _nInitialBytes );
    if (_pBuffer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
    }

    DWFCORE_ZERO_MEMORY( _pBuffer, _nInitialBytes );
    _nBufferBytes = _nInitialBytes;
    _pOutPtr = _pBuffer;
    _pEndPtr = _pBuffer;
}

_DWFCORE_API
void
DWFStreamOutputBufferDescriptor::close()
throw( DWFException )
{
    // NOOP
}

_DWFCORE_API
void
DWFStreamOutputBufferDescriptor::reset()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
        _pOutPtr = NULL;
        _pEndPtr = NULL;
        _nBufferBytes = 0;
    }
}

_DWFCORE_API 
size_t
DWFStreamOutputBufferDescriptor::size()
throw( DWFException )
{
    return size_t(_pEndPtr - _pBuffer);
}

_DWFCORE_API
size_t
DWFStreamOutputBufferDescriptor::read( void* /*pBuffer*/,
                                       size_t /*nBytesToRead*/ )
throw( DWFException )
{
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Not implemented." );
}

_DWFCORE_API
off_t
DWFStreamOutputBufferDescriptor::seek( int   iOrigin,
                                       off_t nOffset )
throw( DWFException )
{
    off_t nCurrentOffset = off_t(_pOutPtr - _pBuffer);

    switch (iOrigin)
    {
    case SEEK_CUR:
        {
            if (nOffset>0)
            {
                _pOutPtr = ((_pOutPtr+nOffset)>_pEndPtr) ? _pEndPtr 
                                                         : _pOutPtr+nOffset;
            }
            else
            {
                _pOutPtr = ((_pOutPtr+nOffset)<_pBuffer) ? _pBuffer
                                                         : _pOutPtr+nOffset;
            }
            break;
        }
    case SEEK_SET:
        {
            _pOutPtr = (nOffset<0) ? _pBuffer 
                                   : ((_pBuffer+nOffset)>_pEndPtr) ? _pEndPtr
                                   : _pBuffer+nOffset;
            break;
        }
    case SEEK_END:
        {
            _pOutPtr = (nOffset<0) ? _pEndPtr
                                   : ((_pEndPtr-nOffset)<_pBuffer) ? _pBuffer
                                   : _pEndPtr-nOffset;
            break;
        }
    default:
        {
        }
    }

    return nCurrentOffset;
}

_DWFCORE_API
size_t
DWFStreamOutputBufferDescriptor::write( const void* pBuffer, size_t nBytesToWrite )
throw( DWFException )
{
    size_t nOffset = size_t(_pOutPtr - _pBuffer);
    size_t nBytesRemaining = _nBufferBytes - nOffset;

    if (nBytesToWrite > nBytesRemaining)
    {
        //
        //  Calculate the new buffer size, and allocate it.
        //        
        _nBufferBytes = max( 2*_nBufferBytes, 2*nBytesToWrite + nOffset);
        unsigned char* pNewBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, _nBufferBytes );

        //
        //  Copy over the previous data, only upto the offset. If pOutPtr is less than
        //  pEndPtr, anything beyond pOutPtr will get overwritten with the new data.
        //
        DWFCORE_COPY_MEMORY( pNewBuffer, _pBuffer, nOffset );
        DWFCORE_FREE_MEMORY( _pBuffer );
        
        _pBuffer = pNewBuffer;
        _pOutPtr = _pBuffer + nOffset;

        DWFCORE_COPY_MEMORY( _pOutPtr, pBuffer, nBytesToWrite );
        _pOutPtr += nBytesToWrite;
        _pEndPtr = _pOutPtr;
    }
    else
    {
        DWFCORE_COPY_MEMORY( _pOutPtr, pBuffer, nBytesToWrite );
        _pOutPtr += nBytesToWrite;

        if (_pOutPtr>_pEndPtr)
        {
            _pEndPtr = _pOutPtr;
        }
    }

    return nBytesToWrite;
}

_DWFCORE_API
void
DWFStreamOutputBufferDescriptor::flush()
throw( DWFException )
{
    //NO-OP
}


