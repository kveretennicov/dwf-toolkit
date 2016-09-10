//
//  Copyright 2004-2006 by Autodesk, Inc.  All rights reserved.
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


#include "dwfcore/MIME.h"
#include "dwfcore/UUID.h"
#include "dwfcore/Pointer.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/package/ObjectDefinitionResource.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;


    //
    // use this as a starting point for the document serialization
    //
#define _DWFTK_OBJECT_DEFINITION_RESOURCE_INITIAL_BUFFER_BYTES  16384




_DWFTK_API
DWFObjectDefinitionResource::DWFObjectDefinitionResource( const DWFString& zType,
                                                          const DWFString& zRole )
throw()
                            : DWFResource( /*NOXLATE*/L"",
                                            zRole,
                                            DWFMIME::kzMIMEType_XML,
                                            /*NOXLATE*/L"" )
#ifndef DWFTK_READ_ONLY
                            , _oAddedNamespaces()
#endif
                            , _zType( zType )
                            , _pBuffer( NULL )
{
    ;
}

_DWFTK_API
DWFObjectDefinitionResource::~DWFObjectDefinitionResource()
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
DWFObjectDefinitionResource::getInputStream( bool )
throw( DWFException )
{
    //
    // create a buffered stream for serialization
    //
    DWFPointer<DWFBufferOutputStream> apOutputStream( DWFCORE_ALLOC_OBJECT(DWFBufferOutputStream(_DWFTK_OBJECT_DEFINITION_RESOURCE_INITIAL_BUFFER_BYTES)), false );

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
    serializeXML( apSerializer, DWFPackageWriter::eObjectDefinition );
    apSerializer->detach();

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
    DWFBufferInputStream* pInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pBuffer, nBufferBytes) );

    if (pInputStream == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
    }

    //
    // return the document stream (we delete the buffer in our dtor)
    //
    return pInputStream;
}


_DWFTK_API
const DWFXMLNamespace& 
DWFObjectDefinitionResource::addNamespace( const DWFString& zNamespace, 
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
DWFObjectDefinitionResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // full detailed dump into object definition document
        //
    if (nFlags & DWFPackageWriter::eObjectDefinition)
    {
        DWFString zNamespace;

        if (nFlags & DWFPackageWriter::eDescriptor)
        {
            zNamespace.assign( namespaceXML(nFlags) );
        }

        rSerializer.startElement( _zType, zNamespace );
        {
            //
            // other namespaces added for extension
            //
            _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.begin();
            for (; iNamespace != _oAddedNamespaces.end(); iNamespace++)
            {
                rSerializer.addAttribute( iNamespace->second.prefix(), iNamespace->second.xmlns(), /*NOXLATE*/L"xmlns:" );
            }
                
            DWFPropertyContainer::getSerializable().serializeXML( rSerializer, nFlags );
            getSerializableObjectContainer().serializeXML( rSerializer, nFlags );
            getSerializableInstanceContainer().serializeXML( rSerializer, nFlags );
        }
        rSerializer.endElement();
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

