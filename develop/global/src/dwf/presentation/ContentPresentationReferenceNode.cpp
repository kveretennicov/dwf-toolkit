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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationReferenceNode.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/presentation/ContentPresentationReferenceNode.h"
#include "dwf/package/Content.h"
#include "dwf/package/ContentElement.h"
#include "dwf/package/Resource.h"
#include "dwf/presentation/PropertyReference.h"
#include "dwf/package/Constants.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationReferenceNode::DWFContentPresentationReferenceNode( const DWFString& zLabel,
                                                                          const DWFString& zID )
throw()
                                   : DWFContentPresentationNode( zLabel, zID )
                                   , _pContentElement( NULL )
                                   , _zContentElementID()
                                   , _zResourceURI()
                                   , _pResource( NULL )
                                   , _pSectionContainingResource( NULL )
                                   , _zURI()
                                   , _oPRMap()
                                   , _oPRIDMap()
                                   , _zEmptyString()
{
    ;
}

_DWFTK_API
DWFContentPresentationReferenceNode::~DWFContentPresentationReferenceNode()
throw()
{
    ;
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::setContentElement( const DWFContentElement& rContentElement )
throw()
{
    _pContentElement = &rContentElement;
    _setContentElement(rContentElement.id());
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::setResource( DWFResource& rResource, DWFSection& rContainingSection)
throw()
{
    _pResource = &rResource;
    _pSectionContainingResource = &rContainingSection;
    _zResourceURI = /*NOXLATE*/L"";
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::setResource( DWFResource& rResource)
throw()
{
    _pResource = &rResource;
    _zResourceURI = /*NOXLATE*/L"";
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::setResource( const DWFString& rResourceURI )
    throw()
{
    _zResourceURI = rResourceURI;
    _pResource = NULL;
}


_DWFTK_API
void
DWFContentPresentationReferenceNode::setPropertyReferenceURI( DWFPropertyReference& rPropertyReference,
                                                              const DWFString& zURI)
throw(DWFException)
{
    if (zURI.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"URI cannot be a null string" );
    }
    _oPRMap.insert( &rPropertyReference, zURI );
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::setPropertyReferenceURI( const DWFString& zID,
                                                              const DWFString& zURI )
throw(DWFException)
{
    if (zURI.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"URI cannot be a null string" );
    }
    _oPRIDMap.insert( zID, zURI );
}

_DWFTK_API
const DWFString&
DWFContentPresentationReferenceNode::getPropertyReferenceURI( const DWFPropertyReference& rPropertyReference ) const
throw()
{
    //
    // first look in the object map
    //
    const DWFString* pzURI = _oPRMap.find( (DWFPropertyReference*)&rPropertyReference );

    if (pzURI)
    {
        return *pzURI;
    }

    //
    // next, look in the id map
    //
    pzURI = _oPRIDMap.find( rPropertyReference.id() );
    if (pzURI)
    {
        return *pzURI;
    }
    else
    {
        return _zEmptyString;
    }    
}


_DWFTK_API
void
DWFContentPresentationReferenceNode::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    DWFContentPresentationNode::parseAttributeList( ppAttributeList );

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

            //
            // Extract the content element refs field.
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ContentElementRefs) == 0))
        {
            nFound |= 0x01;
            _setContentElement( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the resource URI field.
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ResourceURI) == 0))
        {
            nFound |= 0x02;
            setResource( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the URI attribute
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_URI) == 0))
        {
            nFound |= 0x04;
            setURI( ppAttributeList[iAttrib+1] );
        }
    }
}

#ifndef DWFTK_READ_ONLY

void
DWFContentPresentationReferenceNode::serializeAttributes( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
    throw( DWFException )
{
    //
    // attributes
    //

    if (_zContentElementID.chars() > 0)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_ContentElementRefs, _zContentElementID );
    }

    if ( (_zResourceURI.chars() == 0) && (_pResource != NULL) )
    {
        DWFString zResourceURI;
        generateResourceURI( rSerializer, _pResource, _pSectionContainingResource, _zResourceURI);
    }
    if (_zResourceURI.chars() > 0)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_ResourceURI, _zResourceURI );
    }

    if (_zURI.chars() > 0)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_URI, _zURI );
    }
}

_DWFTK_API
void
DWFContentPresentationReferenceNode::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
            //
            // if the derived class has already open the element
            // we can only serialize the base attributes here
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_ReferenceNode );

            //
            // let base class know not to start the element
            //
            nFlags |= DWFXMLSerializer::eElementOpen;

            serializeAttributes(rSerializer, nFlags);

            //
            // base attributes
            //
            DWFContentPresentationNode::serializeXML( rSerializer, nFlags );

            //
            // property reference URIs
            //

            //
            // go through the object map
            //
            _tPropertyReferenceURIMap::Iterator* piPropertyRef = _oPRMap.iterator();

            bool bOpenElement = false; 
            if (piPropertyRef && piPropertyRef->valid())
            {
                rSerializer.startElement( DWFXML::kzElement_PropertyReferenceURIs );
                bOpenElement = true;
                for (; piPropertyRef->valid(); piPropertyRef->next())
                {
                    rSerializer.startElement( DWFXML::kzElement_PropertyReferenceURI );

                    DWFPropertyReference* pPropertyReference = piPropertyRef->key();
                    if (pPropertyReference->id().chars() == 0)
                    {
                        pPropertyReference->setID( rSerializer.nextUUID(true) );
                    }

                    rSerializer.addAttribute( DWFXML::kzAttribute_PropertyReferenceID, pPropertyReference->id() );
                    rSerializer.addAttribute( DWFXML::kzAttribute_URI, piPropertyRef->value() );

                    rSerializer.endElement();
                }

                rSerializer.endElement();
            }

            DWFCORE_FREE_OBJECT( piPropertyRef );

            //
            // now go through the id map
            //
            _tPropertyReferenceIDURIMap::Iterator* piPropertyRefID = _oPRIDMap.iterator();
            if (piPropertyRefID && piPropertyRefID->valid())
            {
                //
                // if the element is not already opened, open it
                //
                if (!bOpenElement)
                {
                    rSerializer.startElement( DWFXML::kzElement_PropertyReferenceURIs );
                }

                for (; piPropertyRefID->valid(); piPropertyRefID->next())
                {
                    rSerializer.startElement( DWFXML::kzElement_PropertyReferenceURI );

                    const DWFString& zID = piPropertyRefID->key();

                    rSerializer.addAttribute( DWFXML::kzAttribute_PropertyReferenceID, zID );
                    rSerializer.addAttribute( DWFXML::kzAttribute_URI, piPropertyRefID->value() );

                    rSerializer.endElement();
                }

                if (!bOpenElement)
                {
                    rSerializer.endElement();
                }
            }

            DWFCORE_FREE_OBJECT( piPropertyRefID );

            //
            // clear this bit
            //
            nFlags &= ~DWFXMLSerializer::eElementOpen;
        }
            //
            // element already started by derived class
            //
        else
        {
            serializeAttributes(rSerializer, nFlags);

            //
            // base attributes
            //
            DWFContentPresentationNode::serializeXML( rSerializer, nFlags );
        }


            //
            // close element if this bit is not set
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
    else if (nFlags & DWFPackageWriter::eDescriptor)
    {
        if (_pContentElement)
        {
            DWFContent* pContent = _pContentElement->getContent();
            if (pContent)
            {
                DWFResource* pResource = containingResource();
                if (pResource)
                {
                    pResource->addContentID( pContent->id() );
                }
            }
        }

        DWFContentPresentationNode::serializeXML( rSerializer, nFlags );
    }
}


#endif
