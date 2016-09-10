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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/ContentElement.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/Constants.h"
#include "dwf/package/ContentElement.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentElement::DWFContentElement( const DWFString& zID,
                                      DWFContent* pContent )
throw()
                 : DWFPropertySet()
{
    identify( zID );
    setContent( pContent );
}

_DWFTK_API
DWFContentElement::~DWFContentElement()
throw()
{
}

//////

_DWFTK_API
const DWFProperty*
DWFContentElement::getProperty( const DWFString& zName,
                                const DWFString& zCategory,
                                bool bSearchClosedSets )
throw( DWFException )
{
    const DWFProperty* pProperty = DWFPropertySet::getProperty( zName, zCategory, bSearchClosedSets );

    //
    //  Search the inherited properties
    //
    if (pProperty == NULL)
    {
        DWFContentElement::tVector oAncestors;
        insertPropertyAncestors( oAncestors );

        DWFContentElement::tVector oTmp(0);

        while (!oAncestors.empty())
        {
            //
            //  Get the properties from each ancestor. Cast it to a property set 
            //
            DWFContentElement::tVector::iterator iElement = oAncestors.begin();
            for (; iElement != oAncestors.end(); ++iElement)
            {
                DWFContentElement* pElement = *iElement;

                pProperty = pElement->DWFPropertySet::getProperty( zName, zCategory, bSearchClosedSets );
                if (pProperty)
                {
                    return pProperty;
                }

                //
                // Collect ancestors in oTmp for searching next level of ancestors
                //
                pElement->insertPropertyAncestors( oTmp );
            }

            //
            //  For the next round of ancestor property searches grab the ancestors from oTmp, and clear oTmp
            //
            oAncestors.clear();
            oAncestors.swap( oTmp );
        }
    }

    return pProperty;
}

//////////

_DWFTK_API
DWFProperty::tIterator*
DWFContentElement::getAllProperties( bool bSearchClosedSets )
throw()
{
    //
    //  Collect the properties in this list
    //
    DWFProperty::tList oProperties;

    //
    //  This map ensures that duplicates do not occur
    //
    DWFProperty::tMap oMap;

    //
    // First get properties directly owned by this element
    //
    _getAllProperties( oProperties, oMap, bSearchClosedSets );

    //
    // Now get the inherited properties
    //

    DWFContentElement::tVector oAncestors;
    insertPropertyAncestors( oAncestors );

    DWFContentElement::tVector oTmp(0);

    while (!oAncestors.empty())
    {
        //
        //  Get the properties from each ancestor
        //
        DWFContentElement::tVector::iterator iElement = oAncestors.begin();
        for (; iElement != oAncestors.end(); ++iElement)
        {
            DWFContentElement* pElement = *iElement;

            pElement->_getAllProperties( oProperties, oMap, bSearchClosedSets );

            //
            // Also grab the ancestors 
            //
            pElement->insertPropertyAncestors( oTmp );
        }

        //
        //  For the next round of ancestor properties grab the ancestors from oTmp, and clear oTmp
        //
        oAncestors.clear();
        oAncestors.swap( oTmp );
    }

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

//////////

_DWFTK_API
DWFProperty::tIterator*
DWFContentElement::getAllProperties( const DWFString& zCategory,
                                     bool bSearchClosedSets )
throw()
{
    //
    //  Collect the properties in this list
    //
    DWFProperty::tList oProperties;

    //
    //  This map ensures that duplicates do not occur
    //
    DWFProperty::tMap oMap;

    //
    // First get properties directly owned by this element
    //
    _getAllProperties( oProperties, oMap, zCategory, bSearchClosedSets );

    //
    // Now get the inherited properties
    //

    DWFContentElement::tVector oAncestors;
    insertPropertyAncestors( oAncestors );

    DWFContentElement::tVector oTmp(0);

    while (!oAncestors.empty())
    {
        //
        //  Get the properties from each ancestor
        //
        DWFContentElement::tVector::iterator iElement = oAncestors.begin();
        for (; iElement != oAncestors.end(); ++iElement)
        {
            DWFContentElement* pElement = *iElement;

            pElement->_getAllProperties( oProperties, oMap, zCategory, bSearchClosedSets );

            //
            // Also grab the ancestors 
            //
            pElement->insertPropertyAncestors( oTmp );
        }

        //
        //  For the next round of ancestor properties grab the ancestors from oTmp, and clear oTmp
        //
        oAncestors.clear();
        oAncestors.swap( oTmp );
    }

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
DWFProperty::tIterator*
DWFContentElement::getAllPropertiesBySchemaID( const DWFString& zSchemaID )
throw()
{
    DWFPropertySet* pSet = getPropertySet( zSchemaID, eSchemaID );
    if (pSet)
    {
        return pSet->getAllProperties();
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFProperty::tIterator*
DWFContentElement::getAllPropertiesBySetID( const DWFString& zSetID )
throw()
{
    DWFPropertySet* pSet = getPropertySet( zSetID, eSetID );
    if (pSet)
    {
        return pSet->getAllProperties();
    }
    else
    {
        return NULL;
    }
}

//////////

_DWFTK_API
DWFPropertySet*
DWFContentElement::getPropertySet( const DWFString& zID,
                                   DWFPropertySet::teQueryIDType eIDType )
throw()
{
    DWFPropertySet* pSet = NULL;

    pSet = DWFPropertySet::getPropertySet( zID, eIDType );
    
    //
    //  If not found, check the ancestors
    //
    if (pSet == NULL)
    {
        //
        //  This will be used to swap ancestor lists to collect ancestors the next level up
        //
        DWFContentElement::tVector oTmp;

        DWFContentElement::tVector oAncestors;
        insertPropertyAncestors( oAncestors );

        while (!oAncestors.empty())
        {
            //
            //  Get the properties from each ancestor
            //
            DWFContentElement::tVector::iterator iElement = oAncestors.begin();
            for (; iElement != oAncestors.end(); ++iElement)
            {
                pSet = (*iElement)->DWFPropertySet::getPropertySet( zID, eIDType );
                if (pSet)
                {
                    return pSet;
                }
            }

            //
            //  Let the temporary list take over the ancestors clearing the other for ancestors higher up.
            //
            oTmp.clear();
            oTmp.swap( oAncestors );

            //
            //  Now collect the ancestors higher up
            //
            iElement = oTmp.begin();
            for (; iElement != oTmp.end(); ++iElement)
            {
                (*iElement)->insertPropertyAncestors( oAncestors );
            }
        }
    }

    return pSet;
}

//////////

_DWFTK_API
DWFPropertySet::tIterator*
DWFContentElement::getPropertySets( bool bReturnClosedSets )
throw()
{
    DWFPropertySet::tList oPropertySets;

    _getPropertySets( oPropertySets, bReturnClosedSets );

    //
    // Now get the inherited property sets
    //

    DWFContentElement::tVector oAncestors;
    insertPropertyAncestors( oAncestors );

    DWFContentElement::tVector oTmp(0);

    while (!oAncestors.empty())
    {
        //
        //  Get the properties from each ancestor
        //
        DWFContentElement::tVector::iterator iElement = oAncestors.begin();
        for (; iElement != oAncestors.end(); ++iElement)
        {
            DWFContentElement* pElement = *iElement;

            pElement->_getPropertySets( oPropertySets, bReturnClosedSets );

            //
            // Also grab the ancestors 
            //
            pElement->insertPropertyAncestors( oTmp );
        }

        //
        //  For the next round of ancestor properties grab the ancestors from oTmp, and clear oTmp
        //
        oAncestors.clear();
        oAncestors.swap( oTmp );
    }

    if (oPropertySets.empty())
    {
        return NULL;
    }
    else
    {
        DWFPropertySet::tListIterator* piPropertySet = DWFCORE_ALLOC_OBJECT( DWFPropertySet::tListIterator(oPropertySets) );
        return piPropertySet;
    }
}

//////////

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFContentElement::_serializeAttributes( DWFXMLSerializer& rSerializer, 
                                         unsigned int nFlags )
throw( DWFException )
{
    DWFPropertySet::_serializeAttributes( rSerializer, nFlags );
}

_DWFTK_API
void
DWFContentElement::_serializeXML( DWFXMLSerializer& rSerializer, 
                                  unsigned int nFlags )
throw( DWFException )
{
    //
    //  This will only be called from a derived class during content serialization
    //
    if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            //
            //  Open an element
            //
            DWFString zNamespace( _oSerializer.namespaceXML( nFlags ) );
            rSerializer.startElement( DWFXML::kzElement_ContentElement, zNamespace );

            //
            //  Serialize the attributes only if the element is opened here
            //
            _serializeAttributes( rSerializer, nFlags );
        }

        //
        //  Serialize child elements
        //
        {
            //
            //  Let the baseclass know not to start a new XML element
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

            DWFPropertySet::_serializeXML( rSerializer, nFlags );

            if (bElementOpenFlag == false)
            {
                //
                //  Unset the element open bit to allow proper ending
                //
                nFlags &= ~DWFXMLSerializer::eElementOpen;
            }
        }

        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
}

#endif



