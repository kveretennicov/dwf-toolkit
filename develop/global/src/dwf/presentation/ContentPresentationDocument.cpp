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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationDocument.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/MIME.h"
#include "dwfcore/UUID.h"
#include "dwfcore/Pointer.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/presentation/ContentPresentationDocument.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

    //
    // use this as a starting point for the document serialization
    //
#define _DWFTK_CONTENT_PRESENTATION_RESOURCE_INITIAL_BUFFER_BYTES  16384




_DWFTK_API
DWFContentPresentationDocument::DWFContentPresentationDocument( DWFPackageReader*  pPackageReader )
throw()
                              : DWFContentPresentationReader( pPackageReader )
                              , _zType()
                              , _pBuffer( NULL )
                              , _nVersion( _DWF_FORMAT_CONTENT_PRESENTATION_VERSION_CURRENT_FLOAT )
                              , _zHRef()
                              , _nProviderFlags( DWFContentPresentationReader::eProvideAll )
                              , _oIDToNode()
                              , _oNodes()
                              , _oIDToView()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFContentPresentationDocument::DWFContentPresentationDocument()
throw()
                              : _zType()
                              , _pBuffer( NULL )
                              , _nVersion( _DWF_FORMAT_CONTENT_PRESENTATION_VERSION_CURRENT_FLOAT )
                              , _zHRef()
                              , _nProviderFlags( DWFContentPresentationReader::eProvideAll )
                              , _oIDToNode()
                              , _oNodes()
                              , _oIDToView()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFContentPresentationDocument::~DWFContentPresentationDocument()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }
}


_DWFTK_API
double
DWFContentPresentationDocument::provideVersion( double nVersion)
throw()
{
    return nVersion;
}

_DWFTK_API
DWFContentPresentation*
DWFContentPresentationDocument::providePresentation( DWFContentPresentation* pPresentation )
throw()
{
    addPresentation( pPresentation );
    return pPresentation;
}

_DWFTK_API
DWFContentPresentationView*
DWFContentPresentationDocument::provideView( DWFContentPresentationView* pView )
throw()
{
    _oIDToView.insert( pView->id(), pView );
    return pView;
}

_DWFTK_API
DWFPropertyReference*
DWFContentPresentationDocument::providePropertyReference( DWFPropertyReference* pPropertyReference )
throw()
{
    return pPropertyReference;
}
    
_DWFTK_API
DWFContentPresentationNode*
DWFContentPresentationDocument::provideNode( DWFContentPresentationNode* pNode )
throw()
{
    _oNodes.insert( pNode );
    _oIDToNode.insert( pNode->id(), pNode );
    return pNode;
}

_DWFTK_API
DWFContentPresentationReferenceNode*
DWFContentPresentationDocument::provideReferenceNode( DWFContentPresentationReferenceNode* pReferenceNode )
throw()
{
    _oNodes.insert( pReferenceNode );
    _oIDToNode.insert( pReferenceNode->id(), pReferenceNode );
    return pReferenceNode;
}

_DWFTK_API
DWFContentPresentationModelViewNode*
DWFContentPresentationDocument::provideModelViewNode( DWFContentPresentationModelViewNode* pModelViewNode )
throw()
{
    _oNodes.insert( pModelViewNode );
    _oIDToNode.insert( pModelViewNode->id(), pModelViewNode );
    return pModelViewNode;
}

#ifndef DWFTK_READ_ONLY

void
DWFContentPresentationDocument::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // full detailed dump into the content presentation document
        //
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFString zNamespace;

        if (nFlags & DWFPackageWriter::eDescriptor)
        {
            zNamespace.assign( namespaceXML(nFlags) );
        }

        rSerializer.startElement( DWFXML::kzElement_ContentPresentation, zNamespace );
        {                
            wchar_t zTempBuffer[512] = {0};
            _DWFCORE_SWPRINTF( zTempBuffer, 512, /*NOXLATE*/L"%#0.2g", _pDocument->_nVersion );
            rSerializer.addAttribute( DWFXML::kzAttribute_Version, zTempBuffer );

            _pDocument->DWFContentPresentationContainer::getSerializable().serializeXML( rSerializer, nFlags );
        }
        rSerializer.endElement();
    }
}

#endif

