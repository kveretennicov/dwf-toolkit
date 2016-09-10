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


#include "dwfcore/String.h"
#include "dwfcore/XMLEncodingOutputStream.h"
using namespace DWFCore;

_DWFCORE_API
DWFXMLEncodingOutputStream::DWFXMLEncodingOutputStream( DWFOutputStream* pOutputStream,
                                                        bool             bOwnStream,
                                                        bool             bAlwaysEncodeWideStrings )
throw()
                           : _pOutputStream( pOutputStream )
                           , _bOwnStream( bOwnStream )
                           , _bAlwaysEncodeWideStrings( bAlwaysEncodeWideStrings )
                           , _nBytesEncoded( 0 )
                           , _pBuffer( NULL )
                           , _nBufferBytes( 0 )
{
    ;
}

_DWFCORE_API
DWFXMLEncodingOutputStream::~DWFXMLEncodingOutputStream()
throw()
{
    detach();

    if (_pBuffer)
    {
        ::free( _pBuffer );
    }
}

_DWFCORE_API
void
DWFXMLEncodingOutputStream::flush()
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No stream to flush" );
    }

    _pOutputStream->flush();
}

_DWFCORE_API
size_t
DWFXMLEncodingOutputStream::write( const void*   pBuffer,
                                   size_t        nBytesToWrite )
throw( DWFException )
{
    if (_pOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No stream to write to" );
    }

    size_t nEncodedBytes = 0;

    if (_bAlwaysEncodeWideStrings)
    {
        nEncodedBytes = DWFString::EncodeXML( (const wchar_t*)pBuffer,
                                               nBytesToWrite,
                                               NULL, 0 );

        nEncodedBytes += sizeof( wchar_t );
    }
    else
    {
        nEncodedBytes = DWFString::EncodeXML( (const ASCII_char_t*)pBuffer,
                                               nBytesToWrite,
                                               NULL, 0 );

        nEncodedBytes += sizeof( ASCII_char_t );
    }

    if (nEncodedBytes > _nBufferBytes)
    {
        if (_pBuffer)
        {
            if (nEncodedBytes < 2*_nBufferBytes)
            {
                nEncodedBytes = 2*_nBufferBytes;
            }

            void *pTemp = ::realloc( _pBuffer, nEncodedBytes );
            if (pTemp)
            {
                _pBuffer = (char*)pTemp;
                _nBufferBytes = nEncodedBytes;
            }
            else
            {
                ::free( _pBuffer );
                _pBuffer = NULL;
                _nBufferBytes = 0;
            }
        }
        
        if (_pBuffer == NULL)
        {
            _pBuffer = (char*)::malloc( nEncodedBytes );
            if (_pBuffer == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
            }

            _nBufferBytes = nEncodedBytes;
        }
    }

    if (_bAlwaysEncodeWideStrings)
    {
        nEncodedBytes = DWFString::EncodeXML( (const wchar_t*)pBuffer,
                                               nBytesToWrite,
                                              (wchar_t*)_pBuffer,
                                              _nBufferBytes );
    }
    else
    {
        nEncodedBytes = DWFString::EncodeXML( (const ASCII_char_t*)pBuffer,
                                               nBytesToWrite,
                                              _pBuffer,
                                              _nBufferBytes );
    }

    _nBytesEncoded += nEncodedBytes;

    return _pOutputStream->write( _pBuffer, nEncodedBytes );
}

_DWFCORE_API
size_t
DWFXMLEncodingOutputStream::bytes() const
throw()
{
    return _nBytesEncoded;
}

_DWFCORE_API
void
DWFXMLEncodingOutputStream::attach( DWFOutputStream* pOutputStream,
                                    bool             bOwnStream )
throw()
{
    detach();

    _pOutputStream = pOutputStream;
    _bOwnStream = bOwnStream;
}

_DWFCORE_API
void
DWFXMLEncodingOutputStream::detach()
throw()
{
    if (_pOutputStream)
    {
        if (_bOwnStream)
        {
            DWFCORE_FREE_OBJECT( _pOutputStream );
            
            _bOwnStream = false;
        }

        _pOutputStream = NULL;
        _nBytesEncoded = 0;
    }
}
