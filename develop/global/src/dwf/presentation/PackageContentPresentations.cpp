//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/PackageContentPresentations.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/MIME.h"
#include "dwfcore/UUID.h"
#include "dwfcore/Pointer.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/presentation/PackageContentPresentations.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

    //
    // use this as a starting point for the document serialization
    //
#define _DWFTK_CONTENT_PRESENTATION_RESOURCE_INITIAL_BUFFER_BYTES  16384




_DWFTK_API
DWFPackageContentPresentations::DWFPackageContentPresentations( DWFPackageReader* pPackageReader )
throw()
#ifndef DWFTK_READ_ONLY
                               : _oAddedNamespaces()
                               , _zHRef()
#else
                               : _zHRef()
#endif
                               , _pPackageReader( pPackageReader )
                               , _bLoaded( false )
{
    ;
}

_DWFTK_API
DWFPackageContentPresentations::DWFPackageContentPresentations()
throw()
#ifndef DWFTK_READ_ONLY
                               : _oAddedNamespaces()
                               , _zHRef()
#else
                               : _zHRef()
#endif
                               , _pPackageReader( NULL )
                               , _bLoaded( false )
{
    ;
}

_DWFTK_API
DWFPackageContentPresentations::~DWFPackageContentPresentations()
throw()
{
}

_DWFTK_API
void
DWFPackageContentPresentations::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
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
            // set the href
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_HRef) == 0))
        {
            nFound |= 0x01;

            _zHRef.assign( ppAttributeList[iAttrib+1] );
        }
    }
}

_DWFTK_API
DWFInputStream*
DWFPackageContentPresentations::getInputStream()
throw( DWFException )
{
    return _pPackageReader->extract( href() );
}

_DWFTK_API
void DWFPackageContentPresentations::load( DWFContentPresentationReader* pReaderFilter )
throw( DWFException )
{
    //
    // if this presentation is being created then there is nothing to load
    //
    if (_pPackageReader == NULL)
    {
        return;
    }

    if (_bLoaded)
    {
        return;
    }

    //
    //  Create the input stream for the content file.
    //
    DWFPointer<DWFInputStream> apPresentationInput(getInputStream(), false );

    //
    //  Set the filter on the default reader
    //
    if (pReaderFilter)
    {
        setFilter( pReaderFilter );
    }

    //
    //  Do the actual parsing of the document
    //
    _parseDocument( *(DWFInputStream*)apPresentationInput, *this );

    _bLoaded = true;
}

_DWFTK_API
void DWFPackageContentPresentations::load( DWFContentPresentationReader& rCustomReader )
throw( DWFException )
{
    if (_pPackageReader == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"If a package reader is not set load cannot be called on the content" );
    }

    //
    //  Create the input stream for the content file.
    //
    DWFPointer<DWFInputStream> apPresentationInput(getInputStream(), false );

    _parseDocument( *(DWFInputStream*)apPresentationInput, rCustomReader );
}

_DWFTK_API
void
DWFPackageContentPresentations::_parseDocument( DWFInputStream& rDocumentStream,
                                                DWFXMLCallback& rDocumentReader )
throw( DWFException )
{
	DWFXMLParser docParser(&rDocumentReader);
    docParser.parseDocument( rDocumentStream );
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
const DWFXMLNamespace& 
DWFPackageContentPresentations::addNamespace( const DWFString& zNamespace, 
                                              const DWFString& zXMLNS )
throw( DWFException )
{
    _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.find( zNamespace );
    
    if (iNamespace != _oAddedNamespaces.end())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"This namespace has already been defined." );
    }

    _oAddedNamespaces[zNamespace] = DWFXMLNamespace( zNamespace, zXMLNS );

    return _oAddedNamespaces[zNamespace];
}

_DWFTK_API
void
DWFPackageContentPresentations::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eManifest)
    {
        if (presentationCount() > 0)
        {
            rSerializer.startElement( DWFXML::kzElement_Presentations, DWFXML::kzNamespace_DWF );

            DWFString zID = rSerializer.nextUUID( true );
            SetHRef( zID );
            rSerializer.addAttribute( DWFXML::kzAttribute_HRef, href() );
            
            rSerializer.endElement();
        }
    }
        //
        // full detailed dump into the content presentation document
        //
    else if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFContentPresentationDocument::getSerializable().serializeXML( rSerializer, nFlags );
    }
}

#endif

