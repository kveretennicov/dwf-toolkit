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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationResource.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/MIME.h"
#include "dwfcore/UUID.h"
#include "dwfcore/Pointer.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/presentation/ContentPresentationResource.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

    //
    // use this as a starting point for the document serialization
    //
#define _DWFTK_CONTENT_PRESENTATION_RESOURCE_INITIAL_BUFFER_BYTES  16384




_DWFTK_API
DWFContentPresentationResource::DWFContentPresentationResource( DWFPackageReader*  pPackageReader )
throw()
                              : DWFResource( pPackageReader )
#ifndef DWFTK_READ_ONLY
                              , _oAddedNamespaces()
#endif
                              , _zType()
                              , _pBuffer( NULL )
{
    _bSerialized = false;
}

_DWFTK_API
DWFContentPresentationResource::DWFContentPresentationResource( const DWFString& zRole )
throw()
                              : DWFResource( /*NOXLATE*/L"",
                                            zRole,
                                            DWFMIME::kzMIMEType_XML,
                                            /*NOXLATE*/L"" )
#ifndef DWFTK_READ_ONLY
                              , _oAddedNamespaces()
#endif
                              , _zType()
                              , _pBuffer( NULL )
{
    _bSerialized = false;
}

_DWFTK_API
DWFContentPresentationResource::~DWFContentPresentationResource()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
DWFInputStream*
DWFContentPresentationResource::getInputStream( bool )
throw( DWFException )
{
    //
    // if we have not processed the input stream yet then just get the stream from the base class
    //
	DWFInputStream * pInputStream = DWFResource::getInputStream();
    DWFContentPresentation::tList::Iterator* piPresentations = getPresentations();
    if (piPresentations == NULL || !piPresentations->valid())
    {
        DWFCORE_FREE_OBJECT( piPresentations );
        return pInputStream;
    }
	else if( _bSerialized && pInputStream != NULL )
	{
		return pInputStream;
	}
    else
    {
        if (pInputStream != NULL )
        {
            DWFCORE_FREE_OBJECT(pInputStream);
        }

        DWFCORE_FREE_OBJECT( piPresentations );

        //
        // create a buffered stream for serialization
        //
        DWFPointer<DWFBufferOutputStream> apOutputStream( DWFCORE_ALLOC_OBJECT(DWFBufferOutputStream(_DWFTK_CONTENT_PRESENTATION_RESOURCE_INITIAL_BUFFER_BYTES)), false );

        if (apOutputStream.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
        }

        //
        // create an XMLSerializer
        //
        DWFUUID oUUID;
        DWFPointer<DWFXMLSerializer> apSerializer( DWFCORE_ALLOC_OBJECT(DWFXMLSerializer(oUUID)), false );

        if (apSerializer.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate serializer" );
        }

        //
        // serialize
        //
        apSerializer->attach( apOutputStream );
        serializeXML( apSerializer, DWFPackageWriter::eContentPresentation );

        apSerializer->detach();
		
		//
		// The XML has already been serialized. So the next time we just need to get the 
		// stream
		//
		_bSerialized = true;

		//
		// free the buffer if it was previously allocated
		//
		if (_pBuffer)
		{
			DWFCORE_FREE_MEMORY( _pBuffer );
		}
		
		//
        // copy the stream buffer
        //
        size_t nBufferBytes = apOutputStream->buffer( (void**)&_pBuffer );

        //
        // create a buffered input stream for reading
        //
        DWFInputStream* pInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pBuffer, nBufferBytes) );

        if (pInputStream == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
        }

		//
        // return the document stream (we delete the buffer in our dtor)
        //
        return pInputStream;
    }
}


_DWFTK_API
const DWFXMLNamespace& 
DWFContentPresentationResource::addNamespace( const DWFString& zNamespace, 
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
DWFContentPresentationResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // full detailed dump into the content presentation document
        //
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFContentPresentationDocument::getSerializable().serializeXML( rSerializer, nFlags );
    }
    else if (nFlags & DWFPackageWriter::eDescriptor)
    {
        DWFString zNamespace;
        zNamespace.assign( namespaceXML(nFlags) );

        //
        //  Start element
        //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_ContentPresentationResource, zNamespace );
        }

        //
        //  Let base class know not to start the element
        //
        nFlags |= DWFXMLSerializer::eElementOpen;

        DWFContentPresentationContainer::getSerializable().serializeXML( rSerializer, nFlags );

        //
        //  Base attributes
        //
        DWFResource::serializeXML( rSerializer, nFlags );

        //
        //  Clear this bit
        //
        nFlags &= ~DWFXMLSerializer::eElementOpen;

        //
        //  Close element if this bit is not set
        //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }

        //
        // otherwise defer to the base class
        //
    else
    {
        DWFResource::serializeXML( rSerializer, nFlags );
    }
}

#endif

