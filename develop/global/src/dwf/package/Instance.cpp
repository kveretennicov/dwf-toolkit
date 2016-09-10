//
//  Copyright (c) 2005-2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Instance.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Instance.h"
#include "dwf/package/Constants.h" 
#include "dwf/package/Renderable.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

#define _DWFINSTANCE_DEFAULT_GEOMETRIC_VARIATION_INDEX -1

_DWFTK_API
DWFInstance::DWFInstance( const DWFString& zInstanceID,
                          DWFRenderable* pRenderedElement, 
                          int nNodeID,
                          unsigned int nAttributes,
                          int nGeometricVariationIndex )
throw( DWFException )
           : _zID( zInstanceID )
           , _pRenderedElement( pRenderedElement )
           , _nNodeID( nNodeID )
           , _nAttributeFlags( nAttributes )
           , _nGeometricVariationIndex( nGeometricVariationIndex )
{
    if (_nAttributeFlags > (eVisible | eTransparent))
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unknown graphics attributes provided to the instance constructor" );
    }
}


_DWFTK_API
DWFInstance::DWFInstance()
throw( ) // documented to throw none
           : _zID( /*NOXLATE*/L"" )
           , _pRenderedElement( NULL )
           , _nNodeID( 0 )
           , _nAttributeFlags( eVisible )
           , _nGeometricVariationIndex( _DWFINSTANCE_DEFAULT_GEOMETRIC_VARIATION_INDEX )
{}

_DWFTK_API
DWFInstance::~DWFInstance()
throw()
{
}

_DWFTK_API
void 
DWFInstance::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

        //
        // set the ID
        //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x01;
            _zID.assign( ppAttributeList[iAttrib+1] );
        }

        //
        // set the nodeID
        //
        if (!(nFound & 0x02) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Node) == 0))
        {
            nFound |= 0x02;
            _nNodeID = atoi( ppAttributeList[iAttrib+1] );
        }

        //
        // set visibility
        //
        if (!(nFound & 0x04) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Visible) == 0))
        {
            nFound |= 0x04;
            setVisibility( DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], "true") == 0 );
        }

        //
        // get transparency
        //
        if (!(nFound & 0x08) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Transparent) == 0))
        {
            nFound |= 0x08;
            setTransparency( DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], "true") == 0 );
        }

        //
        // get geometric variation index
        //
        if (!(nFound & 0x10) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_GeometricVariation) == 0))
        {
            nFound |= 0x10;
            _nGeometricVariationIndex = atoi( ppAttributeList[iAttrib+1] );           
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void 
DWFInstance::serializeXML( DWFXMLSerializer& rSerializer, 
                           unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eSectionContent)
    {
        rSerializer.startElement( DWFXML::kzElement_Instance, DWFXML::kzNamespace_DWF );
        rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );

        //
        //  Serialize reference to the rendered object/feature
        //
        if (_pRenderedElement)
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_RenderableRef, _pRenderedElement->id() );
        }
        else
        {
            _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The renderable reference in object should not be NULL" );
        }

        //
        //  Serialize the node id
        //
        rSerializer.addAttribute( DWFXML::kzAttribute_Node, _nNodeID );

        //
        //  Serialize the visibility flag only if its not the default - if its not true.
        //  Most instances will indeed be visible.
        //
        if (!getVisibility())
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Visible, /*NOXLATE*/L"false" );
        }

        //
        //  Serialize the transparency flag only if its not the default - if its not false.
        //  Most instances will indeed be opaque. (NOTE: This is not the same transparency 
        //  as material transparency).
        //
        if (getTransparency())
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Transparent, /*NOXLATE*/L"true" );
        }

        //
        //  Serialize the geometric variation index only in the few cases that it is not
        //  the default value of 0.
        //
        if (_nGeometricVariationIndex != _DWFINSTANCE_DEFAULT_GEOMETRIC_VARIATION_INDEX)
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_GeometricVariation, _nGeometricVariationIndex );
        }

        rSerializer.endElement();
    }
}

#endif

