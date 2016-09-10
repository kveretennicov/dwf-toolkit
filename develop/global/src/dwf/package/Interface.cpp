//
//  Copyright (c) 2003-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/package/Interface.cpp 8     8/11/05 7:56p Kuk $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Interface.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
using namespace DWFToolkit;

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const wchar_t* const DWFInterface::kzEPlot_Name    = L"ePlot";
const wchar_t* const DWFInterface::kzEPlot_HRef    = L"http://www.autodesk.com/viewers";
const wchar_t* const DWFInterface::kzEPlot_ID      = L"715941D4-1AC2-4545-8185-BC40E053B551";

const wchar_t* const DWFInterface::kzEModel_Name   = L"eModel";
const wchar_t* const DWFInterface::kzEModel_HRef   = L"http://www.autodesk.com/viewers";
const wchar_t* const DWFInterface::kzEModel_ID     = L"75E513A9-6C41-4C91-BAA6-81E593FAAC10";

const wchar_t* const DWFInterface::kzData_Name   = L"Data";
const wchar_t* const DWFInterface::kzData_HRef   = L"http://www.autodesk.com/viewers";
const wchar_t* const DWFInterface::kzData_ID     = L"12D1F3EE-1178-4C8F-A706-F27E156F7522";

const wchar_t* const DWFInterface::kzSignatures_Name   = L"Signatures";
const wchar_t* const DWFInterface::kzSignatures_HRef   = L"http://www.autodesk.com/viewers";
const wchar_t* const DWFInterface::kzSignatures_ID     = L"22D1F3EE-1178-4C8F-A706-F27E156F7522";
//DNT_End

#endif

_DWFTK_API
DWFInterface::DWFInterface()
throw()
{
    ;
}

_DWFTK_API
DWFInterface::DWFInterface( const DWFString& zName,
                            const DWFString& zHRef,
                            const DWFString& zObjectID )
throw()
            : _zName( zName )
            , _zHRef( zHRef )
            , _zObjectID( zObjectID )
{
    ;
}

_DWFTK_API
DWFInterface::~DWFInterface()
throw()
{
    ;
}

_DWFTK_API
void
DWFInterface::parseAttributeList( const char** ppAttributeList )
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
        //
        // skip over any "dwf:" in the attribute name
        //
        pAttrib = (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0) ?
                  &ppAttributeList[iAttrib][4] :
                  &ppAttributeList[iAttrib][0];
            //
            // set the name
            //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x01;

            _zName.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the href
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_HRef) == 0))
        {
            nFound |= 0x02;

            _zHRef.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the object id
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
        {
            nFound |= 0x04;

            _zObjectID.assign( ppAttributeList[iAttrib+1] );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFInterface::serializeXML( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Interface, DWFXML::kzNamespace_DWF );

    rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
    rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zName );
    rSerializer.addAttribute( DWFXML::kzAttribute_HRef, _zHRef );

    rSerializer.endElement();
}

#endif

