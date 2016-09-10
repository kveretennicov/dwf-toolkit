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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/package/Units.cpp 5     8/11/05 7:56p Kuk $
//

#include "dwf/package/Units.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
#include "dwfcore/DWFXMLSerializer.h"
using namespace DWFToolkit;


_DWFTK_API
DWFUnits::DWFUnits()
throw()
{
    ;
}

_DWFTK_API
DWFUnits::~DWFUnits()
throw()
{
    ;
}

_DWFTK_API
DWFUnits::DWFUnits( DWFUnits::teType eType )
throw()
{
    switch (eType)
    {
        case eMillimeters:
        {
            _zType.assign( /*NOXLATE*/L"mm" );
            break;
        }
        case eCentimeters:
        {
            _zType.assign( /*NOXLATE*/L"cm" );
            break;
        }
        case eMeters:
        {
            _zType.assign( /*NOXLATE*/L"m" );
            break;
        }
        case eInches:
        {
            _zType.assign( /*NOXLATE*/L"in" );
            break;
        }
        case eFeet:
        {
            _zType.assign( /*NOXLATE*/L"ft" );
            break;
        }
    }
}

_DWFTK_API
void
DWFUnits::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
            //
            // skip over any acceptable prefixes in the element name
            //
        if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][4];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][8];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][6];
        }
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][7];
        }
        else
        {
            pAttrib = &ppAttributeList[iAttrib][0];
        }

            //
            // set the type
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x01;

            _zType.assign( ppAttributeList[iAttrib+1] );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFUnits::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Units, namespaceXML(nFlags) );
    rSerializer.addAttribute( DWFXML::kzAttribute_Type, _zType );
    rSerializer.endElement();
}

#endif

