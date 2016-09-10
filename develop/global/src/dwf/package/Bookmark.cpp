//
//  Copyright (c) 2003-2004 by Autodesk, Inc.  All rights reserved.
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
// $Header: /Components/Internal/DWF Toolkit/v7/develop/global/src/dwf/package/Bookmark.cpp 3     12/09/04 11:24p Evansg $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Bookmark.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
using namespace DWFToolkit;


_DWFTK_API
DWFBookmark::DWFBookmark()
throw()
           : _zName( )
           , _zHRef( )
           , _oChildren()
{
    ;
}

_DWFTK_API
DWFBookmark::DWFBookmark( const DWFString& zName,
                          const DWFString& zHRef )
throw()
           : _zName( zName )
           , _zHRef( zHRef )
           , _oChildren()
{
    ;
}

_DWFTK_API
DWFBookmark::~DWFBookmark()
throw()
{
    DWFBookmark::tList::iterator iChild = _oChildren.begin();
    for (; iChild != _oChildren.end(); iChild++)
    {
        DWFCORE_FREE_OBJECT( *iChild );
    }
}

_DWFTK_API
void
DWFBookmark::addChildBookmark( DWFBookmark* pBookmark )
throw()
{
    if (pBookmark)
    {
        _oChildren.push_back( pBookmark );
    }
}

_DWFTK_API
void
DWFBookmark::parseAttributeList( const char** ppAttributeList )
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
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFBookmark::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    DWFBookmark::tList::iterator iChild = _oChildren.begin();

    if (iChild != _oChildren.end())
    {
        DWFString zNamespace( namespaceXML(nFlags) );

        rSerializer.startElement( DWFXML::kzElement_Bookmark, zNamespace );
        {
            for (; iChild != _oChildren.end(); iChild++)
            {
                _serialize( **iChild, rSerializer, zNamespace );
            }
        }
        rSerializer.endElement();
    }
}

void
DWFBookmark::_serialize( DWFBookmark&                       rBookmark, 
                         DWFXMLSerializer&   rSerializer, 
                         const DWFString&                   zNamespace )
throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Bookmark, zNamespace );
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Name, rBookmark.name() );
        rSerializer.addAttribute( DWFXML::kzAttribute_HRef, rBookmark.href() );

        const DWFBookmark::tList& rChildren = rBookmark.getChildBookmarks();
        DWFBookmark::tList::const_iterator iChild = rChildren.begin();

        for (; iChild != rChildren.end(); iChild++)
        {
            _serialize( **iChild, rSerializer, zNamespace );
        }
    }
    rSerializer.endElement();
}

#endif
