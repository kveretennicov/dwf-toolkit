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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/reader/DWFPropertiesReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/reader/DWFPropertiesReader.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;

//DNT_Start

_DWFTK_API
DWFXDWFPropertiesReader::DWFXDWFPropertiesReader( unsigned int nProviderFlags )
throw()
                       : _nProviderFlags( nProviderFlags )
                       , _pFilter( NULL )
{
    ;
}

_DWFTK_API
DWFXDWFPropertiesReader::~DWFXDWFPropertiesReader()
throw()
{
    ;
}

_DWFTK_API
DWFXDWFPropertiesReader*
DWFXDWFPropertiesReader::filter() const
throw()
{
    return _pFilter;
}

_DWFTK_API
void
DWFXDWFPropertiesReader::setFilter( DWFXDWFPropertiesReader* pFilter )
throw()
{
    _pFilter = pFilter;
}

_DWFTK_API
void
DWFXDWFPropertiesReader::notifyStartElement( const char*  zName,
                                             const char** ppAttributeList )
throw()
{
    if (DWFCORE_COMPARE_ASCII_STRINGS(zName, DWFXML::kzElement_Property) == 0)
    {
        _provideProperty( ppAttributeList );
    }
}

_DWFTK_API
void
DWFXDWFPropertiesReader::notifyEndElement( const char* /*zName*/ )
throw()
{
    ;
}

_DWFTK_API
void
DWFXDWFPropertiesReader::notifyStartNamespace( const char* /*zPrefix*/,
                                               const char* /*zURI*/ )
throw()
{
}

_DWFTK_API
void
DWFXDWFPropertiesReader::notifyEndNamespace( const char* /*zPrefix*/ )
throw()
{
}

_DWFTK_API
void
DWFXDWFPropertiesReader::notifyCharacterData( const char* /*zCData*/, 
                                              int         /*nLength*/ ) 
throw()
{
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideSourceProductVendor( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideSourceProductName( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideSourceProductVersion( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideDWFProductVendor( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideDWFProductVersion( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideDWFToolkitVersion( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFPropertiesReader::provideDWFFormatVersion( const DWFString& zValue )
throw()
{
    return zValue;
}

_DWFTK_API
DWFString 
DWFXDWFPropertiesReader::providePasswordEncrypted( const DWFString& zValue )
throw()
{
	return zValue;
}

void DWFXDWFPropertiesReader::_provideProperty( const char** ppAttributeList )
throw()
{
    const char* pName = NULL;
    const char* pValue = NULL;

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;
    for (; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x01;

            pName = ppAttributeList[iAttrib+1];
        }
            //
            // set the value
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Value) == 0))
        {
            nFound |= 0x02;

            pValue = ppAttributeList[iAttrib+1];
        }
    }

    if (pName != NULL &&
        pValue != NULL)
    {
        DWFString zValue(pValue);

        if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_SourceProductVendor) == 0)
        {
            provideSourceProductVendor( _pFilter ? _pFilter->provideSourceProductVendor( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_SourceProductName) == 0)
        {
            provideSourceProductName( _pFilter ? _pFilter->provideSourceProductName( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_SourceProductVersion) == 0)
        {
            provideSourceProductVersion( _pFilter ? _pFilter->provideSourceProductVersion( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_DWFProductVendor) == 0)
        {
            provideDWFProductVendor( _pFilter ? _pFilter->provideDWFProductVendor( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_DWFProductVersion) == 0)
        {
            provideDWFProductVersion( _pFilter ? _pFilter->provideDWFProductVersion( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_DWFToolkitVersion) == 0)
        {
            provideDWFToolkitVersion( _pFilter ? _pFilter->provideDWFToolkitVersion( zValue ) : zValue );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_DWFFormatVersion) == 0)
        {
            provideDWFFormatVersion( _pFilter ? _pFilter->provideDWFFormatVersion( zValue ) : zValue );
        }
		else if (DWFCORE_COMPARE_ASCII_STRINGS(pName, DWFXML::kzDWFProperty_PasswordEncryptedDocument) == 0)
        {
            providePasswordEncrypted( _pFilter ? _pFilter->providePasswordEncrypted( zValue ) : zValue );
        }
    }
}

//DNT_End

