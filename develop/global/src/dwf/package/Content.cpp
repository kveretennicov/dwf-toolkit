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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Content.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/STL.h"
#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/Toolkit.h"
#include "dwf/Version.h"
#include "dwf/package/Content.h"
#include "dwf/package/Constants.h"
#include "dwf/package/ContentManager.h"
using namespace DWFToolkit;

template< class K, class V, class Compare, class Alloc >
bool _removeFromMultiMap( std::multimap<K, V, Compare, Alloc>& m,
                          K& k,
                          V& v,
                          bool bDeleteAllOccurancesOfValue = false )
{
    bool bErased = false;

    typename std::multimap<K, V, Compare, Alloc>::iterator it = m.find( k );
    for (; it != m.end() &&
           it->first == k; ++it)
    {
        if (it->second == v)
        {
            m.erase( it );
            bErased = true;
            if (bDeleteAllOccurancesOfValue == false)
            {
                break;
            }
        }
    }

    return bErased;
}



#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const char* const DWFContent::kzExtension_ContentXML    = ".content.xml";
//DNT_End

#endif



_DWFTK_API
DWFContent::DWFContent( DWFPackageReader* pPackageReader,
                        const DWFString& zID )
throw( DWFException )
          : DWFContentReader()
          , DWFContentResourceReader()
          , _zID( zID )
          , _zHREF( L"" )
          , _pContentManager( NULL )
          , _pPackageReader( pPackageReader )
          , _oClasses()
          , _oFeatures()
          , _oEntities()
          , _oObjects()
          , _oGroups()
          , _oInstances()
          , _oSharedProperties()
          , _oRefProperties()
          , _oBaseClassToClass()
          , _oClassToEntity()
          , _oClassToFeature()
          , _oEntityToObject()
          , _oFeatureToEntity()
          , _oFeatureToObject()
          , _oElementToGroup()
          , _oSetToSet()
          , _oResourceRenderableToInstance()
          , _bElementsLoaded(false)
          , _zVersion( _DWF_FORMAT_CONTENT_VERSION_CURRENT_STRING )
          , _oModifiedContentResources()
          , _zSerializingResource( /*NOXLATE*/L"" )
          , _oReadClassBaseClassRefs()
          , _oReadEntityChildEntityRefs()
          , _oReadGroupElementRefs()
          , _oReadPropertySetRefs()
          , _oMapFromSetToSet()
#ifndef DWFTK_READ_ONLY
          , _oAddedNamespaces()
#endif
{
    if (pPackageReader == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The package reader pointer cannot be NULL in the content constructor." );
    }

    //
    //  If an idea is provided, then it's ok to set the HREF, otherwise it will be read
    //  in since this content was constructed with a valid package reader.
    //
    if (zID.chars() > 0)
    {
        _zHREF.assign( _zID );
        _zHREF.append( DWFContent::kzExtension_ContentXML );
    }
}

_DWFTK_API
DWFContent::DWFContent( DWFContentManager* pContentManager,
                        const DWFString& zID )
throw( DWFException )
          : DWFContentReader()
          , DWFContentResourceReader()
          , _zID( zID )
          , _zHREF( /*NOXLATE*/L"" )
          , _pContentManager( pContentManager )
          , _pPackageReader( NULL )
          , _oClasses()
          , _oFeatures()
          , _oEntities()
          , _oObjects()
          , _oGroups()
          , _oInstances()
          , _oSharedProperties()
          , _oRefProperties()
          , _oBaseClassToClass()
          , _oClassToEntity()
          , _oClassToFeature()
          , _oEntityToObject()
          , _oFeatureToEntity()
          , _oFeatureToObject()
          , _oElementToGroup()
          , _oSetToSet()
          , _oResourceRenderableToInstance()
          , _bElementsLoaded(true)
          , _zVersion( _DWF_FORMAT_CONTENT_VERSION_CURRENT_STRING )
          , _oModifiedContentResources()
          , _zSerializingResource( /*NOXLATE*/L"" )
          , _oReadClassBaseClassRefs()
          , _oReadEntityChildEntityRefs()
          , _oReadGroupElementRefs()
          , _oReadPropertySetRefs()
          , _oMapFromSetToSet()
#ifndef DWFTK_READ_ONLY
          , _oAddedNamespaces()
#endif
{
    if (_pContentManager == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The content manager pointer cannot be NULL in the content constructor." );
    }

    if (_zID.chars() == 0)
    {
        _zID = _pContentManager->getIDProvider()->next(false);
    }

    _zHREF.assign( _zID );
    _zHREF.append( DWFContent::kzExtension_ContentXML );
}

_DWFTK_API
DWFContent::~DWFContent()
throw()
{
    //
    //  This will delete all allocated objects and maps
    //
    unload( false );

    //
    //  Notify observers that this class is about to be deleted
    //
    _notifyDelete();
}

_DWFTK_API
DWFString
DWFContent::href()
throw()
{
    if (_zHREF.chars() == 0)
    {
        _zHREF.assign( _zID );
        _zHREF.append( DWFContent::kzExtension_ContentXML );
    }
    return _zHREF;
}

_DWFTK_API
DWFClass::tIterator*
DWFContent::findClassByBaseClass( DWFClass* pClass )
throw()
{
    _tClassToClassMultiMap::iterator it;
    it = _oBaseClassToClass.find( pClass );

    if (it != _oBaseClassToClass.end())
    {
        DWFClass::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFClass::tCachingIterator );
        for (; it != _oBaseClassToClass.end() &&
               it->first == pClass; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFClass::tIterator*
DWFContent::findClassByBaseClass( const DWFString& zID )
throw()
{
    DWFClass* pClass = getClass( zID );
    if (pClass)
    {
        return findClassByBaseClass( pClass );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity::tIterator*
DWFContent::findEntitiesByClass( DWFClass* pClass )
throw()
{
    _tClassToEntityMultiMap::iterator it;
    it = _oClassToEntity.find( pClass );

    if (it != _oClassToEntity.end())
    {
        DWFEntity::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFEntity::tCachingIterator );
        for (; it != _oClassToEntity.end() && 
               it->first == pClass; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity::tIterator*
DWFContent::findEntitiesByClass( const DWFString& zID )
throw()
{
    DWFClass* pClass = getClass( zID );
    if (pClass)
    {
        return findEntitiesByClass( pClass );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFFeature::tIterator*
DWFContent::findFeaturesByClass( DWFClass* pClass )
throw()
{
    _tClassToFeatureMultiMap::iterator it;
    it = _oClassToFeature.find( pClass );

    if (it != _oClassToFeature.end())
    {
        DWFFeature::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFFeature::tCachingIterator );
        for (; it != _oClassToFeature.end() && 
               it->first == pClass; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFFeature::tIterator*
DWFContent::findFeaturesByClass( const DWFString& zID )
throw()
{
    DWFClass* pClass = getClass( zID );
    if (pClass)
    {
        return findFeaturesByClass( pClass );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject::tIterator*
DWFContent::findObjectsByEntity( DWFEntity* pEntity )
throw()
{
    _tEntityToObjectMultiMap::iterator it;
    it = _oEntityToObject.find( pEntity );

    if (it != _oEntityToObject.end())
    {
        DWFObject::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFObject::tCachingIterator );
        for (; it != _oEntityToObject.end() && 
               it->first == pEntity; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject::tIterator*
DWFContent::findObjectsByEntity( const DWFString& zID )
throw()
{
    DWFEntity* pEntity = getEntity( zID );
    if (pEntity)
    {
        return findObjectsByEntity( pEntity );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity::tIterator*
DWFContent::findEntitiesByFeature( DWFFeature* pFeature )
throw()
{
    _tFeatureToEntityMultiMap::iterator it;
    it = _oFeatureToEntity.find( pFeature );

    if (it != _oFeatureToEntity.end())
    {
        DWFEntity::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFEntity::tCachingIterator );
        for (; it != _oFeatureToEntity.end() && 
               it->first == pFeature; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity::tIterator*
DWFContent::findEntitiesByFeature( const DWFString& zID )
throw()
{
    DWFFeature* pFeature = getFeature( zID );
    if (pFeature)
    {
        return findEntitiesByFeature( pFeature );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject::tIterator*
DWFContent::findObjectsByFeature( DWFFeature* pFeature )
throw()
{
    _tFeatureToObjectMultiMap::iterator it;
    it = _oFeatureToObject.find( pFeature );

    if (it != _oFeatureToObject.end())
    {
        DWFObject::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFObject::tCachingIterator );
        for (; it != _oFeatureToObject.end() && 
               it->first == pFeature; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject::tIterator*
DWFContent::findObjectsByFeature( const DWFString& zID )
throw()
{
    DWFFeature* pFeature = getFeature( zID );
    if (pFeature)
    {
        return findObjectsByFeature( pFeature );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFGroup::tIterator*
DWFContent::findGroupsByElement( DWFContentElement* pElement )
throw()
{
    _tElementToGroupMultiMap::iterator it;
    it = _oElementToGroup.find( pElement );

    if (it != _oElementToGroup.end())
    {
        DWFGroup::tCachingIterator* pIter = DWFCORE_ALLOC_OBJECT( DWFGroup::tCachingIterator );
        for (; it != _oElementToGroup.end() && 
               it->first == pElement; ++it)
        {
            pIter->add( it->second );
        }

        return pIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFGroup::tIterator*
DWFContent::findGroupsByElement( const DWFString& zID )
throw()
{
    DWFContentElement* pElement = getElement( zID );
    if (pElement)
    {
        return findGroupsByElement( pElement );
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFInstance::tIterator*
DWFContent::findInstancesByRenderable( DWFRenderable* pRendered,
                                       const DWFString& zResourceID )
throw()
{
    if (pRendered == NULL)
    {
        return NULL;
    }

    DWFInstance::tCachingIterator* pInstanceIter = DWFCORE_ALLOC_OBJECT( DWFInstance::tCachingIterator );

    if (zResourceID == "")
    {
        _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.begin();
        for (; iResource != _oResourceRenderableToInstance.end(); ++iResource)
        {
            const _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

            _tRenderableToInstanceMultiMap::const_iterator itRendered = pRIMap->find( pRendered );
            for (; itRendered != pRIMap->end() && 
                   itRendered->first == pRendered; ++itRendered)
            {
                DWFInstance* pInstance = itRendered->second;
                pInstanceIter->add( pInstance );
            }
        }
    }
    else
    {
        _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.find( zResourceID );
        if (iResource != _oResourceRenderableToInstance.end())
        {
            const _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

            _tRenderableToInstanceMultiMap::const_iterator itRendered = pRIMap->find( pRendered );
            for (; itRendered != pRIMap->end() && 
                   itRendered->first == pRendered; ++itRendered)
            {
                DWFInstance* pInstance = itRendered->second;
                pInstanceIter->add( pInstance );
            }
        }
    }

    //
    // After a reset if the iterator is invalid, that implies that the iterator
    // does not have any elements to iterate over. So delete it.
    //
    pInstanceIter->reset();
    if (pInstanceIter->valid() == false)
    {
        DWFCORE_FREE_OBJECT( pInstanceIter );
        pInstanceIter = NULL;
    }

    return pInstanceIter;
}

_DWFTK_API
DWFInstance::tIterator*
DWFContent::findInstancesByRenderable( const DWFString& zRenderedID,
                                       const DWFString& zResourceID )
throw()
{
    //
    //  Try to find the renderable in object first and then in feature
    //
    DWFRenderable* pRendered = getObject( zRenderedID );
    if (pRendered == NULL)
    {
        pRendered = getFeature( zRenderedID );
    }

    if (pRendered == NULL)
    {
        return NULL;
    }
    else
    {
        return findInstancesByRenderable( pRendered, zResourceID );
    }
}

_DWFTK_API
DWFInstance::tIterator*
DWFContent::findInstancesByResourceID( const DWFString& zResourceID ) const
throw( DWFException )
{
    _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.find( zResourceID );

    if (iResource != _oResourceRenderableToInstance.end())
    {
        _tRenderableToInstanceMultiMap* pMap = iResource->second;

        if (pMap && 
            pMap->size() > 0)
        {
            DWFInstance::tCachingIterator* piInstance = DWFCORE_ALLOC_OBJECT( DWFInstance::tCachingIterator );

            _tRenderableToInstanceMultiMap::const_iterator iInstance = pMap->begin();
            for (; iInstance != pMap->end(); ++iInstance)
            {
                DWFInstance* pInstance = iInstance->second;
                piInstance->add( pInstance );
            }

            return piInstance;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The given resource ID was not found in the map" );
    }
}

_DWFTK_API
DWFIterator<DWFString>* DWFContent::findResourceIDsByRenderable( DWFRenderable* pRenderable )
throw()
{
    typedef DWFCachingIterator<DWFString> _tStringIterator;

    if (_oResourceRenderableToInstance.size() > 0)
    {
        _tStringIterator* piResourceID = DWFCORE_ALLOC_OBJECT( _tStringIterator );

        _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.begin();
        for (; iResource != _oResourceRenderableToInstance.end(); ++iResource)
        {
            _tRenderableToInstanceMultiMap* pMap = iResource->second;

            //
            //  If a valid iterator is found for the renderable, then the resource is associated with
            //  the renderable
            //
            if (pMap->find( pRenderable ) != pMap->end())
            {
                DWFString zID( iResource->first );
                piResourceID->add( zID );
            }
        }

        return piResourceID;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFIterator<DWFString>* 
DWFContent::findResourceIDsByRenderable( const DWFString& zRenderedID )
throw()
{
    //
    //  Try to find the renderable in object first and then in feature
    //
    DWFRenderable* pRendered = getObject( zRenderedID );
    if (pRendered == NULL)
    {
        pRendered = getFeature( zRenderedID );
    }

    if (pRendered == NULL)
    {
        return NULL;
    }
    else
    {
        return findResourceIDsByRenderable( pRendered );
    }
}

_DWFTK_API
DWFClass*
DWFContent::getClass( const DWFString& zID )
throw()
{
    DWFClass** ppClass = _oClasses.find( zID );
    if (ppClass)
    {
        return *ppClass;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFFeature*
DWFContent::getFeature( const DWFString& zID )
throw()
{
    DWFFeature** ppFeature = _oFeatures.find( zID );
    if (ppFeature)
    {
        return *ppFeature;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity*
DWFContent::getEntity( const DWFString& zID )
throw()
{
    DWFEntity** ppEntity = _oEntities.find( zID );
    if (ppEntity)
    {
        return *ppEntity;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject*
DWFContent::getObject( const DWFString& zID )
throw()
{
    DWFObject** ppObject = _oObjects.find( zID );
    if (ppObject)
    {
        return *ppObject;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFGroup*
DWFContent::getGroup( const DWFString& zID )
throw()
{
    DWFGroup** ppGroup = _oGroups.find( zID );
    if (ppGroup)
    {
        return *ppGroup;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFContentElement*
DWFContent::getElement( const DWFString& zID )
throw()
{
    DWFContentElement* pElement = NULL;

    pElement = getObject( zID );
    if (pElement)
    {
        return pElement;
    }

    pElement = getGroup( zID );
    if (pElement)
    {
        return pElement;
    }

    pElement = getEntity( zID );
    if (pElement)
    {
        return pElement;
    }

    pElement = getFeature( zID );
    if (pElement)
    {
        return pElement;
    }

    pElement = getClass( zID );

    return pElement;
}

_DWFTK_API
DWFInstance*
DWFContent::getInstance( const DWFString& zID )
throw()
{
    DWFInstance** ppInstance = _oInstances.find( zID );
    if (ppInstance)
    {
        return *ppInstance;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFPropertySet*
DWFContent::getSharedPropertySet( const DWFString& zID )
throw()
{
    DWFPropertySet** ppSet = _oSharedProperties.find( zID );
    if (ppSet)
    {
        return *ppSet;
    }
    else
    {
        return NULL;
    }
}

//
// The following all ADD new elements to the content library
//

_DWFTK_API
DWFClass*
DWFContent::addClass( const DWFClass::tList& oBaseClass,
                      const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the class and insert it into to the storage map.
        //
        DWFClass* pClass = DWFCORE_ALLOC_OBJECT( DWFClass( zID, this ) );
        if (_oClasses.insert( zID, pClass, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pClass );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A class with the provided ID already exists" );
        }

        //
        // Iterate over the provided base classes to add them to the class.
        //
        DWFClass::tList::ConstIterator* iter = oBaseClass.constIterator();
        if (iter)
        {
            for (; iter->valid(); iter->next())
            {
                DWFClass* pBase = iter->get();

                if (pBase)
                {
                    //
                    // Add the base classes to the class, and for downward
                    // traversal save the base-to-class relationship.
                    //
                    pClass->_addBaseClass( pBase );
                    _oBaseClassToClass.insert( std::make_pair( pBase, pClass ) );
                }
            }
            DWFCORE_FREE_OBJECT( iter );
        }

        return pClass;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFClass*
DWFContent::addClass( DWFClass* pBaseClass,
                      const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the class and insert it into to the storage map.
        //
        DWFClass* pClass = DWFCORE_ALLOC_OBJECT( DWFClass( zID, this ) );
        if (_oClasses.insert( zID, pClass, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pClass );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A class with the provided ID already exists" );
        }

        if (pBaseClass)
        {
            //
            // Add the base class to the class, and for downward
            // traversal save the base-to-class relationship.
            //
            pClass->_addBaseClass( pBaseClass );
            _oBaseClassToClass.insert( std::make_pair( pBaseClass, pClass ) );
        }

        return pClass;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity*
DWFContent::addEntity( const DWFClass::tList& oClass,
                       DWFEntity* pParentEntity,
                       const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the entity and insert it into to the storage map.
        //
        DWFEntity* pEntity = DWFCORE_ALLOC_OBJECT( DWFEntity( zID, this ) );
        if (_oEntities.insert( zID, pEntity, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pEntity );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"An entity with the provided ID already exists" );
        }

        if (pParentEntity)
        {
            pParentEntity->_addChild( pEntity );
        }

        //
        // Iterate over the provided classes to add them to the entity
        //
        DWFClass::tList::ConstIterator* iter = oClass.constIterator();
        if (iter)
        {
            for (; iter->valid(); iter->next())
            {
                DWFClass* pClass = iter->get();

                if (pClass)
                {
                    // Add the classes to the entity, and for downward
                    // traversal save the class-to-entity relationship
                    pEntity->_addClass( pClass );
                    _oClassToEntity.insert( std::make_pair( pClass, pEntity ) );
                }
            }
            DWFCORE_FREE_OBJECT( iter );
        }

        return pEntity;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity*
DWFContent::addEntity( DWFClass* pClass,
                       DWFEntity* pParentEntity,
                       const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the entity and insert it into to the storage map.
        //
        DWFEntity* pEntity = DWFCORE_ALLOC_OBJECT( DWFEntity( zID, this ) );
        if (_oEntities.insert( zID, pEntity, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pEntity );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"An entity with the provided ID already exists" );
        }

        if (pParentEntity)
        {
            pParentEntity->_addChild( pEntity );
        }

        if (pClass)
        {
            // Add the class to the entity, and for downward
            // traversal save the class-to-entity relationship
            pEntity->_addClass( pClass );
            _oClassToEntity.insert( std::make_pair( pClass, pEntity ) );
        }

        return pEntity;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFEntity*
DWFContent::addEntity( DWFEntity* pParentEntity,
                       const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the entity and insert it into to the storage map.
        //
        DWFEntity* pEntity = DWFCORE_ALLOC_OBJECT( DWFEntity( zID, this ) );
        if (_oEntities.insert( zID, pEntity, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pEntity );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"An entity with the provided ID already exists" );
        }

        if (pParentEntity)
        {
            pParentEntity->_addChild( pEntity );
        }

        return pEntity;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFObject*
DWFContent::addObject( DWFEntity* pRealizedEntity,
                       DWFObject* pParentObject,
                       const DWFString& zUUID )
throw( DWFException )
{
    if (pRealizedEntity == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Cannot create an object without a valid entity" );
    }

    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the object and insert it into to the storage map.
        //
        DWFObject* pObject = DWFCORE_ALLOC_OBJECT( DWFObject( zID, pRealizedEntity, this ) );
        if (_oObjects.insert( zID, pObject, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pObject );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"An object with the provided ID already exists" );
        }

        if (pParentObject)
        {
            pParentObject->_addChild( pObject );
        }

        //
        // For downward traversal save the entity-to-object relationship.
        //
        _oEntityToObject.insert( std::make_pair( pRealizedEntity, pObject ) );

        return pObject;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFFeature*
DWFContent::addFeature( const DWFClass::tList& oClass,
                        const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the feature and insert it into to the storage map.
        //
        DWFFeature* pFeature = DWFCORE_ALLOC_OBJECT( DWFFeature( zID, this ) );
        if (_oFeatures.insert( zID, pFeature, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pFeature );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A feature with the provided ID already exists" );
        }

        //
        // Iterate over the provided classes to add them to the feature.
        //
        DWFClass::tList::ConstIterator* iter = oClass.constIterator();
        if (iter)
        {
            for (; iter->valid(); iter->next())
            {
                DWFClass* pClass = iter->get();

                if (pClass)
                {
                    //
                    // Add the classes to the feature, and for downward
                    // traversal save the class-to-feature relationship.
                    //
                    pFeature->_addClass( pClass );
                    _oClassToFeature.insert( std::make_pair( pClass, pFeature ) );
                }
            }
            DWFCORE_FREE_OBJECT( iter );
        }

        return pFeature;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFFeature*
DWFContent::addFeature( DWFClass* pClass,
                        const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the feature and insert it into to the storage map.
        //
        DWFFeature* pFeature = DWFCORE_ALLOC_OBJECT( DWFFeature( zID, this ) );
        if (_oFeatures.insert( zID, pFeature, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pFeature );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A feature with the provided ID already exists" );
        }

        if (pClass)
        {
            //
            // Add the class to the feature, and for downward
            // traversal save the class-to-feature relationship.
            //
            pFeature->_addClass( pClass );
            _oClassToFeature.insert( std::make_pair( pClass, pFeature ) );
        }

        return pFeature;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFGroup*
DWFContent::addGroup( const DWFContentElement::tList& oElements,
                      const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the feature and insert it into to the storage map.
        //
        DWFGroup* pGroup = DWFCORE_ALLOC_OBJECT( DWFGroup( zID, this ) );
        if (_oGroups.insert( zID, pGroup, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pGroup );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A group with the provided ID already exists" );
        }

        //
        // Iterate over the provided elements to add them to the group.
        //
        DWFContentElement::tList::ConstIterator* iter = oElements.constIterator();
        if (iter)
        {
            for (; iter->valid(); iter->next())
            {
                DWFContentElement* pElement = iter->get();
                if (pElement)
                {
                    //
                    // Add the elements to the group, and for group discovery
                    // save the element-to-group relationship.
                    //
                    pGroup->_addElement( pElement );
                    _oElementToGroup.insert( std::make_pair( pElement, pGroup ) );
                }
            }
            DWFCORE_FREE_OBJECT( iter );
        }

        return pGroup;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFGroup*
DWFContent::addGroup( const DWFString& zUUID )
throw( DWFException )
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the feature and insert it into to the storage map.
        //
        DWFGroup* pGroup = DWFCORE_ALLOC_OBJECT( DWFGroup( zID, this ) );
        if (_oGroups.insert( zID, pGroup, false ) == false)
        {
            DWFCORE_FREE_OBJECT( pGroup );
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A group with the provided ID already exists" );
        }

        return pGroup;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFPropertySet*
DWFContent::addSharedPropertySet( const DWFString& zLabel,
                                  const DWFString& zUUID )
throw()
{
    DWFString zID;
    if (zUUID.chars() == 0)
    {
        DWFUUID* pUUID = getIDProvider();
        zID.assign( pUUID->next(true) );
    }
    else
    {
        zID.assign( zUUID );
    }

    if (zID.chars())
    {
        //
        // Create the group and insert it into to the storage map.
        //
        DWFPropertySet* pSet = DWFCORE_ALLOC_OBJECT( DWFPropertySet( zLabel ) );
        pSet->identify( zID );
        pSet->setContent( this );
        _oSharedProperties.insert( zID, pSet );

        return pSet;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFInstance*
DWFContent::addInstance( const DWFString& zResourceID,
                         DWFRenderable* pRenderable,
                         int nNodeID,
                         unsigned int nGraphicsAttributes,
                         int nGeometricVariationIndex )
throw( DWFException )
{
    if (pRenderable == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Cannot create an instance without a valid renderable feature or object." );
    }

    DWFUUID* pUUID = getIDProvider();

    if (pUUID)
    {
        DWFString zID = pUUID->next(true);

        //
        // Create the instance and insert it into to the storage map.
        //
        DWFInstance* pInstance = DWFCORE_ALLOC_OBJECT( DWFInstance( zID, pRenderable, nNodeID, nGraphicsAttributes, nGeometricVariationIndex ) );
        _oInstances.insert( zID, pInstance );
        _oModifiedContentResources.insert( zResourceID );

        //
        // Find the renderable-to-instance multimap to store the renderable-instance pair. If the map
        // doesn't exist, create it by inserting into the main map.
        //
        _tResourceRIMapIterator iResource = _oResourceRenderableToInstance.find( zResourceID );
        if (iResource == _oResourceRenderableToInstance.end())
        {
            std::pair<_tResourceRIMapIterator, bool> result;
            _tRenderableToInstanceMultiMap* pRIMap = DWFCORE_ALLOC_OBJECT( _tRenderableToInstanceMultiMap() );
            result = _oResourceRenderableToInstance.insert( std::make_pair( zResourceID, pRIMap ) );
            iResource = result.first;
        }
        _tRenderableToInstanceMultiMap* pRIMap = iResource->second;
        //
        // Now insert the pair
        //
        pRIMap->insert( std::make_pair( pRenderable, pInstance ) );

        return pInstance;
    }
    else
    {
        return NULL;
    }
}

//
// The following all REMOVE new elements from the content library
//

_DWFTK_API
void
DWFContent::removeClass( DWFClass* pClass )
throw()
{
    if (pClass == NULL)
    {
        return;
    }

    //
    // Remove all mapping between given class and other classes 
    // - both "base" and "derived" classes
    //
    _removeClassToClassMappings( pClass );

    //
    // Identify entities using pClass for properties, and remove it from their lists.
    //
    _removeClassToEntityMappings( pClass );

    //
    // Identify features using pClass for properties, and remove it from their lists.
    //
    _removeClassToFeatureMappings( pClass );

    //
    // Identify groups containing pClass, and remove pClass from the them.
    //
    _removeGroupToElementMappings( pClass );

    _oClasses.erase( pClass->id() );
    DWFCORE_FREE_OBJECT( pClass );
}

_DWFTK_API
void
DWFContent::removeClass( const DWFString& zID )
throw()
{
    DWFClass* pClass = getClass( zID );
    removeClass( pClass );
}


_DWFTK_API
void
DWFContent::removeEntity( DWFEntity* pEntity )
throw()
{
    if (pEntity == NULL)
    {
        return;
    }

    //
    // Remove references to and from pEntity with its parent and child entities
    //
    _removeEntityToEntityMappings( pEntity );

    //
    // Identify classes used by the entity and remove the mappings.
    //
    _removeClassToEntityMappings( pEntity );

    //
    // Identify features used by the entity and remove the mappings.
    //
    _removeFeatureToEntityMappings( pEntity );

    //
    // Remove all objects that render this entity. This will recursively remove
    // descendent objects, and related instances.
    //
    _tEntityToObjectMultiMap::iterator it = _oEntityToObject.find( pEntity );
    for (; it != _oEntityToObject.end() && 
           it->first == pEntity; ++it)
    {
        DWFObject* pObject = it->second;
        if (pObject)
        {
            removeObject( pObject );
        }
    }
    //
    // Remove all mappings of the entity to objects
    //
    _oEntityToObject.erase( pEntity );

    //
    // Identify groups containing pEntity, and remove pClass from the them.
    //
    _removeGroupToElementMappings( pEntity );

    _oEntities.erase( pEntity->id() );
    DWFCORE_FREE_OBJECT( pEntity );
}

_DWFTK_API
void
DWFContent::removeEntity( const DWFString& zID )
throw()
{
    DWFEntity* pEntity = getEntity( zID );
    removeEntity( pEntity );
}


_DWFTK_API
void
DWFContent::removeObject( DWFObject* pObject )
throw()
{
    if (pObject == NULL)
    {
        return;
    }

    //
    // Remove the entity-to-object mapping
    //
    DWFEntity* pEntity = pObject->getEntity();
    if (pEntity)
    {
        _removeFromMultiMap( _oEntityToObject, pEntity, pObject );
    }

    //
    // Remove references to pObject from its parent object
    //
    DWFObject* pParent = pObject->getParent();
    if (pParent != NULL)
    {
        pParent->_removeChild( pObject );
    }

    //
    // All children of the corresponding object need to be deleted
    //
    if (pObject->getChildCount() > 0)
    {
        DWFObject::tList::Iterator* pIterChild = pObject->getChildren();
        if (pIterChild)
        {
            for (; pIterChild->valid(); pIterChild->next())
            {
                DWFObject* pChild = pIterChild->get();
                if (pChild)
                {
                    removeObject( pChild );
                }
            }
            DWFCORE_FREE_OBJECT( pIterChild );
        }
    }

    //
    // Now remove all instances that render this object and also clean up the
    // mapping from object to the instances.
    //
    _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.begin();
    for (; iResource != _oResourceRenderableToInstance.end(); ++iResource)
    {
        _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

        _tRenderableToInstanceMultiMap::const_iterator itObject = pRIMap->find( pObject );
        for (; itObject != pRIMap->end() && 
               itObject->first == pObject; ++itObject)
        {
            DWFInstance* pInstance = itObject->second;
            _removeInstance( pInstance, false );
        }

        pRIMap->erase( pObject );
    }

    //
    // Remove the feature to object mapping
    //
    _removeFeatureToObjectMappings( pObject );

    //
    // Identify groups containing pObject, and remove pObject from the them.
    //
    _removeGroupToElementMappings( pObject );

    _oObjects.erase( pObject->id() );
    DWFCORE_FREE_OBJECT( pObject );
}

_DWFTK_API
void
DWFContent::removeObject( const DWFString& zID )
throw()
{
    DWFObject* pObject = getObject( zID );
    removeObject( pObject );
}

_DWFTK_API
void
DWFContent::removeFeature( DWFFeature* pFeature )
throw()
{
    if (pFeature == NULL)
    {
        return;
    }

    //
    // Remove all instances that render this feature and also clean up the
    // mapping from feature to the instances.
    //
    _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.begin();
    for (; iResource != _oResourceRenderableToInstance.end(); ++iResource)
    {
        _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

        _tRenderableToInstanceMultiMap::const_iterator itFeature = pRIMap->find( pFeature );
        for (; itFeature != pRIMap->end() && 
               itFeature->first == pFeature; ++itFeature)
        {
            DWFInstance* pInstance = itFeature->second;
            _removeInstance( pInstance, false );
        }

        pRIMap->erase( pFeature );
    }

    //
    // Remove mappings between classes and the feature
    //
    _removeClassToFeatureMappings( pFeature );

    //
    // Remove mappings between the feature and entities
    //
    _removeFeatureToEntityMappings( pFeature );

    //
    // Remove mappings between the feature and objects
    //
    _removeFeatureToObjectMappings( pFeature );

    //
    // Remove mappings between the feature and groups
    //
    _removeGroupToElementMappings( pFeature );

    _oFeatures.erase( pFeature->id() );
    DWFCORE_FREE_OBJECT( pFeature );
}

_DWFTK_API
void
DWFContent::removeFeature( const DWFString& zID )
throw()
{
    DWFFeature* pFeature = getFeature( zID );
    removeFeature( pFeature );
}

_DWFTK_API
void 
DWFContent::removeGroup( DWFGroup* pGroup )
throw()
{
    if (pGroup == NULL)
    {
        return;
    }

    DWFContentElement::tList::Iterator* pIterElem = pGroup->getElements();
    if (pIterElem)
    {
        for (; pIterElem->valid(); pIterElem->next())
        {
            DWFContentElement* pElement = pIterElem->get();
            if (pElement)
            {
                _removeFromMultiMap( _oElementToGroup, pElement, pGroup );
            }
        }
        DWFCORE_FREE_OBJECT( pIterElem );

        pGroup->_removeElements();
    }

    _oGroups.erase( pGroup->id() );
    DWFCORE_FREE_OBJECT( pGroup );
}

_DWFTK_API
void
DWFContent::removeGroup( const DWFString& zID )
throw()
{
    DWFGroup* pGroup = getGroup( zID );
    removeGroup( pGroup );
}

_DWFTK_API
void
DWFContent::removeInstance( DWFInstance* pInstance )
throw()
{
    _removeInstance( pInstance, true );
}

_DWFTK_API
void
DWFContent::removeInstance( const DWFString& zID )
throw()
{
    DWFInstance* pInstance = getInstance( zID );
    removeInstance( pInstance );
}

_DWFTK_API
void
DWFContent::removeSharedPropertySet( DWFPropertySet* pSet )
throw()
{
    if (pSet == NULL)
    {
        return;
    }

    _tSetToSetMultiMap::iterator it = _oSetToSet.find( pSet );
    if (it != _oSetToSet.end())
    {
        _tSetToSetMultiMap::iterator itStartErase = it;

        //
        // First tell each element that refers to this set to remove it from the it's list
        //
        for (; it != _oSetToSet.end() && 
               it->first == pSet; ++it)
        {
            DWFPropertySet* pReferingSet = it->second;
            if (pReferingSet)
            {
                pReferingSet->removeReferencedPropertyContainer( *pSet );
            }
        }

        //
        // Then remove the mapping.
        //
        _oSetToSet.erase( itStartErase, it );
    }

    _oSharedProperties.erase( pSet->id() );
    DWFCORE_FREE_OBJECT( pSet );
}

_DWFTK_API
void
DWFContent::removeSharedPropertySet( const DWFString& zID )
throw()
{
    DWFPropertySet* pSet = getSharedPropertySet( zID );
    removeSharedPropertySet( pSet );
}

//
// The following all MODIFY RELATIONSHIPS between elements in the content
//

_DWFTK_API
void
DWFContent::addBaseClassToClass( DWFClass* pClass,
                                 DWFClass* pBaseClass )
throw()
{
    if (pClass != NULL &&
        pBaseClass != NULL)
    {
        if (pClass->isBaseClass( pBaseClass ) == false)
        {
            pClass->_addBaseClass( pBaseClass );
            _oBaseClassToClass.insert( std::make_pair( pBaseClass, pClass ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addClassToEntity( DWFEntity* pEntity,
                              DWFClass* pClass )
throw()
{
    if (pEntity != NULL &&
        pClass != NULL)
    {
        if (pEntity->hasClass( pClass ) == false)
        {
            pEntity->_addClass( pClass );
            _oClassToEntity.insert( std::make_pair( pClass, pEntity ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addClassToFeature( DWFFeature* pFeature,
                               DWFClass* pClass )
throw()
{
    if (pFeature != NULL &&
        pClass != NULL)
    {
        if (pFeature->hasClass( pClass ) == false)
        {
            pFeature->_addClass( pClass );
            _oClassToFeature.insert( std::make_pair( pClass, pFeature ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addFeatureToEntity( DWFEntity* pEntity,
                                DWFFeature* pFeature )
throw()
{
    if (pEntity != NULL &&
        pFeature != NULL)
    {
        if (pEntity->hasFeature( pFeature ) == false)
        {
            pEntity->_addFeature( pFeature );
            _oFeatureToEntity.insert( std::make_pair( pFeature, pEntity ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addFeatureToObject( DWFObject* pObject,
                                DWFFeature* pFeature )
throw()
{
    if (pObject != NULL &&
        pFeature != NULL)
    {
        if (pObject->hasFeature( pFeature ) == false)
        {
            pObject->_addFeature( pFeature );
            _oFeatureToObject.insert( std::make_pair( pFeature, pObject ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addChildEntity( DWFEntity* pParentEntity,
                            DWFEntity* pChildEntity )
throw()
{
    if (pParentEntity != NULL &&
        pChildEntity != NULL &&
        pParentEntity != pChildEntity)
    {
        pParentEntity->_addChild( pChildEntity );
    }
}

_DWFTK_API
void
DWFContent::addChildObject( DWFObject* pParentObject,
                            DWFObject* pChildObject )
throw()
{
    if (pParentObject != NULL &&
        pChildObject != NULL &&
        pParentObject != pChildObject)
    {
        pParentObject->_addChild( pChildObject );
    }
}

_DWFTK_API
void
DWFContent::addElementToGroup( DWFGroup* pGroup,
                               DWFContentElement* pElement )
throw()
{
    if (pGroup != NULL &&
        pElement != NULL &&
        pElement != pGroup)
    {
        if (pGroup->hasElement( pElement ) == false)
        {
            pGroup->_addElement( pElement );
            _oElementToGroup.insert( std::make_pair( pElement, pGroup ) );
        }
    }
}

_DWFTK_API
void
DWFContent::addSharedPropertySetToElement( DWFContentElement* pElement,
                                           DWFPropertySet* pSet )
throw( DWFException )
{
    addReferenceToPropertySet( pElement, pSet );
}

_DWFTK_API
void
DWFContent::addReferenceToPropertySet( DWFPropertySet* pReferingSet,
                                       DWFPropertySet* pSet )
throw( DWFException )
{
    if (pReferingSet != NULL &&
        pSet != NULL)
    {
        if (pReferingSet->referencesContainer(pSet) == false)
        {
            pReferingSet->referencePropertySet( pSet );
            _oSetToSet.insert( std::make_pair( pSet, pReferingSet ) );
        }
    }
}

_DWFTK_API
void
DWFContent::removeBaseClassFromClass( DWFClass* pClass,
                                      DWFClass* pBaseClass )
throw()
{
    if (pClass != NULL &&
        pBaseClass != NULL)
    {
        if (pClass->_removeBaseClass( pBaseClass ))
        {
            _removeFromMultiMap( _oBaseClassToClass, pBaseClass, pClass );
        }
    }
}

_DWFTK_API
void
DWFContent::removeClassFromEntity( DWFEntity* pEntity,
                                   DWFClass* pClass )
throw()
{
    if (pEntity != NULL &&
        pClass != NULL)
    {
        if (pEntity->_removeClass( pClass ))
        {
            _removeFromMultiMap( _oClassToEntity, pClass, pEntity );
        }
    }
}

_DWFTK_API
void
DWFContent::removeClassFromFeature( DWFFeature* pFeature,
                                    DWFClass* pClass )
throw()
{
    if (pFeature != NULL &&
        pClass != NULL)
    {
        if (pFeature->_removeClass( pClass ))
        {
            _removeFromMultiMap( _oClassToFeature, pClass, pFeature );
        }
    }
}

_DWFTK_API
void
DWFContent::removeFeatureFromEntity( DWFEntity* pEntity,
                                     DWFFeature* pFeature )
throw()
{
    if (pEntity != NULL &&
        pFeature != NULL)
    {
        if (pEntity->_removeFeature( pFeature ))
        {
            _removeFromMultiMap( _oFeatureToEntity, pFeature, pEntity );
        }
    }
}

_DWFTK_API
void
DWFContent::removeFeatureFromObject( DWFObject* pObject,
                                     DWFFeature* pFeature )
throw()
{
    if (pObject != NULL &&
        pFeature != NULL)
    {
        if (pObject->_removeFeature( pFeature ))
        {
            _removeFromMultiMap( _oFeatureToObject, pFeature, pObject );
        }
    }
}

_DWFTK_API
void
DWFContent::removeChildEntity( DWFEntity* pParentEntity,
                               DWFEntity* pChildEntity )
throw()
{
    if (pParentEntity != NULL &&
        pChildEntity != NULL)
    {
        pParentEntity->_removeChild( pChildEntity );
    }
}

_DWFTK_API
void
DWFContent::removeChildObject( DWFObject* pParentObject,
                               DWFObject* pChildObject )
throw()
{
    if (pParentObject != NULL &&
        pChildObject != NULL)
    {
        pParentObject->_removeChild( pChildObject );
    }
}

_DWFTK_API
void
DWFContent::removeElementFromGroup( DWFGroup* pGroup,
                                    DWFContentElement* pElement )
throw()
{
    if (pGroup != NULL &&
        pElement != NULL)
    {
        if (pGroup->_removeElement( pElement ))
        {
            _removeFromMultiMap( _oElementToGroup, pElement, pGroup );
        }
    }
}

_DWFTK_API
void
DWFContent::removeSharedPropertySetFromElement( DWFContentElement* pElement,
                                                DWFPropertySet* pPropertySet )
throw()
{
    if (pElement != NULL &&
        pPropertySet != NULL)
    {
        if (pElement->removeReferencedPropertyContainer( *pPropertySet ))
        {
            _oSetToSet.insert( std::make_pair( pPropertySet, pElement ) );
        }
    }
}

_DWFTK_API
DWFUUID*
DWFContent::getIDProvider() const
throw( DWFException )
{
    if (_pContentManager == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"ContentManager was not set for Content" );
    }

    return _pContentManager->getIDProvider();
}


_DWFTK_API
void 
DWFContent::_removeClassToClassMappings( DWFClass* pClass )
throw()
{
    //
    // Remove mappings between the given class and it's base classes
    //
    if (pClass->getBaseClassCount()>0)
    {
        DWFClass::tList::Iterator* pIterBase = pClass->getBaseClasses();
        if (pIterBase)
        {
            for (; pIterBase->valid(); pIterBase->next())
            {
                DWFClass* pBase = pIterBase->get();
                if (pBase)
                {
                    _removeFromMultiMap( _oBaseClassToClass, pBase, pClass );
                }
            }
            DWFCORE_FREE_OBJECT( pIterBase );
        }
    }

    //
    // Remove mappings between the given class and those that consider 
    // it a base class
    //
    _tClassToClassMultiMap::iterator it = _oBaseClassToClass.find( pClass );
    if (it != _oBaseClassToClass.end())
    {
        _tClassToClassMultiMap::iterator itStartErase = it;

        for (; it != _oBaseClassToClass.end() && 
               it->first == pClass; ++it)
        {
            DWFClass* pDerived = it->second;
            if (pDerived)
            {
                pDerived->_removeBaseClass( pClass );
            }
        }
        _oBaseClassToClass.erase( itStartErase, it );
    }
}

_DWFTK_API
void 
DWFContent::_removeClassToEntityMappings( DWFEntity* pEntity )
throw()
{
    DWFClass::tList::Iterator* pIter = pEntity->getClasses();
    if (pIter)
    {
        for ( ; pIter->valid(); pIter->next())
        {
            DWFClass* pClass = pIter->get();
            if (pClass)
            {
                _removeFromMultiMap( _oClassToEntity, pClass, pEntity );
            }
        }
        DWFCORE_FREE_OBJECT( pIter );

        pEntity->_removeClasses();
    }
}

_DWFTK_API
void 
DWFContent::_removeClassToEntityMappings( DWFClass* pClass )
throw()
{
    _tClassToEntityMultiMap::iterator it = _oClassToEntity.find( pClass );
    if (it != _oClassToEntity.end())
    {
        _tClassToEntityMultiMap::iterator itStartErase = it;

        //
        // First tell each entity using the class to remove it from the entity's list
        //
        for (; it != _oClassToEntity.end() && 
               it->first == pClass; ++it)
        {
            DWFEntity* pEntity = it->second;
            if (pEntity)
            {
                pEntity->_removeClass( pClass );
            }
        }

        //
        // Then remove the mapping
        //
       _oClassToEntity.erase( itStartErase, it );
    }
}

_DWFTK_API
void 
DWFContent::_removeClassToFeatureMappings( DWFClass* pClass )
throw()
{
    _tClassToFeatureMultiMap::iterator it = _oClassToFeature.find( pClass );
    if (it != _oClassToFeature.end())
    {
        _tClassToFeatureMultiMap::iterator itStartErase = it;
        
        //
        // First tell each feature that refers to this class to remove it from the feature's list
        //
        for (; it != _oClassToFeature.end() && 
               it->first == pClass; ++it)
        {
            DWFFeature* pFeature = it->second;
            if (pFeature)
            {
                pFeature->_removeClass( pClass );
            }
        }

        //
        // Then remove the mapping.
        //
        _oClassToFeature.erase( itStartErase, it );
    }
}

_DWFTK_API
void 
DWFContent::_removeFeatureToEntityMappings( DWFEntity* pEntity )
throw()
{
    DWFFeature::tList::Iterator* pIterFeature = pEntity->getFeatures();
    if (pIterFeature)
    {
        for (; pIterFeature->valid(); pIterFeature->next())
        {
            DWFFeature* pFeature = pIterFeature->get();
            if (pFeature)
            {
                _removeFromMultiMap( _oFeatureToEntity, pFeature, pEntity );
            }
        }
        DWFCORE_FREE_OBJECT( pIterFeature );

        pEntity->_removeFeatures();
    }
}

_DWFTK_API
void 
DWFContent::_removeEntityToEntityMappings( DWFEntity* pEntity )
throw()
{
    //
    // Remove parent mappings
    //
    DWFEntity::tSortedList::ConstIterator* pIterParent = pEntity->getParents();
    if (pIterParent)
    {
        for (; pIterParent->valid(); pIterParent->next())
        {
            DWFEntity* pParentEntity = pIterParent->get();
            if (pParentEntity)
            {
                pParentEntity->_removeChild( pEntity );
            }
        }
        DWFCORE_FREE_OBJECT( pIterParent );
    }

    //
    // Remove child mappings
    //
    pEntity->_removeChildren();
}

_DWFTK_API
void
DWFContent::_removeFeatureToObjectMappings( DWFObject* pObject )
throw()
{
    DWFFeature::tList::Iterator* pIterFeature = pObject->getFeatures();
    if (pIterFeature)
    {
        for (; pIterFeature->valid(); pIterFeature->next())
        {
            DWFFeature* pFeature = pIterFeature->get();
            if (pFeature)
            {
                _removeFromMultiMap( _oFeatureToObject, pFeature, pObject );
            }
        }
        DWFCORE_FREE_OBJECT( pIterFeature );

        pObject->_removeFeatures();
    }
}

_DWFTK_API
void 
DWFContent::_removeGroupToElementMappings( DWFContentElement* pElement )
throw()
{
    _tElementToGroupMultiMap::iterator it = _oElementToGroup.find( pElement );
    if (it != _oElementToGroup.end())
    {
        _tElementToGroupMultiMap::iterator itStartErase = it;

        //
        // First tell each group that refers to this element to remove it from the group's list
        //
        for (; it != _oElementToGroup.end() &&
               it->first == pElement; ++it)
        {
            DWFGroup* pGroup = it->second;
            if (pGroup)
            {
                pGroup->_removeElement( pElement );
            }
        }

        //
        // Then remove the mapping.
        //
        _oElementToGroup.erase( itStartErase, it );
    }
}

_DWFTK_API
void 
DWFContent::_removeClassToFeatureMappings( DWFFeature* pFeature )
throw()
{
    DWFClass::tList::Iterator* pIterClass = pFeature->getClasses();
    if (pIterClass)
    {
        for (; pIterClass->valid(); pIterClass->next())
        {
            DWFClass* pClass = pIterClass->get();
            if (pClass)
            {
                _removeFromMultiMap( _oClassToFeature, pClass, pFeature );
            }
        }
        DWFCORE_FREE_OBJECT( pIterClass );

        pFeature->_removeClasses();
    }
}

_DWFTK_API
void
DWFContent::_removeFeatureToEntityMappings( DWFFeature* pFeature )
throw()
{
    _tFeatureToEntityMultiMap::iterator it = _oFeatureToEntity.find( pFeature );
    if (it != _oFeatureToEntity.end())
    {
        _tFeatureToEntityMultiMap::iterator itStartErase = it;

        //
        // First tell each entity using the feature to remove it from the feature's list
        //
        for (; it != _oFeatureToEntity.end() &&
               it->first == pFeature; ++it)
        {
            DWFEntity* pEntity = it->second;
            if (pEntity)
            {
                pEntity->_removeFeature( pFeature );
            }
        }

        //
        // Then remove the mapping from the feature to the entities
        //
       _oFeatureToEntity.erase( itStartErase, it );
    }
}

_DWFTK_API
void
DWFContent::_removeFeatureToObjectMappings( DWFFeature* pFeature )
throw()
{
    _tFeatureToObjectMultiMap::iterator it = _oFeatureToObject.find( pFeature );
    if (it != _oFeatureToObject.end())
    {
        _tFeatureToObjectMultiMap::iterator itStartErase = it;

        //
        // First tell each object using the feature to remove it from the feature's list
        //
        for (; it != _oFeatureToObject.end() &&
               it->first == pFeature; ++it)
        {
            DWFObject* pObject = it->second;
            if (pObject)
            {
                pObject->_removeFeature( pFeature );
            }
        }

        //
        // Then remove the mapping from the feature to the objects
        //
       _oFeatureToObject.erase( itStartErase, it );
    }
}

_DWFTK_API
void
DWFContent::_removeInstance( DWFInstance* pInstance, bool bRemoveMappings )
throw()
{
    DWFRenderable* pRendered = pInstance->getRenderedElement();
    
    if (bRemoveMappings)
    {
        _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.begin();
        for (; iResource != _oResourceRenderableToInstance.end(); ++iResource)
        {
            _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

            //
            // There is only one instance. If it erased, break out of the loop
            //
            if (_removeFromMultiMap( *pRIMap, pRendered, pInstance ))
            {
                _oModifiedContentResources.insert( iResource->first );
                break;
            }
        }
    }

    //
    // Delete the instance now
    //
    _oInstances.erase( pInstance->id() );
    DWFCORE_FREE_OBJECT( pInstance );
}

/////////////////////////////////////////////////////////////////////////////////////////////////

_DWFTK_API
void
DWFContent::mergeContent( DWFContent* pContent,
                          bool bLocalHasPriority )
throw( DWFException )
{
    if (!isLoaded())
    {
        load();
    }
    if (!pContent->isLoaded())
    {
        pContent->load();
    }

    _oMapFromSetToSet.clear();

    //
    //  First copy over all necessary owned properties
    //
    _mergeSharedProperties( pContent, bLocalHasPriority );
    _mergeClasses( pContent, bLocalHasPriority );
    _mergeFeatures( pContent, bLocalHasPriority );
    _mergeEntities( pContent, bLocalHasPriority );
    _mergeObjects( pContent, bLocalHasPriority );
    _mergeGroups( pContent, bLocalHasPriority );

    _mergeRefsSharedProperties( pContent );
    _mergeRefsClasses( pContent );
    _mergeRefsFeatures( pContent );
    _mergeRefsEntities( pContent );
    _mergeRefsObjects( pContent );
    _mergeRefsGroups( pContent );

    _oMapFromSetToSet.clear();
}

void
DWFContent::_mergeSharedProperties( DWFContent* pContent, bool bLocalHasPriority )
throw()
{
    DWFPropertySet::tMap::Iterator* piPropertySet = pContent->_oSharedProperties.iterator();
    if (piPropertySet)
    {
        for (; piPropertySet->valid(); piPropertySet->next())
        {
            DWFPropertySet* pSet = piPropertySet->value();
            DWFPropertySet* pLocalSet = getSharedPropertySet( pSet->id() );
            if (pLocalSet)
            {
                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pSet, pLocalSet, bLocalHasPriority, true );
            }
            else
            {
                pLocalSet = addSharedPropertySet( pSet->getLabel(), pSet->id() );
                if (pSet->isClosed())
                {
                    pLocalSet->setClosed( pSet->getSetID() );
                }
                else
                {
                    pLocalSet->setSetID( pSet->getSetID() );
                }
                pLocalSet->setSchemaID( pSet->getSchemaID() );
                _acquireOwnedProperties( pSet, pLocalSet, bLocalHasPriority, false );
            }
        }

        DWFCORE_FREE_OBJECT( piPropertySet );
    }
}

void
DWFContent::_mergeClasses( DWFContent* pContent, bool bLocalHasPriority )
throw()
{
    DWFClass::tMap::Iterator* piClass = pContent->_oClasses.iterator();
    if (piClass)
    {
        for (; piClass->valid(); piClass->next())
        {
            DWFClass* pClass = piClass->value();
            DWFClass* pLocalClass = getClass( pClass->id() );
            if (pLocalClass)
            {
                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pClass, pLocalClass, bLocalHasPriority, true );
            }
            else
            {
                pLocalClass = addClass( NULL, pClass->id() );
                _acquireOwnedProperties( pClass, pLocalClass, bLocalHasPriority, false );
            }
        }

        DWFCORE_FREE_OBJECT( piClass );
    }
}

void
DWFContent::_mergeFeatures( DWFContent* pContent, bool bLocalHasPriority )
throw()
{
    DWFFeature::tMap::Iterator* piFeature = pContent->_oFeatures.iterator();
    if (piFeature)
    {
        for (; piFeature->valid(); piFeature->next())
        {
            DWFFeature* pFeature = piFeature->value();
            DWFFeature* pLocalFeature = getFeature( pFeature->id() );
            if (pLocalFeature)
            {
                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pFeature, pLocalFeature, bLocalHasPriority, true );
            }
            else
            {
                pLocalFeature = addFeature( NULL, pFeature->id() );
                _acquireOwnedProperties( pFeature, pLocalFeature, bLocalHasPriority, false );
            }
        }

        DWFCORE_FREE_OBJECT( piFeature );
    }
}

void
DWFContent::_mergeEntities( DWFContent* pContent, bool bLocalHasPriority )
throw()
{
    DWFEntity::tMap::Iterator* piEntity = pContent->_oEntities.iterator();
    if (piEntity)
    {
        for (; piEntity->valid(); piEntity->next())
        {
            DWFEntity* pEntity = piEntity->value();
            DWFEntity* pLocalEntity = getEntity( pEntity->id() );
            if (pLocalEntity)
            {
                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pEntity, pLocalEntity, bLocalHasPriority, true );
            }
            else
            {
                pLocalEntity = addEntity( NULL, pEntity->id() );
                _acquireOwnedProperties( pEntity, pLocalEntity, bLocalHasPriority, false );
            } 
        }

        DWFCORE_FREE_OBJECT( piEntity );
    }
}

void
DWFContent::_mergeObjects( DWFContent* pContent, bool bLocalHasPriority )
throw( DWFException )
{
    //
    //  Objects need to be merged while maintaining sensible parent-child relationships.
    //  Hence we start with only the objects at the root level in the source content and
    //  recurse into their children.
    //

    DWFObject::tMap::Iterator* piObject = pContent->_oObjects.iterator();
    if (piObject)
    {
        for (; piObject->valid(); piObject->next())
        {
            DWFObject* pObject = piObject->value();

            if (pObject->getParent() == NULL)
            {
                DWFObject* pLocalObject = getObject( pObject->id() );

                if (pLocalObject)
                {
                    //
                    // Copy over values where necessary.
                    //
                    _acquireOwnedProperties( pObject, pLocalObject, bLocalHasPriority, true );

                    //
                    //  Recursive ensure that the children match up
                    //
                    _acquireChildObjects( pObject, pLocalObject, bLocalHasPriority );
                }
                else
                {
                    DWFEntity* pEntity = pObject->getEntity();
                    DWFEntity* pLocalEntity = getEntity( pEntity->id() );
                    if (pLocalEntity == NULL)
                    {
                        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Entity corresponding to object was not found in local content." );
                    }

                    pLocalObject = addObject( pLocalEntity, NULL, pObject->id() );
                    _oEntityToObject.insert( std::make_pair(pLocalEntity, pLocalObject) );
                    _acquireOwnedProperties( pObject, pLocalObject, bLocalHasPriority, false );

                    //
                    //  Recursively ensure that the children match up
                    //
                    _acquireChildObjects( pObject, pLocalObject, bLocalHasPriority );
                }
            }
        }

        DWFCORE_FREE_OBJECT( piObject );
    }
}

void
DWFContent::_mergeGroups( DWFContent* pContent, bool bLocalHasPriority )
throw()
{
    DWFGroup::tMap::Iterator* piGroup = pContent->_oGroups.iterator();
    if (piGroup)
    {
        for (; piGroup->valid(); piGroup->next())
        {
            DWFGroup* pGroup = piGroup->value();
            DWFGroup* pLocalGroup = getGroup( pGroup->id() );
            if (pLocalGroup)
            {
                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pGroup, pLocalGroup, bLocalHasPriority, true );
            }
            else
            {
                pLocalGroup = addGroup( pGroup->id() );
                _acquireOwnedProperties( pGroup, pLocalGroup, bLocalHasPriority, false );
            }
        }

        DWFCORE_FREE_OBJECT( piGroup );
    }
}

void
DWFContent::_mergeRefsSharedProperties( DWFContent* pContent )
throw( DWFException )
{
    DWFPropertySet::tMap::Iterator* piPropertySet = pContent->_oSharedProperties.iterator();
    if (piPropertySet)
    {
        for (; piPropertySet->valid(); piPropertySet->next())
        {
            DWFPropertySet* pSet = piPropertySet->value();
            DWFPropertySet* pLocalSet = getSharedPropertySet( pSet->id() );
            if (pLocalSet)
            {
                //
                // Copy over references where necessary.
                //
                _acquirePropertySetReferences( pSet, pLocalSet );
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Property set was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piPropertySet );
    }
}

void
DWFContent::_mergeRefsClasses( DWFContent* pContent )
throw( DWFException )
{
    DWFClass::tMap::Iterator* piClass = pContent->_oClasses.iterator();
    if (piClass)
    {
        for (; piClass->valid(); piClass->next())
        {
            DWFClass* pClass = piClass->value();
            DWFClass* pLocalClass = getClass( pClass->id() );
            if (pLocalClass)
            {
                //
                //  Copy over property set references.
                //
                _acquirePropertySetReferences( pClass, pLocalClass );

                //
                //  Copy over base class references.
                //
                DWFClass::tIterator* piBase = pClass->getBaseClasses();
                if (piBase)
                {
                    for (; piBase->valid(); piBase->next())
                    {
                        DWFClass* pBase = piBase->get();
                        _tSetToSet::iterator iBase = _oMapFromSetToSet.find( pBase );
                        if (iBase != _oMapFromSetToSet.end())
                        {
                            DWFClass* pLocalBase = (DWFClass*)(iBase->second);
                            addBaseClassToClass( pLocalClass, pLocalBase );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piBase );
                }
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Class was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piClass );
    }
}

void
DWFContent::_mergeRefsFeatures( DWFContent* pContent )
throw( DWFException )
{
    DWFFeature::tMap::Iterator* piFeature = pContent->_oFeatures.iterator();
    if (piFeature)
    {
        for (; piFeature->valid(); piFeature->next())
        {
            DWFFeature* pFeature = piFeature->value();
            DWFFeature* pLocalFeature = getFeature( pFeature->id() );
            if (pLocalFeature)
            {
                //
                //  Copy over property set references.
                //
                _acquirePropertySetReferences( pFeature, pLocalFeature );

                //
                //  Copy over class references.
                //
                DWFClass::tIterator* piClass = pFeature->getClasses();
                if (piClass)
                {
                    for (; piClass->valid(); piClass->next())
                    {
                        DWFClass* pClass = piClass->get();
                        _tSetToSet::iterator iClass = _oMapFromSetToSet.find( pClass );
                        if (iClass != _oMapFromSetToSet.end())
                        {
                            DWFClass* pLocalClass = (DWFClass*)(iClass->second);
                            addClassToFeature( pLocalFeature, pLocalClass );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piClass );
                }
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Feature was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piFeature );
    }
}

void
DWFContent::_mergeRefsEntities( DWFContent* pContent )
throw( DWFException )
{
    DWFEntity::tMap::Iterator* piEntity = pContent->_oEntities.iterator();
    if (piEntity)
    {
        for (; piEntity->valid(); piEntity->next())
        {
            DWFEntity* pEntity = piEntity->value();
            DWFEntity* pLocalEntity =getEntity( pEntity->id() );
            if (pLocalEntity)
            {
                //
                //  Copy over property set references.
                //
                _acquirePropertySetReferences( pEntity, pLocalEntity );

                //
                //  Copy over child entity references.
                //
                DWFEntity::tSortedList::ConstIterator* piEntity = pEntity->getChildren();
                if (piEntity)
                {
                    for (; piEntity->valid(); piEntity->next())
                    {
                        DWFEntity* pEntity = piEntity->get();
                        _tSetToSet::iterator iEntity = _oMapFromSetToSet.find( pEntity );
                        if (iEntity != _oMapFromSetToSet.end())
                        {
                            DWFEntity* pLocalChild = (DWFEntity*)(iEntity->second);
                            addChildEntity( pLocalEntity, pLocalChild );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piEntity );
                }

                //
                //  Copy over class references.
                //
                DWFClass::tIterator* piClass = pEntity->getClasses();
                if (piClass)
                {
                    for (; piClass->valid(); piClass->next())
                    {
                        DWFClass* pClass = piClass->get();
                        _tSetToSet::iterator iClass = _oMapFromSetToSet.find( pClass );
                        if (iClass != _oMapFromSetToSet.end())
                        {
                            DWFClass* pLocalClass = (DWFClass*)(iClass->second);
                            addClassToEntity( pLocalEntity, pLocalClass );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piClass );
                }

                //
                //  Copy over feature references.
                //
                DWFFeature::tIterator* piFeature = pEntity->getFeatures();
                if (piFeature)
                {
                    for (; piFeature->valid(); piFeature->next())
                    {
                        DWFFeature* pFeature = piFeature->get();
                        _tSetToSet::iterator iFeature = _oMapFromSetToSet.find( pFeature );
                        if (iFeature != _oMapFromSetToSet.end())
                        {
                            DWFFeature* pLocalFeature = (DWFFeature*)(iFeature->second);
                            addFeatureToEntity( pLocalEntity, pLocalFeature );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piFeature );
                }
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Entity was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piEntity );
    }
}

void
DWFContent::_mergeRefsObjects( DWFContent* pContent )
throw( DWFException )
{
    DWFObject::tMap::Iterator* piObject = pContent->_oObjects.iterator();
    if (piObject)
    {
        for (; piObject->valid(); piObject->next())
        {
            DWFObject* pObject = piObject->value();
            DWFObject* pLocalObject = getObject( pObject->id() );
            if (pLocalObject)
            {
                //
                //  Copy over property set references.
                //
                _acquirePropertySetReferences( pObject, pLocalObject );

                //
                //  Copy over feature references.
                //
                DWFFeature::tIterator* piFeature = pObject->getFeatures();
                if (piFeature)
                {
                    for (; piFeature->valid(); piFeature->next())
                    {
                        DWFFeature* pFeature = piFeature->get();
                        _tSetToSet::iterator iFeature = _oMapFromSetToSet.find( pFeature );
                        if (iFeature != _oMapFromSetToSet.end())
                        {
                            DWFFeature* pLocalFeature = (DWFFeature*)(iFeature->second);
                            addFeatureToObject( pLocalObject, pLocalFeature );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piFeature );
                }
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Object was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piObject );
    }
}

void
DWFContent::_mergeRefsGroups( DWFContent* pContent )
throw( DWFException )
{
    DWFGroup::tMap::Iterator* piGroup = pContent->_oGroups.iterator();
    if (piGroup)
    {
        for (; piGroup->valid(); piGroup->next())
        {
            DWFGroup* pGroup = piGroup->value();
            DWFGroup* pLocalGroup = getGroup( pGroup->id() );
            if (pLocalGroup)
            {
                //
                //  Copy over property set references.
                //
                _acquirePropertySetReferences( pGroup, pLocalGroup );

                //
                //  Copy over element references.
                //
                DWFContentElement::tIterator* piElement = pGroup->getElements();
                if (piElement)
                {
                    for (; piElement->valid(); piElement->next())
                    {
                        DWFContentElement* pElement = piElement->get();
                        _tSetToSet::iterator iElement = _oMapFromSetToSet.find( pElement );
                        if (iElement != _oMapFromSetToSet.end())
                        {
                            DWFContentElement* pLocalElement = (DWFContentElement*)(iElement->second);
                            addElementToGroup( pLocalGroup, pLocalElement );
                        }
                    }
                    DWFCORE_FREE_OBJECT( piElement);
                }
            }
            else
            {
                //
                // At this point all properties should have been copied
                //
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Group was not found for merging references" );
            }
        }

        DWFCORE_FREE_OBJECT( piGroup );
    }
}

void
DWFContent::_acquireOwnedProperties( DWFPropertySet* pFrom,
                                     DWFPropertySet* pTo,
                                     bool bLocalHasPriority,
                                     bool bTargetExisted )
throw()
{
    _oMapFromSetToSet.insert( std::make_pair(pFrom, pTo) );

    //
    //  Deal with the properties
    //
    DWFProperty::tIterator* piProperty = pFrom->getPropertiesInOrder();
    if (piProperty)
    {
        for (; piProperty->valid(); piProperty->next())
        {
            DWFProperty* pSource = piProperty->get();
            DWFProperty* pTarget = const_cast<DWFProperty*>( pTo->findProperty(pSource->name(), pSource->category()) );
            if (pTarget == NULL)
            {
                pTo->addProperty( pSource, false );
            }
            else
            {
                if (!bLocalHasPriority)
                {
                    pTarget->setValue( pSource->value() );
                    pTarget->setType( pSource->type() );
                    pTarget->setUnits( pSource->units() );
                }
            }
        }
        DWFCORE_FREE_OBJECT( piProperty );
    }

    //
    //  Deal with the owned containers
    //
    DWFPropertyContainer::tList oContainers;
    pFrom->getOwnedPropertyContainers( oContainers );
    DWFPropertyContainer::tList::iterator iContainer = oContainers.begin();
    for (; iContainer != oContainers.end(); ++iContainer)
    {
        DWFPropertySet* pSource = dynamic_cast<DWFPropertySet*>(*iContainer);
        if (pSource)
        {
            DWFPropertySet* pTarget = NULL;
            //
            //  Check for container only if the parent existed in the first place.
            //
            if (bTargetExisted)
            {
                pTarget = dynamic_cast<DWFPropertySet*>(pTo->ownedContainer( pSource->id() ));
            }

            //
            //  If target exists just copy properties
            //
            if (pTarget)
            {
                _acquireOwnedProperties( pSource, pTarget, bLocalHasPriority, true );
            }
            //
            //  Otherwise create a new set
            //
            else
            {
                pTarget = pTo->addPropertySet( pSource->getLabel() );
                if (pSource->isClosed())
                {
                    pTo->setClosed( pSource->getSetID() );
                }
                else
                {
                    pTo->setSetID( pSource->getSetID() );
                }
                pTarget->setSchemaID( pSource->getSchemaID() );
                pTarget->identify( pSource->id() );
                _acquireOwnedProperties( pSource, pTarget, bLocalHasPriority, false );
            }
        }
    }
}

void
DWFContent::_acquirePropertySetReferences( DWFPropertySet* pFrom, DWFPropertySet* pTo )
throw( DWFException )
{
    DWFPropertyContainer::tList oSourceRefs;
    pFrom->getReferencedPropertyContainers( oSourceRefs ); 

    DWFPropertyContainer::tList oTargetRefs;
    pTo->getReferencedPropertyContainers( oTargetRefs ); 

    //
    //  For each reference in the source, if it's corresponding (mapped) reference does not
    //  exist in the target, add it.
    //
    DWFPropertyContainer::tList::iterator iSource = oSourceRefs.begin();
    for (; iSource != oSourceRefs.end(); ++iSource)
    {
        DWFPropertySet* pSource = dynamic_cast<DWFPropertySet*>(*iSource);
        if (pSource)
        {
            //
            //  iSource gives the reference in the source content. 
            //  Get the mapped set in the target content - this is our reference.
            //
            _tSetToSet::iterator iSet = _oMapFromSetToSet.find( pSource );
            if (iSet != _oMapFromSetToSet.end())
            {
                DWFPropertySet* pRef = iSet->second;
                DWFPropertyContainer::tList::iterator iTarget = std::find( oTargetRefs.begin(), oTargetRefs.end(), pRef );
                //
                // If it's not found we need to add it to the list of references.
                //
                if (iTarget == oTargetRefs.end())
                {
                    addReferenceToPropertySet( pTo, pRef );
                }
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Missing a set to set mapping for merging references" );
            }
        }
    }

    //
    //  Now search the contained containers and acquire the references recursively.
    //
    DWFPropertyContainer::tList oContainers;
    pFrom->getOwnedPropertyContainers( oContainers );
    DWFPropertyContainer::tList::iterator iContainer = oContainers.begin();
    for (; iContainer != oContainers.end(); ++iContainer)
    {
        DWFPropertySet* pSource = dynamic_cast<DWFPropertySet*>(*iContainer);
        if (pSource)
        {
            DWFPropertySet* pTarget = dynamic_cast<DWFPropertySet*>(pTo->ownedContainer( pSource->id() ));
            if (pTarget)
            {
                _acquirePropertySetReferences( pSource, pTarget );
            }
       }
    }    
}

void
DWFContent::_acquireChildObjects( DWFObject* pFrom, 
                                  DWFObject* pTo,
                                  bool bLocalHasPriority )
throw( DWFException )
{
    DWFObject::tIterator* piChild = pFrom->getChildren();
    if (piChild)
    {
        for (; piChild->valid(); piChild->next())
        {
            DWFObject* pChild = piChild->get();
            DWFObject* pLocalChild = getObject( pChild->id() );

            if (pLocalChild)
            {
                //
                //  Do we ensure the parents are the same, i.e., pTo really
                //  is the parent of pLocalChild?
                //
                DWFObject* pLocalParent = pLocalChild->getParent();

                if (bLocalHasPriority)
                {
                    if (pLocalParent != pTo)
                    {
                        DWFCORE_FREE_OBJECT( piChild );
                        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Inconsistent object parent-child relationship found during merge." );
                    }
                }
                else if (pLocalParent == NULL)
                {
                    addChildObject( pTo, pLocalChild );
                }

                //
                // Copy over values where necessary.
                //
                _acquireOwnedProperties( pChild, pLocalChild, bLocalHasPriority, true );

                //
                //  Recursive ensure that the children match up
                //
                _acquireChildObjects( pChild, pLocalChild, bLocalHasPriority );
            }
            else
            {
                DWFEntity* pEntity = pChild->getEntity();
                DWFEntity* pLocalEntity = getEntity( pEntity->id() );
                if (pLocalEntity == NULL)
                {
                    DWFCORE_FREE_OBJECT( piChild );
                    _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Entity corresponding to object was not found in local content." );
                }

                pLocalChild = addObject( pLocalEntity, pTo, pChild->id() );
                _oEntityToObject.insert( std::make_pair(pLocalEntity, pLocalChild) );
                _acquireOwnedProperties( pChild, pLocalChild, bLocalHasPriority, false );

                //
                //  Recursively ensure that the children match up
                //
                _acquireChildObjects( pChild, pLocalChild, bLocalHasPriority );
            }
        }

        DWFCORE_FREE_OBJECT( piChild );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

_DWFTK_API
void 
DWFContent::notifyContentResourceAdded( const DWFString& zObjectID )
throw()
{
    _tRenderableToInstanceMultiMap* pRIMap = DWFCORE_ALLOC_OBJECT( _tRenderableToInstanceMultiMap() );
    _oResourceRenderableToInstance.insert( std::make_pair( zObjectID, pRIMap ) );
}

_DWFTK_API
void 
DWFContent::notifyResourceIDUpdated( const DWFString& zOldID, const DWFString& zNewID )
throw()
{
    _tResourceRIMapIterator iResource = _oResourceRenderableToInstance.find( zOldID );
    if (iResource != _oResourceRenderableToInstance.end())
    {
        _tRenderableToInstanceMultiMap* pMap = iResource->second;
        _oResourceRenderableToInstance.erase( iResource );
        _oResourceRenderableToInstance.insert( std::make_pair( zNewID, pMap ) );
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
const DWFXMLNamespace& 
DWFContent::addNamespace( const DWFString& zNamespace, 
                                const DWFString& zXMLNS )
throw( DWFException )
{
    _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.find( zNamespace );
    
    if (iNamespace != _oAddedNamespaces.end())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"This namespace has already been defined." );
    }

    _oAddedNamespaces[zNamespace] = DWFXMLNamespace( zNamespace, zXMLNS );

    return _oAddedNamespaces[zNamespace];
}

_DWFTK_API
void
DWFContent::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eManifest)
    {
        rSerializer.startElement( DWFXML::kzElement_Content, DWFXML::kzNamespace_DWF );

        rSerializer.addAttribute( DWFXML::kzAttribute_HRef, href() );
        rSerializer.addAttribute( DWFXML::kzAttribute_ContentID, _zID );
        rSerializer.addAttribute( DWFXML::kzAttribute_MIME, DWFMIME::kzMIMEType_XML );

        rSerializer.endElement();
    }

    else if (nFlags & DWFPackageWriter::eGlobalContent)
    {
        //
        //  Generate the section content document element with the version information
        //
        DWFString zNamespace( namespaceXML(nFlags) );
        rSerializer.startElement( DWFXML::kzElement_Content, zNamespace );
        
        DWFString zTempString( /*NOXLATE*/L"DWF-Content:" );
        zTempString.append( _zVersion );
        rSerializer.addAttribute( /*NOXLATE*/L"dwf", zTempString, /*NOXLATE*/L"xmlns:" );

        zTempString.append( /*NOXLATE*/L" " );
        zTempString.append( /*NOXLATE*/_DWF_FORMAT_CONTENT_SCHEMA_LOCATION_WIDE_STRING );
        rSerializer.addAttribute( DWFXML::kzAttribute_SchemaLocation, zTempString, /*NOXLATE*/L"xmlns:" );

	        //
            // other namespaces added for extension
            //
        _tNamespaceMap::iterator iNamespace = _oAddedNamespaces.begin();
        for (; iNamespace != _oAddedNamespaces.end(); iNamespace++)
        {
            rSerializer.addAttribute( iNamespace->second.prefix(), iNamespace->second.xmlns(), /*NOXLATE*/L"xmlns:" );
        }

        rSerializer.addAttribute( DWFXML::kzAttribute_Version, _zVersion );

        {
            _serializeXMLSharedProperties( rSerializer, nFlags );
            _serializeXMLClasses( rSerializer, nFlags );
            _serializeXMLFeatures( rSerializer, nFlags );
            _serializeXMLEntities( rSerializer, nFlags );
            _serializeXMLObjects( rSerializer, nFlags );
            _serializeXMLGroups( rSerializer, nFlags );
        }

        rSerializer.endElement();
    }

    else if (nFlags & DWFPackageWriter::eSectionContent &&
             _zSerializingResource.chars() > 0)
    {
        _tResourceRIMapConstIterator iResource = _oResourceRenderableToInstance.find( _zSerializingResource );
        if (iResource != _oResourceRenderableToInstance.end())
        {
            DWFString zNamespace( namespaceXML(nFlags) );
            rSerializer.startElement( DWFXML::kzElement_Instances, zNamespace );

            _tRenderableToInstanceMultiMap* pRIMap = iResource->second;

            if (pRIMap)
            {
                _tRenderableToInstanceMultiMap::const_iterator itMap = pRIMap->begin();
                for (; itMap != pRIMap->end(); ++itMap)
                {
                    DWFInstance* pInstance = itMap->second;
                    if (pInstance)
                    {
                        pInstance->serializeXML( rSerializer, nFlags );
                    }
                }
            }

            rSerializer.endElement();
        }
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLSharedProperties( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oSharedProperties.size() == 0)
    {
        return;
    }

    DWFPropertySet::tMap::Iterator* pIterator = _oSharedProperties.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_SharedProperties, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFPropertySet* pSet = pIterator->value();
            if (pSet)
            {
                pSet->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLClasses( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oClasses.size() == 0)
    {
        return;
    }

    DWFClass::tMap::Iterator* pIterator = _oClasses.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_Classes, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFClass* pClass = pIterator->value();
            if (pClass)
            {
                pClass->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLFeatures( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oFeatures.size() == 0)
    {
        return;
    }

    DWFFeature::tMap::Iterator* pIterator = _oFeatures.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_Features, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFFeature* pFeature = pIterator->value();
            if (pFeature)
            {
                pFeature->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLEntities( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oEntities.size() == 0)
    {
        return;
    }

    DWFEntity::tMap::Iterator* pIterator = _oEntities.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_Entities, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFEntity* pEntity = pIterator->value();
            if (pEntity)
            {
                pEntity->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLObjects( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oObjects.size() == 0)
    {
        return;
    }

    DWFObject::tMap::Iterator* pIterator = _oObjects.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_Objects, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFObject* pObject = pIterator->value();
            if (pObject && 
                pObject->getParent() == NULL)
            {
                pObject->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

_DWFTK_API
void
DWFContent::_serializeXMLGroups( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (_oGroups.size() == 0)
    {
        return;
    }

    DWFGroup::tMap::Iterator* pIterator = _oGroups.iterator();
    if (pIterator)
    {
        rSerializer.startElement( DWFXML::kzElement_Groups, DWFXML::kzNamespace_DWF );

        for (; pIterator->valid(); pIterator->next())
        {
            DWFGroup* pGroup = pIterator->value();
            if (pGroup)
            {
                pGroup->getSerializable().serializeXML( rSerializer, nFlags );
            }
        }

        rSerializer.endElement();

        DWFCORE_FREE_OBJECT( pIterator );
    }
}

#endif

_DWFTK_API
void
DWFContent::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        //
        // skip over any "dwf:" in the attribute name
        //
        pAttrib = (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0) ?
                  &ppAttributeList[iAttrib][4] :
                  &ppAttributeList[iAttrib][0];

            //
            // set the href
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_HRef) == 0))
        {
            nFound |= 0x01;

            _zHREF.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the content ID
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ContentID) == 0))
        {
            nFound |= 0x02;

            _zID.assign( ppAttributeList[iAttrib+1] );
        }
    }

    //
    //  TODO: For any content generated with this case we need to extract the ID from the content href.
    //  Eventually this should be removed since release files should not have this. The href and ID 
    //  should be independent of each other.
    //
    if (_zHREF.chars() > 0 &&
        _zID.chars() == 0)
    {
        DWFString zContentXML( DWFContent::kzExtension_ContentXML );
        off_t nOffset = _zHREF.find( zContentXML ); 
        if (nOffset == -1)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The content ID could not be determined" );
        }
        else
        {
            _zID.assign( _zHREF.substring( 0, nOffset ) );
        }
    }
}

////

_DWFTK_API
void
DWFContent::load( DWFContentReader* pReaderFilter, 
                  unsigned int nProviderFlags )
throw( DWFException )
{

    if (_bElementsLoaded)
    {
        return;
    }

    if (_pPackageReader == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"If a package reader is not set load cannot be called on the content" );
    }

    //
    //  Create the input stream for the content file.
    //
    DWFPointer<DWFInputStream> apContentInput(getInputStream(), false );

    //
    //  Get the content reader
    //
    DWFContentReader* pThisReader = getContentReader();

    //
    //  Change the reader flags if necessary
    //
    unsigned int nExistingFlags = pThisReader->providerFlags();
    if (nExistingFlags != nProviderFlags)
    {
        pThisReader->setProviderFlags( nProviderFlags );
    }

    //
    //  Set the filter on the default reader
    //
    if (pReaderFilter)
    {
        pThisReader->setFilter( pReaderFilter );
    }

    //
    //  Do the actual parsing of the document
    //
    _parseContentDocument( *(DWFInputStream*)apContentInput, *pThisReader );

    //
    //  Change the reader flags back if they were modified
    //
    if (nExistingFlags != nProviderFlags)
    {
        pThisReader->setProviderFlags( nExistingFlags );
    }

    _bElementsLoaded = true;
}

_DWFTK_API
void
DWFContent::load( DWFContentReader& rCustomReader )
throw( DWFException )
{
    if (_pPackageReader == NULL)
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"If a package reader is not set load cannot be called on the content" );
    }

    //
    //  Create the input stream for the content file.
    //
    DWFPointer<DWFInputStream> apContentInput(getInputStream(), false );

    _parseContentDocument( *(DWFInputStream*)apContentInput, rCustomReader );
}

_DWFTK_API
bool
DWFContent::unload( bool bSkipIfAnyResourceModified )
throw( DWFException )
{
    if (bSkipIfAnyResourceModified &&
        _oModifiedContentResources.size() > 0)
    {
        return false;
    }
    else
    {
        //
        //  Delete items before destroying the containers
        //
        DeleteAllocatedValuesInList( _oClasses );
        DeleteAllocatedValuesInList( _oEntities );
        DeleteAllocatedValuesInList( _oFeatures );
        DeleteAllocatedValuesInList( _oObjects );
        DeleteAllocatedValuesInList( _oGroups );
        DeleteAllocatedValuesInList( _oInstances );
        DeleteAllocatedValuesInList( _oSharedProperties );

        //
        // Delete the allocated maps for resource object ID and renderable 
        // to instance mapping
        //
        _tResourceRIMapIterator it = _oResourceRenderableToInstance.begin();
        for (; it != _oResourceRenderableToInstance.end(); ++it)
        {
            DWFCORE_FREE_OBJECT( it->second );
        }
        _oResourceRenderableToInstance.clear();
        _oModifiedContentResources.clear();

        //
        //  Clear out the maps track the heirarchically downward references
        //
        _oBaseClassToClass.clear();
        _oClassToEntity.clear();
        _oClassToFeature.clear();
        _oEntityToObject.clear();
        _oFeatureToEntity.clear();
        _oFeatureToObject.clear();
        _oElementToGroup.clear();
        _oSetToSet.clear();

        //
        //  Clear out the set of IDs of the modififed resources
        //
        _oModifiedContentResources.clear();

        //
        //  Just in case! Clear everything for reload
        //

        //
        _oRefProperties.clear();
        //
        _oReadClassBaseClassRefs.clear();
        _oReadEntityChildEntityRefs.clear();
        _oReadGroupElementRefs.clear();
        _oReadPropertySetRefs.clear();

        //
        //  Set the flag
        //
        _bElementsLoaded = false;

        return true;
    }
}

_DWFTK_API
bool
DWFContent::unloadResourceInstances( const DWFString& zResourceObjectID,
                                     bool bSkipIfResourcesModified )
throw()
{
    if (bSkipIfResourcesModified && 
        isSectionContentResourceModified( zResourceObjectID ))
    {
        return false;
    }
    else
    {
        _tResourceRIMapIterator iResource = _oResourceRenderableToInstance.find( zResourceObjectID );

        if (iResource != _oResourceRenderableToInstance.end())
        {
            //
            //  Get the object to instance map
            //
            _tRenderableToInstanceMultiMap* pMap = iResource->second;

            //
            //  Now iterate over every instance in the map and delete it from the main
            //  instance container, _oInstance
            //
            if (pMap)
            {
                _tRenderableToInstanceMultiMap::const_iterator iInstance = pMap->begin();
                for (; iInstance != pMap->end(); ++iInstance)
                {
                    DWFInstance* pInstance = iInstance->second;
                    _oInstances.erase( pInstance->id() );
                    DWFCORE_FREE_OBJECT( pInstance );
                }

                DWFCORE_FREE_OBJECT( pMap );
            }

            _oResourceRenderableToInstance.erase( iResource );

            //
            // In case the resource was indeed modified - remove it from the set
            //
            _oModifiedContentResources.erase( zResourceObjectID );

            return true;
        }
        else
        {
            return false;
        }
    }
}

_DWFTK_API
bool
DWFContent::isResourceLoaded( const DWFString& zResourceObjectID )
throw()
{
    _tResourceRIMapIterator iResource = _oResourceRenderableToInstance.find( zResourceObjectID );

    return (iResource != _oResourceRenderableToInstance.end());
}

_DWFTK_API
DWFInputStream*
DWFContent::getInputStream()
throw( DWFException )
{
    return _pPackageReader->extract( href() );
}

_DWFTK_API
void
DWFContent::_parseContentDocument( DWFInputStream& rDocumentStream,
                                   DWFCore::DWFXMLCallback& rDocumentReader )
throw( DWFException )
{
	DWFXMLParser contentParser(&rDocumentReader);
    contentParser.parseDocument( rDocumentStream);
}

////

_DWFTK_API
const char*
DWFContent::provideVersion( const char* zVersion )
throw( DWFException )
{
    _zVersion = zVersion;
    return zVersion;
}

DWFContent::_tStringIterator*
DWFContent::_tokenizeBySpace( const DWFString& zInput )
throw()
{
    //
    // TODO: REPLACE wcstok with a thread-safe DWFString tokenizer class
    // for the VC7.1 build
    //

    _DWFTK_STD_VECTOR(DWFString)  oTokens;

    //
    // Extract a "destructible" wchar_t* copy of the DWFString
    //
    wchar_t* zwcCopy = DWFCORE_ALLOC_MEMORY( wchar_t, zInput.chars()+1 );
    DWFCORE_WIDE_STRING_COPY( zwcCopy, zInput );

#if defined(_DWFCORE_WIN32_SYSTEM) && (_MSC_VER<1400)

    wchar_t* zwcToken = ::wcstok( zwcCopy, /*NOXLATE*/L" " );
    while (zwcToken)
    {
        oTokens.push_back( DWFString(zwcToken) );
        zwcToken = ::wcstok( NULL, /*NOXLATE*/L" " );
    }

#else

    typedef wchar_t* (*tpfWCTokenizer)(wchar_t*, const wchar_t*, wchar_t**);

    #if defined(_MSC_VER) && (_MSC_VER>=1400)
        tpfWCTokenizer pfWCTokenizer = &(::wcstok_s);
    #else
        tpfWCTokenizer pfWCTokenizer = &(::wcstok);
    #endif

    //
    //  These are safe tokenizers since the intermediate state information is stored in zwcState
    //
    wchar_t* zwcState;
    wchar_t* zwcToken = (*pfWCTokenizer)( zwcCopy, /*NOXLATE*/L" ", &zwcState);
    while (zwcToken)
    {
        oTokens.push_back( DWFString(zwcToken) );
        zwcToken = (*pfWCTokenizer)( NULL, /*NOXLATE*/L" ", &zwcState);
    }

#endif

    //
    // The just-in-case cleanup
    //
    DWFCORE_FREE_MEMORY( zwcCopy );


    typedef DWFVectorIterator<DWFString, _DWFTK_STL_ALLOCATOR<DWFString> > _tStringVectorIterator;
    _tStringIterator* piToken = DWFCORE_ALLOC_OBJECT( _tStringVectorIterator(oTokens) );

    return piToken;
}

_DWFTK_API
DWFClass*
DWFContent::provideClass( DWFClass* pClass,
                          const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oClasses.insert( pClass->id(), pClass );
    pClass->setContent( this );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Base class refs
        //
        if (rU.nType == DWFClass::eBaseClassReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadClassBaseClassRefs.insert( std::make_pair( pClass, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Property set refs
        //
        else if (rU.nType == DWFClass::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair( pClass, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pClass;
}

_DWFTK_API
DWFFeature*
DWFContent::provideFeature( DWFFeature* pFeature,
                            const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oFeatures.insert( pFeature->id(), pFeature );
    pFeature->setContent( this );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Class refs
        //
        if (rU.nType == DWFFeature::eClassReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                DWFClass* pClass = getClass( piID->get() );
                if (pClass)
                {
                    pFeature->_addClass( pClass );
                    _oClassToFeature.insert( std::make_pair( pClass, pFeature ) );
                }
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Property set refs
        //
        else if (rU.nType == DWFFeature::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pFeature, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pFeature;
}

_DWFTK_API
DWFEntity*
DWFContent::provideEntity( DWFEntity* pEntity,
                           const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oEntities.insert( pEntity->id(), pEntity );
    pEntity->setContent( this );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Child entity refs
        //
        if (rU.nType == DWFEntity::eChildEntityReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadEntityChildEntityRefs.insert( std::make_pair(pEntity, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Class refs
        //
        else if (rU.nType == DWFEntity::eClassReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                DWFClass* pClass = getClass( piID->get() );
                if (pClass)
                {
                    pEntity->_addClass( pClass );
                    _oClassToEntity.insert( std::make_pair(pClass, pEntity) );
                }
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Feature refs
        //
        else if (rU.nType == DWFEntity::eFeatureReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                DWFFeature* pFeature = getFeature( piID->get() );
                if (pFeature)
                {
                    pEntity->_addFeature( pFeature );
                    _oFeatureToEntity.insert( std::make_pair(pFeature, pEntity) );
                }
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Property set refs
        //
        else if (rU.nType == DWFEntity::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pEntity, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }    

    return pEntity;
}

_DWFTK_API
DWFObject*
DWFContent::provideObject( DWFObject* pObject,
                           const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oObjects.insert( pObject->id(), pObject );
    pObject->setContent( this );

    // TODO:
    //  Replace strtok with a thread-safe DWFString tokenizer class
    //
    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  The entity
        //
        if (rU.nType == DWFObject::eEntityReference)
        {
            //
            //  Entities are available at this point
            //
            DWFEntity* pEntity = getEntity( rU.zValue );
            if (pEntity == NULL)
            {
                _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"An entity could not be found for the object." );
            }
            pObject->_setEntity( pEntity );
            _oEntityToObject.insert( std::make_pair(pEntity, pObject) );
        }

        //
        //  Feature refs
        //
        else if (rU.nType == DWFObject::eFeatureReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                DWFFeature* pFeature = getFeature( piID->get() );
                if (pFeature)
                {
                    pObject->_addFeature( pFeature );
                    _oFeatureToObject.insert( std::make_pair(pFeature, pObject) );
                }
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Property set refs
        //
        else if (rU.nType == DWFObject::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pObject, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pObject;
}

_DWFTK_API
DWFObject*
DWFContent::provideChildObject( DWFObject* pObject,
                                DWFObject* pParent,
                                const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    //
    //  The parent
    //
    if (pParent)
    {
        pParent->_addChild( pObject );
    }

    return provideObject( pObject, rUnresolved );
}

_DWFTK_API
DWFGroup*
DWFContent::provideGroup( DWFGroup* pGroup,
                          const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oGroups.insert( pGroup->id(), pGroup );
    pGroup->setContent( this );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Element refs
        //
        if (rU.nType == DWFGroup::eContentElementReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                //
                //  Most elements are available at this point, except some groups maybe
                //
                DWFContentElement* pElement = getElement( piID->get() );
                if (pElement)
                {
                    pGroup->_addElement( pElement );
                    _oElementToGroup.insert( std::make_pair(pElement, pGroup) );
                }
                else
                {
                    _oReadGroupElementRefs.insert( std::make_pair(pGroup, piID->get()) );
                }
            }
            DWFCORE_FREE_OBJECT( piID );
        }

        //
        //  Property set refs
        //
        else if (rU.nType == DWFGroup::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pGroup, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pGroup;
}

_DWFTK_API
DWFPropertySet*
DWFContent::provideSharedProperty( DWFPropertySet* pPropertySet,
                                   const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oSharedProperties.insert( pPropertySet->id(), pPropertySet );
    pPropertySet->setContent( this );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Property set refs
        //
        if (rU.nType == DWFPropertySet::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pPropertySet, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pPropertySet;
}

_DWFTK_API
DWFPropertySet*
DWFContent::providePropertySet( DWFPropertySet* pPropertySet,
                                DWFPropertyContainer* pContainer,
                                const tUnresolvedList& rUnresolved )
throw( DWFException )
{
    _oRefProperties.insert( pPropertySet->id(), pPropertySet );

    pContainer->addPropertyContainer( pPropertySet );

    tUnresolvedList::const_iterator iList = rUnresolved.begin();
    for (; iList != rUnresolved.end(); ++iList)
    {
        const tUnresolved& rU = *iList;

        //
        //  Property set refs
        //
        if (rU.nType == DWFPropertySet::ePropertySetReferences)
        {
            _tStringIterator* piID = _tokenizeBySpace( rU.zValue );
            for (; piID->valid(); piID->next())
            {
                _oReadPropertySetRefs.insert( std::make_pair(pPropertySet, piID->get()) );
            }
            DWFCORE_FREE_OBJECT( piID );
        }
    }

    return pPropertySet;
}

_DWFTK_API
DWFInstance*
DWFContent::provideInstance( DWFInstance* pInstance, 
                             const DWFString& zRenderableID,
                             const DWFString& zResourceObjectID )
throw( DWFException )
{
    DWFRenderable* pRenderable = getObject( zRenderableID );

    if (pRenderable == NULL)
    {
        pRenderable = getFeature( zRenderableID );
    }
    
    if (pRenderable)
    {
        pInstance->setRenderable( pRenderable );
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The renderable element for the instance could not be determined." );
    }

    _oInstances.insert( pInstance->id(), pInstance );

    //
    // Find the renderable-to-instance multimap to store the renderable-instance pair. If the map
    // doesn't exist, create it by inserting into the main map.
    //
    _tResourceRIMapIterator iResource = _oResourceRenderableToInstance.find( zResourceObjectID );
    if (iResource == _oResourceRenderableToInstance.end())
    {
        std::pair<_tResourceRIMapIterator, bool> result;
        _tRenderableToInstanceMultiMap* pRIMap = DWFCORE_ALLOC_OBJECT( _tRenderableToInstanceMultiMap() );
        result = _oResourceRenderableToInstance.insert( std::make_pair( zResourceObjectID, pRIMap ) );
        iResource = result.first;
    }
    _tRenderableToInstanceMultiMap* pRIMap = iResource->second;
    //
    // Now insert the pair
    //
    pRIMap->insert( std::make_pair( pRenderable, pInstance ) );

    return pInstance;
}

////

_DWFTK_API
void
DWFContent::resolveClasses()
throw()
{
    _tClassToRefs::iterator iClass = _oReadClassBaseClassRefs.begin();

    for (; iClass != _oReadClassBaseClassRefs.end(); ++iClass)
    {
        DWFClass* pClass = iClass->first;
        DWFClass* pBaseClass = getClass( iClass->second );    

        if (pBaseClass)
        {
            pClass->_addBaseClass( pBaseClass );
            _oBaseClassToClass.insert( std::make_pair( pBaseClass, pClass ) );
        }
    }

    _oReadClassBaseClassRefs.clear();
}

_DWFTK_API
void
DWFContent::resolveFeatures()
throw()
{
    // Do nothing
}

_DWFTK_API
void
DWFContent::resolveEntities()
throw()
{
    _tEntityToRefs::iterator iEntity = _oReadEntityChildEntityRefs.begin();

    for (; iEntity != _oReadEntityChildEntityRefs.end(); ++iEntity)
    {
        DWFEntity* pEntity = iEntity->first;
        DWFEntity* pChildEntity = getEntity( iEntity->second );    

        if (pChildEntity)
        {
            pEntity->_addChild( pChildEntity );
        }
    }

    _oReadEntityChildEntityRefs.clear();
}

_DWFTK_API
void
DWFContent::resolveObjects()
throw()
{
    // Do nothing
}

_DWFTK_API
void
DWFContent::resolveGroups()
throw()
{
    _tGroupToRefs::iterator iGroup = _oReadGroupElementRefs.begin();

    for (; iGroup != _oReadGroupElementRefs.end(); ++iGroup)
    {
        DWFGroup* pGroup = iGroup->first;
        DWFContentElement* pElement = getElement( iGroup->second );

        if (pElement)
        {
            pGroup->_addElement( pElement );
            _oElementToGroup.insert( std::make_pair( pElement, pGroup ) );
        }
    }

    _oReadGroupElementRefs.clear();
}

_DWFTK_API
void
DWFContent::resolvePropertySets()
throw()
{
    _tContainerToRef::iterator iContainer = _oReadPropertySetRefs.begin();

    for (; iContainer != _oReadPropertySetRefs.end(); ++iContainer)
    {
        DWFPropertyContainer* pContainer = iContainer->first;
        
        //
        //  Try the shared property sets first
        //
        DWFPropertySet* pSet = getSharedPropertySet( iContainer->second );

        if (pSet == NULL)
        {
            //
            //  Next try the property sets owned by others via _oRefProperties
            //
            DWFPropertySet** ppSet = _oRefProperties.find( iContainer->second );
            if (ppSet)
            {
                pSet = *ppSet;
            }
        }

        if (pSet)
        {
            pContainer->referencePropertyContainer( *pSet );
        }
    }

    _oReadPropertySetRefs.clear();
    _oRefProperties.clear();
}

_DWFTK_API
void
DWFContent::resolveInstances()
throw()
{
    // Do nothing
}

_DWFTK_API
DWFContent::DWFStringIterator*
DWFContent::modifiedContentResources()
throw()
{
    if (_oModifiedContentResources.size()>0)
    {
        DWFCachingIterator<DWFString>* piIter = DWFCORE_ALLOC_OBJECT( DWFCachingIterator<DWFString>() );
        _tSetOfIDs::iterator itIDs = _oModifiedContentResources.begin();
        for (; itIDs != _oModifiedContentResources.end(); ++itIDs)
        {
            piIter->add( *itIDs );
        }
        return piIter;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
void
DWFContent::identify( const DWFString& zID )
throw()
{
    if (zID.chars() > 0)
    {
        _zID = zID;

        if (_pPackageReader == NULL)
        {
            _zHREF.assign( _zID );
            _zHREF.append( DWFContent::kzExtension_ContentXML );
        }
    }
}


