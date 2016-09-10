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
// $Header: /Components/Internal/DWF Toolkit/v7/develop/global/src/dwf/package/Source.cpp 3     12/09/04 11:24p Evansg $
//

#include "dwf/package/Source.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
#include "dwfcore/DWFXMLSerializer.h"
using namespace DWFToolkit;


_DWFTK_API
DWFSource::DWFSource()
throw()
{
    ;
}

_DWFTK_API
DWFSource::DWFSource( const DWFString& zHRef,
                      const DWFString& zProvider,
                      const DWFString& zObjectID )
throw()
         : _zHRef( zHRef )
         , _zProvider( zProvider )
         , _zObjectID( zObjectID )
{
    ;
}

_DWFTK_API
DWFSource::~DWFSource()
throw()
{
    ;
}

_DWFTK_API
DWFSource::DWFSource( const DWFSource& rSource )
throw()
         : _zHRef( rSource._zHRef )
         , _zProvider( rSource._zProvider )
         , _zObjectID( rSource._zObjectID )
{
}

_DWFTK_API
DWFSource&
DWFSource::operator=( const DWFSource& rSource )
throw()
{
    _zHRef = rSource._zHRef;
    _zProvider = rSource._zProvider;
    _zObjectID = rSource._zObjectID;

    return *this;
}

_DWFTK_API
void
DWFSource::parseAttributeList( const char** ppAttributeList )
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
            // set the href
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_HRef) == 0))
        {
            nFound |= 0x01;

            _zHRef.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the provider
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Provider) == 0))
        {
            nFound |= 0x02;

            _zProvider.assign( ppAttributeList[iAttrib+1] );
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
DWFSource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
throw( DWFException )
{
    //
    // root element
    //
    rSerializer.startElement( DWFXML::kzElement_Source, DWFXML::kzNamespace_DWF );
    {
            //
            // attributes
            //
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Provider, _zProvider );
            rSerializer.addAttribute( DWFXML::kzAttribute_HRef, _zHRef );
            rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
        }
    }
    rSerializer.endElement();
}

#endif
