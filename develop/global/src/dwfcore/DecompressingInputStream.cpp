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


#include "dwfcore/DecompressingInputStream.h"
using namespace DWFCore;



DWFDecompressingInputStream::DWFDecompressingInputStream( DWFInputStream* pInputStream,
                                                          bool            bOwnStream )
throw()
        : DWFInputStream()
        , _pSourceStream( pInputStream )
        , _bOwnStream( bOwnStream )
        , _bDecompressionStreamInit( false )
        , _nSourceBufferBytes( 0 )
        , _pSourceBuffer( NULL )
        , _nDecompressedBytes( 0 )
        , _nDecompressionBufferSize( 0 )
        , _nDecompressionBufferOffset( 0 )
        , _nDecompressedBytesBuffered( 0 )
        , _pDecompressionBuffer( NULL )
        , _bPending( false )
{
    _oDecompressionStream.zalloc = NULL;
    _oDecompressionStream.zfree = NULL;
    _oDecompressionStream.opaque = NULL;

    _oDecompressionStream.next_in = NULL;
    _oDecompressionStream.next_out = NULL;

    _oDecompressionStream.avail_in = 0;
    _oDecompressionStream.avail_out = 0;

    if (_pSourceStream)
    {
        _bDecompressionStreamInit = (inflateInit2(&_oDecompressionStream, -MAX_WBITS) == Z_OK);
    }
}

DWFDecompressingInputStream::~DWFDecompressingInputStream()
throw()
{
    if (_bDecompressionStreamInit)
    {
        inflateEnd( &_oDecompressionStream );
    }

    if (_pDecompressionBuffer)
    {
        DWFCORE_FREE_MEMORY( _pDecompressionBuffer );
    }

    if (_pSourceBuffer)
    {
        ::free( _pSourceBuffer );
    }

    if (_pSourceStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pSourceStream );
    }
}

size_t
DWFDecompressingInputStream::available() const
throw( DWFException )
{
    if (_bDecompressionStreamInit == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Decompression stream failed to initialize" );
    }

    //
    // first check if there are pre-inflated bytes
    //
    if (_nDecompressedBytesBuffered > 0)
    {
        return _nDecompressedBytesBuffered;
    }

    //
    // if there are more bytes left to decompress in the zlib stream,
    // return that number before resorting to the underlying input stream.
    //
    if (_bPending && (_oDecompressionStream.avail_in > 0))
    {
        return _oDecompressionStream.avail_in;
    }
    
    //
    // anything left in the source input stream?
    //
    size_t nAvailable = _pSourceStream->available();

    return nAvailable;
}

size_t
DWFDecompressingInputStream::read( void*    pBuffer,
                                   size_t   nBytesToRead )
throw( DWFException )
{
    if (_bDecompressionStreamInit == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Decompression stream failed to initialize" );
    }

    _nDecompressedBytes = 0;

    //
    // Flush the existing buffered bytes before decompressing new bytes. 
    //
    if (_nDecompressedBytesBuffered > 0)
    {
        size_t nBytesToFlush = min( nBytesToRead, _nDecompressedBytesBuffered );

        DWFCORE_COPY_MEMORY( pBuffer, (_pDecompressionBuffer + _nDecompressionBufferOffset), nBytesToFlush );

        _nDecompressedBytesBuffered -= nBytesToFlush;
        _nDecompressionBufferOffset += nBytesToFlush;

        _nDecompressedBytes = nBytesToFlush;
    }

    //
    // If the number of the bytes in the buffer is less then "nBytesToRead", we'll continue to 
    // inflate compressed bytes until we get "nBytesToRead" decompressed bytes or all the 
    // underlying input stream is inflated.
    //
    while (_nDecompressedBytes < nBytesToRead)
    {
        if (_bPending == false)
        {
            //
            // Choose a size for our new buffer
            // We wish to reasonably maximize read buffer size, and minimize reallocs.
            //
            size_t nDesiredBufferBytes = _nSourceBufferBytes;
            if (_nSourceBufferBytes < nBytesToRead)
            {
                //
                // If we'd previously allocated a buffer, but we need
                // more space, the new buffer should be at least twice
                // as large as the previous one...
                //
                nDesiredBufferBytes = 2 * _nSourceBufferBytes;

                //
                // And its size should be at least 1024. Don't mess
                // with too small a buffer, it just causes trouble
                // for the decompressor...
                //
                nDesiredBufferBytes = max( nDesiredBufferBytes, (size_t)1024 );

                //
                // It should also be at least as large as the number 
                // of *decompressed* bytes we were asked for...
                //
                //      [This condition is debatable.]
                //
                nDesiredBufferBytes = max( nDesiredBufferBytes, nBytesToRead );

            }

            //
            // resize existing buffer?
            //
            if (_pSourceBuffer && (nDesiredBufferBytes > _nSourceBufferBytes))
            {
                void* pTemp = ::realloc( _pSourceBuffer, nDesiredBufferBytes );

                if (pTemp)
                {
                    _pSourceBuffer = (unsigned char*)pTemp;
                    _nSourceBufferBytes = nDesiredBufferBytes;
                }
            }

            //
            // need new buffer?
            //
            if (_pSourceBuffer == NULL)
            {
                _pSourceBuffer = (unsigned char*)::malloc( nDesiredBufferBytes );

                if (_pSourceBuffer == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate input buffer" );
                }

                _nSourceBufferBytes = nDesiredBufferBytes;
            }

            //
            // Read compressed bytes from underlying source stream.
            //
            // Note: Attempt to fill the buffer. Don't just read the size
            // that our caller requested. They might be asking for an amount
            // that's too small to decompress. In this case, even if they
            // request repeatedly, it won't help.
            //
            _oDecompressionStream.avail_in = (uInt)_pSourceStream->read( _pSourceBuffer, _nSourceBufferBytes /* was: nBytesToRead */ );

            //
            // If there are no more compressed bytes to inflate, 
            // then return the bytes that have been decompressed already
            //
            if (_oDecompressionStream.avail_in == 0)
            {
                return _nDecompressedBytes;
            }
            _oDecompressionStream.next_in = _pSourceBuffer;
        }

        //
        // decompress directly to the caller's buffer
        //
        _oDecompressionStream.next_out = (Bytef*)pBuffer + _nDecompressedBytes;
        _oDecompressionStream.avail_out = (uInt)nBytesToRead - (uInt)_nDecompressedBytes;

        // perform decompression
        int eResult = inflate( &_oDecompressionStream, Z_SYNC_FLUSH );

        switch (eResult)
        {
        case Z_STREAM_END:
        case Z_FINISH:
        case Z_OK:
            {
                size_t nLastDecompressedBytes = _nDecompressedBytes;
                _nDecompressedBytes = nBytesToRead - _oDecompressionStream.avail_out;

                //
                // are there decompressed bytes leftover that couldn't
                // fit into the caller's buffer? if so, cache them.
                //
                // Note: "_oDecompressionStream.avail_out == 0" dosen't indicate
                //       there are more compressed bytes in _oDecompressionStream to inflate.
                //       And if "_oDecompressionStream.avail_in == 0", then the next invoking
                //       of inflate() function will return Z_BUF_ERROR!
                //
                // Only "_oDecompressionStream.avail_in > 0" is not enouth to indicate that there are more compressed bytes in _oDecompressionStream to inflate.
                // it still need the "_oDecompressionStream.avail_out == 0" to judge if we need to call inflate(), even sometime invoking of inflate() function will return Z_BUF_ERROR!
                // that's acceptable, for get all the data.
                //
                if ((eResult == Z_OK) && (_oDecompressionStream.avail_out == 0))
                {
                    //
                    // DWFCORE_DECOMPRESSING_INPUT_STREAM_DECOMPRESSION_BUFFER_SIZE is not enough when user set a large size buff to get the data (for performance),
                    // so it is better to allocate more size for the _pDecompressionBuffer;
                    //
                    size_t nMaxBuffSize = _oDecompressionStream.avail_in + nLastDecompressedBytes;
                    //
                    // If we'd previously allocated a buffer, but we need
                    // more space, the new buffer should be at least twice
                    // as large as the previous one...
                    //
                    nMaxBuffSize *= 2; 
                    nMaxBuffSize = max(nMaxBuffSize, (size_t)DWFCORE_DECOMPRESSING_INPUT_STREAM_DECOMPRESSION_BUFFER_SIZE);
                    if (_pDecompressionBuffer == NULL)
                    {
                        _pDecompressionBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, nMaxBuffSize );
                    }
                    else
                    {
                        void* pTemp = ::realloc( _pDecompressionBuffer, nMaxBuffSize );
                        if (pTemp)
                        {
                            _pDecompressionBuffer = (unsigned char*)pTemp;
                        }
                    }
                    if (_pDecompressionBuffer == NULL)
                    {
                        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
                    }

                    //
                    // capture whatever we can
                    //
                    _oDecompressionStream.next_out = (Bytef*)_pDecompressionBuffer;
                    _oDecompressionStream.avail_out = (uInt)nMaxBuffSize;

                    eResult = inflate( &_oDecompressionStream, Z_SYNC_FLUSH );

                    if ((eResult == Z_STREAM_END) ||
                        (eResult == Z_FINISH)     ||
                        (eResult == Z_OK))
                    {
                        //
                        // still more? we have to wait until these bytes are consumed
                        // then we continue inflating; only then do we read again from the stream
                        //
                        _bPending = (_oDecompressionStream.avail_in > 0);
                        _nDecompressionBufferOffset = 0;
                        _nDecompressedBytesBuffered = (nMaxBuffSize - _oDecompressionStream.avail_out);

                    }
                    //
                    // when "_oDecompressionStream.avail_out == 0" it force to invoke inflate(), but sometimes there is no uncompressed stream to inflate, it will
                    // return Z_BUF_ERROR for this condition.
                    // But it is hard to judge whether there is uncompressed stream or not; and to get the date completely, it should call inflate() when "avail_out == 0"
                    // So it cannot avoid the Z_BUF_ERROR, and it cannot throw exception when Z_BUF_ERROR.
                    //
                    else if(eResult == Z_BUF_ERROR)
                    {
                        _bPending = false;
                    }
                    else
                    {
                        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Unexpected ZLIB error occured during inflation!" );
                    }
                }
                else
                {
                    _bPending = false;
                }
                break;
            }
        case Z_STREAM_ERROR:
            {
                _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"ZLIB stream error occured" );
            }

        case Z_DATA_ERROR:
            {
                _DWFCORE_THROW( DWFZlibDataException, /*NOXLATE*/L"ZLIB data error occured" );
            }

        case Z_MEM_ERROR:
            {
                _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"ZLIB memory error occured" );
            }

        case Z_BUF_ERROR:
            {
                _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"ZLIB buffer error occured" );
            }

        default:
            {
                _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Unexpected ZLIB error occured" );
            }
        }
    }

    //
    // return the number of bytes that zlib filled into the calling buffer.
    //
    return _nDecompressedBytes;
}

off_t
DWFDecompressingInputStream::seek( int      /*eOrigin*/,
                                   off_t    /*nOffset*/ )
throw( DWFException )
{
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Seeking not permitted on this stream" );
}
