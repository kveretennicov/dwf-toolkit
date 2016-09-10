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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/ContentPresentationNode.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/presentation/ContentPresentationNode.h"
#include "dwf/package/Section.h"
#include "dwf/package/GraphicResource.h"
#include "dwf/presentation/utility/ContentPresentationNodeContainer.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationNode::DWFContentPresentationNode( const DWFString& zLabel,
                                                        const DWFString& zID )
throw()
                          : _zID( zID )
                          , _zLabel( zLabel )
                          , _bExclusive( false )
                          , _pDefaultIconResource( NULL )
                          , _pSectionContainingDefaultIconResource( NULL )
                          , _pActiveIconResource( NULL )
                          , _pSectionContainingActiveIconResource( NULL )
                          , _pExpandedIconResource( NULL )
                          , _pSectionContainingExpandedIconResource( NULL )
                          , _bHidden( false )
                          , _pParent( NULL )
{
}

_DWFTK_API
DWFContentPresentationNode::~DWFContentPresentationNode()
throw()
{
}


_DWFTK_API
void
DWFContentPresentationNode::setID( const DWFString& zID )
throw( DWFException )
{
    if (zID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A null id cannot be assigned to a content presentation node" );
    }

    _zID = zID;
}

_DWFTK_API
const DWFString&
DWFContentPresentationNode::id() const
throw()
{
    return _zID;
}

_DWFTK_API
void
DWFContentPresentationNode::setLabel( const DWFString& zLabel )
throw()
{
    _zLabel = zLabel;
}

_DWFTK_API
const DWFString&
DWFContentPresentationNode::label() const
throw()
{
    return _zLabel;
}

_DWFTK_API
void
DWFContentPresentationNode::setDefaultIconResource( DWFImageResource& rImageResource, DWFSection& rContainingSection )
throw()
{
    _pDefaultIconResource = &rImageResource;
    _pSectionContainingDefaultIconResource = &rContainingSection;
}

_DWFTK_API
void
DWFContentPresentationNode::setDefaultIconResource( DWFImageResource& rImageResource )
throw()
{
    _pDefaultIconResource = &rImageResource;
}

_DWFTK_API
void
DWFContentPresentationNode::setDefaultIconResourceURI( const DWFString& zURI )
throw()
{
    _zDefaultIconResourceURI = zURI;
}

_DWFTK_API
const DWFString&
DWFContentPresentationNode::defaultIconResourceURI() const
throw()
{
    return _zDefaultIconResourceURI;
}

_DWFTK_API
void
DWFContentPresentationNode::setActiveIconResource( DWFImageResource& rImageResource, DWFSection& rContainingSection )
throw()
{
    _pActiveIconResource = &rImageResource;
    _pSectionContainingActiveIconResource = &rContainingSection;
}

_DWFTK_API
void
DWFContentPresentationNode::setActiveIconResource( DWFImageResource& rImageResource )
throw()
{
    _pActiveIconResource = &rImageResource;
}

_DWFTK_API
void
DWFContentPresentationNode::setActiveIconResourceURI( const DWFString& zURI )
throw()
{
    _zActiveIconResourceURI = zURI;
}

_DWFTK_API
const DWFString&
DWFContentPresentationNode::activeIconResourceURI() const
throw()
{
    return _zActiveIconResourceURI;
}

_DWFTK_API
void
DWFContentPresentationNode::setExpandedIconResource( DWFImageResource& rImageResource, DWFSection& rContainingSection )
throw()
{
    _pExpandedIconResource = &rImageResource;
    _pSectionContainingExpandedIconResource = &rContainingSection;
}

_DWFTK_API
void
DWFContentPresentationNode::setExpandedIconResource( DWFImageResource& rImageResource )
throw()
{
    _pExpandedIconResource = &rImageResource;
}

_DWFTK_API
void
DWFContentPresentationNode::setExpandedIconResourceURI( const DWFString& zURI )
throw()
{
    _zExpandedIconResourceURI = zURI;
}

_DWFTK_API
const DWFString&
DWFContentPresentationNode::expandedIconResourceURI() const
throw()
{
    return _zExpandedIconResourceURI;
}

_DWFTK_API
void
DWFContentPresentationNode::addChild( DWFContentPresentationNode* pNode )
throw( DWFException )
{
    addNode( pNode );
}

_DWFTK_API
DWFContentPresentationNode::tList::Iterator*
DWFContentPresentationNode::getChildren( )
throw()
{
    return getNodes();
}

_DWFTK_API
void
DWFContentPresentationNode::removeChild( DWFContentPresentationNode* pNode,
                                         bool                        bDelete )
throw( DWFException )
{
    removeNode(pNode, bDelete);
}

_DWFTK_API
void DWFContentPresentationNode::setGroupExclusivity( bool bExclusive )
throw()
{
    _bExclusive = bExclusive;
}

_DWFTK_API
bool
DWFContentPresentationNode::groupExclusivity() const
throw()
{
    return _bExclusive;
}

DWFResource*
DWFContentPresentationNode::containingResource()
{
    DWFResource* pResource = NULL;

    DWFContentPresentationNode* pCurrentNode = this;
    
    while (pResource == NULL)
    {
        DWFContentPresentationNodeContainer* pCurrentContainer = pCurrentNode->parent();
        pCurrentNode = dynamic_cast<DWFContentPresentationNode*>(pCurrentContainer);

        if (pCurrentNode == NULL)
        {
            //
            // this must be a view
            //
            DWFContentPresentationView* pCurrentView = dynamic_cast<DWFContentPresentationView*>(pCurrentContainer);
            if (pCurrentView)
            {
                DWFContentPresentation* pPresentation = dynamic_cast<DWFContentPresentation*>(pCurrentView->parent());
                if (pPresentation)
                {
                    pResource = dynamic_cast<DWFResource*>(pPresentation->parent());
                }
                break;
            }
            else
            {
                //
                // something unexpected has happend
                break;
            }
        }
    }

    return pResource;
}

_DWFTK_API
void
DWFContentPresentationNode::parseAttributeList( const char** ppAttributeList )
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
        pAttrib = &ppAttributeList[iAttrib][0];

            //
            // Extract the id field.
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x01;
            setID( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the label field.
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Label) == 0))
        {
            nFound |= 0x02;
            setLabel( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the hidden attribute
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Hidden) == 0))
        {
            nFound |= 0x04;
            if(DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"true") == 0)
            {
                setHidden( true );
            }
            else
            {
                setHidden( false );
            }
        }
            //
            // Extract the exclusivity
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Exclusivity) == 0))
        {
            nFound |= 0x08;
            if(DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], /*NOXLATE*/"true") == 0)
            {
                setGroupExclusivity( true );
            }
            else
            {
                setGroupExclusivity( false );
            }
        }
            //
            // Extract the default icon URI.
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_DefaultIconURI) == 0))
        {
            nFound |= 0x10;
            setDefaultIconResourceURI( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the active icon URI.
            //
        else if (!(nFound & 0x20) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ActiveIconURI) == 0))
        {
            nFound |= 0x20;
            setActiveIconResourceURI( ppAttributeList[iAttrib+1] );
        }
            //
            // Extract the default icon URI.
            //
        else if (!(nFound & 0x40) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ExpandedIconURI) == 0))
        {
            nFound |= 0x40;
            setExpandedIconResourceURI( ppAttributeList[iAttrib+1] );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFContentPresentationNode::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
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
            rSerializer.startElement( DWFXML::kzElement_Node );
        }

            //
            // add attributes
            //
        {
            //
            // assign a new id if there isn't one
            //
            if (_zID.chars() == 0)
            {
                _zID.assign(rSerializer.nextUUID(true));
            }
            rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );
            rSerializer.addAttribute( DWFXML::kzAttribute_Label, _zLabel );
            if (_bExclusive)
            {
                //
                // default is false
                //
                rSerializer.addAttribute( DWFXML::kzAttribute_Exclusivity, /*NOXLATE*/"true");
            }

            if ( _bHidden )
            {
                //
                // default is false
                //
                rSerializer.addAttribute( DWFXML::kzAttribute_Hidden, /*NOXLATE*/"true");
            }


            //
            // default icon
            //
            if (_zDefaultIconResourceURI.chars() == 0
                && _pDefaultIconResource)
            {
                generateResourceURI( rSerializer, _pDefaultIconResource, _pSectionContainingDefaultIconResource, _zDefaultIconResourceURI );
            }

            if (_zDefaultIconResourceURI.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_DefaultIconURI, _zDefaultIconResourceURI );
            }

            //
            // active icon
            //
            if (_zActiveIconResourceURI.chars() == 0
                && _pActiveIconResource)
            {
                generateResourceURI( rSerializer, _pActiveIconResource, _pSectionContainingActiveIconResource, _zActiveIconResourceURI );
            }

            if (_zActiveIconResourceURI.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_ActiveIconURI, _zActiveIconResourceURI);
            }

            //
            // expanded icon
            //
            if (_zExpandedIconResourceURI.chars() == 0
                && _pExpandedIconResource)
            {
                generateResourceURI( rSerializer, _pExpandedIconResource, _pSectionContainingExpandedIconResource, _zExpandedIconResourceURI );
            }

            if (_zExpandedIconResourceURI.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_ExpandedIconURI, _zExpandedIconResourceURI);
            }

            //
            // Serialize our children...
            //
            // Need to clear the ElementOpen bit, or they won't get a new element!
            //
            getSerializable().serializeXML(rSerializer, nFlags&~DWFXMLSerializer::eElementOpen);
        }

        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
    else
    {
            getSerializable().serializeXML(rSerializer, nFlags);
    }
}


void
DWFContentPresentationNode::generateResourceURI( DWFXMLSerializer& rSerializer, DWFResource* pResource, DWFSection* pSection, DWFString& zResourceURI )
{
    //
    // no resource was passed in, pass back an empty string
    //
    if (pResource == NULL)
    {
        zResourceURI.destroy();
        return;
    }

    //
    // get the section name, which is unique within the package
    //
    DWFString zSectionName;

    if (pSection)
    {
        zSectionName = pSection->name();
        if (zSectionName.chars() == 0)
        {
            //
            // if the section doesn't have a name yet, generate one based on the UUID
            //
            pSection->rename(rSerializer.nextUUID( true ));
            zSectionName = pSection->name();
        }
    }
    
    //
    // get resource id
    //
    DWFString zResourceID = pResource->objectID();
    if (zResourceID.chars() == 0)
    {
        //
        // if the resource doesn't have an id yet, give it one
        //      
        pResource->setObjectID(rSerializer.nextUUID( true ));
        zResourceID = pResource->objectID();
    }

    if (zSectionName.chars() > 0)
    {
        DWFString zResourceURIFormat = /*NOXLATE*/"section=%ls&resource=%ls";
        size_t nTempBufferSize = zResourceURIFormat.chars() + zSectionName.chars() + zResourceID.chars();
        wchar_t* zTempBuffer = new wchar_t[nTempBufferSize];
        *zTempBuffer = 0;
        _DWFCORE_SWPRINTF( zTempBuffer, nTempBufferSize, zResourceURIFormat, (const wchar_t*)zSectionName, (const wchar_t*)zResourceID);
        zResourceURI = zTempBuffer;
        delete[] zTempBuffer;
    }
    else
    {
        DWFString zResourceURIFormat = /*NOXLATE*/"resource=%ls";
        size_t nTempBufferSize = zResourceURIFormat.chars() + zResourceID.chars();
        wchar_t* zTempBuffer = new wchar_t[nTempBufferSize];
        *zTempBuffer = 0;
        _DWFCORE_SWPRINTF( zTempBuffer, nTempBufferSize, zResourceURIFormat, (const wchar_t*)zResourceID);
        zResourceURI = zTempBuffer;
        delete[] zTempBuffer;
    }
}


#endif

