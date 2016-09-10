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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationView.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/ContentPresentationView.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationView::DWFContentPresentationView( const DWFString& zLabel,
                                                        const DWFString& zID )
throw()
                          : _zID( zID )
                          , _zLabel( zLabel )
                          , _bUseDefaultIfNotFound( false )
                          , _bHidden( false )
                          , _pParent( NULL )
{
   ; 
}

_DWFTK_API
DWFContentPresentationView::~DWFContentPresentationView()
throw()
{
    ;
}

_DWFTK_API
void
DWFContentPresentationView::setID( const DWFString& zID )
throw( DWFException )
{
    if (zID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A null id cannot be assigned to a content presentation view" );
    }

    _zID = zID;
}

_DWFTK_API
const DWFString&
DWFContentPresentationView::id() const
throw()
{
    return _zID;
}

_DWFTK_API
void
DWFContentPresentationView::setLabel( const DWFString& zLabel )
throw()
{
    _zLabel = zLabel;
}

_DWFTK_API
const DWFString&
DWFContentPresentationView::label() const
throw()
{
    return _zLabel;
}

_DWFTK_API
void
DWFContentPresentationView::setPropertySetID( const DWFString& zPropertySetID, bool bUseDefaultIfNotFound )
    throw( DWFException )
{
    if (zPropertySetID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Property Set ID cannot be NULL" );
    }
    _zPropertySetID = zPropertySetID;
    _zPropertySchemaID = /*NOXLATE*/L"";
    _bUseDefaultIfNotFound = bUseDefaultIfNotFound;
}

_DWFTK_API
const DWFString&
DWFContentPresentationView::propertySetID() const
    throw()
{
    return _zPropertySetID;
}

_DWFTK_API
void
DWFContentPresentationView::setPropertySchemaID( const DWFString& zPropertySchemaID, bool bUseDefaultIfNotFound )
    throw( DWFException )
{
    if (zPropertySchemaID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Property Schema ID cannot be NULL" );
    }
    _zPropertySchemaID = zPropertySchemaID;
    _zPropertySetID = /*NOXLATE*/L"";
    _bUseDefaultIfNotFound = bUseDefaultIfNotFound;
}

_DWFTK_API
const DWFString&
DWFContentPresentationView::propertySchemaID() const
    throw()
{
    return _zPropertySchemaID;
}

_DWFTK_API
void
DWFContentPresentationView::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;
    bool bUseDefaultIfNotFound = false;
    const char* pPropertySetID = NULL;
    const char* pPropertySchemaID = NULL;

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
            // Extract the label field.
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Label) == 0))
        {
            nFound |= 0x02;
            setLabel( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the hidden attribute
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Hidden) == 0))
        {
            nFound |= 0x04;
            if(DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"true") == 0)
            {
                setHidden( true );
            }
            else
            {
                setHidden( false );
            }
        }
            //
            // use default
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_UseDefaultPropertyIfMissing) == 0))
        {
            nFound |= 0x08;
            if(DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"true") == 0)
            {
                bUseDefaultIfNotFound = true;
            }
            else
            {
                bUseDefaultIfNotFound = false;
            }
        }
            //
            // property set id
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SetID) == 0))
        {
            nFound |= 0x10;
            pPropertySetID = ppAttributeList[iAttrib+1];
        }
            //
            // property schema id
            //
        else if (!(nFound & 0x20) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SchemaID) == 0))
        {
            nFound |= 0x20;
            pPropertySchemaID = ppAttributeList[iAttrib+1];
        }
    }

    if (nFound & 0x10)
    {
        if (nFound & 0x08)
        {
            setPropertySetID( pPropertySetID, bUseDefaultIfNotFound );
        }
        else
        {
            setPropertySetID( pPropertySetID );
        }
    }

    if (nFound & 0x20)
    {
        if (nFound & 0x08)
        {
            setPropertySchemaID( pPropertySchemaID, bUseDefaultIfNotFound );
        }
        else
        {
            setPropertySchemaID( pPropertySchemaID );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFContentPresentationView::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        rSerializer.startElement( DWFXML::kzElement_View );

            //
            // add attributes
            //
        //
        // assign a new id if there isn't one
        //
        if (_zID.chars() == 0)
        {
            _zID.assign(rSerializer.nextUUID(true));
        }
        rSerializer.addAttribute( DWFXML::kzAttribute_ID,                          _zID );
        rSerializer.addAttribute( DWFXML::kzAttribute_Label,                       _zLabel );
        rSerializer.addAttribute( DWFXML::kzAttribute_SetID,                       _zPropertySetID );
        rSerializer.addAttribute( DWFXML::kzAttribute_SchemaID,                    _zPropertySchemaID );

        //
        // if a property set or schema ID has been set, then serialize the
        // UseDefaultPropertyIfMissing
        //
        if (_zPropertySetID.chars() || _zPropertySchemaID.chars())
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_UseDefaultPropertyIfMissing,
                _bUseDefaultIfNotFound ? /*NOXLATE*/"true" : /*NOXLATE*/"false" );
        }

        if ( _bHidden )
        {
            //
            // default is false
            //
            rSerializer.addAttribute( DWFXML::kzAttribute_Hidden, /*NOXLATE*/"true");
        }

        //
        // Property references.
        //
        DWFPropertyReferenceContainer::getSerializable().serializeXML(rSerializer, nFlags);

        //
        // nodes
        //
        DWFContentPresentationNodeContainer::getSerializable().serializeXML(rSerializer, nFlags);

        rSerializer.endElement();
    }
    else
    {
        //
        // Property references.
        //
        DWFPropertyReferenceContainer::getSerializable().serializeXML(rSerializer, nFlags);

        //
        // nodes
        //
        DWFContentPresentationNodeContainer::getSerializable().serializeXML(rSerializer, nFlags);
    }
}

#endif

