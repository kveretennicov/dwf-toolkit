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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/PropertyReference.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/PropertyReference.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFPropertyReference::DWFPropertyReference( const DWFString& zID )
throw()
                    : _zID( zID )
{
}

_DWFTK_API
DWFPropertyReference::~DWFPropertyReference()
throw()
{
    ;
}

_DWFTK_API
void DWFPropertyReference::setReference( const DWFString& zName,
                                         const DWFString& zCategory,
                                         const DWFString& zLabel)
throw( DWFException )
{
    if (zName.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A non-null property name must be provided");
    }

    _zName                = zName;
    _zCategory            = zCategory;
    _zLabel               = zLabel;
    _zPropertySetID       = /*NOXLATE*/L"";
    _zPropertySchemaID    = /*NOXLATE*/L"";
}

_DWFTK_API
void DWFPropertyReference::setReferenceWithSetID( const DWFString& zPropertySetID,
                                                  const DWFString& zName,
                                                  const DWFString& zCategory,
                                                  const DWFString& zLabel)
throw( DWFException )
{
    if (zPropertySetID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A non-null property set id must be provided");
    }

    if (zName.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A non-null property name must be provided");
    }

    _zPropertySetID       = zPropertySetID;
    _zName                = zName;
    _zCategory            = zCategory;
    _zLabel               = zLabel;
    _zPropertySchemaID    = /*NOXLATE*/L"";
}


_DWFTK_API
void DWFPropertyReference::setReferenceWithSchemaID( const DWFString& zPropertySchemaID,
                                                     const DWFString& zName,
                                                     const DWFString& zCategory,                                                     
                                                     const DWFString& zLabel)
throw( DWFException )
{
    if (zPropertySchemaID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A non-null property schema id must be provided");
    }

    if (zName.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A non-null property name must be provided");
    }

    _zPropertySchemaID    = zPropertySchemaID;
    _zName                = zName;
    _zCategory            = zCategory;
    _zLabel               = zLabel;
    _zPropertySetID       = /*NOXLATE*/L"";
}

_DWFTK_API
void
DWFPropertyReference::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    const char* pSetId      = NULL;
    const char* pSchemaId   = NULL;
    const char* pName       = "";
    const char* pCategory   = "";
    const char* pLabel      = "";

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

            //
            // Extract the id field.
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x01;
            setID( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the set id
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SetID) == 0))
        {
            nFound |= 0x02;
            pSetId = ppAttributeList[iAttrib+1];
        }
            //
            // Extract the schema id.
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SchemaID) == 0))
        {
            nFound |= 0x04;
            pSchemaId = ppAttributeList[iAttrib+1];
        }
            //
            // Extract the name field.
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x08;
            pName = ppAttributeList[iAttrib+1];
        }
            //
            // Extract the category
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Category) == 0))
        {
            nFound |= 0x10;
            pCategory = ppAttributeList[iAttrib+1];
        }
            //
            // Extract the label field.
            //
        else if (!(nFound & 0x20) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Label) == 0))
        {
            nFound |= 0x20;
            pLabel = ppAttributeList[iAttrib+1];
        }
    }

    if (pSetId != NULL)
    {
        setReferenceWithSetID( pSetId, pName, pCategory, pLabel );
    }
    else if (pSchemaId != NULL)
    {
        setReferenceWithSchemaID( pSchemaId, pName, pCategory, pLabel );
    }
    else
    {
        setReference( pName, pCategory, pLabel );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFPropertyReference::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
            //
            // if the derived class has already open the element
            // we can only serialize the base attributes here
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_PropertyReference );
        }

            //
            // add attributes
            //
        {
            //
            // assign a new id if there isn't one
            //
            if (_zID.chars() == 0)
            {
                _zID.assign(rSerializer.nextUUID(true));
            }

            rSerializer.addAttribute( DWFXML::kzAttribute_ID,       _zID );
            rSerializer.addAttribute( DWFXML::kzAttribute_SetID,    _zPropertySetID );
            rSerializer.addAttribute( DWFXML::kzAttribute_SchemaID, _zPropertySchemaID );
            rSerializer.addAttribute( DWFXML::kzAttribute_Name,     _zName );
            rSerializer.addAttribute( DWFXML::kzAttribute_Category, _zCategory );
            rSerializer.addAttribute( DWFXML::kzAttribute_Label,    _zLabel );
        }

        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
}

#endif
