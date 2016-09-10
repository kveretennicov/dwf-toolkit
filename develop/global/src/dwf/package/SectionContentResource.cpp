//
//  Copyright (c) 2005-2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/SectionContentResource.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//


#include "dwfcore/MIME.h"
#include "dwfcore/Pointer.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
using namespace DWFCore;

#include "dwf/Toolkit.h"
#include "dwf/Version.h"
#include "dwf/package/Content.h"
#include "dwf/package/Constants.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/package/writer/PackageWriter.h"
#include "dwf/package/reader/PackageReader.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/Manifest.h"
using namespace DWFToolkit;

//
// use this as a starting point for the document serialization
//
#define _DWFTK_SECTION_CONTENT_RESOURCE_INITIAL_BUFFER_BYTES  16384

_DWFTK_API
DWFSectionContentResource::DWFSectionContentResource( DWFContent* pContent )
throw( DWFException )
                         : DWFResource( /*NOXLATE*/L"", 
                                        DWFXML::kzRole_ContentDefinition,
                                        DWFMIME::kzMIMEType_XML )
                         , _pContent( pContent )
                         , _zVersion( _DWF_FORMAT_SECTION_CONTENT_VERSION_CURRENT_STRING )
                         , _pBuffer( NULL )
{
    if (pContent == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"An null content pointer was passed to the resource." );
    }

    addContentID( pContent->id() );

    setObjectID( pContent->getIDProvider()->next(true) );
    pContent->notifyContentResourceAdded( objectID() ); 
    pContent->own( *this );
	_bSerialized = false;
}

_DWFTK_API
DWFSectionContentResource::DWFSectionContentResource( DWFPackageReader* pPackageReader )
throw( DWFException )
                         : DWFResource( pPackageReader )
                         , _pContent( NULL )
                         , _zVersion( _DWF_FORMAT_SECTION_CONTENT_VERSION_CURRENT_STRING )
                         , _pBuffer( NULL )
{
	_bSerialized = false;
}

_DWFTK_API
DWFSectionContentResource::~DWFSectionContentResource()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }

    if (_pContent != NULL)
    {
        if (_pContent->owner() == this)
        {
            _pContent->disown( *this, true );
            DWFCORE_FREE_OBJECT( _pContent );
        }
        else
        {
            _pContent->unobserve( *this );
            _pContent = NULL;
        }
    }
}

_DWFTK_API
void
DWFSectionContentResource::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    DWFResource::parseAttributeList( ppAttributeList );

    //
    //  Now connect to the correct content corresponding to the content ID.
    //

    DWFString zContentID( contentID() );
    if (zContentID.chars()==0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Cannot create a section content resource without a valid content." );
    }

    DWFManifest& rManifest = _pPackageReader->getManifest();
    DWFContentManager* pContentManager = rManifest.getContentManager();
    if (pContentManager == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The manifest did not return a valid content manager." );
    }

    DWFContent* pContent = pContentManager->getContent( zContentID );
    if (pContent == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A content with the given ID was not found in the content manager" );
    }
    else
    {
        _pContent = pContent;
        _pContent->own( *this );
    }
}

_DWFTK_API
void
DWFSectionContentResource::setObjectID( const DWFString& zObjectID )
throw()
{
    if (_pContent)
    {
        _pContent->notifyResourceIDUpdated( objectID(), zObjectID );
    }
    DWFResource::setObjectID( zObjectID );
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
DWFInputStream*
DWFSectionContentResource::getInputStream( bool /*bCache*/ )
throw( DWFException )
{
	DWFInputStream * pInputStream = DWFResource::getInputStream();

    if (_pContent == NULL )
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Content object is null" );
    }

	if (!_pContent->isSectionContentResourceModified( objectID() ) || ( _bSerialized && pInputStream != NULL ))
    {
        return pInputStream;
    }
    else
    {
		if (pInputStream != NULL )
        {
            DWFCORE_FREE_OBJECT(pInputStream);
        }
        //
        // create a buffered stream for serialization
        //
        DWFPointer<DWFBufferOutputStream> apOutputStream( DWFCORE_ALLOC_OBJECT(DWFBufferOutputStream(_DWFTK_SECTION_CONTENT_RESOURCE_INITIAL_BUFFER_BYTES)), false );

        if (apOutputStream.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
        }

        //
        // create an XMLSerializer
        //
        DWFUUID* pUUID = _pContent->getIDProvider();
        DWFPointer<DWFXMLSerializer> apSerializer( DWFCORE_ALLOC_OBJECT(DWFXMLSerializer(*pUUID)), false );

        if (apSerializer.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate serializer" );
        }

        //
        // serialize
        //
        apSerializer->attach( apOutputStream );
        serializeXML( apSerializer, DWFPackageWriter::eSectionContent );
        apSerializer->detach();
		
		//
		// the XML has already been constructed and serialized
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
void
DWFSectionContentResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eSectionContent)
    {
        //
        //  Generate the section content document element with the version information
        //
        rSerializer.startElement( DWFXML::kzElement_SectionContent, namespaceXML( nFlags ) );
        
        DWFString zTempString( /*NOXLATE*/L"DWF-ContentResource:" );
        zTempString.append( _zVersion );
        rSerializer.addAttribute( /*NOXLATE*/L"dwf", zTempString, /*NOXLATE*/L"xmlns:" );

        zTempString.append( /*NOXLATE*/L" " );
        zTempString.append( /*NOXLATE*/_DWF_FORMAT_SECTION_CONTENT_SCHEMA_LOCATION_WIDE_STRING );
        rSerializer.addAttribute( DWFXML::kzAttribute_SchemaLocation, zTempString, /*NOXLATE*/L"xmlns:" );
        rSerializer.addAttribute( DWFXML::kzAttribute_Version, _zVersion );

        {
            //
            //  Let the content know which section is being serialized 
            //
            _pContent->setResourceForSerialization( objectID() );
            //
            //  Serialize the body of the section content document
            //
            _pContent->serializeXML( rSerializer, nFlags );
            //
            //  Cleanup the serializing section reference in the content
            //
            _pContent->setResourceForSerialization( /*NOXLATE*/L"" );
        }

        //
        //  Close the section content document element
        //
        rSerializer.endElement();

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
            rSerializer.startElement( DWFXML::kzElement_Resource, zNamespace );
        }

        //
        //  Let base class know not to start the element
        //
        nFlags |= DWFXMLSerializer::eElementOpen;

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

    else if (nFlags & DWFPackageWriter::eManifest)
    {
        //
        //  Let the base perform the serialization into the manifest
        //
        DWFResource::serializeXML( rSerializer, nFlags );
    }
}

#endif

_DWFTK_API
DWFContent*
DWFSectionContentResource::getContent() const
throw()
{
    return _pContent;
}

void DWFSectionContentResource::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    if (&rOwnable == _pContent && _pContent != NULL)
    {
        _pContent->unobserve( *this );
        _pContent = NULL;
    }

    DWFResource::notifyOwnableDeletion( rOwnable );
}
