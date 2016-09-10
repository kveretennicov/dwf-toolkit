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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Object.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Object.h"
#include "dwf/package/Entity.h"
#include "dwf/package/Constants.h" 
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFObject::DWFObject( const DWFString& zID,
                      DWFEntity* pEntity,
                      DWFContent* pContent )
throw( DWFInvalidArgumentException )
         : DWFRenderable( zID, pContent )
         , _pEntity( pEntity )
         , _pParent( NULL )
         , _oChildren()
         , _oFeatures()
{
    if (zID.chars() == 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"An new object ID was not provided" );
    }
    if (pEntity == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Objects cannot be created without a valid entity" );
    }
}

_DWFTK_API
DWFObject::DWFObject()
    throw()
         : DWFRenderable( /*NOXLATE*/L"" )
         , _pEntity( NULL )
         , _pParent( NULL )
         , _oChildren()
         , _oFeatures()
{
}

_DWFTK_API
DWFObject::~DWFObject()
throw()
{
}

_DWFTK_API
void 
DWFObject::parseAttributeList( const char** ppAttributeList,
                               tUnresolvedList& rUnresolved )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    DWFPropertySet::parseAttributeList( ppAttributeList, rUnresolved );

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

        //
        // get the entity reference
        //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_EntityRef) == 0))
        {
            nFound |= 0x01;
            rUnresolved.push_back( tUnresolved(eEntityReference, DWFString(ppAttributeList[iAttrib+1])) );
        }

        //
        // get the feature references
        //
        if (!(nFound & 0x02) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_FeatureRefs) == 0))
        {
            nFound |= 0x02;
            rUnresolved.push_back( tUnresolved(eFeatureReferences, DWFString(ppAttributeList[iAttrib+1])) );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFObject::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                         unsigned int nFlags )
throw( DWFException )
{
    //
    //  First let the base class serialize it attributes
    //
    DWFContentElement::_serializeAttributes( rSerializer, nFlags );

    //
    //  Serialize the entity reference.
    //
    if (_pEntity)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_EntityRef, _pEntity->id() );
    }
    else
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The entity reference in object should not be NULL" );
    }

    //
    //  Serialize any feature references.
    //
    if (_oFeatures.size() > 0)
    {
        DWFFeature::tIterator* piFeature = _oFeatures.iterator();
        if (piFeature)
        {
            //
            //  String together all the references 
            //
            DWFString zReferences;
            DWFFeature* pReference = NULL;
            for (; piFeature->valid(); piFeature->next())
            {
                pReference = piFeature->get();
                
                zReferences.append( pReference->id() );
                zReferences.append( /*NOXLATE*/L" " );
            }

            if (zReferences.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_FeatureRefs, zReferences );
            }

            DWFCORE_FREE_OBJECT( piFeature );
        }
    }
}

_DWFTK_API
void 
DWFObject::_serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        //
        //  Open the object element
        //
        DWFString zNamespace( _oSerializer.namespaceXML( nFlags ) );
        rSerializer.startElement( DWFXML::kzElement_Object, zNamespace );

        //
        //  Serialize attributes
        //
        _serializeAttributes( rSerializer, nFlags );

        //
        //  Serialize child elements
        //
        {
            //
            //  Let baseclass know not to start a new XML element
            //
            bool bElementOpenFlag = false;
            if (nFlags & DWFXMLSerializer::eElementOpen)
            {
                bElementOpenFlag = true;
            }
            else
            {
                nFlags |= DWFXMLSerializer::eElementOpen;
            }

            DWFContentElement::_serializeXML( rSerializer, nFlags );

            if (bElementOpenFlag == false)
            {
                //
                //  Unset the element open bit to allow proper ending
                //
                nFlags &= ~DWFXMLSerializer::eElementOpen;
            }

            //
            //  Serialize the child objects recursively as subnodes in the XML tree.
            //  We can safely do this since objects can have only one parent. This 
            //  will speed up the load, since we can setup the relationships without
            //  having to do a second pass.
            //
            if (_oChildren.size() > 0)
            {
                DWFObject::tIterator* piChild = _oChildren.iterator();
                if (piChild)
                {
                    for (; piChild->valid(); piChild->next())
                    {
                        DWFObject* pChild = piChild->get();
                        pChild->getSerializable().serializeXML( rSerializer, nFlags );
                    }

                    DWFCORE_FREE_OBJECT( piChild );
                }
            }
        }

        rSerializer.endElement();
    }
}

#endif

_DWFTK_API
void
DWFObject::insertPropertyAncestors( DWFContentElement::tVector& rAncestorElements ) const
throw()
{
    rAncestorElements.push_back( _pEntity );
}


_DWFTK_API
void
DWFObject::_addChild( DWFObject* pObject )
throw()
{
    DWFObject* pPreviousParent = pObject->getParent();

    //
    // Do something only if this isn't already the parent.
    //
    if (pPreviousParent != this)
    {
        //
        // Remove the existing parent-child relationship.
        //
        if (pPreviousParent != NULL)
            pPreviousParent->_removeChild( pObject );

        //
        // Go ahead and setup the relationship
        pObject->_pParent = this;
        _oChildren.push_back( pObject );
    }
}

_DWFTK_API
bool
DWFObject::_removeChild( DWFObject* pObject )
throw()
{
    DWFOrderedVector<size_t> oIndices;

    size_t iLocation;
    if (_oChildren.findFirst( pObject, iLocation ))
    {
        //
        // Unset the parent informaton on the child before erasing.
        //
        _oChildren[ iLocation ]->_pParent = NULL;
        _oChildren.erase( pObject );
        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
void
DWFObject::_removeChildren()
throw()
{
    //
    // Unset all parent information first.
    //
    for (size_t i=0; i<_oChildren.size(); ++i)
    {
        _oChildren[i]->_pParent = NULL;
    }
    
    _oChildren.clear();
}

