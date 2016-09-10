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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/reader/CustomPropertiesReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/reader/CustomPropertiesReader.h"
#include "dwf/package/Constants.h"
#include "dwf/package/Property.h"
using namespace DWFToolkit;

//DNT_Start

_DWFTK_API
DWFXCustomPropertiesReader::DWFXCustomPropertiesReader()
throw()
                       : _oDefaultElementBuilder()
                       , _pElementBuilder( &_oDefaultElementBuilder )
                       , _pFilter( NULL )
                       , _pCurrentProperty( NULL )
{
    ;
}

_DWFTK_API
DWFXCustomPropertiesReader::~DWFXCustomPropertiesReader()
throw()
{
    ;
}

_DWFTK_API
DWFXCustomPropertiesReader*
DWFXCustomPropertiesReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFXCustomPropertiesReader::setFilter( DWFXCustomPropertiesReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
void
DWFXCustomPropertiesReader::notifyStartElement( const char*  zName,
                                                const char** ppAttributeList )
throw()
{
    _pCurrentProperty = NULL;
    if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0)
    {
        _pCurrentProperty = _pElementBuilder->buildProperty( ppAttributeList );
    }
}

_DWFTK_API
void
DWFXCustomPropertiesReader::notifyEndElement( const char* /*zName*/ )
throw()
{
    if (_pCurrentProperty)
    {
        provideProperty( _pFilter ? _pFilter->provideProperty(_pCurrentProperty) : _pCurrentProperty );    
    }
    _pCurrentProperty = NULL;
}

_DWFTK_API
void
DWFXCustomPropertiesReader::notifyStartNamespace( const char* /*zPrefix*/,
                                                  const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFXCustomPropertiesReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFXCustomPropertiesReader::notifyCharacterData( const char* /*zCData*/, 
                                                 int         /*nLength*/ ) 
throw()
{
}

_DWFTK_API
DWFProperty*
DWFXCustomPropertiesReader::provideProperty( DWFProperty* pProperty )
throw()
{
    return pProperty;
}

//DNT_End

