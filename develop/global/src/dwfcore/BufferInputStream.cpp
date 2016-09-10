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



#include "dwfcore/BufferInputStream.h"
using namespace DWFCore;


_DWFCORE_API
DWFBufferInputStream::DWFBufferInputStream( const void*   pBuffer,
                                            size_t        nBufferBytes,
                                            bool          bOwnBuffer )
throw()
                    : _pBuffer( pBuffer )
                    , _iBufferPos( 0 )
                    , _nBufferBytes( nBufferBytes )
                    , _nAvailableBytes( nBufferBytes )
                    , _pChainedStream( NULL )
                    , _bOwnStream( false )
                    , _bOwnBuffer( bOwnBuffer )
{
    ;
}

_DWFCORE_API
DWFBufferInputStream::DWFBufferInputStream( DWFInputStream* pInputStream,
                                            bool            bOwnStream )
throw()
                    : _pBuffer( NULL )
                    , _iBufferPos( 0 )
                    , _nBufferBytes( 0 )
                    , _nAvailableBytes( 0 )
                    , _pChainedStream( pInputStream )
                    , _bOwnStream( bOwnStream )
                    , _bOwnBuffer( false )
{
}

_DWFCORE_API
DWFBufferInputStream::~DWFBufferInputStream()
throw()
{
    if (_pChainedStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pChainedStream );
    }
    if (_pBuffer && _bOwnBuffer)
    {
		char* pSrc = (char*)_pBuffer;
		DWFCORE_FREE_MEMORY( pSrc );
		_pBuffer = NULL;
    }
}

_DWFCORE_API
size_t
DWFBufferInputStream::available() const
throw( DWFException )
{
    return (_pChainedStream == NULL) ? _nAvailableBytes : _pChainedStream->available();
}

_DWFCORE_API
size_t
DWFBufferInputStream::read( void*  pBuffer,
                            size_t nBytesToRead )
throw( DWFException )
{
    if (_pChainedStream)
    {
        return _pChainedStream->read( pBuffer, nBytesToRead );
    }
    else
    {
        size_t nBytesRead = (nBytesToRead < _nAvailableBytes) ? nBytesToRead : _nAvailableBytes;

        char* pSrc = (char*)_pBuffer + _iBufferPos;

        DWFCORE_COPY_MEMORY( pBuffer, pSrc, nBytesRead );

        _iBufferPos += nBytesRead;

        // available left to read from current position == total, minus position
        _nAvailableBytes = _nBufferBytes - _iBufferPos;

        return nBytesRead;
    }
}

_DWFCORE_API
off_t
DWFBufferInputStream::seek( int     eOrigin,
                            off_t   nOffset )
throw( DWFException )
{
    if (_pChainedStream)
    {
        return _pChainedStream->seek( eOrigin, nOffset );
    }
    else
    {
        off_t iPrevious = (off_t)_iBufferPos;

        if (eOrigin == SEEK_SET)
        {
            _iBufferPos = (nOffset <= 0) ? 0 : nOffset;
        }
        else if (eOrigin == SEEK_CUR)
        {
            _iBufferPos += nOffset;
        }
        else
        {
            _iBufferPos = (nOffset > 0) ? _nBufferBytes : (_nBufferBytes - nOffset);
        }

        // available left to read from current position == total, minus position
        _nAvailableBytes = _nBufferBytes - _iBufferPos;

        return iPrevious;
    }
}

_DWFCORE_API
void
DWFBufferInputStream::chainInputStream( DWFInputStream* pInputStream,
                                        bool            bOwnStream )
throw()
{
    if (_pChainedStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pChainedStream );
    }

    _pChainedStream = pInputStream;
    _bOwnStream = bOwnStream;
}

