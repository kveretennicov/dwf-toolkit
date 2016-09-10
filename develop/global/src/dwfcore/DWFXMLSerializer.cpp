//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#include "dwfcore/DWFXMLSerializer.h"

namespace DWFCore
{

#define _DWFTK_DWFXMLSERIALIZER_WRITE( s )                              \
    _pStream->encode( false );                                          \
    _pStream->writeXML( s, DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(s) );    \
    _pStream->encode( true );

#define _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( s )                    \
    {                                                                   \
        size_t n = s.bytes();                                           \
        if (n > 0)                                                      \
        {                                                               \
            _pStream->writeXML( s, n );                                 \
        }                                                               \
    }


_DWFCORE_API
DWFXMLSerializer::DWFXMLSerializer( DWFUUID& rUUID )
throw()
                : _pStream( NULL )
                , _rUUID( rUUID )
                , _bLastElementOpen( false )
                , _pLastElement( NULL )
                , _zBuffer( NULL )
                , _nBufferSize( 0 )
{
}

_DWFCORE_API
DWFXMLSerializer::~DWFXMLSerializer()
throw()
{
    detach();
    DWFCORE_FREE_MEMORY( _zBuffer );
}

_DWFCORE_API
const DWFString&
DWFXMLSerializer::nextUUID( bool bSquash )
throw()
{
    return _rUUID.next( bSquash );
}

_DWFCORE_API
void
DWFXMLSerializer::attach( DWFOutputStream& rStream )
throw( DWFException )
{
    detach();

    _pStream = DWFCORE_ALLOC_OBJECT( XMLOutputStream(rStream) );

    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffered stream" );
    }
}

_DWFCORE_API
void
DWFXMLSerializer::detach()
throw( DWFException )
{
    if (_pStream)
    {
        _pStream->flush();
        DWFCORE_FREE_OBJECT( _pStream );
    }

    _pStream = NULL;
}

_DWFCORE_API
void
DWFXMLSerializer::emitXMLHeader()
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

    _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"<?xml version=\"1.0\" encoding=\"utf-8\" ?>" );
}


_DWFCORE_API
void
DWFXMLSerializer::startElement( const DWFString& zName, const DWFString& zNamespace )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

        //
        // close off the previously starting element
        //
    if (_bLastElementOpen)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L">" );
    }

    //
    // this string is needed in the case that the element
    // has to be explicitly closed off
    //
    DWFString zFull( zNamespace );
              zFull.append( zName );

    _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"<" );
    _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zFull );

    _oDepthChart.push_back( zFull );
    _pLastElement = &(_oDepthChart.back());

    //
    // will close this element with ">" before opening new one
    //
    _bLastElementOpen = true;
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const DWFString& zValue, const DWFString& zNamespace )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

        //
        // skip it if there is no value string,
        // don't add any extra bytes to the file
        //
    if (zValue.chars() > 0)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L" " );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zNamespace );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zName );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"=\"" );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zValue );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"\"" );
    }
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, int nValue, const DWFString& zNamespace )
throw( DWFException )
{
    addAttribute( zName, &nValue, 1, zNamespace );
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const int* anValues, size_t nCount, const DWFString& zNamespace )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

    if (anValues == NULL || nCount == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No values provided for attribute" );
    }

    if (_nBufferSize == 0)
    {
        //
        // start with a 1024 byte buffer
        //
        _nBufferSize = 1024;
        _zBuffer = DWFCORE_ALLOC_MEMORY(wchar_t, _nBufferSize);
        if (_zBuffer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for double to string conversion" );
        }
    }

    wchar_t* pCurrentPosition = _zBuffer;
    size_t nCurrentBufferSize = _nBufferSize;

    for (size_t i = 0; i < nCount; i++)
    {
        int nCharactersStored;

        //
        // don't add trailing space
        //
        if (i == nCount - 1)
        {
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%d", anValues[i] );
        }
        else
        {
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%d ", anValues[i] );
        }

        //
        // 
        // the return value is -1 if the buffer is not big enough
        // if the return value is same as the buffer size then the null terminator was
        // not stored. In either case, our buffer is not large enough, so double it
        // we'll keep trying until we have a big enough buffer
        //
        while (nCharactersStored == -1 || nCharactersStored == (int)nCurrentBufferSize)
        {
            size_t nNewBufferSize = _nBufferSize * 2;
            wchar_t* zNewBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, nNewBufferSize);
            if (zNewBuffer == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for int to string conversion" );
            }

            //
            // copy values we've converted until now
            //
            memcpy( zNewBuffer, _zBuffer, (_nBufferSize - nCurrentBufferSize) * sizeof( wchar_t ) );
            int nPreviousOffset = (int)(pCurrentPosition - _zBuffer);

            DWFCORE_FREE_MEMORY( _zBuffer );

            _nBufferSize = nNewBufferSize;
            _zBuffer = zNewBuffer;

            pCurrentPosition = _zBuffer + nPreviousOffset;
            nCurrentBufferSize = _nBufferSize - nPreviousOffset;
            if (i == nCount - 1)
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%d", anValues[i] );
            }
            else
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%d ", anValues[i] );
            }
        }

        pCurrentPosition += nCharactersStored;
        nCurrentBufferSize-= nCharactersStored;
    }

    //
    // fix up the decimal separators
    //
    DWFString::RepairDecimalSeparators( _zBuffer );

        //
        // skip it if there is no value string,
        // don't add any extra bytes to the file
        //
    if (DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS(_zBuffer) > 0)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L" " );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zNamespace );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zName );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"=\"" );
        _DWFTK_DWFXMLSERIALIZER_WRITE( _zBuffer );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"\"" );
    }
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const float& fValue, const DWFString& zNamespace )
throw( DWFException )
{
    addAttribute( zName, &fValue, 1, zNamespace );
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const float* afValues, size_t nCount, const DWFString& zNamespace )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

    if (afValues == NULL || nCount == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No values provided for attribute" );
    }

    if (_nBufferSize == 0)
    {
        //
        // start with a 1024 byte buffer
        //
        _nBufferSize = 1024;
        _zBuffer = DWFCORE_ALLOC_MEMORY(wchar_t, _nBufferSize);
        if (_zBuffer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for float to string conversion" );
        }
    }

    wchar_t* pCurrentPosition = _zBuffer;
    size_t nCurrentBufferSize = _nBufferSize;

    for (size_t i = 0; i < nCount; i++)
    {
        int nCharactersStored;

        //
        // Note: Doubles require 17 significant digits, but floats only need at most 9 significant digits. Specifying
        // this lower precision cuts down on useless digits being spewn.
        //
        if (i == nCount - 1)
        {
            //
            // don't add trailing space
            //
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.9g", afValues[i] );
        }
        else
        {
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.9g ", afValues[i] );
        }

        //
        // 
        // the return value is -1 if the buffer is not big enough
        // if the return value is same as the buffer size then the null terminator was
        // not stored. In either case, our buffer is not large enough, so double it
        // we'll keep trying until we have a big enough buffer
        //
        while (nCharactersStored == -1 || nCharactersStored == (int)nCurrentBufferSize)
        {
            size_t nNewBufferSize = _nBufferSize * 2;
            wchar_t* zNewBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, nNewBufferSize);
            if (zNewBuffer == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for double to string conversion" );
            }

            //
            // copy values we've converted until now
            //
            memcpy( zNewBuffer, _zBuffer, (_nBufferSize - nCurrentBufferSize) * sizeof( wchar_t ) );
            int nPreviousOffset = (int)(pCurrentPosition - _zBuffer);

            DWFCORE_FREE_MEMORY( _zBuffer );

            _nBufferSize = nNewBufferSize;
            _zBuffer = zNewBuffer;

            pCurrentPosition = _zBuffer + nPreviousOffset;
            nCurrentBufferSize = _nBufferSize - nPreviousOffset;
            if (i == nCount - 1)
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.9g", afValues[i] );
            }
            else
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.9g ", afValues[i] );
            }
        }

        pCurrentPosition += nCharactersStored;
        nCurrentBufferSize-= nCharactersStored;
    }

    //
    // fix up the decimal separators
    //
    DWFString::RepairDecimalSeparators( _zBuffer );

        //
        // skip it if there is no value string,
        // don't add any extra bytes to the file
        //
    if (DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS(_zBuffer) > 0)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L" " );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zNamespace );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zName );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"=\"" );
        _DWFTK_DWFXMLSERIALIZER_WRITE( _zBuffer );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"\"" );
    }
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const double& dValue, const DWFString& zNamespace )
throw( DWFException )
{
    addAttribute( zName, &dValue, 1, zNamespace );
}

_DWFCORE_API
void
DWFXMLSerializer::addAttribute( const DWFString& zName, const double* anValues, size_t nCount, const DWFString& zNamespace )
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

    if (anValues == NULL || nCount == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No values provided for attribute" );
    }

    if (_nBufferSize == 0)
    {
        //
        // start with a 1024 byte buffer
        //
        _nBufferSize = 1024;
        _zBuffer = DWFCORE_ALLOC_MEMORY(wchar_t, _nBufferSize);
        if (_zBuffer == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for double to string conversion" );
        }
    }

    wchar_t* pCurrentPosition = _zBuffer;
    size_t nCurrentBufferSize = _nBufferSize;

    for (size_t i = 0; i < nCount; i++)
    {
        int nCharactersStored;

        //
        // don't add trailing space
        //
        if (i == nCount - 1)
        {
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.17g", anValues[i] );
        }
        else
        {
            nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.17g ", anValues[i] );
        }

        //
        // 
        // the return value is -1 if the buffer is not big enough
        // if the return value is same as the buffer size then the null terminator was
        // not stored. In either case, our buffer is not large enough, so double it
        // we'll keep trying until we have a big enough buffer
        //
        while (nCharactersStored == -1 || nCharactersStored == (int)nCurrentBufferSize)
        {
            size_t nNewBufferSize = _nBufferSize * 2;
            wchar_t* zNewBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, nNewBufferSize);
            if (zNewBuffer == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for double to string conversion" );
            }

            //
            // copy values we've converted until now
            //
            memcpy( zNewBuffer, _zBuffer, (_nBufferSize - nCurrentBufferSize) * sizeof( wchar_t ) );
            int nPreviousOffset = (int)(pCurrentPosition - _zBuffer);

            DWFCORE_FREE_MEMORY( _zBuffer );

            _nBufferSize = nNewBufferSize;
            _zBuffer = zNewBuffer;

            pCurrentPosition = _zBuffer + nPreviousOffset;
            nCurrentBufferSize = _nBufferSize - nPreviousOffset;
            if (i == nCount - 1)
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.17g", anValues[i] );
            }
            else
            {
                nCharactersStored = _DWFCORE_SWPRINTF( pCurrentPosition, nCurrentBufferSize, /*NOXLATE*/L"%.17g ", anValues[i] );
            }
        }

        pCurrentPosition += nCharactersStored;
        nCurrentBufferSize-= nCharactersStored;
    }

    //
    // fix up the decimal separators
    //
    DWFString::RepairDecimalSeparators( _zBuffer );

        //
        // skip it if there is no value string,
        // don't add any extra bytes to the file
        //
    if (DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS(_zBuffer) > 0)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L" " );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zNamespace );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zName );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"=\"" );
        _DWFTK_DWFXMLSERIALIZER_WRITE( _zBuffer );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"\"" );
    }
}

_DWFCORE_API
 void DWFXMLSerializer::addCData( const DWFString& zCData )
 throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }
    
        //
        // close off the previously starting element
        //
    if (_bLastElementOpen)
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L">" );
        _pLastElement = NULL;

        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( zCData );
        
        _bLastElementOpen = false;
    }
    else 
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No open element" );
    }
}

_DWFCORE_API
 void DWFXMLSerializer::insertXMLStream( DWFInputStream* pInputStream, bool bCloseCurrentElement )
 throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }
    
    if (pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No input stream specified" );
    }

    if (bCloseCurrentElement)
    {
            //
            // close off the previously starting element
            //
        if (_bLastElementOpen)
        {
            _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L">" );
            _bLastElementOpen = false;
        }

        // We won't know what element was last output.
        _pLastElement = NULL;
    }

    //
    // pull the bytes from the given stream, write directly to the output stream
    //
    size_t  nBytesRead = 0;
    char    aBuffer[DWFCORE_XMLSERIALIZER_WRITE_BUFFER_BYTES];
    while (pInputStream->available() > 0)
    {
        nBytesRead = pInputStream->read( aBuffer, DWFCORE_XMLSERIALIZER_WRITE_BUFFER_BYTES );
        _pStream->write( aBuffer, nBytesRead );
    }       
}

_DWFCORE_API
void
DWFXMLSerializer::endElement()
throw( DWFException )
{
    if (_pStream == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No output stream attached" );
    }

        //
        // closing off the same element, use simple bracket
        //
    const DWFString& rElement = _oDepthChart.back();
    if (_pLastElement && (rElement == *_pLastElement))
    {
        _pLastElement = NULL;
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"/>" );
    }
        //
        // otherwise, we need to explicitly declare the element
        //
    else
    {
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L"</" );
        _DWFTK_DWFXMLSERIALIZER_WRITE_DWFSTRING( rElement );
        _DWFTK_DWFXMLSERIALIZER_WRITE( /*NOXLATE*/L">" );
    }

    //
    // remove last element
    //
    _oDepthChart.pop_back();

    //
    // prevents extra ">"
    //
    _bLastElementOpen = false;
}

///
///
///

DWFXMLSerializer::XMLOutputStream::XMLOutputStream( DWFOutputStream& rOutputStream )
throw()
                : DWFBufferOutputStream( &rOutputStream, false, DWFCORE_XMLSERIALIZER_WRITE_BUFFER_BYTES )
                , _bXML( true )
                , _oXMLStream( NULL, false, true )
                , _oUTF8Stream()
{
    ;
}

DWFXMLSerializer::XMLOutputStream::~XMLOutputStream()
throw()
{
    ;
}

size_t
DWFXMLSerializer::XMLOutputStream::writeXML( const void*   pBuffer,
                                             size_t        nBytesToWrite )
throw( DWFException )
{
        //
        // this set up will perform a simple XML special character
        // encoding and then send the result through the UTF-8 encoder
        //
    if (_bXML)
    {
        _oUTF8Stream.attach( this, false );
        _oXMLStream.attach( &_oUTF8Stream, false );

        return _oXMLStream.write( pBuffer, nBytesToWrite );
    }
        //
        // UTF-8 encode only
        //
    else
    {
        _oUTF8Stream.attach( this, false );

        return _oUTF8Stream.write( pBuffer, nBytesToWrite );
    }
}

size_t
DWFXMLSerializer::XMLOutputStream::write( const void*   pBuffer,
                                          size_t        nBytesToWrite )
throw( DWFException )
{
    return DWFBufferOutputStream::write( pBuffer, nBytesToWrite );
}

}
