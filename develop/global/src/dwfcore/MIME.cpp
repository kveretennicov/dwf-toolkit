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


#include "dwfcore/MIME.h"
using namespace DWFCore;


//
// Holds the singleton - will be deleted when this module unloads
//
DWFPointer<DWFMIME>             g_apDWFMIME( NULL, false );
DWFPointer<DWFMIME::_tList>     DWFMIME::_kapTypeMap( NULL, false );
DWFPointer<DWFMIME::_tMultiMap> DWFMIME::_kapExtensionMap( NULL, false );


#if defined(DWFCORE_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_START

const wchar_t* const DWFMIME::kzMIMEType_DWF1 =                         L"model/vnd.dwf";
const wchar_t* const DWFMIME::kzMIMEType_DWF2 =                         L"application/x-dwf";
const wchar_t* const DWFMIME::kzMIMEType_DWF3 =                         L"drawing/x-dwf";
const wchar_t* const DWFMIME::kzMIMEType_W2D =                          L"application/x-w2d";
const wchar_t* const DWFMIME::kzMIMEType_W3D =                          L"application/x-w3d";
const wchar_t* const DWFMIME::kzMIMEType_DSD =                          L"application/x-dsd";
const wchar_t* const DWFMIME::kzMIMEType_PIA =                          L"application/x-dwg-state";
const wchar_t* const DWFMIME::kzMIMEType_BMP =                          L"image/bmp";
const wchar_t* const DWFMIME::kzMIMEType_CALS =                         L"image/cals";
const wchar_t* const DWFMIME::kzMIMEType_GIF =                          L"image/gif";
const wchar_t* const DWFMIME::kzMIMEType_FLIC =                         L"image/flic";
const wchar_t* const DWFMIME::kzMIMEType_IG4 =                          L"image/ig4";
const wchar_t* const DWFMIME::kzMIMEType_JPG =                          L"image/jpeg";
const wchar_t* const DWFMIME::kzMIMEType_PCX =                          L"image/pcx";
const wchar_t* const DWFMIME::kzMIMEType_PICT =                         L"image/pict";
const wchar_t* const DWFMIME::kzMIMEType_PNG =                          L"image/png";
const wchar_t* const DWFMIME::kzMIMEType_RLC =                          L"image/rlc";
const wchar_t* const DWFMIME::kzMIMEType_TARGA =                        L"image/targa";
const wchar_t* const DWFMIME::kzMIMEType_TIFF =                         L"image/tiff";
const wchar_t* const DWFMIME::kzMIMEType_TXT =                          L"text/plain";
const wchar_t* const DWFMIME::kzMIMEType_XML =                          L"text/xml";
const wchar_t* const DWFMIME::kzMIMEType_FONT =                         L"application/x-font";
const wchar_t* const DWFMIME::kzMIMEType_RELS =                         L"application/vnd.openxmlformats-package.relationships+xml";
const wchar_t* const DWFMIME::kzMIMEType_DWFSEQ =                       L"application/vnd.adsk-package.dwfx-dwfdocumentsequence+xml";
const wchar_t* const DWFMIME::kzMIMEType_FDSEQ =                        L"application/vnd.ms-package.xps-fixeddocumentsequence+xml";
const wchar_t* const DWFMIME::kzMIMEType_FDOC =                         L"application/vnd.ms-package.xps-fixeddocument+xml";
const wchar_t* const DWFMIME::kzMIMEType_FPAGE =                        L"application/vnd.ms-package.xps-fixedpage+xml";
const wchar_t* const DWFMIME::kzMIMEType_COREPROPERTIES =               L"application/vnd.openxmlformats-package.core-properties+xml";
const wchar_t* const DWFMIME::kzMIMEType_FIXEDPAGEXML =                 L"application/vnd.adsk-package.dwfx-fixedpage+xml";
const wchar_t* const DWFMIME::kzMIMEType_TTF =                          L"application/vnd.ms-opentype";
const wchar_t* const DWFMIME::kzMIMEType_REMOTERESOURCEDICTIONARY =     L"application/vnd.ms-package.xps-resourcedictionary+xml";
const wchar_t* const DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE =           L"application/vnd.ms-package.obfuscated-opentype";

const wchar_t* const DWFMIME::kzExtension_DWF =     L"dwf";
const wchar_t* const DWFMIME::kzExtension_W2D =     L"w2d";
const wchar_t* const DWFMIME::kzExtension_W3D =     L"w3d";
const wchar_t* const DWFMIME::kzExtension_DSD =     L"dsd";
const wchar_t* const DWFMIME::kzExtension_PIA =     L"pia";
const wchar_t* const DWFMIME::kzExtension_BMP =     L"bmp";
const wchar_t* const DWFMIME::kzExtension_CALS =    L"cals";
const wchar_t* const DWFMIME::kzExtension_FLIC =    L"flc";
const wchar_t* const DWFMIME::kzExtension_GIF =     L"gif";
const wchar_t* const DWFMIME::kzExtension_IG4 =     L"ig4";
const wchar_t* const DWFMIME::kzExtension_JPG =     L"jpg";
const wchar_t* const DWFMIME::kzExtension_PCX =     L"pcx";
const wchar_t* const DWFMIME::kzExtension_PICT =    L"pct";
const wchar_t* const DWFMIME::kzExtension_PNG =     L"png";
const wchar_t* const DWFMIME::kzExtension_RLC =     L"rlc";
const wchar_t* const DWFMIME::kzExtension_TARGA =   L"tga";
const wchar_t* const DWFMIME::kzExtension_TIFF =    L"tif";
const wchar_t* const DWFMIME::kzExtension_TXT =     L"txt";
const wchar_t* const DWFMIME::kzExtension_XML =     L"xml";
const wchar_t* const DWFMIME::kzExtension_FONT =    L"ef_";
const wchar_t* const DWFMIME::kzExtension_RELS =    L"rels";
const wchar_t* const DWFMIME::kzExtension_DWFSEQ =  L"dwfseq";
const wchar_t* const DWFMIME::kzExtension_FDSEQ =   L"fdseq";
const wchar_t* const DWFMIME::kzExtension_FDOC =    L"fdoc";
const wchar_t* const DWFMIME::kzExtension_FPAGE =   L"fpage";
const wchar_t* const DWFMIME::kzExtension_TTF =     L"ttf";
const wchar_t* const DWFMIME::kzExtension_ODTTF =   L"odttf";

const wchar_t* const DWFMIME::kzMIMEType_W2D_S    = L"application/x-w2d-s";
const wchar_t* const DWFMIME::kzMIMEType_W3D_S    = L"application/x-w3d-s";
const wchar_t* const DWFMIME::kzMIMEType_DSD_S    = L"application/x-dsd-s";
const wchar_t* const DWFMIME::kzMIMEType_PIA_S    = L"application/x-dwg-state-s";
const wchar_t* const DWFMIME::kzMIMEType_BMP_S    = L"image/bmp-s";
const wchar_t* const DWFMIME::kzMIMEType_CALS_S   = L"image/cals-s";
const wchar_t* const DWFMIME::kzMIMEType_GIF_S    = L"image/gif-s";
const wchar_t* const DWFMIME::kzMIMEType_FLIC_S   = L"image/flic-s";
const wchar_t* const DWFMIME::kzMIMEType_IG4_S    = L"image/ig4-s";
const wchar_t* const DWFMIME::kzMIMEType_JPG_S    = L"image/jpeg-s";
const wchar_t* const DWFMIME::kzMIMEType_PCX_S    = L"image/pcx-s";
const wchar_t* const DWFMIME::kzMIMEType_PICT_S   = L"image/pict-s";
const wchar_t* const DWFMIME::kzMIMEType_PNG_S    = L"image/png-s";
const wchar_t* const DWFMIME::kzMIMEType_RLC_S    = L"image/rlc-s";
const wchar_t* const DWFMIME::kzMIMEType_TARGA_S  = L"image/targa-s";
const wchar_t* const DWFMIME::kzMIMEType_TIFF_S   = L"image/tiff-s";
const wchar_t* const DWFMIME::kzMIMEType_TXT_S    = L"text/plain-s";
const wchar_t* const DWFMIME::kzMIMEType_XML_S    = L"text/xml-s";
const wchar_t* const DWFMIME::kzMIMEType_FONT_S   = L"application/x-font-s";
const wchar_t* const DWFMIME::kzMIMEType_FPAGE_S                    = L"application/vnd.ms-package.xps-fixedpage+xml-s";
const wchar_t* const DWFMIME::kzMIMEType_TTF_S                      = L"application/vnd.ms-opentype-s";
const wchar_t* const DWFMIME::kzMIMEType_REMOTERESOURCEDICTIONARY_S = L"application/vnd.ms-package.xps-resourcedictionary+xml-s";
const wchar_t* const DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE_S       = L"application/vnd.ms-package.obfuscated-opentype-s";
const wchar_t* const DWFMIME::kzMIMEType_FIXEDPAGEXML_S             = L"application/vnd.adsk-package.dwfx-fixedpage+xml-s";
                                                                    
const wchar_t* const DWFMIME::kzExtension_W2D_S =     L"w2d_s";
const wchar_t* const DWFMIME::kzExtension_W3D_S =     L"w3d_s";
const wchar_t* const DWFMIME::kzExtension_DSD_S =     L"dsd_s";
const wchar_t* const DWFMIME::kzExtension_PIA_S =     L"pia_s";
const wchar_t* const DWFMIME::kzExtension_BMP_S =     L"bmp_s";
const wchar_t* const DWFMIME::kzExtension_CALS_S =    L"cals_s";
const wchar_t* const DWFMIME::kzExtension_FLIC_S =    L"flc_s";
const wchar_t* const DWFMIME::kzExtension_GIF_S =     L"gif_s";
const wchar_t* const DWFMIME::kzExtension_IG4_S =     L"ig4_s";
const wchar_t* const DWFMIME::kzExtension_JPG_S =     L"jpg_s";
const wchar_t* const DWFMIME::kzExtension_PCX_S =     L"pcx_s";
const wchar_t* const DWFMIME::kzExtension_PICT_S =    L"pct_s";
const wchar_t* const DWFMIME::kzExtension_PNG_S =     L"png_s";
const wchar_t* const DWFMIME::kzExtension_RLC_S =     L"rlc_s";
const wchar_t* const DWFMIME::kzExtension_TARGA_S =   L"tga_s";
const wchar_t* const DWFMIME::kzExtension_TIFF_S =    L"tif_s";
const wchar_t* const DWFMIME::kzExtension_TXT_S =     L"txt_s";
const wchar_t* const DWFMIME::kzExtension_XML_S =     L"xml_s";
const wchar_t* const DWFMIME::kzExtension_FONT_S =    L"ef__s";
const wchar_t* const DWFMIME::kzExtension_FPAGE_S =    L"fpage_s";
const wchar_t* const DWFMIME::kzExtension_TTF_S =    L"ttf_s";
const wchar_t* const DWFMIME::kzExtension_ODTTF_S =    L"odttf_s";

//DNT_END
#endif


DWFMIME::DWFMIME()
throw()
{
    ;
}

_DWFCORE_API
DWFIterator<const wchar_t* const>*
DWFMIME::GetMIMEType( const wchar_t* const zExtension )
throw( DWFException )
{
        //
        // create the map on first access
        //
    if (g_apDWFMIME.isNull())
    {
        _Build();
    }

    return DWFCORE_ALLOC_OBJECT( _Iterator(_kapExtensionMap->lower_bound(zExtension), _kapExtensionMap->upper_bound(zExtension)) );
}

_DWFCORE_API
const wchar_t* const
DWFMIME::GetExtension( const wchar_t* const zMIMEType )
throw( DWFException )
{
        //
        // create the map on first access
        //
    if (g_apDWFMIME.isNull())
    {
        _Build();
    }

    const wchar_t** pzExtension = _kapTypeMap->find( zMIMEType );
    
    return (pzExtension ? *pzExtension : NULL);
}

_DWFCORE_API
bool
DWFMIME::CompressByDefault( const wchar_t* const zMIMEType )
throw( DWFException )
{
    if ((DWFCORE_COMPARE_WIDE_STRINGS(zMIMEType, DWFMIME::kzMIMEType_DWF2) == 0) ||
        (DWFCORE_COMPARE_WIDE_STRINGS(zMIMEType, DWFMIME::kzMIMEType_JPG) == 0)  ||
        (DWFCORE_COMPARE_WIDE_STRINGS(zMIMEType, DWFMIME::kzMIMEType_PNG) == 0)
        )
    {
        return false;
    }
    else
    {
        return true;
    }
}

void
DWFMIME::_Build()
throw( DWFException )
{
        g_apDWFMIME = DWFCORE_ALLOC_OBJECT( DWFMIME );
        if (g_apDWFMIME.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
        }

        _kapExtensionMap = DWFCORE_ALLOC_OBJECT( DWFMIME::_tMultiMap );
        if (_kapExtensionMap.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
        }

        _kapTypeMap = DWFCORE_ALLOC_OBJECT( DWFMIME::_tList );
        if (_kapTypeMap.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
        }

        //
        // extension -> MIME
        //
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_DWF,   kzMIMEType_DWF1) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_DWF,   kzMIMEType_DWF2) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_DWF,   kzMIMEType_DWF3) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_W2D,   kzMIMEType_W2D) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_W3D,   kzMIMEType_W3D)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_DSD,   kzMIMEType_DSD)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_PIA,   kzMIMEType_PIA)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_BMP,   kzMIMEType_BMP) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_CALS,  kzMIMEType_CALS) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FLIC,  kzMIMEType_FLIC) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_GIF,   kzMIMEType_GIF) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_IG4,   kzMIMEType_IG4)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_JPG,   kzMIMEType_JPG) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_PCX,   kzMIMEType_PCX)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_PICT,  kzMIMEType_PICT)  );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_PNG,   kzMIMEType_PNG) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_RLC,   kzMIMEType_RLC) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_TARGA, kzMIMEType_TARGA) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_TIFF,  kzMIMEType_TIFF) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_TXT,   kzMIMEType_TXT) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_XML,   kzMIMEType_XML) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FONT,  kzMIMEType_FONT) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_RELS,  kzMIMEType_RELS) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_DWFSEQ,kzMIMEType_DWFSEQ) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FDSEQ, kzMIMEType_FDSEQ) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FDOC,  kzMIMEType_FDOC) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FPAGE, kzMIMEType_FPAGE) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_TTF,   kzMIMEType_TTF) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_XML,   kzMIMEType_REMOTERESOURCEDICTIONARY) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_ODTTF, kzMIMEType_OBFUSCATEDOPENTYPE) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_FPAGE_S, kzMIMEType_FIXEDPAGEXML_S) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_TTF_S, kzMIMEType_TTF_S) );
        _kapExtensionMap->insert( _tMultiMap::value_type(kzExtension_ODTTF_S, kzMIMEType_OBFUSCATEDOPENTYPE_S) );

        //
        // MIME -> extension
        //
        _kapTypeMap->insert( kzMIMEType_DWF1,                       kzExtension_DWF );
        _kapTypeMap->insert( kzMIMEType_DWF2,                       kzExtension_DWF );
        _kapTypeMap->insert( kzMIMEType_DWF3,                       kzExtension_DWF );
        _kapTypeMap->insert( kzMIMEType_W2D,                        kzExtension_W2D );
        _kapTypeMap->insert( kzMIMEType_W3D,                        kzExtension_W3D );
        _kapTypeMap->insert( kzMIMEType_DSD,                        kzExtension_DSD );
        _kapTypeMap->insert( kzMIMEType_PIA,                        kzExtension_PIA );
        _kapTypeMap->insert( kzMIMEType_BMP,                        kzExtension_BMP );
        _kapTypeMap->insert( kzMIMEType_CALS,                       kzExtension_CALS );
        _kapTypeMap->insert( kzMIMEType_FLIC,                       kzExtension_FLIC );
        _kapTypeMap->insert( kzMIMEType_GIF,                        kzExtension_GIF );
        _kapTypeMap->insert( kzMIMEType_IG4,                        kzExtension_IG4 );
        _kapTypeMap->insert( kzMIMEType_JPG,                        kzExtension_JPG );
        _kapTypeMap->insert( kzMIMEType_PCX,                        kzExtension_PCX );
        _kapTypeMap->insert( kzMIMEType_PICT,                       kzExtension_PICT );
        _kapTypeMap->insert( kzMIMEType_PNG,                        kzExtension_PNG );
        _kapTypeMap->insert( kzMIMEType_RLC,                        kzExtension_RLC );
        _kapTypeMap->insert( kzMIMEType_TARGA,                      kzExtension_TARGA );
        _kapTypeMap->insert( kzMIMEType_TIFF,                       kzExtension_TIFF );
        _kapTypeMap->insert( kzMIMEType_TXT,                        kzExtension_TXT );
        _kapTypeMap->insert( kzMIMEType_XML,                        kzExtension_XML );
        _kapTypeMap->insert( kzMIMEType_FONT,                       kzExtension_FONT );
        _kapTypeMap->insert( kzMIMEType_RELS,                       kzExtension_RELS );
        _kapTypeMap->insert( kzMIMEType_DWFSEQ,                     kzExtension_DWFSEQ );
        _kapTypeMap->insert( kzMIMEType_FDSEQ,                      kzExtension_FDSEQ );
        _kapTypeMap->insert( kzMIMEType_FDOC,                       kzExtension_FDOC );
        _kapTypeMap->insert( kzMIMEType_FPAGE,                      kzExtension_FPAGE );
        _kapTypeMap->insert( kzMIMEType_TTF,                        kzExtension_TTF );
        _kapTypeMap->insert( kzMIMEType_REMOTERESOURCEDICTIONARY,   kzExtension_XML );
        _kapTypeMap->insert( kzMIMEType_OBFUSCATEDOPENTYPE,         kzExtension_ODTTF );

        _kapTypeMap->insert( kzMIMEType_W2D_S,   kzExtension_W2D_S );
        _kapTypeMap->insert( kzMIMEType_W3D_S,   kzExtension_W3D_S );
        _kapTypeMap->insert( kzMIMEType_DSD_S,   kzExtension_DSD_S );
        _kapTypeMap->insert( kzMIMEType_PIA_S,   kzExtension_PIA_S );
        _kapTypeMap->insert( kzMIMEType_BMP_S,   kzExtension_BMP_S );
        _kapTypeMap->insert( kzMIMEType_CALS_S,  kzExtension_CALS_S );
        _kapTypeMap->insert( kzMIMEType_FLIC_S,  kzExtension_FLIC_S );
        _kapTypeMap->insert( kzMIMEType_GIF_S,   kzExtension_GIF_S );
        _kapTypeMap->insert( kzMIMEType_IG4_S,   kzExtension_IG4_S );
        _kapTypeMap->insert( kzMIMEType_JPG_S,   kzExtension_JPG_S );
        _kapTypeMap->insert( kzMIMEType_PCX_S,   kzExtension_PCX_S );
        _kapTypeMap->insert( kzMIMEType_PICT_S,  kzExtension_PICT_S );
        _kapTypeMap->insert( kzMIMEType_PNG_S,   kzExtension_PNG_S );
        _kapTypeMap->insert( kzMIMEType_RLC_S,   kzExtension_RLC_S );
        _kapTypeMap->insert( kzMIMEType_TARGA_S, kzExtension_TARGA_S );
        _kapTypeMap->insert( kzMIMEType_TIFF_S,  kzExtension_TIFF_S );
        _kapTypeMap->insert( kzMIMEType_TXT_S,   kzExtension_TXT_S );
        _kapTypeMap->insert( kzMIMEType_XML_S,   kzExtension_XML_S );
        _kapTypeMap->insert( kzMIMEType_FONT_S,  kzExtension_FONT_S );
        _kapTypeMap->insert( kzMIMEType_FPAGE_S,                      kzExtension_FPAGE_S );
        _kapTypeMap->insert( kzMIMEType_TTF_S,                        kzExtension_TTF_S );
        _kapTypeMap->insert( kzMIMEType_REMOTERESOURCEDICTIONARY_S,   kzExtension_XML_S );
        _kapTypeMap->insert( kzMIMEType_OBFUSCATEDOPENTYPE_S,         kzExtension_ODTTF_S );
}
