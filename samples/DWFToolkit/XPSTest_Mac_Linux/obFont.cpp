// obFont.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "dwfcore/Core.h"
#include "dwfcore/UUID.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/BufferInputStream.h"
#include "OpcHelper.h"

using namespace DWFCore;
#ifdef _DWFCORE_WIN32_SYSTEM
int 
    subsetFont(void* pFontBuffer,               /*in*/
    DWORD nFontBufferLen,                       /*in*/
    const unsigned short * keepList,            /*in*/
    unsigned short count,                       /*out*/
    unsigned char*& puchFontPackageBuffer,      /*out*/
    unsigned long&   ulFontPackageBufferSize,   /*out*/
    unsigned long&   ulBytesWritten);           /*out*/
#endif
int 
    fuscator(DWFInputStream* in, DWFOutputStream* out, const wchar_t* key);

void 
    parseKey( wchar_t* keyOut, const wchar_t* guidIn);

extern OpcHelper gHelper;

WT_Result getSubsetFontName( WT_String& zFontName, WT_Integer32 nHeight, const WT_String &zStr )
{
    return gHelper.getSubsetFontName( zFontName, nHeight, zStr, true );
}
#ifdef _DWFCORE_WIN32_SYSTEM
// overload version of getSubsetFontName to make it compatable with w2dTest.
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

    zFontName = (const wchar_t*)zObfuscatedFontFilename;
    
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
#endif

void parseKey( wchar_t* keyOut, const wchar_t* guidIn)
{
	/*
		MUST be of the form ìB03B02B01B00-B11B10-B21B20-B30B31-B32B33B34B35B36B37î or 
		ìB03B02B01B00-B11B10-B21B20-B30B31-B32B33B34B35B36B37.extî where each Bx represents a 
		placeholder for one byte of the GUID, represented as two hex digits [M2.54].		
	*/

	/*
		Key: B37B36B35B34B33B32B31B30B20B21B10B11B00B01B02B03
	*/

	int i, idx=15;
	bool high = true;
	size_t len = wcslen( guidIn );

	memset(keyOut, 0, 16*sizeof(wchar_t));

	for (i=0; i<(int)len && idx>=0; i++)
	{
		wchar_t val = 0xFFFF;
		if ( guidIn[i] >= '0' && guidIn[i] <= '9' )
		{
			val = guidIn[i] - '0';
		}
		else if ( guidIn[i] >= 'A' && guidIn[i] <= 'F' )
		{
			val = 10 + (guidIn[i] - 'A');
		}
		else if ( guidIn[i] >= 'a' && guidIn[i] <= 'f' )
		{
			val = 10 + (guidIn[i] - 'a');
		}

		if (val != 0xFFFF)
		{
			if (high)
			{
				keyOut[idx] = (wchar_t) (val*16);
				high = false;
			}
			else
			{
				keyOut[idx] += (val & 0xFFFF);
				high = true;
				idx--;
			}
		}
	}
}

//It does both obfuscation and deobfuscation
int fuscator(DWFInputStream* in, DWFOutputStream* out, const wchar_t* key)
{
	if (in == NULL || out == NULL)
	{
		return -1;
	}

	char buf[32];
	bool done = false;
	bool xorComplete = false;
	while( !done )
	{
        size_t bytes = in->read(buf, 32);
        if (bytes != 0)
        {
            //This only works when we're actually embedding in XPS
            //documents.  For testing purposes (XamlPad, IE7, etc.)
            //we leave the font de-obfuscated.

            xorComplete; key; //reference these for compiler happiness

            #ifdef OBFUSCATE
                if ( !xorComplete )
                {
                    int i,j;
                    for( j=0; j<32; j+=16 )
                    {
                        for( i=0; i<16; i++ )
                        {
                            buf[i+j] ^= key[i];
                        }
                    }
                    xorComplete = true;
                }
            #endif
            size_t written = out->write(buf, bytes);
            WD_Assert( written == bytes);
        }

		if ( bytes == 0 && in->available() == 0)
		{
			done = true;
		}
	}

    out->flush();

	return 0;
}
