//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/RelationshipContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwfcore/BufferInputStream.h"
#include "dwfcore/XMLParser.h"
using namespace DWFCore;

#include "dwf/opc/RelationshipContainer.h"
#include "dwf/opc/Constants.h"
#include "dwf/opc/Part.h"
using namespace DWFToolkit;


//
// use this as a starting point for the document serialization
//
#define _DWFTK_OPC_RELATIONSHIPCONTAINER_INITIAL_BUFFER_BYTES  16384


_DWFTK_API
OPCRelationshipContainer::OPCRelationshipContainer()
throw()
                        : OPCRelationshipContainerReader()
                        , _pBuffer( NULL )
                        , _oRelationships()
{;}

_DWFTK_API
OPCRelationshipContainer::~OPCRelationshipContainer()
throw()
{
    if (_oRelationships.size() > 0)
    {
        OPCRelationship::tIterator* piRels = _oRelationships.iterator();
        if (piRels != NULL)
        {
            while (piRels->valid())
            {
                OPCRelationship* pRel = piRels->get();
                DWFCORE_FREE_OBJECT( pRel );
                piRels->next();
            }
            DWFCORE_FREE_OBJECT( piRels );
        }
    }

    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }
}



_DWFTK_API
OPCRelationship* 
OPCRelationshipContainer::addRelationship( OPCPart* pTargetPart, 
                                           const DWFString& zRelationshipType, 
                                           OPCRelationship::teTargetMode eTargetMode )
throw( DWFException )
{
    if (pTargetPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No target part specified when defining a relationship" );
    }

    if (zRelationshipType.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No relationship type specified when defining a relationship" );
    }

    OPCRelationship* pRel = DWFCORE_ALLOC_OBJECT( OPCRelationship( pTargetPart, zRelationshipType, eTargetMode ) );
    if (pRel == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate relationship." );
    }
    _oRelationships.push_back( pRel );

    return pRel;
}


_DWFTK_API
bool 
OPCRelationshipContainer::deleteRelationship( OPCRelationship* pRelationship )
throw()
{
    if (_oRelationships.erase( pRelationship ))
    {
        DWFCORE_FREE_OBJECT( pRelationship );
        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
size_t
OPCRelationshipContainer::relationshipCount() const
throw()
{
    return _oRelationships.size();
}

_DWFTK_API
OPCRelationship::tIterator*
OPCRelationshipContainer::relationships()
throw()
{
    if (_oRelationships.size() > 0)
    {
        return _oRelationships.iterator();
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
OPCRelationship::tIterator*
OPCRelationshipContainer::relationshipsByTarget( const DWFString& zTargetURI ) const
throw()
{
    OPCRelationship::tList oTargetRels;

    if (_oRelationships.size() > 0)
    {

        OPCRelationship::tList::ConstIterator* piRels = _oRelationships.constIterator();
        if (piRels != NULL)
        {
            while (piRels->valid())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel->target()->uri() == zTargetURI)
                {
                    oTargetRels.push_back( pRel );
                }
                piRels->next();
            }
            DWFCORE_FREE_OBJECT( piRels );
        }
    }

    if (oTargetRels.size() > 0)
    {
        return oTargetRels.iterator();
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
OPCRelationship::tIterator* 
OPCRelationshipContainer::relationshipsByTarget( OPCPart* pTargetPart ) const
throw()
{
    OPCRelationship::tList oTargetRels;

    if (_oRelationships.size() > 0)
    {

        OPCRelationship::tList::ConstIterator* piRels = _oRelationships.constIterator();
        if (piRels != NULL)
        {
            while (piRels->valid())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel->target() == pTargetPart)
                {
                    oTargetRels.push_back( pRel );
                }
                piRels->next();
            }
            DWFCORE_FREE_OBJECT( piRels );
        }
    }

    if (oTargetRels.size() > 0)
    {
        return oTargetRels.iterator();
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
OPCRelationship::tIterator*
OPCRelationshipContainer::relationshipsByType( const DWFString& zType ) const
throw()
{
    OPCRelationship::tList oTargetRels;

    if (_oRelationships.size() > 0)
    {

        OPCRelationship::tList::ConstIterator* piRels = _oRelationships.constIterator();
        if (piRels != NULL)
        {
            while (piRels->valid())
            {
                OPCRelationship* pRel = piRels->get();
                if (pRel->relationshipType() == zType)
                {
                    oTargetRels.push_back( pRel );
                }
                piRels->next();
            }
            DWFCORE_FREE_OBJECT( piRels );
        }
    }

    if (oTargetRels.size() > 0)
    {
        return oTargetRels.iterator();
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
void
OPCRelationshipContainer::deleteRelationshipsByTarget( OPCPart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A valid part must be provided" );
    }

    OPCRelationship::tIterator* piRels = relationshipsByTarget( pPart );
    for ( ; piRels && piRels->valid(); piRels->next() )
    {
        OPCRelationship* pRelationship = piRels->get();
        deleteRelationship( pRelationship );
    }

    DWFCORE_FREE_OBJECT( piRels );
}

_DWFTK_API
DWFInputStream*
OPCRelationshipContainer::getInputStream()
throw( DWFException )
{
#ifdef DWFTK_READ_ONLY
    // TODO: Decide the best thing to do here. Should we:
    //          - return NULL?
    //          - throw an exception?
    //          - conditionally remove the function?
    return NULL;
#else

    //
    // create a buffered stream for serialization
    //
    DWFPointer<DWFBufferOutputStream> apOutputStream( DWFCORE_ALLOC_OBJECT(DWFBufferOutputStream(_DWFTK_OPC_RELATIONSHIPCONTAINER_INITIAL_BUFFER_BYTES)), false );

    if (apOutputStream.isNull())
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream" );
    }

    //
    // TODO: Need to implement someway to share a single UUID generator across multiple parts
    //       without passing around arguments through getInputStream.
    //

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
    serializeRelationship( apSerializer );
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
#endif
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
OPCRelationshipContainer::serializeRelationship( DWFXMLSerializer& rSerializer )
throw()
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( OPCXML::kzElement_Relationships );
    rSerializer.addAttribute( /*NOXLATE*/"xmlns", OPCXML::kzNamespaceURI_Relationsips );

    OPCRelationship::tIterator* piRels = _oRelationships.iterator();
    if (piRels != NULL)
    {
        while (piRels->valid())
        {
            OPCRelationship* pRel = piRels->get();

            if (pRel != NULL)
            {
                pRel->serializeXML( rSerializer );
            }
            piRels->next();
        }
        DWFCORE_FREE_OBJECT( piRels );
    }

    rSerializer.endElement();
}

#endif

_DWFTK_API
void
OPCRelationshipContainer::loadRelationships(DWFInputStream* pRelsInputStream)
throw( DWFException )
{
    if(relationshipCount() > 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"This RelationshipsContainer already contains Relationships." );
    }

    DWFXMLParser manifestRelsParser( this );
    manifestRelsParser.parseDocument( *pRelsInputStream );
}



_DWFTK_API
OPCRelationship*
OPCRelationshipContainer::provideRelationship( OPCRelationship* pRelationship )
throw()
{
    _oRelationships.push_back( pRelationship );
    return pRelationship;
}


