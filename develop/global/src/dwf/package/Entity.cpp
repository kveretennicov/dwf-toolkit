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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Entity.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Entity.h"
#include "dwf/package/Constants.h" 
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFEntity::DWFEntity( const DWFString& zID,
                      DWFContent* pContent )
throw()
         : DWFContentElement( zID, pContent )
         , _oParents( false )
         , _oChildren( false )
         , _oClasses()
         , _oFeatures()
{
}

_DWFTK_API
DWFEntity::~DWFEntity()
throw()
{
}

_DWFTK_API
void
DWFEntity::parseAttributeList( const char** ppAttributeList,
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
        // get any child entity references
        //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_EntityRefs) == 0))
        {
            nFound |= 0x01;
            rUnresolved.push_back( tUnresolved(eChildEntityReferences, DWFString(ppAttributeList[iAttrib+1])) );
        }

        //
        // get the class references
        //
        if (!(nFound & 0x02) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ClassRefs) == 0))
        {
            nFound |= 0x02;
            rUnresolved.push_back( tUnresolved(eClassReferences, DWFString(ppAttributeList[iAttrib+1])) );
        }

        //
        // get the feature references
        //
        if (!(nFound & 0x04) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_FeatureRefs) == 0))
        {
            nFound |= 0x04;
            rUnresolved.push_back( tUnresolved(eFeatureReferences, DWFString(ppAttributeList[iAttrib+1])) );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFEntity::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                 unsigned int nFlags )
throw( DWFException )
{
    //
    //  First let the base class serialize it attributes
    //
    DWFContentElement::_serializeAttributes( rSerializer, nFlags );

    //
    //  Serialize only child entity references. The parent entity reference
    //  will be rebuilt from this information upon reload
    //
    if (_oChildren.size() > 0)
    {
        DWFEntity::tConstIterator* piEntity = _oChildren.constIterator();
        if (piEntity)
        {
            //
            //  String together all the references 
            //
            DWFString zReferences;
            DWFEntity* pReference = NULL;
            for (; piEntity->valid(); piEntity->next())
            {
                pReference = piEntity->get();
                
                zReferences.append( pReference->id() );
                zReferences.append( /*NOXLATE*/L" " );
            }

            if (zReferences.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_EntityRefs, zReferences );
            }

            DWFCORE_FREE_OBJECT( piEntity );
        }
    }

    //
    //  Serialize any class references.
    //
    if (_oClasses.size() > 0)
    {
        DWFClass::tIterator* piClass = _oClasses.iterator();
        if (piClass)
        {
            //
            //  String together all the references 
            //
            DWFString zReferences;
            DWFClass* pReference = NULL;
            for (; piClass->valid(); piClass->next())
            {
                pReference = piClass->get();
                
                zReferences.append( pReference->id() );
                zReferences.append( /*NOXLATE*/L" " );
            }

            if (zReferences.chars() > 0)
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_ClassRefs, zReferences );
            }

            DWFCORE_FREE_OBJECT( piClass );
        }
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
DWFEntity::_serializeXML( DWFXMLSerializer& rSerializer, 
                          unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        //
        //  Open the element
        //
        DWFString zNamespace( _oSerializer.namespaceXML( nFlags ) );
        rSerializer.startElement( DWFXML::kzElement_Entity, zNamespace );

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
        }

        rSerializer.endElement();
    }
}

#endif

_DWFTK_API
void
DWFEntity::insertPropertyAncestors( DWFContentElement::tVector& rAncestorElements ) const
throw()
{
    for (size_t i = 0; i < _oClasses.size(); ++i)
    {
        rAncestorElements.push_back( _oClasses[i] );
    }
}

_DWFTK_API
void
DWFEntity::_addChild( DWFEntity* pEntity )
throw()
{
    pEntity->_oParents.insert( this );
    _oChildren.insert( pEntity );
}

_DWFTK_API
bool
DWFEntity::_removeChild( DWFEntity* pEntity )
throw()
{
    pEntity->_oParents.erase( this );
    return _oChildren.erase( pEntity );
}

_DWFTK_API
void
DWFEntity::_removeChildren()
throw()
{
    DWFEntity::tSortedList::ConstIterator* piChild = _oChildren.constIterator();

    for (; piChild->valid(); piChild->next())
    {
        DWFEntity* pEntity = piChild->get();
        pEntity->_oParents.erase( this );
    }

    DWFCORE_FREE_OBJECT( piChild );

    _oChildren.clear();
}

