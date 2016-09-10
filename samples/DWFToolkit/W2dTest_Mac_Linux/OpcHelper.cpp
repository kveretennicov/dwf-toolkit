#include "StdAfx.h"
#include "OpcHelper.h"
#include "dwf/XAML/XamlClassFactory.h"

#if defined(_DWFCORE_WIN32_SYSTEM)
extern int 
    subsetFont(void* pFontBuffer,               /*in*/
    DWORD nFontBufferLen,                       /*in*/
    const unsigned short * keepList,            /*in*/
    unsigned short count,                       /*out*/
    unsigned char*& puchFontPackageBuffer,      /*out*/
    unsigned long&   ulFontPackageBufferSize,   /*out*/
    unsigned long&   ulBytesWritten);           /*out*/
#else
#define MAX_PATH 256
#endif


extern int 
    fuscator(DWFInputStream* in, DWFOutputStream* out, const wchar_t* key);

extern void 
    parseKey( wchar_t* keyOut, const wchar_t* guidIn);

extern WT_Result 
    getSubsetFontName( 
        WT_String& zFontName, 
        WT_Font& oFont,
        const WT_String &zStr, 
        bool bObfuscate );

OpcHelper::OpcHelper()
: _nPartCount(0)
{
}

OpcHelper::~OpcHelper()
{
}

//WT_OpcResourceMaterializer
WT_Result 
OpcHelper::getPartInputStream(
	const DWFCore::DWFString& rzPath, 
	DWFCore::DWFInputStream** ppStream)
{
    if (ppStream == NULL)
        return WT_Result::Toolkit_Usage_Error;

    DWFFile oFile( rzPath );

    DWFStreamFileDescriptor *pFileDescriptor = 
        DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oFile, L"rb" ) );
    WD_NULLCHECK( pFileDescriptor );

	*ppStream = DWFCORE_ALLOC_OBJECT(DWFFileInputStream);
    WD_NULLCHECK( *ppStream );
	pFileDescriptor->open();
	((DWFFileInputStream*)(*ppStream))->attach(pFileDescriptor, true);

    return WT_Result::Success;
}

//WT_OpcResourceSerializer
WT_Result 
OpcHelper::getPartOutputStream(		
    const DWFCore::DWFString& rzMimeType,
    DWFCore::DWFString& rzPath,
    DWFCore::DWFOutputStream** ppStream)
{
    if (ppStream == NULL)
        return WT_Result::Toolkit_Usage_Error;

    DWFUUID oUuidGen;

	
    rzPath.append( oUuidGen.next(false) );

    if (rzMimeType == L"application/vnd.ms-package.obfuscated-opentype")
    {
        rzPath.append(L".odttf");
    }
    else if (rzMimeType == L"application/vnd.ms-opentype")
    {
        rzPath.append(L".ttf");
    }
    else if (rzMimeType == L"application/vnd.ms-package.xps-resourcedictionary+xml")
    {
        rzPath.append(L".xaml");
    }
    else if (rzMimeType == L"image/jpeg")
    {
        rzPath.append(L".jpg");
    }
    else if (rzMimeType == L"image/png")
    {
        rzPath.append(L".png");
    }
    else if (rzMimeType == L"image/tiff")
    {
        rzPath.append(L".tif");
    }
    else
    {
        WD_Assert( false ); //unknown mime type!
    }

	DWFFile oFile( rzPath );

    DWFStreamFileDescriptor *pFileDescriptor = 
        DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oFile, L"wb" ) );
    WD_NULLCHECK( pFileDescriptor );

	*ppStream = DWFCORE_ALLOC_OBJECT(DWFFileOutputStream);
    WD_NULLCHECK( *ppStream );
	pFileDescriptor->open();
	((DWFFileOutputStream*)(*ppStream))->attach(pFileDescriptor, true);

    return WT_Result::Success;
}

