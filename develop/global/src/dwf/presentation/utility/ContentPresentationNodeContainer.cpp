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

//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/presentation/utility/ContentPresentationNodeContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $

#include "dwf/package/Constants.h"
#include "dwf/presentation/utility/ContentPresentationNodeContainer.h"
#include "dwf/presentation/ContentPresentationNode.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentPresentationNodeContainer::DWFContentPresentationNodeContainer()
throw()
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFContentPresentationNodeContainer::~DWFContentPresentationNodeContainer()
throw()
{
    //
    // delete all nodes we own
    //
    DWFContentPresentationNode::tList::Iterator* piNodes = getNodes();

    if (piNodes)
    {
        for (piNodes->reset(); piNodes->valid(); piNodes->next())
        {
            DWFCORE_FREE_OBJECT(piNodes->get());
        }
    }

    DWFCORE_FREE_OBJECT( piNodes );
}

_DWFTK_API
void
DWFContentPresentationNodeContainer::addNode( DWFContentPresentationNode* pNode )
throw( DWFException )
{
    if (pNode == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A non-null node must be provided" );
    }

    size_t tIndex = _oNodes.size();
	size_t*	tempPos = NULL;
    const DWFString& zID = pNode->id();
	DWFString	tempID(zID);
    //
    // if we have an ID check for duplicate and replace it if it already exists
    //
	if (zID.chars())
	{
		if (tIndex > 0)
		{
			tempPos = posMap.find(tempID);
			if (tempPos)
			{
				tIndex = *tempPos;
				 DWFContentPresentationNode* pExistingNode = _oNodes[tIndex];
				_oNodes.eraseAt( tIndex );
				DWFCORE_FREE_OBJECT( pExistingNode );
			}			 
		}
	}

	pNode->setParent( this );
	_oNodes.insertAt( pNode, tIndex );

	if (zID.chars() > 0)
	{
		posMap.insert(tempID, tIndex, true);
	}
	else
	{
		posMap.insert(tempID, tIndex, false);
	}
}

_DWFTK_API
void 
DWFContentPresentationNodeContainer::insertAt( DWFContentPresentationNode* pNode, size_t index )
throw( DWFException )
{
	if (pNode == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A non-null node must be provided" );
    }

    DWFString   tempID(pNode->id());
    if (!(_oNodes.empty()))
    {
        //
        // if we have an ID check for duplicate and replace it if it already exists
        //
        size_t*	tempPos = posMap.find(tempID);
        if (tempPos)
        {
            //
            // Deal with collection becoming smaller... If our insertion point is
            // after the node that we're deleting, drop the insertion index by one.
            //
            if(index > *tempPos)
            {
                index--;
            }
            DWFContentPresentationNode* pExistingNode = _oNodes[*tempPos];
            _oNodes.eraseAt( *tempPos );
            DWFCORE_FREE_OBJECT( pExistingNode );
        }
    }

    pNode->setParent( this );
    _oNodes.insertAt(pNode, index); // will throw DWFUnexpectedException if index > size.
    posMap.insert(tempID, index, true);
}

_DWFTK_API
DWFContentPresentationNode::tList::Iterator*
DWFContentPresentationNodeContainer::getNodes( )
throw()
{
    return _oNodes.iterator();
}

_DWFTK_API
void
DWFContentPresentationNodeContainer::removeNode( DWFContentPresentationNode* pNode,
                                                 bool                        bDelete )
throw( DWFException )
{
    const DWFString& zID = pNode->id();
    DWFString   tempID(zID);
	posMap.erase(tempID);

    _oNodes.erase(pNode);
    
    if (bDelete)
    {
        DWFCORE_FREE_OBJECT(pNode);
    }    
}

#ifndef DWFTK_READ_ONLY

void
DWFContentPresentationNodeContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eContentPresentation)
    {
        DWFContentPresentationNode::tList::Iterator* piNode = _pContainer->_oNodes.iterator();

        if (piNode && piNode->valid())
        {
            DWFString zNamespace;

                //
                // namespace dictated by document and section type 
                //
            if (nFlags & DWFPackageWriter::eDescriptor)
            {
                zNamespace.assign( namespaceXML(nFlags) );
            }

            rSerializer.startElement( DWFXML::kzElement_Nodes, zNamespace );
            {
                for (; piNode->valid(); piNode->next())
                {            
                    piNode->get()->serializeXML( rSerializer, nFlags );
                }
            }

            rSerializer.endElement();
        }

        if (piNode)
        {
            DWFCORE_FREE_OBJECT( piNode );
        }
    }
    else
    {
        DWFContentPresentationNode::tList::Iterator* piNode = _pContainer->_oNodes.iterator();

        if (piNode && piNode->valid())
        {
            for (; piNode->valid(); piNode->next())
            {            
                piNode->get()->serializeXML( rSerializer, nFlags );
            }
        }

        if (piNode)
        {
            DWFCORE_FREE_OBJECT( piNode );
        }
    }
}

#endif
