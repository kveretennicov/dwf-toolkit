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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/PropertySet.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Constants.h"
#include "dwf/package/PropertySet.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFPropertySet::DWFPropertySet( const DWFString& zLabel )
throw()
              : DWFPropertyContainer( /*NOXLATE*/L"" )
              , _zLabel( zLabel )
              , _zSchemaID( "" )
              , _zSetID( "" )
              , _bClosed( false )
              , _pContent( NULL )
              , _pParent( NULL )
{
}

_DWFTK_API
DWFPropertySet::~DWFPropertySet()
throw()
{
}

_DWFTK_API
void
DWFPropertySet::setClosed( const DWFString& zSetID )
throw( DWFException )
{
    if (zSetID.chars()==0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A property set cannot be made closed without providing a Set ID." );
    }
    _zSetID = zSetID;
    _bClosed = true;
}

//////

_DWFTK_API
const DWFProperty*
DWFPropertySet::getProperty( const DWFString& zName,
                             const DWFString& zCategory,
                             bool bSearchClosedSets )
throw( DWFException )
{
    const DWFProperty* pProperty = findProperty( zName, zCategory );

    //
    // If the property wasn't found, recurse into the owned subsets
    //
    if (pProperty == NULL)
    {
        //
        // Search order should always be BFS - i.e. search the next level down before going deeper
        //
        DWFPropertyContainer::tList oPSets( _oContainers );

        while (!oPSets.empty())
        {
            DWFPropertyContainer::tList::const_iterator iSet = oPSets.begin();
            for (; iSet != oPSets.end(); ++iSet)
            {
                pProperty = (*iSet)->findProperty( zName, zCategory );
                if (pProperty)
                {
                    return pProperty;
                }
            }

            if (pProperty == NULL)
            {
                DWFPropertyContainer::tList oTmp(0);
                oPSets.swap( oTmp );

                //
                //  Now grab the subsets of each set checked above
                //
                DWFPropertyContainer::tList::const_iterator iTmp = oTmp.begin();
                for (; iTmp != oTmp.end(); ++iTmp)
                {
                    DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iTmp);
                    if (bSearchClosedSets || !(pSet->isClosed()))
                    {
                        oPSets.insert( oPSets.end(), pSet->_oContainers.begin(), pSet->_oContainers.end() );
                    }
                }
            }
        }
    }

    //
    // If the property wasn't found, recurse into the referenced subsets
    //
    if (pProperty == NULL)
    {
        //
        // Search order should always be BFS - i.e. search the next level down before going deeper
        //
        DWFPropertyContainer::tList oPSets( _oReferences );

        while (!oPSets.empty())
        {
            DWFPropertyContainer::tList::const_iterator iSet = oPSets.begin();
            for (; iSet != oPSets.end(); ++iSet)
            {
                pProperty = (*iSet)->findProperty( zName, zCategory );
                if (pProperty)
                {
                    return pProperty;
                }
            }

            if (pProperty == NULL)
            {
                DWFPropertyContainer::tList oTmp(0);
                oPSets.swap( oTmp );

                //
                //  Now grab the subsets of each set checked above
                //
                DWFPropertyContainer::tList::const_iterator iTmp = oTmp.begin();
                for (; iTmp != oTmp.end(); ++iTmp)
                {
                    DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iTmp);
                    if (bSearchClosedSets || !(pSet->isClosed()))
                    {
                        oPSets.insert( oPSets.end(), pSet->_oReferences.begin(), pSet->_oReferences.end() );
                    }
                }
            }
        }
    }

    return pProperty;
}

_DWFTK_API
const DWFProperty* 
DWFPropertySet::getProperty( const DWFString& zID,
                             const DWFString& zName,
                             const DWFString& zCategory,
                             teQueryIDType eIDType )
throw()
{
    DWFPropertySet* pSet = getPropertySet( zID, eIDType );

    if (pSet)
    {
        return pSet->getProperty( zName, zCategory );
    }
    else
    {
        return NULL;
    }
}

//////

_DWFTK_API
DWFProperty::tIterator*
DWFPropertySet::getAllProperties( bool bSearchClosedSets )
throw()
{
    //
    //  We need to cache all properties in oMap to ensure we don't get properties
    //  with duplicate name and category from nested subsets.
    //
    DWFProperty::tMap oMap;

    DWFProperty::tList oProperties(0);
    _getAllProperties( oProperties, oMap, bSearchClosedSets );

    if (oProperties.empty())
    {
        return NULL;
    }
    else
    {
        DWFProperty::tListIterator* piProperty = DWFCORE_ALLOC_OBJECT( DWFProperty::tListIterator(oProperties) );
        return piProperty;
    }
}

_DWFTK_API
void
DWFPropertySet::_getAllProperties( DWFProperty::tList& oProperties,
                                   DWFProperty::tMap& oMap,
                                   bool bSearchClosedSets )
throw()
{
    //
    //  First grab all the properties directly in this set - there can't be any duplicates at this level.
    //
    _getProperties( oProperties, oMap, _oOrderedProperties );


    //
    //  Now grab all properties from the subsets. Note: The search will be performed in BFS order, i.e., properties
    //  from the subsets are collected before the properties from their subsets. This affects which property with
    //  duplicate name/category pairs will be rejected.
    //

    //
    // First search owned containers, and then search referenced containers.
    //
    std::vector<DWFPropertyContainer::tList*> oContainerList;
    oContainerList.push_back( &_oContainers );
    oContainerList.push_back( &_oReferences );

    size_t n = 0;
    for (; n<oContainerList.size(); ++n)
    {
        //
        //  These will be used for the process of gathering the next level of subsets.
        //
        DWFPropertyContainer::tList oSubSets(0);

        DWFPropertyContainer::tList oPSets = *(oContainerList[n]);
        while (!oPSets.empty())
        {
            //
            //  First loop over the current property sets to see if their properties can be included
            //
            DWFPropertyContainer::tList::iterator iSet = oPSets.begin();
            for (; iSet != oPSets.end(); ++iSet)
            {
                DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);

                if (!(pSet->isClosed()) || bSearchClosedSets)
                {
                    _getProperties( oProperties, oMap, pSet->_oOrderedProperties );

                    //
                    // Set up for the next level of searching
                    //
                    oSubSets.insert( oSubSets.end(), pSet->_oContainers.begin(), pSet->_oContainers.end() );
                    oSubSets.insert( oSubSets.end(), pSet->_oReferences.begin(), pSet->_oReferences.end() );
                }
            }

            //
            // For next level of searching, tramsfer the subsets to oPSets (and clear oSubSets in the process).
            //
            oPSets.clear();
            oPSets.swap( oSubSets );
        }
    }
}

void
DWFPropertySet::_getProperties( DWFProperty::tList& oProperties,
                                DWFProperty::tMap& oMap,
                                const DWFProperty::tList& oOrderedProperties )
    throw()
{
    DWFProperty::tList::const_iterator iProperty = oOrderedProperties.begin();
    for (; iProperty != oOrderedProperties.end(); ++iProperty)
    {
        DWFProperty* pProperty = *iProperty;
        if (oMap.insert( pProperty->category(), pProperty->name(), pProperty, false ))
        {
            oProperties.push_back( pProperty );
        }
    }
}

//////

_DWFTK_API
DWFProperty::tIterator*
DWFPropertySet::getAllProperties( const DWFString& zCategory,
                                  bool bSearchClosedSets )
    throw()
{
    //
    //  We need to cache all properties in oMap to ensure we don't get properties
    //  with duplicate name and category from nested subsets.
    //
    DWFProperty::tMap oMap;

    DWFProperty::tList oProperties(0);

    _getAllProperties( oProperties, oMap, zCategory, bSearchClosedSets );

    if (oProperties.empty())
    {
        return NULL;
    }
    else
    {
        DWFProperty::tListIterator* piProperty = DWFCORE_ALLOC_OBJECT( DWFProperty::tListIterator(oProperties) );
        return piProperty;
    }
}

_DWFTK_API
void
DWFPropertySet::_getAllProperties( DWFProperty::tList& oProperties,
                                   DWFProperty::tMap& oMap,
                                   const DWFString& zCategory,
                                   bool bSearchClosedSets )
    throw()
{
    //
    //  First grab all the properties directly in this set - there can't be any duplicates at this level.
    //
    _getProperties( oProperties, oMap, zCategory, _oProperties );

    //
    //  Now grab all properties from the subsets. Note: The search will be performed in BFS order, i.e., properties
    //  from the subsets are collected before the properties from their subsets. This affects which property with
    //  duplicate name/category will be rejected.
    //

    //
    // First search owned containers, and then search referenced containers.
    //
    std::vector<DWFPropertyContainer::tList*> oContainerList;
    oContainerList.push_back( &_oContainers );
    oContainerList.push_back( &_oReferences );

    size_t n = 0;
    for (; n<oContainerList.size(); ++n)
    {
        //
        //  These will be used for the process of gathering the next level of subsets.
        //
        DWFPropertyContainer::tList oSubSets(0);

        DWFPropertyContainer::tList oPSets = *(oContainerList[n]);
        while (!oPSets.empty())
        {
            //
            //  First loop over the current property sets to see if their properties can be included
            //
            DWFPropertyContainer::tList::iterator iSet = oPSets.begin();
            for (; iSet != oPSets.end(); ++iSet)
            {
                DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);

                if (!(pSet->isClosed()) || bSearchClosedSets)
                {
                    _getProperties( oProperties, oMap, zCategory, pSet->_oProperties );

                    //
                    // Set up for the next level of searching
                    //
                    oSubSets.insert( oSubSets.end(), pSet->_oContainers.begin(), pSet->_oContainers.end() );
                    oSubSets.insert( oSubSets.end(), pSet->_oReferences.begin(), pSet->_oReferences.end() );
                }
            }

            //
            // For next level of searching, tramsfer the subsets to oPSets (and clear oSubSets in the process).
            //
            oPSets.clear();
            oPSets.swap( oSubSets );
        }
    }
}

void
DWFPropertySet::_getProperties( DWFProperty::tList& oProperties,
                                DWFProperty::tMap& oMap,
                                const DWFString& zCategory,
                                DWFProperty::tMap& oSetProperties )
    throw()
{
    DWFProperty::tMap::Iterator* piProperty = oSetProperties.iterator( zCategory );
    for (; piProperty->valid(); piProperty->next())
    {
        DWFProperty* pProperty = piProperty->value();
        if (oMap.insert( pProperty->category(), pProperty->name(), pProperty, false ))
        {
            oProperties.push_back( pProperty );
        }
    }
    DWFCORE_FREE_OBJECT( piProperty );
}

//////

_DWFTK_API
DWFPropertySet*
DWFPropertySet::getPropertySet( const DWFString& zID,
                                teQueryIDType eIDType ) const
throw()
{
    DWFPropertySet* pMatchedSet = NULL;

    //
    //  Check owned subsets
    //
    DWFPropertyContainer::tList::const_iterator iSet = _oContainers.begin();
    for (; iSet != _oContainers.end(); ++iSet)
    {
        DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);
        if ((eIDType == DWFPropertySet::eSchemaID) && (pSet->getSchemaID() == zID) ||
            (eIDType == DWFPropertySet::eSetID) && (pSet->getSetID() == zID) ||
            (eIDType == DWFPropertySet::eUUID) && (pSet->id() == zID))
        {
            return pSet;
        }
    }

    //
    //  Recurse into the owned subsets
    //
    if (pMatchedSet == NULL)
    {
        iSet = _oContainers.begin();
        for (; iSet != _oContainers.end(); ++iSet)
        {
            DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);
            pMatchedSet = pSet->getPropertySet( zID, eIDType );
            if (pMatchedSet)
            {
                return pMatchedSet;
            }
        }
    }

    //
    //  Check referenced sets
    //
    if (pMatchedSet == NULL)
    {
        DWFPropertyContainer::tList::const_iterator iSet = _oReferences.begin();
        for (; iSet != _oReferences.end(); ++iSet)
        {
            DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);
            if ((eIDType == DWFPropertySet::eSchemaID) && (pSet->getSchemaID() == zID) ||
                (eIDType == DWFPropertySet::eSetID) && (pSet->getSetID() == zID) ||
                (eIDType == DWFPropertySet::eUUID) && (pSet->id() == zID))
            {
                return pSet;
            }
        }
    }

    //
    //  Recurse into the referenced subsets
    //
    if (pMatchedSet == NULL)
    {
        iSet = _oReferences.begin();
        for (; iSet != _oReferences.end(); ++iSet)
        {
            DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iSet);
            pMatchedSet = pSet->getPropertySet( zID, eIDType );
            if (pMatchedSet)
            {
                return pMatchedSet;
            }
        }
    }
    return pMatchedSet;
}

//////

_DWFTK_API
DWFPropertySet::tIterator*
DWFPropertySet::getPropertySets( bool bReturnClosedSets )
throw()
{
    DWFPropertySet::tList oPropertySets(0);

    _getPropertySets( oPropertySets, bReturnClosedSets );

    if (oPropertySets.empty())
    {
        return NULL;
    }
    else
    {
        DWFPropertySet::tListIterator* piSets = DWFCORE_ALLOC_OBJECT( DWFPropertySet::tListIterator(oPropertySets) );
        return piSets;
    }
}

_DWFTK_API
void 
DWFPropertySet::_getPropertySets( DWFPropertySet::tList& oPropertySets,
                                  bool bReturnClosedSets )
throw()
{
    if (_oContainers.empty() && _oReferences.empty())
    {
        return;
    }
    else
    {
        //
        //  The following will collect the sets in BFS search order, i.e. all sets at a
        //  given level are returned before their contained sets.
        //
        DWFPropertySet::tList::iterator iBegin = oPropertySets.end();

        DWFPropertyContainer::tList::iterator iContainer = _oContainers.begin();
        for (; iContainer != _oContainers.end(); ++iContainer)
        {
            oPropertySets.push_back( dynamic_cast<DWFPropertySet*>(*iContainer) );
        }
        iContainer = _oReferences.begin();
        for (; iContainer != _oReferences.end(); ++iContainer)
        {
            oPropertySets.push_back( dynamic_cast<DWFPropertySet*>(*iContainer) );
        }

        DWFPropertySet::tList::iterator iEnd = oPropertySets.end();

        while (iBegin != iEnd)
        {
            DWFPropertySet::tList::iterator iContainer = iBegin;
            for (; iContainer != iEnd; ++iContainer)
            {
                DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(*iContainer);
                if (bReturnClosedSets || !(pSet->isClosed()))
                {
                    DWFPropertyContainer::tList::iterator iSubset = pSet->_oContainers.begin();
                    for (; iSubset != pSet->_oContainers.end(); ++iSubset)
                    {
                        oPropertySets.push_back( dynamic_cast<DWFPropertySet*>(*iSubset) );
                    }
                    iSubset = pSet->_oReferences.begin();
                    for (; iSubset != pSet->_oReferences.end(); ++iSubset)
                    {
                        oPropertySets.push_back( dynamic_cast<DWFPropertySet*>(*iSubset) );
                    }
                }
            }
            iBegin = iEnd;
            iEnd = oPropertySets.end();
        }
    }
}

//////

_DWFTK_API
DWFPropertySet*
DWFPropertySet::addPropertySet( const DWFString& zLabel )
throw( DWFException )
{
    //
    // Create the set and add using the base class call. For the add we need to cast it to
    // the type that the base class understands for it's containers.
    //
    DWFPropertySet* pSet = DWFCORE_ALLOC_OBJECT( DWFPropertySet( zLabel ) );
    pSet->setContent( _pContent );
    pSet->setParentSet( this );

    DWFPropertyContainer::addPropertyContainer( pSet );

    return pSet;
}

_DWFTK_API
void
DWFPropertySet::addPropertySet( DWFPropertySet* pSet )
throw( DWFException )
{
    pSet->setContent( _pContent );
    pSet->setParentSet( this );
    DWFPropertyContainer::addPropertyContainer( pSet );
}

_DWFTK_API
void
DWFPropertySet::referencePropertySet( DWFPropertySet* pSet )
throw( DWFException )
{
    DWFPropertyContainer::referencePropertyContainer( *pSet );
}

_DWFTK_API
void
DWFPropertySet::addPropertyContainer( DWFPropertyContainer* pPropertySet )
throw( DWFException )
{
    DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>( pPropertySet );
    if (pSet == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Property sets only accept property set derived classes as containers to add." );
    }
    pSet->setContent( _pContent );

    DWFPropertyContainer::addPropertyContainer( pPropertySet );
}

_DWFTK_API
void
DWFPropertySet::referencePropertyContainer( const DWFPropertyContainer& rPropertySet )
throw( DWFException )
{
    const DWFPropertySet* pPropertySet = dynamic_cast<const DWFPropertySet*>(&rPropertySet);
    if (pPropertySet == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Property sets only accept property set derived classes as containers to reference." );
    }

    DWFPropertyContainer::referencePropertyContainer( *pPropertySet );
}

_DWFTK_API
void
DWFPropertySet::setContent( DWFContent* pContent )
throw()
{
    if (pContent && 
        (pContent != _pContent))
    {
        _pContent = pContent;

        if (_oContainers.size() > 0)
        {
            DWFPropertyContainer::tList::iterator iContainer = _oContainers.begin();
            for (; iContainer != _oContainers.end(); ++iContainer)
            {
                //
                // All containers should be property sets if in a property set
                //
                DWFPropertySet* pPropertySet = dynamic_cast<DWFPropertySet*>(*iContainer);
                if (pPropertySet)
                {
                    pPropertySet->setContent( pContent );
                }
            }
        }
    }
}

_DWFTK_API
void
DWFPropertySet::parseAttributeList( const char** ppAttributeList,
                                    tUnresolvedList& rUnresolved )
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
        // set the ID
        //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x01;
            identify( ppAttributeList[iAttrib+1] );
        }

        //
        // set the label
        //
        if (!(nFound & 0x02) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Label) == 0))
        {
            nFound |= 0x02;
            _zLabel.assign( ppAttributeList[iAttrib+1] );
        }

        //
        // set the property set schema ID
        //
        if (!(nFound & 0x04) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SchemaID) == 0))
        {
            nFound |= 0x04;
            _zSchemaID.assign( ppAttributeList[iAttrib+1] );
        }

        //
        // set the property set set ID
        //
        if (!(nFound & 0x08) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_SetID) == 0))
        {
            nFound |= 0x08;
            _zSetID.assign( ppAttributeList[iAttrib+1] );
        }

        //
        // determine if the set is closed or not
        //
        if (!(nFound & 0x10) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Closed) == 0))
        {
            nFound |= 0x10;
            _bClosed = (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], "true") == 0);
        }

        //
        // get any property set references
        //
        if (!(nFound & 0x20) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Refs) == 0))
        {
            nFound |= 0x20;
            rUnresolved.push_back( tUnresolved(ePropertySetReferences, DWFString(ppAttributeList[iAttrib+1])) );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFPropertySet::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                      unsigned int /*nFlags*/ )
throw( DWFException )
{
    if (id().bytes() == 0)
    {
        identify( rSerializer.nextUUID(true) );
    }
    rSerializer.addAttribute( DWFXML::kzAttribute_ID, id() );
    rSerializer.addAttribute( DWFXML::kzAttribute_Label, _zLabel );
    rSerializer.addAttribute( DWFXML::kzAttribute_SchemaID, _zSchemaID );
    rSerializer.addAttribute( DWFXML::kzAttribute_SetID, _zSetID );
    if (_bClosed)
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Closed, /*NOXLATE*/L"true" );
    }

    //
    //  If the set has references, serialize them into an attribute, adding IDs if necessary.
    //
    if (_oReferences.size() > 0)
    {
        DWFPropertyContainer::tList::iterator iReference = _oReferences.begin();

        DWFString zReferences;
        DWFPropertySet* pReference = NULL;
        for (; iReference != _oReferences.end(); ++iReference)
        {
            //
            //  Property sets can only reference property set derived classes
            //
            pReference = dynamic_cast<DWFPropertySet*>(*iReference);
            if (pReference->id().bytes() == 0)
            {
                pReference->identify( rSerializer.nextUUID(true) );
            }

            zReferences.append( pReference->id() );
            zReferences.append( /*NOXLATE*/L" " );
        }

        if (zReferences.bytes() > 0)
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Refs, zReferences );
        }
    }
}

_DWFTK_API
void
DWFPropertySet::_serializeXML( DWFXMLSerializer& rSerializer,
                               unsigned int nFlags )
throw( DWFException )
{
    if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
    {
        DWFString zNamespace;
        //
        // namespace dictated by document and section type
        //
        if (nFlags & DWFPackageWriter::eDescriptor ||
            nFlags & DWFPackageWriter::eGlobalContent ||
            nFlags & DWFPackageWriter::eSectionContent)
        {
            zNamespace.assign( _oSerializer.namespaceXML( nFlags ) );
        }
        //
        // Start the property set element
        //
        rSerializer.startElement( DWFXML::kzElement_Properties, zNamespace );
        _serializeAttributes( rSerializer, nFlags );
    }

    //
    //  Serialize child elements
    //
    {
        bool bCallerOpenedElement = false;
        if (nFlags & DWFXMLSerializer::eElementOpen)
        {
            bCallerOpenedElement = true;
            nFlags &= ~DWFXMLSerializer::eElementOpen;
        }

        //
        //  If the set directly owns properties - serialize them first
        //
        DWFProperty::tList::iterator iProperty = _oOrderedProperties.begin();
        for (; iProperty != _oOrderedProperties.end(); ++iProperty)
        {
            (*iProperty)->serializeXML( rSerializer, nFlags );
        }

        //
        //  If the set has subsets - property sets or property containers, serialize them now.
        //
        DWFPropertyContainer::tList::iterator iContainer = _oContainers.begin();
        for (; iContainer != _oContainers.end(); iContainer++)
        {
            DWFXMLSerializable& rSerializable = (*iContainer)->getSerializable();
            rSerializable.serializeXML( rSerializer, nFlags );
        }

        //
        //  Set the flag back to the way it came in
        //
        if (bCallerOpenedElement)
        {
            nFlags |= DWFXMLSerializer::eElementOpen;
        }
    }

    if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
    {
        rSerializer.endElement();
    }
}

#endif
