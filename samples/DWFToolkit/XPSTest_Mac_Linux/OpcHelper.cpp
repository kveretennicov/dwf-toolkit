#include "StdAfx.h"
#include "OpcHelper.h"
#include "dwf/XAML/XamlClassFactory.h"
#include "dwf/dwfx/Constants.h"
#include "dwfcore/Pointer.h"

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

OpcHelper::OpcHelper()
: _nPartCount(0)
, _pSection( NULL )
, _pStreamCopy( NULL )
{
}

OpcHelper::~OpcHelper()
{
}

void OpcHelper::setCurrentSection( DWFSection* pSection)
{
    _pSection = pSection;
}

//WT_OpcResourceMaterializer
WT_Result 
OpcHelper::getPartInputStream(
	const DWFCore::DWFString& rzPath, 
	DWFCore::DWFInputStream** ppStream)
{
    if (ppStream == NULL)
        return WT_Result::Toolkit_Usage_Error;

    XamlResourceInputStream* pStream = _oMap.find( rzPath )->second;
    XamlResourceInputStream* pStreamCopy = DWFCORE_ALLOC_OBJECT( XamlResourceInputStream( *pStream ) );
    *ppStream = pStreamCopy;
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

    if (_pSection == NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    if (_zNextOutputStreamPath.chars())
    {
        rzPath = _zNextOutputStreamPath;
        *ppStream = _pStreamCopy;
        _pStreamCopy = NULL;
        _zNextOutputStreamPath.assign( L"" );
        return WT_Result::Success;
    }

    XamlResourceInputStream* pXamlResourceInputStream = DWFCORE_ALLOC_OBJECT( XamlResourceInputStream() );
    WD_NULLCHECK( pXamlResourceInputStream );

    XamlResourceOutputStream* pXamlResourceOutputStream = DWFCORE_ALLOC_OBJECT( XamlResourceOutputStream(pXamlResourceInputStream) );
    WD_NULLCHECK( pXamlResourceOutputStream );

    DWFString zPartName;
    zPartName = _oUuidGen.next(false);
    zPartName.append( L"." );
    zPartName.append(DWFMIME::GetExtension( rzMimeType ));

    rzPath = L"";
    rzPath.append( zPartName );

    DWFResource* pResource = NULL;
    if (rzMimeType == L"application/vnd.ms-package.obfuscated-opentype")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFResource( L"Obfuscated Font", DWFXML::kzRole_Font, DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE, rzPath ));
    }
    else if (rzMimeType == L"application/vnd.ms-opentype")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFResource( L"Font", DWFXML::kzRole_Font, DWFMIME::kzMIMEType_TTF, rzPath ));
    }
    else if (rzMimeType == L"application/vnd.ms-package.xps-resourcedictionary+xml")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFResource( L"Remote Resource Dictionary", DWFXML::kzRole_Graphics2dDictionary, DWFMIME::kzMIMEType_REMOTERESOURCEDICTIONARY, rzPath ));
    }
    else if (rzMimeType == L"image/jpeg")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Image", DWFXML::kzRole_RasterReference, DWFMIME::kzMIMEType_JPG, rzPath ));
    }
    else if (rzMimeType == L"image/png")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Image", DWFXML::kzRole_RasterReference, DWFMIME::kzMIMEType_PNG, rzPath ));
    }
    else if (rzMimeType == L"image/tiff")
    {
        pResource = DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Image", DWFXML::kzRole_RasterReference, DWFMIME::kzMIMEType_TIFF, rzPath ));
    }
    else
    {
        WD_Assert( false ); //unknown mime type!
    }

    if ( pResource )
    {
        _addGfxRelationship( DWFXXML::GetRelationship( pResource->role() ), pResource );   
    }

    _oMap.insert( _tPathToStreamMap::value_type(rzPath, pXamlResourceInputStream) );

    pResource->setRequestedName( zPartName );
    pResource->setInputStream( pXamlResourceInputStream );
    *ppStream = pXamlResourceOutputStream;
    _pSection->addResource( pResource, true );
    return WT_Result::Success;
}

void OpcHelper::_addGfxRelationship(const DWFString& zRel, DWFResource* pTarget )
{
    if ( _pSection )
    {
        DWFPointer<DWFResourceContainer::ResourceIterator> pIt(
            _pSection->findResourcesByRole( DWFXML::kzRole_Graphics2d ), false );
        if ( !pIt.isNull() )
        {
            pIt->get()->addRelationship( pTarget, zRel );
        }
    }
}

#ifndef _DWFCORE_WIN32_SYSTEM
WT_Result 
OpcHelper::getSubsetFontName(
							 WT_String& /*zFontName*/,
							 WT_Integer32 /*nHeight*/,
							 const WT_String & /*zStr*/,
							 bool /*bForEmbeddedFontObject*/)
{
	return WT_Result::Success;
}
#endif

#if defined(_DWFCORE_WIN32_SYSTEM)
WT_Result 
OpcHelper::getSubsetFontName(
        WT_String& zFontName,
        WT_Integer32 nHeight,
        const WT_String &zStr,
        bool bForEmbeddedFontObject)
{
    //Load the font into a file
    HDC hdc = ::GetDC( GetDesktopWindow() );
    LOGFONTW logFont;
    ZeroMemory( &logFont, sizeof(logFont) );
    logFont.lfHeight = nHeight;
    wcscpy(logFont.lfFaceName, (const wchar_t*) zFontName.unicode());
    HFONT hfont = ::CreateFontIndirect(&logFont);
    if (hfont == NULL || hfont == INVALID_HANDLE_VALUE)
    {
        return WT_Result::Internal_Error;
    }
    HGDIOBJ holdFont = ::SelectObject(hdc, hfont);
    DWORD nBytesRequired = ::GetFontData( hdc, NULL, 0, NULL, 0);
    void* pFontBuffer = malloc( nBytesRequired );
    WD_NULLCHECK( pFontBuffer );
    DWORD nBytesRead = ::GetFontData(hdc, NULL, 0, pFontBuffer, nBytesRequired);
    WD_Assert( nBytesRequired == nBytesRead );
    ::SelectObject(hdc, holdFont);


    //Do the font subsetting
    unsigned char* puchFontPackageBuffer = NULL; //holds the subset
    unsigned long  ulFontPackageBufferSize = 0; //holds the buffer size
    unsigned long  ulBytesWritten = 0; //holds the number of bytes written (should be == buffer size)
    int retval = subsetFont( 
        pFontBuffer, 
        nBytesRead, 
        (const unsigned short*) zStr.unicode(), 
        (unsigned short) zStr.length(), 
        puchFontPackageBuffer, 
        ulFontPackageBufferSize, 
        ulBytesWritten );
    ::ReleaseDC( GetDesktopWindow(), hdc );

    WD_Assert( !retval );

    //Now, create a GUID and filename
    DWFString zObfuscatedFontFilename;
    DWFString zGuid = _oUuidGen.next(false);
    DWFString zPartName = zGuid;

#ifdef OBFUSCATE
    zPartName.append( L".odttf" );
#else
    zPartName.append( L".ttf" );
#endif
    zObfuscatedFontFilename.append( zPartName );

    zFontName = (const wchar_t*)zObfuscatedFontFilename;
    
    XamlResourceInputStream* pXamlResourceInputStream = DWFCORE_ALLOC_OBJECT( XamlResourceInputStream() );
    WD_NULLCHECK( pXamlResourceInputStream );

    XamlResourceOutputStream* pXamlResourceOutputStream = DWFCORE_ALLOC_OBJECT( XamlResourceOutputStream(pXamlResourceInputStream) );
    WD_NULLCHECK( pXamlResourceOutputStream );

    DWFBufferInputStream oReadFrom( puchFontPackageBuffer, ulBytesWritten );

    //Create a key
    wchar_t key[64];
    memset(key, 0, sizeof(key));
    parseKey( key, zGuid );
    
    //obfuscate the data as we write the data to a file
    fuscator( &oReadFrom, pXamlResourceOutputStream, key );

    if (bForEmbeddedFontObject)
    {
        _pStreamCopy = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream( pXamlResourceOutputStream->bytes() ) );
        _pStreamCopy->write( pXamlResourceOutputStream->buffer(), pXamlResourceOutputStream->bytes() );
    }

    DWFCORE_FREE_OBJECT( pXamlResourceOutputStream );

    DWFToolkit::DWFFontResource* pFontResource = NULL;
    //DWFToolkit::DWFResource* pResource = NULL;
	//Add DWFFontResource
	//
	int												iRequest(0);
	DWFToolkit::DWFFontResource::tePrivilege		ePrivilege(DWFToolkit::DWFFontResource::eEditable);
    DWFToolkit::DWFFontResource::teCharacterCode	eCharacterCode(DWFToolkit::DWFFontResource::eUnicode);
	DWFCore::DWFString								strCanonicalName(logFont.lfFaceName);
    DWFCore::DWFString								strLogFontName(logFont.lfFaceName);
#ifdef OBFUSCATE
    //pFontResource = DWFCORE_ALLOC_OBJECT( DWFToolkit::DWFResource( (wchar_t*)zFontName.unicode(), DWFXML::kzRole_Font, DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE, zObfuscatedFontFilename ));

    pFontResource = DWFCORE_ALLOC_OBJECT( DWFToolkit::DWFFontResource(	iRequest,
			ePrivilege,
			eCharacterCode,
			strCanonicalName,
			strLogFontName));
    pFontResource->setMIME(DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE);;
#else
    pFontResource = DWFCORE_ALLOC_OBJECT( DWFToolkit::DWFFontResource( (wchar_t*)zFontName.unicode(), DWFXML::kzRole_Font, DWFMIME::kzMIMEType_TTF, zObfuscatedFontFilename ));
    pFontResource->setMIME(DWFMIME::kzMIMEType_TTF);
#endif

    _oMap.insert( _tPathToStreamMap::value_type(zObfuscatedFontFilename, pXamlResourceInputStream) );
    pFontResource->setRequestedName( zPartName );
    pFontResource->setInputStream( pXamlResourceInputStream );
    _pSection->addResource( pFontResource, true );

    if (puchFontPackageBuffer != NULL)
    {
        free( puchFontPackageBuffer );
    }
    if (pFontBuffer != NULL)
    {
        free( pFontBuffer );
    }

    if (bForEmbeddedFontObject)
    {
        _zNextOutputStreamPath = (const wchar_t*)zFontName.unicode();
        //
        // copy this to a local file so that W2dTest can find it
        //
        XamlResourceInputStream oStreamCopy( *pXamlResourceInputStream );
        DWFFile oFile( _zNextOutputStreamPath );
        DWFStreamFileDescriptor oFD( oFile, L"w" );

        oFD.open();
        oFD.write( oStreamCopy._pBuffer, oStreamCopy._nBufferBytes );
    }
    return WT_Result::Success;
}
#endif