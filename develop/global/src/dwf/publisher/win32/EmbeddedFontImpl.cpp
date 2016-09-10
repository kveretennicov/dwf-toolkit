//
//  Copyright (c) 2000-2006 by Autodesk, Inc.
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


#include "dwf/publisher/win32/EmbeddedFontImpl.h"
using namespace DWFToolkit;


#ifdef  _DWFCORE_WIN32_SYSTEM


#include <gdiplus.h>


HINSTANCE               DWFEmbeddedFontImpl_Win32::_khLib                   = NULL;
int                     DWFEmbeddedFontImpl_Win32::_knLibRef                = 0;
fpTTEmbedFont           DWFEmbeddedFontImpl_Win32::_kfpTTEmbedFont          = NULL;
fpTTLoadEmbeddedFont    DWFEmbeddedFontImpl_Win32::_kfpTTLoadEmbeddedFont   = NULL;
fpTTDeleteEmbeddedFont  DWFEmbeddedFontImpl_Win32::_kfpTTDeleteEmbeddedFont = NULL;


#ifdef DWFTK_STATIC

const char*             DWFEmbeddedFontImpl_Win32::_kz_T2Embed              = /*NOXLATE*/"t2embed.dll";

#endif


#ifndef DWFTK_READ_ONLY

_DWFTK_API
DWFEmbeddedFontImpl_Win32::DWFEmbeddedFontImpl_Win32( HFONT     hFont,
                                                      size_t    nHeapLimit)
throw()
                         : DWFEmbeddedFontImpl()
                         , _nHeapLimit( nHeapLimit )
                         , _hFont( hFont )
                         , _pFontFile( NULL )
                         , _pFontOutputStream( NULL )
                         , _nCodePoints( 0 )
                         , _pSparseCharacters( NULL )
                         , _pFontResource( NULL )
                         , _pFontInputStream( NULL )
                         , _pReadAheadStream( NULL )
                         , _hLoadedFont( NULL )
                         , _nPriv( EMBED_INSTALLABLE )
                         , _nRequest( TTEMBED_SUBSET | TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA )
                         , _nCharacterCode( CHARSET_UNICODE )
{
    DWFCORE_ZERO_MEMORY( _abCodePoints, 65536*sizeof(bool) );
    _loadFontLibrary();
}

#endif

_DWFTK_API
DWFEmbeddedFontImpl_Win32::DWFEmbeddedFontImpl_Win32( DWFFontResource& rFontResource )
throw()
                         : DWFEmbeddedFontImpl()
#ifndef DWFTK_READ_ONLY
                         , _nHeapLimit( 0 )
                         , _hFont( NULL )
                         , _pFontFile( NULL )
                         , _nCodePoints( 0 )
                         , _pSparseCharacters( NULL )
#endif
                         , _pFontOutputStream( NULL )
                         , _pFontResource( &rFontResource )
                         , _pFontInputStream( NULL )
                         , _pReadAheadStream( NULL )
                         , _hLoadedFont( NULL )
                         , _nPriv( EMBED_INSTALLABLE )
                         , _nRequest( TTEMBED_SUBSET | TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA )
                         , _nCharacterCode( CHARSET_UNICODE )
{
    _loadFontLibrary();
}

_DWFTK_API
DWFEmbeddedFontImpl_Win32::~DWFEmbeddedFontImpl_Win32()
throw()
{

#ifndef DWFTK_READ_ONLY

    if (_hFont)
    {
        ::DeleteObject( _hFont );
    }

    if (_pFontFile)
    {
        DWFCORE_FREE_OBJECT( _pFontFile );
    }

    if (_pSparseCharacters)
    {
        DWFCORE_FREE_MEMORY( _pSparseCharacters );
    }

#endif

    if (_pFontInputStream)
    {
        DWFCORE_FREE_OBJECT( _pFontInputStream );
    }

    if (_pReadAheadStream)
    {
        DWFCORE_FREE_OBJECT( _pReadAheadStream );
    }

    if (_pFontOutputStream)
    {
        DWFCORE_FREE_OBJECT( _pFontOutputStream );
    }

        //
        // decrement module ref count and unload if necessary
        //
    if (--_knLibRef <= 0)
    {
        ::FreeLibrary( _khLib );
        _khLib = NULL;
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFEmbeddedFontImpl_Win32::addCharacters( const DWFString& rCharacters )
throw( DWFException )
{
    const wchar_t* pChar = rCharacters;

        //
        // store unique code points
        //
    unsigned int iChar = 0;
    while (*pChar != 0)
    {
        iChar = (unsigned int)(*pChar);
        if (_abCodePoints[iChar] == false)
        {
            _abCodePoints[iChar] = true;
            _nCodePoints++;
        }

        pChar++;
    }
}

_DWFTK_API
void
DWFEmbeddedFontImpl_Win32::embed()
throw( DWFException )
{
    //DNT_Start

        //
        // first request, create the embedded font
        //
    if ((_pFontOutputStream == NULL) && (_pFontFile == NULL))
    {
        if (_nCodePoints == 0)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No data" );
        }

        //
        // build a sparse array of character mappings
        //
        _pSparseCharacters = DWFCORE_ALLOC_MEMORY( unsigned short, _nCodePoints );
        if (_pSparseCharacters == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate sparse character array" );
        }

        DWFCORE_ZERO_MEMORY( _pSparseCharacters, _nCodePoints*sizeof(unsigned short) );

        //
        // determine indices
        //
        unsigned int iPoint = 0; // needs to be big enough to hold 0xffff + 1
        unsigned int nPoints = 0;

            //
            // first check the codepoints up to 0x300
            //
        while ((nPoints < _nCodePoints) && (iPoint < 0x300))
        {
            if (_abCodePoints[iPoint])
            {
                _pSparseCharacters[nPoints++] = (unsigned short) iPoint;
            }

            iPoint++;
        }

            //
            // then check the remaining codepoints used by cp1252
            //
        if (nPoints < _nCodePoints) 
        {
            //
            // now check the range from 0x2000-0x21ff
            //
            iPoint = 0x2000;
            while ((nPoints < _nCodePoints) && (iPoint < 0x2200))
            {
                if (_abCodePoints[iPoint])
                {
                    _pSparseCharacters[nPoints++] = (unsigned short) iPoint;
                }

                iPoint++;
            }
        }

        if (nPoints < _nCodePoints) 
        {
            //
            // now check the range from 0x300-0x1fff
            //
            iPoint = 0x300;
            while ((nPoints < _nCodePoints) && (iPoint < 0x2000)) 
            {
                if (_abCodePoints[iPoint])
                {
                    _pSparseCharacters[nPoints++] = (unsigned short) iPoint;
                }

                iPoint++;
            }
        }

        if (nPoints < _nCodePoints) 
        {
            //
            // now check the remaining range
            //
            iPoint = 0x2200;
            while ((nPoints < _nCodePoints) && (iPoint < 0x10000)) 
            {
                if (_abCodePoints[iPoint])
                {
                    _pSparseCharacters[nPoints++] = (unsigned short) iPoint;
                }

                iPoint++;
            }
        }

        //
        // clean this up here since we are done with it
        //
        DWFCORE_ZERO_MEMORY( _abCodePoints, 65536*sizeof(bool) );

        //
        // need a DC for the font and object
        //
        HDC hDC = ::GetDC( NULL );
        if (hDC == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to acquire a NULL DC" );
        }

        //
        // need a GDI object 
        //
        HGDIOBJ hObj = ::SelectObject( hDC, _hFont );
        if (hObj == NULL)
        {
            ::ReleaseDC( NULL, hDC );
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to select font into DC" );
        }

        //
        // determine embedding status
        //
        unsigned int nMetricSize = ::GetOutlineTextMetrics( hDC, 0, NULL );
        if (nMetricSize) 
        {
            DWFPointer<OUTLINETEXTMETRIC> apOTM( DWFCORE_ALLOC_MEMORY(OUTLINETEXTMETRIC, sizeof(OUTLINETEXTMETRIC)), true );

            if (apOTM.isNull())
            {
                //
                // clean up
                //
                ::SelectObject( hDC, hObj );
                ::ReleaseDC( NULL, hDC );
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer for metrics" );
            }
            
            nMetricSize = ::GetOutlineTextMetrics( hDC, nMetricSize, apOTM );
            if (nMetricSize)
            {
                //otmfsType: Specifies whether the font is licensed. Licensed fonts must not be 
                //           modified or exchanged.  If bit 1 is set, the font may not be embedded 
                //           in a document. If bit 1 is clear, the font can be embedded. If bit 2 
                //           is set, the embedding is read-only. 

                if (apOTM->otmfsType & 0x01)
                {
                    //
                    // clean up
                    //
                    ::SelectObject( hDC, hObj );
                    ::ReleaseDC( NULL, hDC );
                    _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"No license to embed font" );
                }
            }
        }

        //
        // Create the embedded font by using the MS API to serialize
        // into a heap buffer or temporary file.
        //
        _serializeFont( hDC );

        //
        // clean up
        //
        ::SelectObject( hDC, hObj );
        ::ReleaseDC( NULL, hDC );
    }
    //DNT_End
}

_DWFTK_API
DWFInputStream*
DWFEmbeddedFontImpl_Win32::getInputStream()
throw( DWFException )
{
    //
    // be sure the data is available
    //
    embed();

        //
        // return an input stream directly from the temp file
        //
    if (_pFontFile)
    {
        return _pFontFile->getInputStream();
    }
        //
        // create a buffer input stream for the caller
        //
    else
    {
        return DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pFontOutputStream->buffer(),
                                                          _pFontOutputStream->bytes()) );

    }
}

void
DWFEmbeddedFontImpl_Win32::_serializeFont( HDC hDC )
throw( DWFException )
{
    if ((_pSparseCharacters == NULL) || (_nCodePoints == 0))
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No data" );
    }

    //
    // Create an auto-sizing buffer output stream to receive the feed from MS DLL
    //
    _pFontOutputStream = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream(DWFTK_EMBEDDED_FONT_HEAP_INIT) );
    if (_pFontOutputStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
    }

    _nPriv = EMBED_INSTALLABLE;

    //
    // At this time the 'compress' option will produce an access violation on Win64
    // so we're turning this off for now.  (A defect report has been submitted to MS)
    //
#ifndef _DWFCORE_X64_SYSTEM
    _nRequest = TTEMBED_SUBSET | TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA;
#else
    _nRequest = TTEMBED_SUBSET | TTEMBED_ENCRYPTDATA;
#endif
    _nCharacterCode = CHARSET_UNICODE;
    
    unsigned long nStatus = 0;

        //
        // first pass:
        //  Font:   Subset
        //  Priv:   Max Embeddable
        //
    if ((*_kfpTTEmbedFont)( hDC,
                           _nRequest,
                            CHARSET_UNICODE,
                          &_nPriv,
                           &nStatus,
                           _WriteFontBuffer, 
                           (void*)_pFontOutputStream,
                           _pSparseCharacters, 
                           _nCodePoints,
                            0, NULL))
    {
            //
            // second pass:
            //  Font:   Subset
            //  Priv:   Less Embeddable
            //
            //  We may have failed to subset with maximum embeddable privilege.
            //  So now, let use try to subset with less embeddable privilege.
            //  _nPriv from the previous would hold the maximum permissible
            //  embeddable privilege value, so let us try to make use of it.
            //
        _nPriv = EMBED_PREVIEWPRINT;
        if ((*_kfpTTEmbedFont)( hDC,
                               _nRequest,
                                CHARSET_UNICODE,
                              &_nPriv,
                               &nStatus,
                               _WriteFontBuffer, 
                               (void*)_pFontOutputStream,
                               _pSparseCharacters, 
                               _nCodePoints,
                                0, NULL))
        {
                //
                // third pass:
                //  Font:   Full
                //  Priv:   Max Embeddable
                //
                // We may still have failed because, the embedding sdk couldn't subset
                // the font for some reason. So, now let us try embedding the entire font
                // information with maximum privilege.
                //
            _nPriv = EMBED_INSTALLABLE;
#ifndef _DWFCORE_X64_SYSTEM
            _nRequest = TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA;
#else
            _nRequest = TTEMBED_ENCRYPTDATA;
#endif
            if ((*_kfpTTEmbedFont)( hDC,
                                   _nRequest,
                                    CHARSET_UNICODE,
                                  &_nPriv,
                                   &nStatus,
                                  &_WriteFontBuffer, 
                                   (void*)_pFontOutputStream,
                                   _pSparseCharacters, 
                                   _nCodePoints,
                                    0, NULL))
            {
                    //
                    // final pass:
                    //  Font:   Full
                    //  Priv:   Less Embeddable
                    //
                    // We may have failed to embed the entire font with maximum embeddability
                    // privilege. So, let us try embedding the entire font with least
                    // embeddability privilege.
                    // _nPriv from the previous would hold the maximum permissible
                    // embeddable privilege value, so let us try to make use of it.
                    //
                _nPriv = EMBED_PREVIEWPRINT;
                if ((*_kfpTTEmbedFont)( hDC,
                                       _nRequest,
                                        CHARSET_UNICODE,
                                      &_nPriv,
                                       &nStatus,
                                      &_WriteFontBuffer, 
                                       (void*)_pFontOutputStream,
                                       _pSparseCharacters, 
                                       _nCodePoints,
                                        0, NULL))
                {
                    DWFCORE_FREE_OBJECT( _pFontOutputStream );
                    DWFCORE_FREE_MEMORY( _pSparseCharacters );
                    _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"All attempts to embed the font failed" );
                }
            }
        }
    }

        //
        // check the buffer stream to see if we should 
        // dump this EFF to disk. why do this if it's already in memory?
        // well because this may not be the only one in the process
        //
    if (_pFontOutputStream->bytes() > _nHeapLimit)
    {
        DWFString zTemplate( /*NOXLATE*/"dwf_eff_" );
        _pFontFile = DWFTempFile::Create( zTemplate, true );

            //
            // doesn't make much sense to lose this info
            // so just keep it in memory if we cannot create teh file
            //
        if (_pFontFile)
        {
            try
            {
                //
                // fill the file with the EFF
                //
                DWFOutputStream& rFileStream = _pFontFile->getOutputStream();
                rFileStream.write( _pFontOutputStream->buffer(), _pFontOutputStream->bytes() );
                rFileStream.flush();

                //
                // free the buffer stream
                //
                DWFCORE_FREE_OBJECT( _pFontOutputStream );
                _pFontOutputStream = NULL;
            }
                //
                // again, if there was a problem, let's keep the memory stream alive
                // normally not a good idea to consume exceptions, especially low level
                // ones but this seems like a reasonable candidate...
                //
            catch (...)
            {
                DWFCORE_FREE_OBJECT( _pFontFile );
                _pFontFile = NULL;
            }
        }
    }
}

_DWFTK_API
const DWFString&
DWFEmbeddedFontImpl_Win32::getLogfontName()
throw( DWFException )
{
    if (_zLogfontName.bytes() > 0)
    {
        return _zLogfontName;
    }

#ifndef Gdiplus

    return getFaceName();

#else 

    //
    // need a DC for the font and object
    // is HFONT the most useless typedef?
    //
    HDC hDC = ::GetDC( NULL );
    if (hDC == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to acquire a NULL DC" );
    }

    Graphics graphics(hDC);

    Font tmpFont(hDC, _hFont);

#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

        //
        // use ANSI
        //
    if (IsWindows9x())
    {
        LOGFONTA logfontA;
        tmpFont.GetLogFontA(&graphics, &logfontA);
        _zLogfontName = logfontA.lfFaceName;
    }
    else

#endif

    {
        LOGFONTW logfontW;
        tmpFont.GetLogFontW(&graphics, &logfontA);
        _zLogfontName = logfontW.lfFaceName;
    }

    ::ReleaseDC( NULL, hDC );
    return _zLogfontName;

#endif

}

_DWFTK_API
const DWFString&
DWFEmbeddedFontImpl_Win32::getFaceName()
throw( DWFException )
{
    if (_zFaceName.bytes() > 0)
    {
        return _zFaceName;
    }

    //
    // need a DC for the font and object
    // is HFONT the most useless typedef?
    //
    HDC hDC = ::GetDC( NULL );
    if (hDC == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to acquire a NULL DC" );
    }

    //
    // need a GDI object 
    //
    HGDIOBJ hObj = ::SelectObject( hDC, _hFont );
    if (hObj == NULL)
    {
        ::ReleaseDC( NULL, hDC );
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to select font into DC" );
    }

#ifdef  DWFCORE_WIN32_INCLUDE_WIN9X_CODE

        //
        // use ANSI
        //
    if (IsWindows9x())
    {
        int nChars = 1 + ::GetTextFaceA( hDC, 0, NULL );
        if (nChars == 1)
        {
            //
            // clean up
            //
            ::SelectObject( hDC, hObj );
            ::ReleaseDC( NULL, hDC );
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to resolve font information" );
        }

        char* zFaceName = DWFCORE_ALLOC_MEMORY( char, nChars );
        if (zFaceName == NULL)
        {
            //
            // clean up
            //
            ::SelectObject( hDC, hObj );
            ::ReleaseDC( NULL, hDC );
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate string buffer" );
        }

        ::GetTextFaceA( hDC, nChars, zFaceName );
        _zFaceName.assign( zFaceName );

        DWFCORE_FREE_OBJECT(zFaceName);
    }
    else

#endif

    {
        int nChars = 1 + ::GetTextFaceW( hDC, 0, NULL );
        if (nChars == 1)
        {
            //
            // clean up
            //
            ::SelectObject( hDC, hObj );
            ::ReleaseDC( NULL, hDC );
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to resolve font information" );
        }

        wchar_t* zFaceName = DWFCORE_ALLOC_MEMORY( wchar_t, nChars );
        if (zFaceName == NULL)
        {
            //
            // clean up
            //
            ::SelectObject( hDC, hObj );
            ::ReleaseDC( NULL, hDC );
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate string buffer" );
        }

        ::GetTextFaceW( hDC, nChars, zFaceName );
        _zFaceName.assign( zFaceName );

        DWFCORE_FREE_OBJECT(zFaceName);
    }

    //
    // clean up
    //
    ::SelectObject( hDC, hObj );
    ::ReleaseDC( NULL, hDC );
    return _zFaceName;
}

#endif

_DWFTK_API
int
DWFEmbeddedFontImpl_Win32::getRequest()
throw( DWFException )
{
    return _nRequest;
}

_DWFTK_API
DWFFontResource::tePrivilege
DWFEmbeddedFontImpl_Win32::getPrivilege()
throw( DWFException )
{
    DWFFontResource::tePrivilege ePriv;

    switch (_nPriv)
    {
        case EMBED_EDITABLE:
        {
            ePriv = DWFFontResource::eEditable;
            break;
        }
        case EMBED_INSTALLABLE:
        {
            ePriv = DWFFontResource::eInstallable;
            break;
        }
        case EMBED_NOEMBEDDING:
        {
            ePriv = DWFFontResource::eNoEmbedding;
            break;
        }
        case EMBED_PREVIEWPRINT:
        {
            ePriv = DWFFontResource::ePreviewPrint;
            break;
        }
        default:
        {
            _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Unknown privilege" );
        }
    }

    return ePriv;
}


_DWFTK_API
DWFFontResource::teCharacterCode
DWFEmbeddedFontImpl_Win32::getCharacterCode()
throw( DWFException )
{
    DWFFontResource::teCharacterCode eCharCode;

    switch (_nCharacterCode)
    {
        case CHARSET_GLYPHIDX:
        {
            eCharCode = DWFFontResource::eGlyphIndex;
            break;
        }
        case CHARSET_SYMBOL:
        {
            eCharCode = DWFFontResource::eSymbol;
            break;
        }
        case CHARSET_UNICODE:
        {
            eCharCode = DWFFontResource::eUnicode;
            break;
        }
        default:
        {
            _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Unknown character code" );
        }
    }

    return eCharCode;
}

#ifndef DWFTK_READ_ONLY

unsigned long
DWFEmbeddedFontImpl_Win32::_WriteFontBuffer( void*               pStream,
                                             const void*         pBuffer,
                                             const unsigned long nBytesToWrite )
throw( DWFException )
{
    size_t nBytesWritten = ((DWFOutputStream*)pStream)->write( pBuffer, nBytesToWrite );
                           ((DWFOutputStream*)pStream)->flush();

    return (unsigned long)nBytesWritten;
}

#endif


//DNT_Start

_DWFTK_API
void
DWFEmbeddedFontImpl_Win32::load( bool bForceRename )
throw ( DWFException )
{
    if (_pFontResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"No font resource available for loading" );
    }

    //
    // When renaming fonts, the new font name must contain the original 
    // font name but may not exceed LF_FACESIZE.  We will simply append
    // 4 character a static counter to the end of the canonical name.
    // There is nothing particularly special about the new name as long
    // as it is unique with the process (thread?) loading it.
    //
    static unsigned short knCount = 0;

    //
    // generate the postfix string
    //
    wchar_t zPostfix[5] = {0};
    _DWFCORE_SWPRINTF( zPostfix, 9, /*NOXLATE*/L"%0.4x", knCount++ );

    //
    // this will be the new name of the embedded font
    //
    wchar_t zEmbeddedFontName[LF_FACESIZE + 1] = {0};

        //
        // the canonical name of the embedded font 
        // exceeds the maximum allowable length (including the postfix)
        // 
    if (_pFontResource->canonicalName().chars() > (LF_FACESIZE - 4))
    {
        //
        // in this case, take the first (LF_FACESIZE - 4)
        // characters as the embedded font name
        //
        DWFCORE_WIDE_STRING_COPY_LENGTH( zEmbeddedFontName, (const wchar_t*)_pFontResource->canonicalName(), (LF_FACESIZE - 4) );
    }
        //
        // the whole name will fit
        //
    else
    {
        DWFCORE_WIDE_STRING_COPY( zEmbeddedFontName, (const wchar_t*)_pFontResource->canonicalName() );
    }

    //
    // append the postfix (counter)
    //
    DWFCORE_WIDE_STRING_CONCATENATE( zEmbeddedFontName, zPostfix );

    //
    // acquire the data stream
    //
    _pFontInputStream = _pFontResource->getInputStream();

        //
        // verify compliance in order to avoid a potential crash us when we try to rename it.
        //
    bool bCompliant = true;
    {
        wchar_t         zName[128];
        wchar_t         zBuffer[128];
        unsigned short  nReadBytes = 0;
        unsigned short  nNameLength = 0;

        //
        // use a buffered stream to cache the bytes we read
        // since there is no seeking on an unzipping stream
        // and that's more than likely what the font resource
        // has given us.
        //
        _pFontOutputStream = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream(1024) );
        
        if (_pFontOutputStream == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
        }

            //
            // skip over unused chunk of the header
            //
        if (_pFontInputStream->read(zBuffer, 82) != (size_t)82)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(zBuffer, 82) != (size_t)82)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // first name length in bytes
            //
        if (_pFontInputStream->read(&nNameLength, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(&nNameLength, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // first name 
            //
        if (_pFontInputStream->read(zName, nNameLength + 2) != (size_t)(nNameLength + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(zName, nNameLength + 2) != (size_t)(nNameLength + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // extra name length in bytes
            //
        if (_pFontInputStream->read(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // extra name 
            //
        if (_pFontInputStream->read(zBuffer, nReadBytes + 2) != (size_t)(nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(zBuffer, nReadBytes + 2) != (size_t)(nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

        //
        // capture this information
        //
        DWFString zComposite( zName );
        zComposite.append( /*NOXLATE*/L" " );
        zComposite.append( zBuffer );

            //
            // another extra name length in bytes
            //
        if (_pFontInputStream->read(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // another extra name - ignore this
            //
        if (_pFontInputStream->read(zBuffer, nReadBytes + 2) != ((size_t)nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(zBuffer, nReadBytes + 2) != (size_t)(nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }


            //
            // second name length in bytes
            //
        if (_pFontInputStream->read(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(&nReadBytes, 2) != (size_t)2)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }

            //
            // second name 
            //
        if (_pFontInputStream->read(zBuffer, nReadBytes + 2) != (size_t)(nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error reading from font stream" );
        }

            //
            // cache these bytes
            //
        if (_pFontOutputStream->write(zBuffer, nReadBytes + 2) != (size_t)(nReadBytes + 2))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Error writing to buffer" );
        }


            //
            // compliance test
            //
        if ((DWFCORE_COMPARE_WIDE_STRINGS((const wchar_t*)zComposite, zBuffer) != 0) && 
            (DWFCORE_COMPARE_WIDE_STRINGS( zName, zBuffer) != 0))
        {
            // we need to deal with the asian font case where the test 
            // above fails, when it really shouldn't (asian fonts do not
            // adhere to the same naming convention and thus give us a 
            // false negative on the test.) 
            bool bIsAscii = true;
            unsigned short nIndex = 0;
            for( ; nIndex <= nNameLength && bIsAscii; nIndex++ )
            {
                if (zName[nIndex] > 0x7F)
                    bIsAscii = false;
            } // for( ; nIndex <= nNameLength && nNonAscii; nIndex++ )

            bCompliant = !bIsAscii;
        }
    }

    //
    // now wrap up the cached bytes in an input stream for later 
    //
    _pReadAheadStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pFontOutputStream->buffer(), _pFontOutputStream->bytes()) );

    if (_pReadAheadStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
    }

    bool bLoaded = false;
    _nRequest = TTLOAD_PRIVATE;

        //
        // did we pass?
        //
    if (bCompliant)
    {
        ULONG nPrivStatus = 0;
        ULONG nCallStatus = 0;

        switch(_pFontResource->privilege())
        {
            case DWFFontResource::ePreviewPrint:
            {
                _nPriv = LICENSE_PREVIEWPRINT;
                break;
            }
            case DWFFontResource::eEditable:
            {
                _nPriv = LICENSE_EDITABLE;
                break;
            }
            case DWFFontResource::eInstallable:
            {
                _nPriv = LICENSE_INSTALLABLE;
                break;
            }
            case DWFFontResource::eNoEmbedding:
            {
                _nPriv = LICENSE_NOEMBEDDING;
                break;
            }
            default:
            {
                _nPriv = LICENSE_DEFAULT;
                break;
            }
        }

            //
            // attempt to load the font with the new name
            //
        if ((*_kfpTTLoadEmbeddedFont)(&_hLoadedFont, 
                                       _nRequest, 
                                      &nPrivStatus,
                                      _nPriv,
                                      &nCallStatus,
                                      &_ReadFontBuffer,
                                        this,
                                        zEmbeddedFontName, 
                                        NULL, 
                                        NULL) == 0)
        {
            bLoaded = true;
        }
    }

        //
        // the attempt failed
        //
        // If the font name cannot be changed, don’t install the font. Use font 
        // replacement instead. If the partial font is installed using the fonts 
        // own name, it will prevent another subset of the same font from being 
        // installed in the future. 
        //
        //
    if ((bLoaded == false) && bForceRename)
    {
        ULONG nPrivStatus = 0;
        ULONG nCallStatus = 0;

        switch(_pFontResource->privilege())
        {
            case DWFFontResource::ePreviewPrint:
            {
                _nPriv = LICENSE_PREVIEWPRINT;
                break;
            }
            case DWFFontResource::eEditable:
            {
                _nPriv = LICENSE_EDITABLE;
                break;
            }
            case DWFFontResource::eInstallable:
            {
                _nPriv = LICENSE_INSTALLABLE;
                break;
            }
            case DWFFontResource::eNoEmbedding:
            {
                _nPriv = LICENSE_NOEMBEDDING;
                break;
            }
            default:
            {
                _nPriv = LICENSE_DEFAULT;
                break;
            }
        }

        //
        // wipe out the generated name
        //
        zEmbeddedFontName[0] = 0;

        //
        // release the previous and acquire a new resource stream
        //
        DWFCORE_FREE_OBJECT( _pFontInputStream );

        _pFontInputStream = _pFontResource->getInputStream();

        //
        // make another attempt to load the font, this will succeed if the font
        // is not currently installed in the system
        //
        LONG nErrorCode = (*_kfpTTLoadEmbeddedFont)(&_hLoadedFont, 
                                       _nRequest, 
                                       &nPrivStatus,
                                       _nPriv,
                                       &nCallStatus,
                                      &_ReadFontBuffer,
                                        this,
                                        NULL, 
                                        NULL, 
                                        NULL);
        if (nErrorCode == E_FONTNAMEALREADYEXISTS)
        {
            //
            // release the previous and acquire a new resource stream
            //
            DWFCORE_FREE_OBJECT( _pFontInputStream );

            _pFontInputStream = _pFontResource->getInputStream();

          // let see if we are trying to load a font that is already in the system, but
          // the font name might be defined with a localized name
          if ((*_kfpTTLoadEmbeddedFont)(&_hLoadedFont, 
                                       _nRequest, 
                                       &nPrivStatus,
                                       _nPriv,
                                       &nCallStatus,
                                      &_ReadFontBuffer,
                                        this,
                                        zEmbeddedFontName, 
                                        NULL, 
                                        NULL))
          {
            // if we still fail, but we have no choice but to throw an exception
            _DWFCORE_THROW( DWFNameCollisionException, /*NOXLATE*/L"Failed to load the embedded font" );
          }
        }
        // for all other errors, we will throw an exception
        else if (nErrorCode != 0)
        {
          //
          // the attempt failed
          //

          _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Failed to load the embedded font" );
        }
    }

        //
        // the embedded font was loaded, we just need to keep track of what it was called
        //
    if (zEmbeddedFontName[0] != 0)
    {
        _zLoadedFontName.assign( zEmbeddedFontName );
    }
    else
    {
        _zLoadedFontName.assign( _pFontResource->canonicalName() );
    }

    //
    // clean up some memory
    //
    DWFCORE_FREE_OBJECT( _pReadAheadStream );       _pReadAheadStream = NULL;
    DWFCORE_FREE_OBJECT( _pFontOutputStream );      _pFontOutputStream = NULL;
    DWFCORE_FREE_OBJECT( _pFontInputStream );       _pFontInputStream = NULL;
}

_DWFTK_API
const DWFString&
DWFEmbeddedFontImpl_Win32::loadedFontName() const
throw()
{
    return _zLoadedFontName;
}

_DWFTK_API
void 
DWFEmbeddedFontImpl_Win32::unload()
throw ( DWFException )
{
    ULONG nStatus = 0;

    (*_kfpTTDeleteEmbeddedFont)( _hLoadedFont, TTLOAD_PRIVATE, &nStatus );
}

void
DWFEmbeddedFontImpl_Win32::_loadFontLibrary( void )
{
    if (_khLib == NULL)
    {
        _khLib = ::LoadLibraryA( _kz_T2Embed );
        if (_khLib == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to load TrueType embedding library" );
        }

        //
        // Increment the ref count
        //
        _knLibRef++;
        
        _kfpTTEmbedFont = (fpTTEmbedFont)::GetProcAddress( _khLib, /*NOXLATE*/"TTEmbedFont" );
        if (_kfpTTEmbedFont == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to locate TTEmbedFont API" );
        }

        _kfpTTLoadEmbeddedFont = (fpTTLoadEmbeddedFont)::GetProcAddress( _khLib, "TTLoadEmbeddedFont" );
        if (_kfpTTLoadEmbeddedFont == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to locate TTLoadEmbeddedFont API" );
        }

        _kfpTTDeleteEmbeddedFont = (fpTTDeleteEmbeddedFont)::GetProcAddress( _khLib, "TTDeleteEmbeddedFont" );
        if (_kfpTTDeleteEmbeddedFont == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Failed to locate TTDeleteEmbeddedFont API" );
        }
    }
        //
        // already loaded, increment the ref count
        //
    else
    {
        _knLibRef++;
    }
}

unsigned long
DWFEmbeddedFontImpl_Win32::_ReadFontBuffer( void*               pStream,
                                            void*               pBuffer,
                                            const unsigned long nBytesToRead )
throw( DWFException )
{
    size_t nBytesRead = 0;
    unsigned char* pIn = (unsigned char*)pBuffer;

    //
    // pStream is actually the impl
    //
    DWFEmbeddedFontImpl_Win32* pThis = (DWFEmbeddedFontImpl_Win32*)pStream;

        //
        // first check the cached stream
        //
    if (pThis->_pReadAheadStream->available() > 0)
    {
        nBytesRead += pThis->_pReadAheadStream->read( pIn, nBytesToRead );
        pIn += nBytesRead;
    }

        //
        // resume use of the resource stream
        //
    if (nBytesRead < nBytesToRead)
    {
        nBytesRead += pThis->_pFontInputStream->read( pIn, (nBytesToRead - nBytesRead) );
    }

    return (unsigned long)nBytesRead;
}

//DNT_End

#endif
