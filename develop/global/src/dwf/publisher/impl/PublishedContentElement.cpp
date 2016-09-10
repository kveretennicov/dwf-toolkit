//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/PublishedContentElement.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#ifndef DWFTK_READ_ONLY

#include "dwf/package/SectionContentResource.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPublishedContentElement::DWFPublishedContentElement( DWFPublishedContentElement::tKey nKey,
                                                        const DWFString&                 zName )
throw()
                          : DWFPublishedObject( nKey, zName )
                          , _bHideFromDefaultModel( false )
                          , _pNamedSegIncludeRef( NULL )
                          //, _oContentRef()
                          //, _pContentRef( NULL )
{
}

_DWFTK_API
DWFPublishedContentElement::DWFPublishedContentElement( DWFPublishedContentElement& rObject )
throw()
                          : DWFPublishedObject( rObject )
                          , _bHideFromDefaultModel( false )
                          , _pNamedSegIncludeRef( NULL )
                          //, _oContentRef()
                          //, _pContentRef( null )
{
}

_DWFTK_API
DWFPublishedContentElement&
DWFPublishedContentElement::operator=( DWFPublishedContentElement& rObject )
throw()
{
    DWFPublishedObject::operator=(rObject);
    _bHideFromDefaultModel = rObject._bHideFromDefaultModel;
    _pNamedSegIncludeRef = rObject._pNamedSegIncludeRef;

    return *this;
}

_DWFTK_API
DWFPublishedContentElement::~DWFPublishedContentElement()
throw()
{
    if (_pNamedSegIncludeRef)
    {
        DWFCORE_FREE_OBJECT( _pNamedSegIncludeRef );
    }
}

_DWFTK_API
void
DWFPublishedContentElement::addReference( DWFPublishedObject*   pObject,
                                          tKey                  nKey,
                                          const DWFString*      pzInstanceName,
                                          bool                  /*bFromNamedSegment*/ )
throw( DWFException )
{
    DWFPublishedContentElement* pObj = dynamic_cast<DWFPublishedContentElement*>(pObject);
    if (pObj == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The object passed in is not a DWFPublishedContentElement" );
    }

    tReference* pNewRef = DWFCORE_ALLOC_OBJECT( tReference() );

    //
    //  Key of the segment reference the object
    //
    pNewRef->nKey     = nKey;
    //
    //  The referenced object
    //
    pNewRef->pObj     = pObj;
    //
    //  Assign a name for the reference
    //
    if (pzInstanceName)
    {
        pNewRef->zName.assign( *pzInstanceName );
    }

    //
    //  If this came from a named object the segment key is the same as the published object key
    //
    if (nKey == key())
    {
        _pNamedSegIncludeRef = pNewRef;
        //_pContentRef = NULL;
    }
    else
    {
        _oReferenceList.push_back( pNewRef );
        //_oContentRef.push_back( NULL );
    }
}

_DWFTK_API
bool
DWFPublishedContentElement::setHideFromDefaultModel( DWFPublishedObject::tKey nKey )
throw()
{
    bool bFound = false;

    if (nKey == key())
    {
        _bHideFromDefaultModel = true;
        bFound = true;
    }
    else
    {
        DWFPublishedObject::tReferenceList& rRefs = references();

        unsigned int iList = 0;
        for (; iList < rRefs.size(); ++iList)
        {
            DWFPublishedObject::tReference* pRef = rRefs[iList];
            if (pRef->nKey == nKey)
            {
                pRef->bHideFromModel = true;
                bFound = true;
            }
        }
    }

    return bFound;
}

_DWFTK_API
void
DWFPublishedContentElement::setContentEntity( DWFEntity*        /*pEntity*/, 
                                              tKey              /*nKey*/, 
                                              const DWFString*  /*pzInstanceName */)
throw( DWFException )
{
    //
    //  TODO: Implement or remove before final release of 7.2.0
    //
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not implemented yet." );

    //if (pEntity == NULL)
    //{
    //    _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A null content entity was passed in the arguments" );
    //}
    //else
    //{
    //    tReference* pNewRef = DWFCORE_ALLOC_OBJECT( tReference() );
    //    pNewRef->nKey = nKey;
    //    if (pzInstanceName)
    //    {
    //        pNewRef->zName.assign( *pzInstanceName );
    //    }

    //    //
    //    //  If this came from a named object the segment key is the same as the published object key
    //    //
    //    if (nKey == key())
    //    {
    //        _pNamedSegIncludeRef = pNewRef;
    //        _pContentRef = pEntity;
    //    }
    //    else
    //    {
    //        _oReferenceList.push_back( pNewRef );
    //        _oContentRef.push_back( pEntity );
    //    }
    //}
}

_DWFTK_API
void
DWFPublishedContentElement::setContentObject( DWFObject*    /*pObject*/, 
                                              tKey          /*nKey*/ )
throw( DWFException )
{
    //
    //  TODO: Implement or remove before final release of 7.2.0
    //
    _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not implemented yet." );

    //if (pObject == NULL)
    //{
    //    _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A null content object was passed in the arguments" );
    //}
    //else
    //{
    //    tReference* pNewRef = DWFCORE_ALLOC_OBJECT( tReference() );
    //    pNewRef->nKey = nKey;

    //    //
    //    //  If this came from a named object the segment key is the same as the published object key
    //    //
    //    if (nKey == key())
    //    {
    //        _pNamedSegIncludeRef = pNewRef;
    //        _pContentRef = pObject;
    //    }
    //    else
    //    {
    //        _oReferenceList.push_back( pNewRef );
    //        _oContentRef.push_back( pObject );
    //    }
    //}
}

//// Factory methods

_DWFTK_API
DWFPublishedContentElement::Factory::Factory()
throw()
                                   : DWFPublishedObject::Factory()
                                   , _oPublishedElements()
{
    ;
}

_DWFTK_API
DWFPublishedContentElement::Factory::~Factory()
throw()
{
    DWFPublishedContentElement::tMap::Iterator* piElement = _oPublishedElements.iterator();

    if (piElement)
    {
        for (; piElement->valid(); piElement->next())
        {
            DWFCORE_FREE_OBJECT( piElement->value() );
        }

        DWFCORE_FREE_OBJECT( piElement );
    }
}

_DWFTK_API
DWFPublishedObject*
DWFPublishedContentElement::Factory::makePublishedObject( tKey              nKey,
                                                          const DWFString&  zName )
throw( DWFException )
{
    //
    // allocate the object
    //
    DWFPublishedContentElement* pObj = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement(nKey, zName) );

    if (pObj == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
    }

    //
    // when this container is destroyed, it will
    // take these objects down with it
    //
    _oPublishedElements.insert( nKey, pObj );

    return pObj;
}

_DWFTK_API
DWFPublishedObject&
DWFPublishedContentElement::Factory::findPublishedObject( tKey nKey )
throw( DWFException )
{
    DWFPublishedContentElement** ppObject = _oPublishedElements.find( nKey );

    if (ppObject == NULL)
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Object not found" );
    }

    return **ppObject;
}

//// Visitor methods

_DWFTK_API
DWFPublishedContentElement::Visitor::Visitor()
throw( DWFException )
                                 : DWFPublishedObject::Visitor()
                                 , _pContent( NULL )
                                 , _pContentResource( NULL )
                                 , _zResourceID()
                                 , _oExcludedInstances( false )
                                 , _oIndexToElement()
                                 , _oNotifySinks()
{;}


_DWFTK_API
void 
DWFPublishedContentElement::Visitor::setContentResource( DWFSectionContentResource* pResource )
throw()
{
    _pContentResource = pResource;

    if (_pContentResource)
    {
        _zResourceID = _pContentResource->objectID();
    }
    else
    {
        _zResourceID.assign( L"" );
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::visitPublishedObject( DWFPublishedObject& rPublishedObject )
throw( DWFException )
{
    DWFPublishedContentElement& rPubElement = (DWFPublishedContentElement&)(rPublishedObject);

    if (_pContent == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The content pointer should not be null when visiting objects." );
    }

    //
    //  This indirectly insures that we have proper resource ID
    //
    if (_pContentResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The resource was not set before visiting the objects." );
    }

    DWFContentElement* pElement = NULL;

    //
    //  Check to see if this is a referenced object - i.e. an entity. If it doesn't exist it will be created
    //  in the process.
    //
    if (rPubElement.isReferenced())
    {
        pElement = _getReferencedEntity( &rPubElement );
    }
    else
    {
        pElement = _findContentElement( rPubElement.index() );
    }

    //
    //  Nothing was found - so it's an object that hasn't been created yet.
    //
    if (pElement == NULL)
    {
        //
        //  If there is a parent object, grab it.
        //
        DWFObject* pParentObject = _getParentContentObject( &rPubElement );

        //
        //  Create the entity, object and instance corresponding to this published element.
        //  If there is an include on a named segment we need to ensure that we use the entity
        //  associated with the include to create the object. This must be done before looping
        //  over the list of references which come from unnamed segments below this object's
        //  segment.
        //
        DWFEntity* pEntity = NULL;
        DWFPublishedObject::tReference* pRef = rPubElement.getNamedSegmentIncludeRef();
        if (pRef)
        {
            pEntity = _getReferencedEntity( pRef->pObj );
        }
        else
        {
            //
            //  Will this ever happen?
            //
            pEntity = _pContent->addEntity();
            //
            //  Notify anyone who is listening about the creation
            //
            _notifyEntityCreated( rPubElement.key(), pEntity );
        }
        //
        // Now create the proper object and instance and set the object properties, if any.
        //
        DWFObject* pObject = _pContent->addObject( pEntity, pParentObject );
        DWFInstance* pInstance = _pContent->addInstance( _zResourceID, pObject, rPubElement.index() );
        _copyProperties( &rPubElement, pObject );
        pObject->setLabel( rPubElement.name() );
        
        //
        //  Notify anyone who is listening about the creation
        //
        _notifyObjectCreated( rPubElement.key(), pObject );
        _notifyInstanceCreated( rPubElement.key(), pInstance );
        
        if (rPubElement.hideFromDefaultModel())
        {
            _oExcludedInstances.insert( pInstance );
        }

        _oIndexToElement.insert( std::make_pair( rPubElement.index(), pObject ) );

        /////////////////////////////////

        //
        //  Now search the references
        //
        DWFPublishedObject::tReferenceList& rRefs = rPubElement.references();

        unsigned int iList = 0;
        for (; iList < rRefs.size(); ++iList) 
        {
            DWFPublishedContentElement::tReference* pRef = rRefs[iList];
            if (pRef->pObj == NULL)
            {
                _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Null published object reference." );
            }

            DWFEntity* pRefEntity = _getReferencedEntity( pRef->pObj );

            DWFObject* pRefObject = _pContent->addObject( pRefEntity, pObject );
            DWFInstance* pRefInstance = _pContent->addInstance( _zResourceID, pRefObject, pRef->nIndex );
            //
            // If a name was provided for the ref use that instead of the entity name for the object
            //
            pRefObject->setLabel( (pRef->zName.chars() > 0) ? pRef->zName : pRefEntity->getLabel() );

            _notifyObjectCreated( pRef->nKey, pRefObject );
            _notifyInstanceCreated( pRef->nKey, pRefInstance );
            
            if (pRef->bHideFromModel)
            {
                _oExcludedInstances.insert( pRefInstance );
            }

            _oIndexToElement.insert( std::make_pair( pRef->nIndex, pRefObject ) );
        }
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::setNotificationSink( DWFPublishedContentElement::NotificationSink* pSink )
throw()
{
    if (pSink)
    {
        _tNotifySinks::iterator iFind = std::find( _oNotifySinks.begin(), _oNotifySinks.end(), pSink );

        //
        //  Don't add the same sink twice.
        //
        if (iFind == _oNotifySinks.end())
        {
            _oNotifySinks.push_back( pSink );
        }
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::removeNotificationSink( DWFPublishedContentElement::NotificationSink* pSink )
throw()
{
    if (pSink)
    {
        _tNotifySinks::iterator iEnd = std::remove( _oNotifySinks.begin(), _oNotifySinks.end(), pSink );
        _oNotifySinks.erase( iEnd, _oNotifySinks.end() );
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::reset()
throw()
{
    _oExcludedInstances.clear();
    _oIndexToElement.clear();
    _oNotifySinks.clear();
}

_DWFTK_API
DWFObject*
DWFPublishedContentElement::Visitor::_getParentContentObject( DWFPublishedContentElement* pPublishedElement )
throw( DWFException )
{
    DWFObject* pParentObject = NULL;

    DWFPublishedContentElement* pPublishedParent = (DWFPublishedContentElement*)(pPublishedElement->parent());
    if (pPublishedParent)
    {
        DWFContentElement* pParentElement = _findContentElement( pPublishedParent->index() );
        if (pParentElement)
        {
            pParentObject = dynamic_cast<DWFObject*>(pParentElement);
            if (pParentObject == NULL)
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Parent of an DWFObject should be a DWFObject" );
            }
        }
        else
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No element corresponding to the published object was found" );
        }
    }

    return pParentObject;
}

_DWFTK_API
DWFContentElement*
DWFPublishedContentElement::Visitor::_findContentElement( DWFPublishedObject::tIndex index )
throw()
{
    _tMapElement::iterator iElement = _oIndexToElement.find( index );
    if (iElement != _oIndexToElement.end())
    {
        return iElement->second;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity*
DWFPublishedContentElement::Visitor::_getReferencedEntity( DWFPublishedObject* pPublishedObject )
throw( DWFException )
{
    DWFEntity* pEntity = NULL;

    if (pPublishedObject->isReferenced())
    {
        DWFContentElement* pElement = _findContentElement( pPublishedObject->index() );
        if (pElement)
        {
            pEntity = dynamic_cast<DWFEntity*>(pElement);
            if (pEntity == NULL)
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A referenced object did not map to an entity" );
            }
        }
        else
        {
            //
            //  Create the entity and set it's properties
            //
            pEntity = _pContent->addEntity();
            pEntity->setLabel( pPublishedObject->name() );
            _copyProperties( pPublishedObject, pEntity );

            //
            //  Notify anyone who is listening about the creation
            //
            _notifyEntityCreated( pPublishedObject->key(), pEntity );

            //
            //  Save to the index map for later lookup
            //
            _oIndexToElement.insert( std::make_pair( pPublishedObject->index(), pEntity ) );
        }
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Entity requested using non-referenced published object" );
    }

    return pEntity;
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::_copyProperties( DWFPropertyContainer* pFrom, 
                                                      DWFPropertyContainer* pTo, 
                                                      bool bTakeContainers )
throw()
{
    
    //
    // copy properties
    //
    DWFProperty::tMap::Iterator* piProperties = pFrom->getProperties();
    if (piProperties)
    {
        for(; piProperties->valid(); piProperties->next())
        {
            pTo->addProperty( piProperties->value(), false );
        }

        DWFCORE_FREE_OBJECT( piProperties );
    }

    //
    // Take the containers - or reference them
    //
    DWFPropertyContainer::tList oContainers;
    pFrom->removeOwnedPropertyContainers( oContainers );

    DWFPropertyContainer::tList::iterator iContainer = oContainers.begin();
    if (bTakeContainers)
    {
        for (; iContainer != oContainers.end(); ++iContainer)
        {
            pTo->addPropertyContainer( *iContainer );
        }
    }
    else
    {
        for (; iContainer != oContainers.end(); ++iContainer)
        {
            pTo->referencePropertyContainer( *(*iContainer) );
        }
    }

    //
    // Copy the references
    //
    oContainers.clear();
    pFrom->removeReferencedPropertyContainers( oContainers );
    iContainer = oContainers.begin();
    for (; iContainer != oContainers.end(); ++iContainer)
    {
        pTo->referencePropertyContainer( *(*iContainer) );
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::_notifyInstanceCreated( tKey nKey, DWFInstance* pInstance )
throw()
{
    if (!_oNotifySinks.empty())
    {
        _tNotifySinks::iterator iSink = _oNotifySinks.begin();
        for (; iSink != _oNotifySinks.end(); ++iSink)
        {
            (*iSink)->notifyInstanceCreated( nKey, pInstance );
        }
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::_notifyObjectCreated( tKey nKey, DWFObject* pObject )
throw()
{
    if (!_oNotifySinks.empty())
    {
        _tNotifySinks::iterator iSink = _oNotifySinks.begin();
        for (; iSink != _oNotifySinks.end(); ++iSink)
        {
            (*iSink)->notifyObjectCreated( nKey, pObject );
        }
    }
}

_DWFTK_API
void
DWFPublishedContentElement::Visitor::_notifyEntityCreated( tKey nKey, DWFEntity* pEntity )
throw()
{
    if (!_oNotifySinks.empty())
    {
        _tNotifySinks::iterator iSink = _oNotifySinks.begin();
        for (; iSink != _oNotifySinks.end(); ++iSink)
        {
            (*iSink)->notifyEntityCreated( nKey, pEntity );
        }
    }
}


#endif


