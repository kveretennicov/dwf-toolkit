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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/ContentTypes.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/ContentTypes.h"
#include "dwf/opc/Constants.h"
using namespace DWFToolkit;



#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const OPCContentTypes::kzName = "[Content_Types].xml";
//DNT_End

#endif



_DWFTK_API
OPCContentTypes::OPCContentTypes()
throw()
    : _oExtensions()
    , _oOverrides()
{
    setPath( /*NOXLATE*/L"/" );
    setName( OPCContentTypes::kzName );
}

_DWFTK_API
OPCContentTypes::~OPCContentTypes()
throw()
{
    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
bool
OPCContentTypes::addContentType( const DWFString& zExtension, const DWFString& zType )
throw()
{
    //
    // check if we already have this extension
    //
    _tTypeMap::const_iterator it = _oExtensions.find( zExtension );

        //
        // if we have it check if the type is same as what is being added now
        //
    if (it != _oExtensions.end())
    {
        if (it->second == zType)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
        //
        // we don't have it, so add it
        //
    else
    {
        _oExtensions.insert( std::make_pair(zExtension, zType) );
        return true;
    }
}

_DWFTK_API
bool
OPCContentTypes::addOverride( const DWFString& zPartURI, const DWFString& zType )
throw()
{
    if (_oOverrides.count( zPartURI ) != 0)
    {
        return false;
    }
    else
    {
        _oOverrides.insert( std::make_pair(zPartURI, zType) );
        return true;
    }
}

_DWFTK_API
DWFString
OPCContentTypes::extensionType( const DWFString& zExtension ) const 
throw()
{
    _tTypeMap::const_iterator it = _oExtensions.find( zExtension );
    if (it != _oExtensions.end())
    {
        return it->second;
    }
    else
    {
        return /*NOXLATE*/L"";
    }
}

_DWFTK_API
DWFString
OPCContentTypes::partType( const DWFString& zPartURI ) const 
throw()
{
    _tTypeMap::const_iterator it = _oOverrides.find( zPartURI );
    if (it != _oOverrides.end())
    {
        return it->second;
    }
    else
    {
        off_t iOffset = zPartURI.findLast( /*NOXLATE*/L'.' );
        //
        //  If the '.' is at the end then we don't have an extension
        //
        if (iOffset != -1 &&
            size_t(iOffset+1) != zPartURI.chars())
        {
            size_t iExtLen = zPartURI.chars()-(iOffset+1);
            return extensionType( zPartURI.substring( iOffset+1, iExtLen) );
        }
        else
        {
            return /*NOXLATE*/L"";
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void OPCContentTypes::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( OPCXML::kzElement_Types );
    rSerializer.addAttribute( /*NOXLATE*/L"xmlns", OPCXML::kzNamespaceURI_ContentTypes );

    _tTypeMap::iterator itExtensions = _oExtensions.begin();
    for (; itExtensions != _oExtensions.end(); ++itExtensions)
    {
        rSerializer.startElement( OPCXML::kzElement_Default );
        rSerializer.addAttribute( OPCXML::kzAttribute_Extension, itExtensions->first );
        rSerializer.addAttribute( OPCXML::kzAttribute_ContentType, itExtensions->second );
        rSerializer.endElement();
    }

    _tTypeMap::iterator itOverrides = _oOverrides.begin();
    for (; itOverrides != _oOverrides.end(); ++itOverrides)
    {
        rSerializer.startElement( OPCXML::kzElement_Override );
        rSerializer.addAttribute( OPCXML::kzAttribute_PartName, itOverrides->first );
        rSerializer.addAttribute( OPCXML::kzAttribute_ContentType, itOverrides->second );
        rSerializer.endElement();
    }

    rSerializer.endElement();
}

#endif

