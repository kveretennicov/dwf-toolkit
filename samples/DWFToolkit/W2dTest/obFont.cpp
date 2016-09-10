// obFont.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dwfcore/Core.h"
#include "dwf/Toolkit.h"
#include "dwfcore/File.h"
#include "dwf/XAML/pch.h"

using namespace DWFCore;

int 
    subsetFont(void* pFontBuffer,               /*in*/
    DWORD nFontBufferLen,                       /*in*/
    const unsigned short * keepList,            /*in*/
    unsigned short count,                       /*out*/
    unsigned char*& puchFontPackageBuffer,      /*out*/
    unsigned long&   ulFontPackageBufferSize,   /*out*/
    unsigned long&   ulBytesWritten);           /*out*/


WT_Result getSubsetFontName( 
    WT_String& zFontName, 
    WT_Font& oFont,
    const WT_String &zStr, 
    bool bObfuscate )
{
    //Load the font into a file
    //Get a convenient DC
    HDC hDesktopDC = ::GetDC(::GetDesktopWindow());
    HDC hOurDC = ::CreateCompatibleDC( hDesktopDC );
    ::ReleaseDC( ::GetDesktopWindow(), hDesktopDC );

    LOGFONTW logFont;
    ZeroMemory( &logFont, sizeof(logFont) );
    wcscpy( logFont.lfFaceName, (const wchar_t*) oFont.font_name().name().unicode() ); 
    logFont.lfCharSet = oFont.charset().charset();
    logFont.lfHeight = oFont.height().height();
    logFont.lfItalic = oFont.style().italic() ? TRUE : FALSE;
    logFont.lfUnderline = oFont.style().underlined() ? TRUE : FALSE;
    logFont.lfWeight = oFont.style().bold() ? FW_BOLD : FW_NORMAL;
    logFont.lfPitchAndFamily = oFont.pitch().pitch() | oFont.family().family();

    HFONT hfont = ::CreateFontIndirect(&logFont);
    if (hfont == NULL || hfont == INVALID_HANDLE_VALUE)
    {
        return WT_Result::Internal_Error;
    }
    HGDIOBJ holdFont = ::SelectObject(hOurDC, hfont);
    DWORD nBytesRequired = ::GetFontData( hOurDC, NULL, 0, NULL, 0);

    if (nBytesRequired == GDI_ERROR)
    {
        //Try again with default font based on pitch
        switch( oFont.pitch().pitch() )
        {
            case WT_Font_Option_Pitch::PITCH_FIXED:
                wcscpy( logFont.lfFaceName, L"Courier New" );
                break;
            default:
            case WT_Font_Option_Pitch::PITCH_VARIABLE:
                wcscpy( logFont.lfFaceName, L"Arial" );
                break;
        }

        hfont = ::CreateFontIndirect(&logFont);
        if (hfont == NULL || hfont == INVALID_HANDLE_VALUE)
        {
            WD_Assert( false );
            ::SelectObject(hOurDC, holdFont);
            ::DeleteDC( hOurDC );  //Apps cannot use ReleaseDC to release a DC that was created by calling CreateDC; instead, it must use DeleteDC.
            return WT_Result::Internal_Error;
        }

        holdFont = ::SelectObject(hOurDC, hfont);
        nBytesRequired = ::GetFontData( hOurDC, NULL, 0, NULL, 0);
        WD_Assert( nBytesRequired != GDI_ERROR );
        if (nBytesRequired == GDI_ERROR)
        {
            WD_Assert( false );
            ::SelectObject(hOurDC, holdFont);
            ::DeleteObject(hfont);
            ::DeleteDC( hOurDC );  //Apps cannot use ReleaseDC to release a DC that was created by calling CreateDC; instead, it must use DeleteDC.
            return WT_Result::Internal_Error;
        }
    }
       
    void* pFontBuffer = malloc( nBytesRequired );
    WD_NULLCHECK( pFontBuffer );
    DWORD nBytesRead = ::GetFontData(hOurDC, NULL, 0, pFontBuffer, nBytesRequired);
    WD_Assert( nBytesRequired == nBytesRead );
    ::SelectObject(hOurDC, holdFont);
    ::DeleteObject(hfont);

    //Do the font subsetting
    unsigned char* puchFontPackageBuffer = NULL; //holds the subset
    unsigned long  ulFontPackageBufferSize = 0; //holds the buffer size
    unsigned long  ulBytesWritten = 0; //holds the number of bytes written (should be == buffer size)
    int retval = subsetFont( 
        pFontBuffer, 
        nBytesRead, 
        zStr.unicode(), 
        (unsigned short) zStr.length(), 
        puchFontPackageBuffer, 
        ulFontPackageBufferSize, 
        ulBytesWritten );
    ::DeleteDC( hOurDC );  //Apps cannot use ReleaseDC to release a DC that was created by calling CreateDC; instead, it must use DeleteDC.

    WD_Assert( !retval );

    //Now, create a GUID and filename
    DWFUUID oGUID;
    wchar_t zBuf[MAX_PATH];
    ::GetCurrentDirectory(MAX_PATH, zBuf );
    DWFString zObfuscatedFontFilename(zBuf);
    DWFString zGuid = oGUID.uuid(false);
    zObfuscatedFontFilename.append( L"\\out\\" );
    ::CreateDirectory( zObfuscatedFontFilename, NULL );
    zObfuscatedFontFilename.append( zGuid );

    if ( bObfuscate )
        zObfuscatedFontFilename.append( L".odttf" );
    else
        zObfuscatedFontFilename.append( L".ttf" );

    zFontName = (const WT_Unsigned_Integer16*)(const wchar_t*)zObfuscatedFontFilename;
    
    //create the obfuscated font file
    DWFFile oObfuscatedFontFile( zObfuscatedFontFilename );
    DWFStreamFileDescriptor oObfuscatedFontFileDescriptor( oObfuscatedFontFile, L"wb" );
    oObfuscatedFontFileDescriptor.open();
    DWFFileOutputStream oWriteTo;
    oWriteTo.attach( &oObfuscatedFontFileDescriptor, false );
    DWFBufferInputStream oReadFrom( puchFontPackageBuffer, ulBytesWritten );

    //Create a key
    if ( bObfuscate )
    {
        char key[16];
        memset(key, 0, sizeof(key));
        WD_CHECK( WT_XAML_Font::parseKey( key, zObfuscatedFontFilename ) );
        
        //obfuscate the data as we write the data to a file
        WD_CHECK( WT_XAML_Font::obfuscate( &oReadFrom, &oWriteTo, key) );
    }
    else
    {
        char buf[1024];
	    bool done = false;
	    while( !done )
	    {
            size_t bytes = oReadFrom.read(buf, sizeof(buf));
            if (bytes != 0)
            {
                size_t written = oWriteTo.write(buf, bytes);
                WD_Assert( written == bytes );
            }

		    if ( bytes == 0 && oReadFrom.available() == 0)
		    {
			    done = true;
		    }
	    }

        oWriteTo.flush();
    }
    //write the bytes to a file (ulBytesWritten)

    if (puchFontPackageBuffer != NULL)
    {
        free( puchFontPackageBuffer );
    }
    if (pFontBuffer != NULL)
    {
        free( pFontBuffer );
    }

    return WT_Result::Success;
}
