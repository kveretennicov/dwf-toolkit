//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/reader/DWFDocumentSequenceReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//


#include "dwf/dwfx/reader/DWFDocumentSequenceReader.h"
#include "dwf/dwfx/Constants.h"
using namespace DWFToolkit;


_DWFTK_API
DWFXDWFDocumentSequenceReader::DWFXDWFDocumentSequenceReader( unsigned int nProviderFlags )
throw()
                             : _oDefaultElementBuilder()
                             , _pElementBuilder( &_oDefaultElementBuilder )
                             , _nProviderFlags( nProviderFlags )
                             , _pFilter( NULL )
                             , _zCurrentURI()
{
    ;
}

_DWFTK_API
DWFXDWFDocumentSequenceReader::~DWFXDWFDocumentSequenceReader()
throw()
{
    ;
}

_DWFTK_API
DWFXDWFDocumentSequenceReader*
DWFXDWFDocumentSequenceReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::setFilter( DWFXDWFDocumentSequenceReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
DWFString
DWFXDWFDocumentSequenceReader::provideDocumentURI( const DWFString& zDocumentURI )
throw()
{
    return zDocumentURI;
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::notifyStartElement( const char*  zName,
                                                   const char** ppAttributeList )
throw()
{
    if ((_nProviderFlags & eProvideDocuments) && 
        (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXXML::kzElement_ManifestReference) == 0))
    {
        size_t iAttrib = 0;
        const char* pAttrib = NULL;

        _zCurrentURI.assign( /*NOXLATE*/L"" );
        for(; ppAttributeList[iAttrib]; iAttrib += 2)
        {
            pAttrib = &ppAttributeList[iAttrib][0];

            //
            // provide the version
            //
            if (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXXML::kzAttribute_Source) == 0)
            {
                _zCurrentURI.assign( ppAttributeList[iAttrib+1] );
            }
        }
    }
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::notifyEndElement( const char* zName )
throw()
{
    if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXXML::kzElement_ManifestReference) == 0)
    {
        _provideDocumentURI( _zCurrentURI );
    }
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                     const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFXDWFDocumentSequenceReader::notifyCharacterData( const char* /*zCData*/, 
                                                    int         /*nLength*/ ) 
throw()
{
}

void
DWFXDWFDocumentSequenceReader::_provideDocumentURI( const DWFString& zDocumentURI )
throw()
{
    provideDocumentURI( _pFilter ? _pFilter->provideDocumentURI(zDocumentURI) : zDocumentURI);
}


