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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/DWFProperties.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/DWFProperties.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;


//DNT_Start

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const char* const DWFXDWFProperties::kzName  = "DWFProperties.xml";

#endif


_DWFTK_API
DWFXDWFProperties::DWFXDWFProperties()
throw( DWFException )
{
    setSchemaID( DWFXXML::kzSchemaID_DWFProperties );
}

_DWFTK_API
DWFXDWFProperties::~DWFXDWFProperties()
throw()
{
    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXDWFProperties::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( DWFXXML::kzElement_DWFProperties );

    //
    //  This class ultimately derives from PropertyContainer. Hence to get access to the
    //  serialization, we must call getSerializable().
    //
    getSerializable().serializeXML( rSerializer, DWFXMLSerializer::eElementOpen );

    rSerializer.endElement();
}

#endif

_DWFTK_API
void
DWFXDWFProperties::setSourceProductVendor( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_SourceProductVendor, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::sourceProductVendor()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_SourceProductVendor );
}

_DWFTK_API
void
DWFXDWFProperties::setSourceProductName( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_SourceProductName, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::sourceProductName()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_SourceProductName );
}

_DWFTK_API
void
DWFXDWFProperties::setSourceProductVersion( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_SourceProductVersion, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::sourceProductVersion()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_SourceProductVersion );
}

_DWFTK_API
void
DWFXDWFProperties::setDWFProductVendor( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_DWFProductVendor, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::dwfProductVendor()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_DWFProductVendor );
}

_DWFTK_API
void
DWFXDWFProperties::setDWFProductVersion( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_DWFProductVersion, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::dwfProductVersion()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_DWFProductVersion );
}

_DWFTK_API
void
DWFXDWFProperties::setDWFToolkitVersion( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_DWFToolkitVersion, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::dwfToolkitVersion()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_DWFToolkitVersion );
}

_DWFTK_API
void
DWFXDWFProperties::setDWFFormatVersion( const DWFString& zValue )
throw()
{
    addProperty( DWFXML::kzDWFProperty_DWFFormatVersion, zValue );
}

_DWFTK_API
DWFString
DWFXDWFProperties::dwfFormatVersion()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_DWFFormatVersion );
}



_DWFTK_API
void
DWFXDWFProperties::setPasswordProtected( const DWFString& zValue )
throw()
{
	addProperty( DWFXML::kzDWFProperty_PasswordEncryptedDocument, zValue );
}

_DWFTK_API	
DWFString
DWFXDWFProperties::passwordProtected()
throw()
{
    return _getProperty( DWFXML::kzDWFProperty_PasswordEncryptedDocument );
}

DWFString
DWFXDWFProperties::_getProperty( const DWFString& zPropertyName )
throw()
{
    const DWFProperty* const pProperty = findProperty( zPropertyName, L"" );

    if ((pProperty != NULL) &&
        (pProperty->value().bytes() > 0))
    {
        return pProperty->value();
    }
    else
    {
        return L"";
    }
}


_DWFTK_API
DWFString
DWFXDWFProperties::provideSourceProductVendor( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setSourceProductVendor( zValue );
    }

    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFProperties::provideSourceProductName( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setSourceProductName( zValue );
    }

    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFProperties::provideSourceProductVersion( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setSourceProductVersion( zValue );
    }

    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFProperties::provideDWFProductVendor( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setDWFProductVendor( zValue );
    }

    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFProperties::provideDWFProductVersion( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setDWFProductVersion( zValue );
    }

    return zValue;
}

_DWFTK_API
DWFString
DWFXDWFProperties::provideDWFToolkitVersion( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setDWFToolkitVersion( zValue );
    }

    return zValue;
}   

_DWFTK_API
DWFString
DWFXDWFProperties::provideDWFFormatVersion( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setDWFFormatVersion( zValue );
    }

    return zValue;
}   


_DWFTK_API
DWFString
DWFXDWFProperties::providePasswordEncrypted( const DWFString& zValue )
throw()
{
    if (zValue.bytes() > 0)
    {
        setPasswordProtected( zValue );
    }

    return zValue;
} 


//DNT_End

