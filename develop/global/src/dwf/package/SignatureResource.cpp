//
//  (C) Copyright 2003-2004 by Autodesk, Inc.  All rights reserved.
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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/SignatureResource.cpp#1 $
//


#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/package/SignatureResource.h"
#include "dwf/package/Constants.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;


_DWFTK_API
DWFSignatureResource::DWFSignatureResource( DWFPackageReader* pPackageReader )
throw()
               : DWFResource( pPackageReader )
{
    ;
}

_DWFTK_API
DWFSignatureResource::DWFSignatureResource()
throw()
               : DWFResource( L"", DWFXML::kzRole_Signature, DWFMIME::kzMIMEType_XML )
{
    ;
}

_DWFTK_API
DWFSignatureResource::~DWFSignatureResource()
throw()
{
    ;
}

_DWFTK_API
void
DWFSignatureResource::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    //
    // parse with base resource first
    //
    DWFResource::parseAttributeList( ppAttributeList );

    // TODO: Add any special attribute processing here, if required.
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFSignatureResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    DWFString zNamespace;

        //
        // namespace dictated by document and section type
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        zNamespace.assign( namespaceXML(nFlags) );
    }

        //
        // serialize in detail for descriptor
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        rSerializer.startElement( DWFXML::kzElement_SignatureResource, zNamespace );

        //
        // let base class know not to start the element
        //
        nFlags |= DWFXMLSerializer::eElementOpen;

        //
        // base attributes
        //
        DWFResource::serializeXML( rSerializer, nFlags );

            //
            // attributes
            //
        {
            // TODO: ...
        }

        //
        // properties - as the most derived resource, we must invoke the
        // property serialization ourselves or we will end up with properties
        // in the middle of our resource element
        //
        DWFXMLSerializable& rSerializable = DWFPropertyContainer::getSerializable();
        rSerializable.serializeXML( rSerializer, nFlags );

            //
            // Serialize relationships.
            //
        if(!_oRelationships.empty())
        {
            DWFResourceRelationship::tList::Iterator *pIter = _oRelationships.iterator();
            if (pIter)
            {
                rSerializer.startElement( DWFXML::kzElement_Relationships, zNamespace );

                for (; pIter->valid(); pIter->next())
                {
                    DWFResourceRelationship *pRelationship = pIter->get();
                    if(pRelationship != NULL)
                    {
                        rSerializer.startElement( DWFXML::kzElement_Relationship, zNamespace );
                        rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, pRelationship->resourceID() );
                        rSerializer.addAttribute( DWFXML::kzAttribute_Type, pRelationship->type());
                        rSerializer.endElement();
                    }
                }
                DWFCORE_FREE_OBJECT( pIter );

                rSerializer.endElement();
            }
        }

        rSerializer.endElement();
    }

        //
        // Intentionally do nothing!
        //
    else if (nFlags & DWFPackageWriter::eManifest)
    {
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

