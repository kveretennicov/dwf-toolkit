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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/PublishedDefinedObject.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#ifndef DWFTK_READ_ONLY

#include "dwf/publisher/impl/PublishedDefinedObject.h"
using namespace DWFToolkit;

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const wchar_t* const DWFPublishedDefinedObject::Visitor::kz_PropName_Published_Object =    /*NOXLATE*/L"_name";
const wchar_t* const DWFPublishedDefinedObject::Visitor::kz_PropCategory_Hidden =          /*NOXLATE*/L"hidden";

#endif


_DWFTK_API
DWFPublishedDefinedObject::DWFPublishedDefinedObject( tKey             nKey,
                                                      const DWFString& zName )
throw()
                         : DWFPublishedObject( nKey, zName )
{
    ;
}

_DWFTK_API
DWFPublishedDefinedObject::~DWFPublishedDefinedObject()
throw()
{
    ;
}

_DWFTK_API
void
DWFPublishedDefinedObject::addPropertyContainer( DWFPropertyContainer* pPropertyContainer )
throw( DWFException )
{
    DWFPropertyContainer::addPropertyContainer( pPropertyContainer );
}

_DWFTK_API
void
DWFPublishedDefinedObject::referencePropertyContainer( const DWFPropertyContainer& rPropertyContainer )
throw( DWFException )
{
    DWFPropertyContainer::referencePropertyContainer( rPropertyContainer );
}


//// Factory methods

DWFPublishedDefinedObject::Factory::Factory()
throw()
                                   : DWFPublishedObject::Factory()
                                   , _oPublishedObjects()
{
    ;
}

_DWFTK_API
DWFPublishedDefinedObject::Factory::~Factory()
throw()
{
    DWFPublishedDefinedObject::tMap::Iterator* piObject = _oPublishedObjects.iterator();

    if (piObject)
    {
        for (; piObject->valid(); piObject->next())
        {
            DWFCORE_FREE_OBJECT( piObject->value() );
        }

        DWFCORE_FREE_OBJECT( piObject );
    }
}

_DWFTK_API
DWFPublishedObject*
DWFPublishedDefinedObject::Factory::makePublishedObject( DWFPublishedObject::tKey nKey,
                                                         const DWFString&         zName )
throw( DWFException )
{
    //
    // allocate the object
    //
    DWFPublishedDefinedObject* pObj = DWFCORE_ALLOC_OBJECT( DWFPublishedDefinedObject(nKey, zName) );
    
    if (pObj == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object" );
    }

    //
    // when this container is destroyed, it will
    // take these objects down with it
    //
    _oPublishedObjects.insert( nKey, pObj );

    return pObj;
}

_DWFTK_API
DWFPublishedObject&
DWFPublishedDefinedObject::Factory::findPublishedObject( DWFPublishedObject::tKey nKey )
throw( DWFException )
{
    DWFPublishedDefinedObject** ppObject = _oPublishedObjects.find( nKey );

    if (ppObject == NULL)
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Object not found" );
    }

    return **ppObject;
}


//// Visitor methods


_DWFTK_API
DWFPublishedDefinedObject::Visitor::Visitor()
throw( DWFException )
                                  : DWFPublishedObject::Visitor()
                                  , _pObjectDefinition( NULL )
								  ,_pReferencedObjectDefinition( NULL)
                                  , _nNextInstanceID( 1 )     // MUST BE NON-ZERO
{;}

_DWFTK_API
void
DWFPublishedDefinedObject::Visitor::visitPublishedObject( DWFPublishedObject& rPublishedObject )
throw( DWFException )
{
    if (_pObjectDefinition == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The object definition resource pointer was not initialized" );
    }

    //
    // acquire the object defining this published object
    //
    DWFDefinedObject* pDefinedObject = _findDefinedObject( rPublishedObject );
    DWFDefinedObjectInstance* pDefinedInstance = NULL;

        //
        // this is the first time we have seen this published object
        //
    if (pDefinedObject == NULL)
    {
        //
        // create a definition of this published object
        //
        _definePublishedObject( rPublishedObject, &pDefinedObject, &pDefinedInstance );
    }

        //
        // we will need the instance of the defined object to
        // process any references, so create one if needed
        //
    if (pDefinedInstance == NULL)
    {
        wchar_t zNode[12];
        _DWFCORE_SWPRINTF( zNode, 12, /*NOXLATE*/L"%u", rPublishedObject.key() );
        
        pDefinedInstance = pDefinedObject->instance( zNode );

		// Since it's not added into _pObjectDefinition, collect it with _pReferencedObjectDefinition.
		// Common instances those are not referenced should be collected by _pObjectDefinition
		// in _definePublishedObject( rPublishedObject, &pDefinedObject, &pDefinedInstance ).
		// Such instances won't be wrote into objectdefinition file as before
		_pReferencedObjectDefinition->addInstance(pDefinedInstance);
    }

    //
    // now we can process any references to this published object
    //
    DWFPublishedObject::tReference* pReference = NULL;
    DWFPublishedObject::tReferenceList& rReferences = rPublishedObject.references();
    DWFPublishedObject::tReferenceList::const_iterator iReference = rReferences.begin();

    for (; iReference != rReferences.end(); iReference++)
    {
        pReference = *iReference;

            //
            // sanity check
            //
        if (pReference == NULL)
        {
            _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Encounted NULL reference" );
        }

        //
        // acquire this published object's definition
        //
        DWFDefinedObject* pReferenceObject = _findDefinedObject( *(pReference->pObj) );

            //
            // this is the first time we have seen this published object
            //
        if (pReferenceObject == NULL)
        {
            //
            // create a definition of the published reference object
            //
            _definePublishedObject( *(pReference->pObj), &pReferenceObject, NULL );
        }

        //
        // the reference relationship is captured hierarchically
        //
        pDefinedObject->addChild( pReferenceObject, pReference->pObj->key() );

        //
        // NOTE: do not add object to the container here - it already exists
        //

        //
        // the published reference object will not have had a matching instance
        // defined for it.  we must handle that here using the index from the
        // reference structure rather than the referenced published object
        //
        // the index of the reference structure corresponds to the
        // graphics directly related to this object instance.
        // this is the instance node
        //
        wchar_t zNode[12] = {0};
        _DWFCORE_SWPRINTF( zNode, 12, /*NOXLATE*/L"%u", pReference->nIndex );

        //
        // now create an instance of the defined reference object
        //
        DWFDefinedObjectInstance* pReferenceInstance = pReferenceObject->instance( zNode );

            //
            // if a name was provided for the reference
            // we add it as a property
            //
        if (pReference->zName.bytes() > 0)
        {
            pReferenceInstance->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(kz_PropName_Published_Object,
                                                                              pReference->zName,
                                                                              kz_PropCategory_Hidden)),
                                             true );
        }

        //
        // assign the instance ID for the object
        //
        pReference->pObj->setInstance( _nNextInstanceID++ );

        //
        // the reference instance becomes a child of the defined instance
        //
        pDefinedInstance->addChild( pReferenceInstance, pReference->nIndex );

        //
        // capture the instance
        //
        _pObjectDefinition->addInstance( pReferenceInstance, pReference->nIndex );
    }
}

_DWFTK_API
DWFDefinedObject*
DWFPublishedDefinedObject::Visitor::_findDefinedObject( DWFPublishedObject& rPublishedObject )
const
throw()
{
    //
    // the key of the published object is also the ID of the defined object
    //
    wchar_t zObjectID[12] = {0};
    _DWFCORE_SWPRINTF( zObjectID, 12, /*NOXLATE*/L"%u", rPublishedObject.key() );

    return _pObjectDefinition->findObject( zObjectID );
}

_DWFTK_API
void
DWFPublishedDefinedObject::Visitor::_definePublishedObject( DWFPublishedObject&        rPublishedObject,
                                                          DWFDefinedObject**         ppDefinedObject,
                                                          DWFDefinedObjectInstance** ppDefinedInstance )
throw( DWFException )
{
    if (ppDefinedObject == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"ppDefinedObject cannot be NULL" );
    }

    //
    // the key of the published object corresponds to ID of the defined object
    //
    wchar_t zObjectID[12] = {0};
    _DWFCORE_SWPRINTF( zObjectID, 12, /*NOXLATE*/L"%u", rPublishedObject.key() );

    //
    // allocate a new defined object
    //
    *ppDefinedObject = DWFCORE_ALLOC_OBJECT( DWFDefinedObject(zObjectID) );
    if (*ppDefinedObject == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new object" );
    }

    //
    // name the object with a hidden property
    // this will show up in the model navigator as browseable object
    //
    (*ppDefinedObject)->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(kz_PropName_Published_Object,
                                                                      rPublishedObject.name(),
                                                                      kz_PropCategory_Hidden)),
                                     true );

    //
    // before adding the defined object to the container,
    // check for a published parent
    //
    DWFDefinedObject* pDefinedParent = NULL;
    DWFPublishedObject* pPublishedParent = rPublishedObject.parent();

        //
        // if the published object has a parent
        // does the published parent have a defined object?
        //
    if (pPublishedParent)
    {
        pDefinedParent = _findDefinedObject( *pPublishedParent );

            //
            // if there is no defined object for the published parent
            // then we need to realize one before proceeding
            //
        if (pDefinedParent == NULL)
        {
            //
            // create a definition of this published object
            //
            _definePublishedObject( *pPublishedParent, &pDefinedParent, NULL );
        }
    }

        //
        // if the parent exists, add this child
        //
    if (pDefinedParent)
    {
        pDefinedParent->addChild( *ppDefinedObject, rPublishedObject.key() );
    }

    //
    // capture the object
    //
    _pObjectDefinition->addObject( *ppDefinedObject, rPublishedObject.key() );

        //
        // if this published object is NOT referenced
        // we need to have a defined instance as well
        //
    if (rPublishedObject.isReferenced() == false)
    {
        //
        // the index of the published object corresponds to the
        // graphics directly related to this object instance.
        // this is the instance node
        //
        wchar_t zNode[12] = {0};
        _DWFCORE_SWPRINTF( zNode, 12, /*NOXLATE*/L"%u", rPublishedObject.index() );

        //
        // create an instance of this defined object
        //
        DWFDefinedObjectInstance* pDefinedInstance = (*ppDefinedObject)->instance( zNode );
        DWFDefinedObjectInstance* pParentInstance = NULL;

            //
            // we already know the state of this published object's parent
            // if the parent exists, and we are here, then it must also have
            // a defined instance at this node
            //
        if (pDefinedParent)
        {
            //
            // look up the parent instance by the index of the published parent
            //
            _DWFCORE_SWPRINTF( zNode, 12, /*NOXLATE*/L"%u", pPublishedParent->index() );

            pParentInstance = pDefinedParent->getInstance( zNode );

                //
                // this had better be there
                //
            if (pParentInstance == NULL)
            {
                _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Inconsistent structure - expected matching parent instance" );
            }
        }

        //
        // acquire the published properties
        //
        pDefinedInstance->copyProperties( rPublishedObject );

            //
            //
            //
        if (pParentInstance)
        {
            pParentInstance->addChild( pDefinedInstance, rPublishedObject.index() );
        }

        //
        // now we can add the instance to the container
        //
        _pObjectDefinition->addInstance( pDefinedInstance, rPublishedObject.index() );

            //
            // return this pointer if possible
            //
        if (ppDefinedInstance)
        {
            *ppDefinedInstance = pDefinedInstance;
        }

        //
        // assign the instance ID for the object
        //
        rPublishedObject.setInstance( _nNextInstanceID++ );
    }
        //
        // referenced objects get the properies
        //
    else
    {
        //
        // acquire the published properties
        //
        (*ppDefinedObject)->copyProperties( rPublishedObject );
    }
}




#endif
