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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentation.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/ContentPresentation.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFContentPresentation::kzID_PackageContents        = "mXbIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_ContentNavigation      = "mnbIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_Views                  = "m3bIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_DefinedViews			  = "m0bIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_Layers                 = "nHbIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_Table                  = "nXbIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_Animations             = "nYbIKdssFUOrLj3K7vZWHg";
const char* const DWFContentPresentation::kzID_Showmotions            = "nZbIKdssFUOrLj3K7vZWHg";
//DNT_End

#endif

_DWFTK_API
DWFContentPresentation::DWFContentPresentation( const DWFString& zLabel,
                                                const DWFString& zID )
    throw()
                          : _zID( zID )
                          , _zLabel( zLabel )
                          , _pParent( NULL )
{
    ;
}

_DWFTK_API
DWFContentPresentation::~DWFContentPresentation()
throw()
{
    //
    // requirement of the DWFOwnable contract
    //
    DWFOwnable::_notifyDelete();
}

_DWFTK_API
void
DWFContentPresentation::setID( const DWFString& zID )
throw( DWFException )
{
    if (zID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A null id cannot be assigned to a content presentation" );
    }

    _zID = zID;
}

_DWFTK_API
const DWFString&
DWFContentPresentation::id() const
throw()
{
    return _zID;
}

_DWFTK_API
void
DWFContentPresentation::setLabel( const DWFString& zLabel )
throw()
{
    _zLabel = zLabel;
}

_DWFTK_API
const DWFString&
DWFContentPresentation::label() const
throw()
{
    return _zLabel;
}

_DWFTK_API
void
DWFContentPresentation::parseAttributeList( const char** ppAttributeList )
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
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFContentPresentation::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        rSerializer.startElement( DWFXML::kzElement_Presentation );

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
        rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );
        rSerializer.addAttribute( DWFXML::kzAttribute_Label, _zLabel );

        //
        // views
        //
        DWFXMLSerializable& rSerializable = DWFContentPresentationViewContainer::getSerializable();
        rSerializable.serializeXML( rSerializer, nFlags );

        rSerializer.endElement();
    }
    else
    {
        //
        // views
        //
        DWFXMLSerializable& rSerializable = DWFContentPresentationViewContainer::getSerializable();
        rSerializable.serializeXML( rSerializer, nFlags );
    }
}

#endif
