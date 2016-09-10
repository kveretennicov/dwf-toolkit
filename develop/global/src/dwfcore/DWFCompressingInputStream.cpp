//
//  Copyright (c) 2003-2007 by Autodesk, Inc.
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


#include "dwfcore/DWFCompressingInputStream.h"
using namespace DWFCore;

DWFCompressingInputStream::DWFCompressingInputStream( DWFInputStream* pInputStream, DWFZipFileDescriptor::teFileMode fileMode, 
													  bool bOwnStream)
throw()
		:DWFInputStream()
		, _pInputStream( pInputStream )
		, _bOwnStream( bOwnStream )
		, _bCompressionStreamInit ( false )
		, _bCompressionStreamFinished (false )
		, _pSourceBuffer ( NULL )
		, _nSourceBufferBytes( 0 )
		, _nCompressionBytes ( 0 )
		, _nCompressionBufferSize ( 0 )
		, _nCompressionBufferOffset ( 0 )
		, _nCompressedBytesBuffered ( 0 )
		, _pCompressionBuffer ( NULL )
		, _bPending ( false )
		, _oFileMode ( fileMode )
{
	_oCompressionStream.zalloc = NULL;
	_oCompressionStream.zfree = NULL;
	_oCompressionStream.opaque = NULL;

	_oCompressionStream.next_in = NULL;
	_oCompressionStream.next_out = NULL;

	_oCompressionStream.avail_in = 0;
	_oCompressionStream.avail_out = 0;
	if( _pInputStream )
	{
		_bCompressionStreamInit = (deflateInit2(&_oCompressionStream, 
					Z_DEFAULT_COMPRESSION,	// The compression level (0 to 9 with 6 as standard default)
					Z_DEFLATED,             // Only Z_DEFLATED is supported in ZLIB 1.1.3
					-MAX_WBITS,				// (size of the history buffer) (8 to 15 max) 15 gives 32K history.
					MAX_MEM_LEVEL,			// Amount of memory to use to speed compression (max is 9, default is 8).
					Z_DEFAULT_STRATEGY) == Z_OK);
	}
}

DWFCompressingInputStream::~DWFCompressingInputStream()
throw()
{
	if (_bCompressionStreamInit)
    {
        deflateEnd( &_oCompressionStream );
    }

    if (_pCompressionBuffer)
    {
        DWFCORE_FREE_MEMORY( _pCompressionBuffer );
    }

    if (_pSourceBuffer)
    {
        ::free( _pSourceBuffer );
    }

    if (_pInputStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pInputStream );
    }
}

size_t DWFCompressingInputStream::available() const
throw( DWFException )
{
	if (_bCompressionStreamInit == false)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Compression stream failed to initialize" );
    }

        //
        // next read can get this many pre-inflated bytes
        //
    if (_nCompressedBytesBuffered > 0)
    {
        return _nCompressedBytesBuffered;
    }
        //
        // source stream and compression buffer fully exhausted
        //
    else if (_bCompressionStreamFinished)
    {
        return 0;
    }
        //
        // this will reflect the number of decompressed bytes remaining
        //
    else
    {
            //
            // if there are more bytes left to decompress in the zlib stream
            // return that number before resorting to the underlying file.
            // this is actually fairly important since for some smaller files
            // (or highly compressed ones) with big read buffers, the underlying
            // file bytes will be consumed before the zlib stream clears.
            //
        if (_bPending && (_oCompressionStream.avail_in > 0))
        {
            return _oCompressionStream.avail_in;
        }

            //
            // anything left in the source file?
            //
        size_t nAvailable = _pInputStream->available();
        if (nAvailable == 0)
        {
            //
            // we can get here if the source has been depleted,
            // the zlib stream has bytes remaining BUT is basically
            // finished and the next call will complete the decompression.
            // at this point, we really have no idea how much is left
            // but it's safe to assume it must be less than
            // DWFCORE_COMPRESSING_INPUT_STREAM_COMPRESSION_BUFFER_SIZE
            // so we return this, so the process can finish.
            //
            return DWFCORE_COMPRESSING_INPUT_STREAM_COMPRESSION_BUFFER_SIZE;
        }

        return nAvailable;
    }
}

size_t DWFCompressingInputStream::read( void *pBuffer, 
									   size_t nBytesToRead )
throw( DWFException )
{
	if ( _bCompressionStreamInit == false )
	{
		_DWFCORE_THROW ( DWFIOException, /*NOXLATE*/L"The compression stream failed to initialize" );
	}
	if( _nCompressedBytesBuffered > 0 )
	{
		size_t nBytesToFlush = min ( nBytesToRead, _nCompressedBytesBuffered );
		DWFCORE_COPY_MEMORY( pBuffer, (_pCompressionBuffer + _nCompressionBufferOffset), nBytesToFlush );
		
		_nCompressedBytesBuffered -= nBytesToFlush;
		_nCompressionBufferOffset += nBytesToFlush;

		return nBytesToFlush;
	}

	if(!_bPending)
	{
		//
		// choose an appropriate size for our read buffer
		//
		size_t nDesiredBufferBytes = _nSourceBufferBytes;
		if(_nSourceBufferBytes < nBytesToRead)
		{
			//
			// allocate twice the size
			//

			nDesiredBufferBytes = 2 * _nSourceBufferBytes;

			//
			// make the size atleast 1024
			//

			nDesiredBufferBytes = max(nDesiredBufferBytes, (size_t)1024);

			//
			// it should also atleast be the number of bytes asked for
			//

			nDesiredBufferBytes = max(nDesiredBufferBytes, nBytesToRead);
		}

		if(_pSourceBuffer == NULL)
		{
			_pSourceBuffer = (unsigned char*) malloc(nDesiredBufferBytes);

			if(_pSourceBuffer == NULL)
			{
				_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate input buffer" );
			}

			_nSourceBufferBytes = nDesiredBufferBytes;
		}
		
		//
		// read the uncompressed bytes from the source stream
		//
		
		_oCompressionStream.avail_in = (uInt)_pInputStream->read( _pSourceBuffer, _nSourceBufferBytes );
		_oCompressionStream.next_in = _pSourceBuffer;
	}

	_oCompressionStream.next_out = (Bytef*)pBuffer;
	_oCompressionStream.avail_out = (uInt)nBytesToRead;
	//
	// if there are no input bytes then there is nothing to deflate (compress)
	//
	if( (_oCompressionStream.avail_in == 0) && (_bPending == false))
	{
		_nCompressionBytes = 0;
		_bCompressionStreamFinished = true;
		return _nCompressionBytes;
	}

	// 
	// perform compression
	//

	int eResult = deflate( &_oCompressionStream, Z_SYNC_FLUSH );

	switch(eResult)
	{
        case Z_STREAM_END:
        case Z_FINISH:
        {
            _bCompressionStreamFinished = true;

            //
            // no break - fall through
            //
        }

        case Z_OK:
        {
            //
            // avail_out is modified during deflate() to reflect the number of
            // bytes remaining in the output buffer that we provided.
            //
            _nCompressionBytes = (nBytesToRead - _oCompressionStream.avail_out);

            //
            // are there compressed bytes leftover that couldn't
            // fit into the caller's buffer? if so, cache them.
            //
            if ((eResult == Z_OK) && (_oCompressionStream.avail_out == 0))
            {
                if (_pCompressionBuffer == NULL)
                {
                    _pCompressionBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, DWFCORE_COMPRESSING_INPUT_STREAM_COMPRESSION_BUFFER_SIZE );

                    if (_pCompressionBuffer == NULL)
                    {
                        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
                    }
                }

                //
                // capture whatever we can
                //
                _oCompressionStream.next_out = (Bytef*)_pCompressionBuffer;
                _oCompressionStream.avail_out = DWFCORE_COMPRESSING_INPUT_STREAM_COMPRESSION_BUFFER_SIZE;

                eResult = deflate( &_oCompressionStream, Z_SYNC_FLUSH );

                    //
                    // did this clear the source?
                    //
                if ((eResult == Z_STREAM_END) ||
                    (eResult == Z_FINISH))
                {
                    _bCompressionStreamFinished = true;
                }
                else
                {
                        //
                        // still more? we have to wait until these bytes are consumed
                        // then we continue inflating; only then do we read again from the stream
                        //
                    _bPending = (_oCompressionStream.avail_out == 0);
                }

                _nCompressionBufferOffset = 0;
                _nCompressedBytesBuffered = (DWFCORE_COMPRESSING_INPUT_STREAM_COMPRESSION_BUFFER_SIZE - _oCompressionStream.avail_out);
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
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"ZLIB data error occured" );
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

    //
    // return the number of bytes that zlib filled into the calling buffer.
    //
    return _nCompressionBytes;
	
}

off_t DWFCompressingInputStream::seek( int /*eOrigin*/, 
									   off_t /*nOffset*/)
throw( DWFException )
{
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Seeking not permitted on this stream" );
}


