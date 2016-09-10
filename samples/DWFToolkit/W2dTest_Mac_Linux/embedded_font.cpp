#include "StdAfx.h"
#include "TestFramework.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/BufferInputStream.h"

//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/embedded_font.cpp#1 $

#ifndef _DWFCORE_WIN32_SYSTEM
WT_Result write_embedded_font(WT_File & /*whip_file*/)
{
	return WT_Result::Success;
}

WT_Result read_embedded_font(WT_Embedded_Font& /*font*/, WT_File& /*whip_file*/)
{
	return WT_Result::Success;
}
#endif

#ifdef _DWFCORE_WIN32_SYSTEM
#include "t2embapi.h"
#include <assert.h>
#include <io.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

HINSTANCE t2embed = NULL;

extern WT_Result 
    getSubsetFontName( WT_String& zFontName, WT_Font& oFont, const WT_String &zStr, bool bObfuscate );

WT_Result embed_font(wchar_t* strbuf, long & charset, WT_String &zFontName, WT_File & whip_file);
WT_Result embed_font_xaml(const wchar_t* zStrbuf, WT_Font& oFont, const wchar_t* zFontName, WT_File & whip_file);

typedef LONG (__stdcall *fpTTEmbedFont)
(
	HDC       hDC,                    // device-context handle
	ULONG     ulFlags,                // flags specifying the request
	ULONG     ulCharSet,              // flags specifying char set
	ULONG*    pulPrivStatus,          // upon completion contains embedding priv of font
	ULONG*    pulStatus,              // on completion may contain status flags for request
	WRITEEMBEDPROC lpfnWriteToStream, // callback function for doc/disk writes
	LPVOID    lpvWriteStream,         // the output stream tokin
	USHORT*   pusCharCodeSet,         // address of buffer containing optional
									  // character codes for subsetting
	USHORT    usCharCodeCount,        // number of characters in the
									  // lpvCharCodeSet buffer
	USHORT    usLanguage,             // specifies the language in the name table to keep
									  //  set to 0 to keep all
	TTEMBEDINFO* pTTEmbedInfo         // optional security
);

fpTTEmbedFont pTTEmbedFont = NULL;

long serialize_embed_font_data(HDC tempDC, 
    WT_File & whip_file,
    char *tempfile,
    unsigned long & privStatus, 
    unsigned long & status, 
    unsigned long & request,
    unsigned short *sparse,
    unsigned short points, 
    const char *canonical_name,
    const char *logfont_name);

WT_Result read_embedded_font(WT_Embedded_Font & /*font*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_embedded_font(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 6, WT_String("Fonts"));
    whip_file.desired_rendition().layer() = *layer;

    WT_Logical_Point    min, max;

    int x = 1500;
    int y = -2350;

    min.m_x = x;
    min.m_y = y-1000;
    max.m_y = y;
    max.m_x = x+1000;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Fonts");
    WD_CHECK(named_view->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;

    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_String str("Testing ANSI, Japanese, Arabic, extended Latin.  (Vineta BT is an embedded font)");
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y-50), str);
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().font().height().set(50);
    wchar_t strbuf[128];
    wchar_t test_str[64];
    //Test characters for ANSI, Japanese, Arabic, and extended Latin
    swprintf(test_str, L"123ABC \x30A4\x30B4\x30C4\x30D4 \x0682\x0692\x06A2\x06B2 \x0105\x0115\x0125\x0135");

    whip_file.desired_rendition().font().font_name().set("Arial");
    swprintf(strbuf, L"%s %s", L"Arial", test_str);
    str = WT_String((const WT_Unsigned_Integer16*)strbuf);
	classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x, y-125), str);
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().font().font_name().set("Courier New");
    swprintf(strbuf, L"%s %s", L"Courier New", test_str);
    str = WT_String((const WT_Unsigned_Integer16*)strbuf);
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x, y-175), str);
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().font().font_name().set("Times New Roman");
    swprintf(strbuf, L"%s %s", L"Times New Roman", test_str);
    str = WT_String((const WT_Unsigned_Integer16*)strbuf);
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x, y-225), str);
    WD_CHECK(text->serialize(whip_file));

    //Embed a font
    long charset = CHARSET_UNICODE;
    WT_String zFontName("Consolas");
    wchar_t *pFontName = WT_String::to_wchar( zFontName.length(), zFontName );
    WD_NULLCHECK( pFontName );
    whip_file.desired_rendition().font().charset().set( (WT_Byte)charset );
    whip_file.desired_rendition().font().font_name().set( zFontName );

    //Generate the string we're using
    swprintf(strbuf, L"%s %s", pFontName, test_str);
    str = WT_String((const WT_Unsigned_Integer16*)strbuf);

    if (CTestFramework::Instance().GetClassFactoryType() != CTestFramework::XAML)
    {
        WD_CHECK( embed_font(strbuf, charset, zFontName, whip_file) );
    }
    else
    {
        WD_CHECK( embed_font_xaml( strbuf, whip_file.desired_rendition().font(), pFontName, whip_file) );
    }

    delete[] pFontName;

    //Use it
    WT_Font_Extension *old_extension = classFactory->Create_Font_Extension();
	*old_extension = whip_file.desired_rendition().font_extension();
    whip_file.desired_rendition().font_extension().set_cannonical_name("Consolas");
    whip_file.desired_rendition().font_extension().set_logfont_name("Consolas");
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x, y-300), str);
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font_extension() = *old_extension;

	classFactory->Destroy(old_extension);
	classFactory->Destroy(text);
	classFactory->Destroy(named_view);
	classFactory->Destroy(layer);

    return WT_Result::Success;
}


WT_Result embed_font_xaml(const wchar_t* zStrbuf, WT_Font& oFont, const wchar_t* zpFontName, WT_File & whip_file)
{
    //Note, we would also need stuff like Bold and Italics, etc., but for
    //this unit test we're just setting the font name and height.
    size_t nFontBufSize = 0;
    WT_Byte *pFontBuf = NULL;
    wchar_t *pFontPath = NULL;
    WT_String zFontName = (const WT_Unsigned_Integer16*)zpFontName;
    WT_String zFontPath = zFontName;
    WT_String zSrc = (const WT_Unsigned_Integer16*)zStrbuf;
    { //scope the streams
        WD_CHECK( getSubsetFontName( zFontPath, oFont, zSrc, false ) );  //this gets us a buffer we can pull into WT_Embedded_Font
        pFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath.unicode() );
        DWFFile oFontFile( pFontPath );
        DWFStreamFileDescriptor oFontFileDescriptor( oFontFile, L"rb" );
        DWFFileInputStream oFontStream;
        oFontFileDescriptor.open();
        oFontStream.attach( &oFontFileDescriptor, false );
        nFontBufSize = oFontStream.available();
        pFontBuf = DWFCORE_ALLOC_MEMORY( WT_Byte, nFontBufSize );
        WD_NULLCHECK( pFontBuf );
        oFontStream.read( pFontBuf, nFontBufSize );
        delete [] pFontPath;
    }
    
    WT_Embedded_Font *pEmbeddedFont = CTestFramework::Instance().GetClassFactory()->Create_Embedded_Font(
        //Note, DO NOT use WT_Embedded_Font::Encrypt_Data for XAML, the toolkit will encrypt/obfuscate automatically
        WT_Embedded_Font::Subset /*request_type */,  
        WT_Embedded_Font::Installable /*privilege_type */,
        WT_Embedded_Font::Unicode /*character_set_type */,
        (WT_Integer32)nFontBufSize /*data_size*/,
        pFontBuf /*data*/,
        zFontName.length() /*font_type_face_name_length*/,
        (WT_Byte*)zFontName.ascii()  /*font_type_face_name_string*/,
        zFontName.length() /*font_logfont_name_length*/,
        (WT_Byte*)zFontName.ascii()  /*font_logfont_name_string*/,
        WD_False /*copy*/);

    WD_NULLCHECK( pEmbeddedFont );

    //Note, serializing the WT_Embedded_Font will archive the font data 
    //via An internal OPC call and register the font Uri with the file
    WD_CHECK( pEmbeddedFont->serialize( whip_file ) );  
    CTestFramework::Instance().GetClassFactory()->Destroy( pEmbeddedFont );

    DWFCORE_FREE_MEMORY( pFontBuf );
    return WT_Result::Success;
}

WT_Result embed_font(wchar_t* strbuf, long & charset, WT_String &zFontName, WT_File & whip_file)
{
    //if the embedding dll isn't already loaded, load it
    if (NULL == t2embed) {
        t2embed = LoadLibraryA(/*MSG0*/"t2embed.dll");
        if (NULL != t2embed) {
            pTTEmbedFont = (fpTTEmbedFont)GetProcAddress(t2embed, "TTEmbedFont");
            if (NULL == pTTEmbedFont)
                assert("Failed to get TTEmbedFont");
        } else
            assert("Failed to load t2embed.dll");
    }
    if (NULL == pTTEmbedFont) {
        assert("TTEmbedFont was unexpectedly NULL");
        return WT_Result::Internal_Error;
    }

    // create a temporary filename
    char *tempfile = _tempnam(NULL, /*NOXLAT*/"embed");
    if(!tempfile)
        return WT_Result::Internal_Error; //cannot get temporary filename

    if (0==_access(tempfile, 0)) //exists
    {
        struct _stat st;
        if (0==_stat(tempfile, &st) && (st.st_mode & _S_IFDIR))
            _rmdir(tempfile);
        else
            remove(tempfile);
    }

    //Create the font.  
    //TODO: Should we use the ANSI charset (it's our default)?  If not, what should we use?!
    HDC tempDC = GetDC(NULL);
    wchar_t* pFontName = WT_String::to_wchar( zFontName.length(), zFontName );
    HFONT hFont = CreateFontW(100, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, pFontName);
    delete[] pFontName;
    HGDIOBJ hObj, hRobj;
    hObj = SelectObject(tempDC, hFont);
    TEXTMETRICW metric;
    GetTextMetricsW(tempDC, &metric);
    charset = metric.tmCharSet;

    const char *logfont_name = zFontName.ascii();
    const char *canonical_name = logfont_name; //Note, may not always be equal! But it is in the case of this font


    unsigned short num_codepoints = 0;
    bool fData[65536];
    memset(fData, 0, 65536*sizeof(bool));

    WORD *indices = new WORD[wcslen(strbuf)];
    if (!indices)
        return WT_Result::Out_Of_Memory_Error;

    //pass 1: get unique count
    for (int i=0;i<(int)wcslen(strbuf);i++)
    {
        if (!fData[(int)strbuf[i]])
        {
            fData[(int)strbuf[i]] = true;
            num_codepoints++;
        }
    }

    unsigned short *sparse = new unsigned short[num_codepoints];
    memset(sparse, 0, num_codepoints*sizeof(unsigned short));

    //pass 2: determine indices
    unsigned short index = 0;
    int points = 0;

    // first check the codepoints up to 0x300
    while ((points < num_codepoints) && (index < 0x300))
    {
        if (fData[index])
            sparse[points++] = index;
        index++;
    }

    // then check the remaining codepoints used by cp1252
    if (points < num_codepoints) 
    {
        // now check the range from 0x2000-0x21ff
        index = 0x2000;
        while ((points < num_codepoints) && (index < 0x2200))
        {
            if (fData[index])
                sparse[points++] = index;
            index++;
        }
    }

    if (points < num_codepoints) 
    {
        // now check the range from 0x300-0x1fff
        index = 0x300;
        while ((points < num_codepoints) && (index < 0x2000)) 
        {
            if (fData[index])
                sparse[points++] = index;
            index++;
        }
    }

    if (points < num_codepoints) 
    {
        // now check the remaining range
        index = 0x2200;
        while ((points < num_codepoints) && (index < 0x10000)) 
        {
            if (fData[index])
                sparse[points++] = index;
            index++;
        }
    }

    //Embed the font
    unsigned long status;
    unsigned long privStatus = EMBED_INSTALLABLE;
    unsigned long request = TTEMBED_SUBSET | TTEMBED_TTCOMPRESSED |
        TTEMBED_ENCRYPTDATA ;

    //We first try to subset font with maximum embeddable privilege.
    long retval = serialize_embed_font_data(tempDC, whip_file, tempfile, privStatus, status, 
                request, sparse, num_codepoints, canonical_name, logfont_name);

    //We may have failed to subset with maximum embeddable privilege. 
    //So now, let use try to subset with less embeddable privilege.
    //privStatus from the previous would hold the maximum permissible 
    //embeddable privilege value, so let us try to make use of it.
    if (E_NONE != retval) {
        status = 0;
        request = TTEMBED_SUBSET | TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA;
        retval = serialize_embed_font_data(tempDC, whip_file, tempfile, privStatus, status, 
                request, sparse, num_codepoints, canonical_name, logfont_name);
    }

    //We may still have failed because, the embedding sdk couldn't subset
    //the font for some reason. So, now let us try embedding the entire font 
    //information with maximum privilege.
    if (E_NONE != retval) {
        status = 0;
        request = TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA;
        privStatus = EMBED_INSTALLABLE;
        retval = serialize_embed_font_data(tempDC, whip_file, tempfile, privStatus, status, 
                request, sparse, num_codepoints, canonical_name, logfont_name);
    }

    //We may have failed to embed the entire font with maximum embeddability 
    //privilege. So, let us try embedding the entire font with least 
    //embeddability privilege. 
    //privStatus from the previous would hold the maximum permissible 
    //embeddable privilege value, so let us try to make use of it.
    
    if (E_NONE != retval) {
        status = 0;
        request = TTEMBED_TTCOMPRESSED | TTEMBED_ENCRYPTDATA;
        retval = serialize_embed_font_data(tempDC, whip_file, tempfile, privStatus, status, 
                request, sparse, num_codepoints, canonical_name, logfont_name);
    }

    if (retval == -1) {
        assert("Font Embedding failed!");
    }

    delete[] indices;
    delete[] sparse;
    free(tempfile);
    // restore the previous font
    hRobj = SelectObject(tempDC, hObj);
    // clean up created font
    retval = DeleteObject(hFont);
    // clean up DC usage
    ReleaseDC(NULL, tempDC);

    return WT_Result::Success;
}

unsigned long write_proc(void * tFile, const void * buffer, 
    const unsigned long count)
{
    return (unsigned long) fwrite((char *)buffer, 1, count, (FILE *)tFile);
}

long serialize_embed_font_data(HDC tempDC, 
    WT_File & whip_file,
    char *tempfile,
    unsigned long & privStatus, 
    unsigned long & status, 
    unsigned long & request,
    unsigned short *sparse,
    unsigned short points, 
    const char *canonical_name,
    const char *logfont_name)
{
    FILE * tFile;
    tFile = fopen(tempfile, /*MSG0*/"wb+");
    if (NULL == tFile) {
        assert("Failed fopen in embed_font");
        return -1;
    }

    long retval = -1; 

    retval = (*pTTEmbedFont)(tempDC, request, CHARSET_UNICODE, &privStatus, 
        &status, &write_proc, tFile, sparse, points, 0, NULL);

    fclose(tFile);

    tFile = fopen(tempfile, "rb+");
    if (NULL == tFile) {
        assert("Failed fopen in embed_font");
        return -1;
    }

    if (E_NONE == retval)
    {
        // the font was successfully embedded
        fseek(tFile, 0, SEEK_END);
        long fsize = ftell(tFile);
        unsigned char * fBuffer = new unsigned char[fsize];
        if (NULL == fBuffer)
            assert("failed to allocate fBuffer");
        fseek(tFile, 0 ,SEEK_SET);
        size_t amount = fread(fBuffer, 1, fsize, tFile);
        assert((long)amount == fsize);
        // now we have the font data in fBuffer

    	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
        if (classFactory == NULL)
        {
            assert("Failed to get class factory");
            return -2;
        }

        WT_Embedded_Font* pEmbeddedFont = classFactory->Create_Embedded_Font(
            request, 
            privStatus, 
            CHARSET_UNICODE, 
            fsize, 
            fBuffer, 
            (WT_Integer32) strlen(canonical_name), 
            (unsigned char *)canonical_name, 
            (WT_Integer32) strlen(logfont_name), 
            (unsigned char *)logfont_name, WD_False);

        if (pEmbeddedFont == WD_Null)
        {
            assert("Failed to create embedded font object");
            return -3;
        }

        pEmbeddedFont->serialize(whip_file);
        delete [] fBuffer;
        classFactory->Destroy( pEmbeddedFont );
    }
    
    fclose(tFile);

    return retval;
}

#endif