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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/SectionDescriptorReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//



#include "dwf/package/Constants.h"
#include "dwf/package/reader/SectionDescriptorReader.h"
using namespace DWFToolkit;




_DWFTK_API
DWFSectionDescriptorReader::DWFSectionDescriptorReader( DWFPackageReader*   pPackageReader,
                                                        unsigned char       nProviderFlags )
throw()
                          : _oDefaultElementBuilder()
                          , _pElementBuilder( NULL )
                          , _pPackageReader( pPackageReader )
                          , _nProviderFlags( nProviderFlags )
                          , _pFilter( NULL )
{
    _pElementBuilder = &_oDefaultElementBuilder;
}

_DWFTK_API
DWFSectionDescriptorReader::~DWFSectionDescriptorReader()
throw()
{
    ;
}

_DWFTK_API
DWFSectionDescriptorReader*
DWFSectionDescriptorReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFSectionDescriptorReader::setFilter( DWFSectionDescriptorReader* pFilter )
throw()
{
    _pFilter = pFilter;
}
    
_DWFTK_API
bool
DWFSectionDescriptorReader::provideType( const char* /*zType*/ )
throw()
{
    return true;
}

_DWFTK_API
void
DWFSectionDescriptorReader::_provideAttributes( const char** /*ppAttributeList*/ )
throw()
{
    ;
}

_DWFTK_API
void
DWFSectionDescriptorReader::notifyStartElement( const char*   zName,
                                                const char**  ppAttributeList )
throw()
{
        //
        //
        //
    if (_nElementDepth == 0)
    {
            //
            // post the type of descriptor being processed
            // NOTE: this element name is provided as-is (with any prefix)!
            //
        if (_nProviderFlags & eProvideType)
        {
            _provideType( zName );
        }

            //
            // evaluate attribute list only if asked
            // NOTE: attributes names ara provided as-is (with any prefix)!
            //
        if (_nProviderFlags & eProvideAttributes)
        {
            _provideAttributes( ppAttributeList );
        }
    }
}

_DWFTK_API
void
DWFSectionDescriptorReader::notifyEndElement( const char* /*zName*/ )
throw()
{
}

_DWFTK_API
void
DWFSectionDescriptorReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                  const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFSectionDescriptorReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFSectionDescriptorReader::notifyCharacterData( const char* /*zCData*/, 
                                                 int         /*nLength*/ ) 
throw()
{
}

void
DWFSectionDescriptorReader::_provideType( const char* zType )
throw()
{
    bool bProvideThis = true;

    if (_pFilter)
    {
        bProvideThis = _pFilter->provideType( zType );
    }
    
    if (bProvideThis)
    {
        provideType( zType );
    }
}

_DWFTK_API
DWFString
DWFSectionDescriptorReader::_findAttributeValue( const char*  zAttributeName,
                                                 const char** ppAttributeList )
{
    size_t iAttrib = 0;
    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        if (DWFCORE_COMPARE_ASCII_STRINGS(zAttributeName, ppAttributeList[iAttrib]) == 0)
        {
            return DWFString( ppAttributeList[iAttrib+1] );
            break;
        } 
    }

    return /*NOXLATE*/L"";
}
