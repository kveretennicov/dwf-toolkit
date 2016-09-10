//
//  Copyright (c) 2004-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/utility/ResourceContainer.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//


#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Constants.h"
#include "dwf/package/utility/ResourceContainer.h"
#include "dwf/package/writer/PackageWriter.h"
using namespace DWFToolkit;



_DWFTK_API
DWFResourceContainer::DWFResourceContainer()
throw()
    : _oUUID()
    , _oResourcesByHREF()
    , _oResourcesByObjectID()
    , _oResourcesByRole()
    , _oResourcesByMIME()
    , _oResourceHierarchy()
    , _zName()
#ifndef DWFTK_READ_ONLY
    , _oSerializer()
#endif
{
#ifndef DWFTK_READ_ONLY

    _oSerializer.is( this );

#endif
}

_DWFTK_API
DWFResourceContainer::~DWFResourceContainer()
throw()
{
    DWFResource::tMap::Iterator* piResource = _oResourcesByHREF.iterator();

    if (piResource)
    {
        for(; piResource->valid();
              piResource->next())
        {
            DWFResource* pRes = piResource->value();

                //
                // delete if owned
                //
            if (pRes->owner() == this)
            {
                DWFCORE_FREE_OBJECT( pRes );
            }
                //
                // remove from observation if not owned
                //
            else
            {
                pRes->unobserve(*this);
            }
        }

        DWFCORE_FREE_OBJECT( piResource );
    }
}

_DWFTK_API
size_t
DWFResourceContainer::resourceCount() const
throw()
{
    return _oResourcesByHREF.size();
}

_DWFTK_API
DWFResource*
DWFResourceContainer::findResourceByHREF( const DWFString& zHRef )
throw()
{
    DWFResource** ppResource = _oResourcesByHREF.find( (const wchar_t*)zHRef );
    return (ppResource ? *ppResource : NULL);
}

_DWFTK_API
DWFResource*
DWFResourceContainer::findResourceByObjectID( const DWFString& zObjectID )
throw()
{
    DWFResource** ppResource = _oResourcesByObjectID.find( (const wchar_t*)zObjectID );
    return (ppResource ? *ppResource : NULL);
}

_DWFTK_API
DWFResourceContainer::ResourceIterator*
DWFResourceContainer::findResourcesByRole( const DWFString& zRole )
throw()
{
    return DWFCORE_ALLOC_OBJECT( ResourceIterator(_oResourcesByRole.lower_bound((const wchar_t*)zRole), _oResourcesByRole.upper_bound((const wchar_t*)zRole)) );
}

_DWFTK_API
DWFResourceContainer::ResourceIterator*
DWFResourceContainer::findResourcesByMIME( const DWFString& zMIME )
throw()
{
    return DWFCORE_ALLOC_OBJECT( ResourceIterator(_oResourcesByMIME.lower_bound((const wchar_t*)zMIME), _oResourcesByMIME.upper_bound((const wchar_t*)zMIME)) );
}

_DWFTK_API
DWFResourceContainer::ResourceKVIterator*
DWFResourceContainer::getResourcesByHREF()
throw()
{
    return (DWFResourceContainer::ResourceKVIterator*)_oResourcesByHREF.iterator();
}

_DWFTK_API
DWFResourceContainer::ResourceKVIterator*
DWFResourceContainer::getResourcesByObjectID()
throw()
{
    return (DWFResourceContainer::ResourceKVIterator*)_oResourcesByObjectID.iterator();
}

_DWFTK_API
DWFResourceContainer::ResourceKVIterator*
DWFResourceContainer::getResourcesByRole()
throw()
{
    return DWFCORE_ALLOC_OBJECT( ResourceKVIterator(_oResourcesByRole.begin(), _oResourcesByRole.end()) );
}

_DWFTK_API
DWFResourceContainer::ResourceKVIterator*
DWFResourceContainer::getResourcesByMIME()
throw()
{
    return DWFCORE_ALLOC_OBJECT( ResourceKVIterator(_oResourcesByMIME.begin(), _oResourcesByMIME.end()) );
}

_DWFTK_API
DWFResource*
DWFResourceContainer::removeResource( DWFResource& rResource,
                                      bool         bDeleteIfOwned )
throw( DWFException )
{
    //
    // look up and remove by HREF
    //
    DWFResource** ppResource = _oResourcesByHREF.find( (const wchar_t*)rResource.href() );

    if (ppResource)
    {
        if (*ppResource == &rResource)
        {
            return _remove( *ppResource, bDeleteIfOwned );
        }
        else
        {
            _DWFCORE_THROW( DWFTypeMismatchException, /*NOXLATE*/L"Resource with matching HREF was found but was not the same object" );
        }
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFResource*
DWFResourceContainer::removeResourceByHREF( const DWFString& zHRef,
                                            bool             bDeleteIfOwned )
throw( DWFException )
{
    DWFResource* pResource = *(_oResourcesByHREF.find((const wchar_t*)zHRef));

    if (pResource)
    {
        return _remove( pResource, bDeleteIfOwned );
    }
    else
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Resource not found" );
    }
}

_DWFTK_API
DWFResource*
DWFResourceContainer::removeResourceByObjectID( const DWFString& zObjectID,
                                                bool             bDeleteIfOwned )
throw( DWFException )
{
    DWFResource* pResource = *(_oResourcesByObjectID.find((const wchar_t*)zObjectID));

    if (pResource)
    {
        return _remove( pResource, bDeleteIfOwned );
    }
    else
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Resource not found" );
    }
}

_DWFTK_API
DWFResourceContainer::ResourceIterator*
DWFResourceContainer::removeResourcesByRole( const DWFString&   zRole,
                                             bool               bDeleteIfOwned )
throw( DWFException )
{
    DWFBasicIteratorImpl<DWFResource*>* piResourceRemoved = NULL;

        //
        // build list of resources to remove
        //
    DWFResource::tMultiMap::iterator iFirst = _oResourcesByRole.lower_bound( (const wchar_t*)zRole );
    if (iFirst != _oResourcesByRole.end())
    {
        piResourceRemoved = DWFCORE_ALLOC_OBJECT( DWFBasicIteratorImpl<DWFResource*> );
        if (piResourceRemoved == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory" );
        }

        DWFResource::tMultiMap::iterator iLast = _oResourcesByRole.upper_bound( (const wchar_t*)zRole );
        for (;iFirst != iLast;iFirst++)
        {
            //
            // capture resource
            //
            piResourceRemoved->add( iFirst->second );
        }
    }

        //
        // remove resources
        //
    if (piResourceRemoved)
    {
        for (; piResourceRemoved->valid();
            piResourceRemoved->next())
        {
            _remove( piResourceRemoved->get(), bDeleteIfOwned );
        }

        if (bDeleteIfOwned)
        {
            DWFCORE_FREE_OBJECT( piResourceRemoved );
        }
        else
        {
            return (DWFResourceContainer::ResourceIterator*)piResourceRemoved;
        }
    }

    return NULL;
}

_DWFTK_API
DWFResourceContainer::ResourceIterator*
DWFResourceContainer::removeResourcesByMIME( const DWFString&   zMIME,
                                             bool               bDeleteIfOwned )
throw( DWFException )
{
    DWFBasicIteratorImpl<DWFResource*>* piResourceRemoved = NULL;

        //
        // build list of resources to remove
        //
    DWFResource::tMultiMap::const_iterator iFirst = _oResourcesByMIME.lower_bound( (const wchar_t*)zMIME );
    if (iFirst != _oResourcesByMIME.end())
    {
        piResourceRemoved = DWFCORE_ALLOC_OBJECT( DWFBasicIteratorImpl<DWFResource*> );
        if (piResourceRemoved == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory" );
        }

        DWFResource::tMultiMap::const_iterator iLast = _oResourcesByMIME.upper_bound( (const wchar_t*)zMIME );
        for (;iFirst != iLast;iFirst++)
        {
            //
            // capture resource
            //
            piResourceRemoved->add( const_cast<DWFResource*&>(iFirst->second) );
        }
    }

        //
        // remove resources
        //
    for (; piResourceRemoved->valid();
           piResourceRemoved->next())
    {
        _remove( piResourceRemoved->get(), bDeleteIfOwned );
    }

    if (bDeleteIfOwned)
    {
        DWFCORE_FREE_OBJECT( piResourceRemoved );
        return NULL;
    }
    else
    {
        return (DWFResourceContainer::ResourceIterator*)piResourceRemoved;
    }
}

DWFResource*
DWFResourceContainer::_remove( DWFResource* pResource, bool bDeleteIfOwned )
throw()
{
    //
    // Stop listening to changes from this resource.
    //
    pResource->removeNotificationSink(this);

    //
    // remove from href list
    //
    _oResourcesByHREF.erase( (const wchar_t*)pResource->href() );

    //
    // remove from ObjectID list
    //
    const DWFString& zObjectID = pResource->objectID();
    if (zObjectID.chars() > 0)
    {
        _oResourcesByObjectID.erase( (const wchar_t*)zObjectID );
    }

    //
    // remove from the role list
    //
    DWFResource::tMultiMap::iterator iResource = _oResourcesByRole.lower_bound( pResource->role() );
    DWFResource::tMultiMap::iterator iLast;

    if (iResource != _oResourcesByRole.end())
    {
        iLast = _oResourcesByRole.upper_bound( pResource->role() );

        for (;iResource != iLast; iResource++)
        {
            if (iResource->second == pResource)
            {
                _oResourcesByRole.erase( iResource );
                break;
            }
        }
    }

    //
    // remove from the MIME list
    //
    iResource = _oResourcesByMIME.lower_bound( pResource->mime() );

    if (iResource != _oResourcesByMIME.end())
    {
        iLast = _oResourcesByMIME.upper_bound( pResource->mime() );

        for (;iResource != iLast; iResource++)
        {
            if (iResource->second == pResource)
            {
                _oResourcesByMIME.erase( iResource );
                break;
            }
        }
    }

    //
    // remove from hierarchy
    //
    _oResourceHierarchy.erase( pResource );

    tResourcePointerMultiMap::iterator iResourceHierarchy = _oResourceHierarchy.begin();
    for (; iResourceHierarchy != _oResourceHierarchy.end(); iResourceHierarchy++)
    {
        if (iResourceHierarchy->second == pResource)
        {
            _oResourceHierarchy.erase( iResourceHierarchy );
            break;
        }
    }

    if (bDeleteIfOwned && (pResource->owner() == this))
    {
        DWFCORE_FREE_OBJECT( pResource );
        return NULL;
    }
    else
    {
        if(pResource->owner() == this)
        {
            pResource->disown( *this, true );
        }
        else
        {
            pResource->unobserve( *this );
        }

        return pResource;
    }
}

_DWFTK_API
DWFResource*
DWFResourceContainer::addResource( DWFResource*       pResource,
                                   bool               bOwnResource,
                                   bool               bReplace,
                                   bool               bDeleteReplacedIfOwned,
                                   const DWFResource* pParentResource )
throw( DWFException )
{
    DWFResource* pReplaced = NULL;

    if (pResource)
    {
            //
            // claim ownership
            //
        if (bOwnResource)
        {
            pResource->own( *this );
        }
            //
            // inform that we're holding a pointer to this
            // resource, and need to be told of its demise.
            //
        else
        {
            pResource->observe( *this );
        }

            //
            // make one up (it's temporary anyway) if none exists
            //
        if (pResource->href().bytes() == 0)
        {
            //
            // TODO: is this OK?
            //
            pResource->setHRef( _oUUID.next( false ) );
        }

        const DWFString& zHREF = pResource->href();

        //
        // Sign up to hear about changes to this resource.
        //
        pResource->setNotificationSink(this);

            //
            // check for previous
            //
        if (bReplace)
        {
            DWFResource** ppFind = _oResourcesByHREF.find( (const wchar_t*)zHREF );
            if (ppFind)
            {
                pReplaced = *ppFind;

                    //
                    // manage old pointer...
                    //
                if ( pReplaced != NULL )
                {
                    _remove( pReplaced, bDeleteReplacedIfOwned );
                }

            }
        }
            
            //
            // insert new resource
            //
        _oResourcesByHREF.insert( (const wchar_t*)pResource->href(), pResource, bReplace );

            //
            // store in aux lists
            //
        const DWFString& zObjectID = pResource->objectID();
        if (zObjectID.chars() > 0)
        {
            _oResourcesByObjectID.insert( (const wchar_t*)zObjectID, pResource, bReplace );
        }

        _oResourcesByRole.insert( DWFResource::tMultiMap::value_type(pResource->role(), pResource) );
        _oResourcesByMIME.insert( DWFResource::tMultiMap::value_type(pResource->mime(), pResource) );

            //
            // capture the structure
            //
        if (pParentResource)
        {
            _oResourceHierarchy.insert( tResourcePointerMultiMap::value_type(pParentResource, pResource) );
        }
    }

    return pReplaced;
}

_DWFTK_API
void
DWFResourceContainer::notifyOwnerChanged( DWFOwnable& /*rOwnable*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFResourceContainer::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    DWFResource* pResource = dynamic_cast<DWFResource*>(&rOwnable);

    if (pResource && _oResourcesByHREF.find(pResource->href()))
    {
        removeResource( *pResource, false );
    }
}

_DWFTK_API
void
DWFResourceContainer::onBeforeResourceHRefChanged( DWFResource* pResource )
    throw()
{
    if(pResource==NULL)
    {
        return;
    }

    //
    // The href for a resource is about to change, remove its entry from the href->resource map.
    //
    // Note: It is very important that we do this *before* the href changes, as the key in
    // the map is the wchar_t pointer of the resource's href DWFString. A change to this string
    // could easily leave the key pointing to garbage!
    //
    const wchar_t *hRef = (const wchar_t *) (pResource->href());
    DWFResource** ppResource = _oResourcesByHREF.find( hRef );
    if((*ppResource)==pResource)
    {
        _oResourcesByHREF.erase( hRef );
    }
}

_DWFTK_API
void
DWFResourceContainer::onAfterResourceHRefChanged( DWFResource* pResource )
    throw()
{
    if(pResource==NULL)
    {
        return;
    }

    //
    // The href for a resource has changed, add a new entry for it to the href->resource map.
    //
    const wchar_t *hRef = (const wchar_t *) (pResource->href());

    _oResourcesByHREF.insert( hRef, pResource, true );
}



#ifndef DWFTK_READ_ONLY

void
DWFResourceContainer::_Serializer::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
        //
        // detail the section in the manifest
        //
    if (nFlags & DWFPackageWriter::eManifest)
    {
        //
        // manually create the collection since there is no corresponding dwf object
        //
        rSerializer.startElement( DWFXML::kzElement_TOC, DWFXML::kzNamespace_DWF );
        {
            DWFResource* pResource = NULL;
            DWFResource::tMultiMap::const_iterator iResource = _pContainer->_oResourcesByRole.begin();

            for (; iResource != _pContainer->_oResourcesByRole.end(); iResource++)
            {
                pResource = iResource->second;

                    //
                    // leave certain resources alone...
                    //
                if (pResource->role() != DWFXML::kzRole_Descriptor)
                {
                    //
                    // give the resource information to create a valid target href
                    //
                    pResource->setPublishedIdentity( _pContainer->_zName, rSerializer.nextUUID( false ) );

                    //
                    // if this resource has any "children", now is the time
                    // to set their ParentObjectID attribute value
                    //
                    tResourcePointerMultiMap::const_iterator iLower = _pContainer->_oResourceHierarchy.lower_bound( pResource );
                    if (iLower != _pContainer->_oResourceHierarchy.end())
                    {
                        tResourcePointerMultiMap::const_iterator iUpper = _pContainer->_oResourceHierarchy.upper_bound( pResource );
                        for (;iLower != iUpper; iLower++)
                        {
                            iLower->second->setParentResource( pResource );
                        }                        
                    }
                }
                    
                //
                // let each resource write itself into the XML document
                //
                pResource->serializeXML( rSerializer, nFlags );
            }
        }
        rSerializer.endElement();
    }
    else if (nFlags & DWFPackageWriter::eDescriptor)
    {
        //
        // only serialize the resource collection if it contains at
        // least one resource other than the descriptor
        //
        DWFResource::tMultiMap::const_iterator iResource = _pContainer->_oResourcesByRole.begin();
        if (iResource != _pContainer->_oResourcesByRole.end())
        {
                //
                // if we have two or more resources or a non-descriptor resource
                //
            if ( (_pContainer->_oResourcesByRole.size() > 1) ||
                 (iResource->second->role() != DWFXML::kzRole_Descriptor) )
            {
                DWFString zNamespace( namespaceXML(nFlags) );

                //
                // manually create the collection since there is no corresponding dwf object
                //
                rSerializer.startElement( DWFXML::kzElement_Resources, zNamespace );
                {
                    DWFResource* pResource = NULL;

                    for (; iResource != _pContainer->_oResourcesByRole.end(); iResource++)
                    {
                        pResource = iResource->second;

                        //
                        // let each resource write itself into the XML document
                        //
                        pResource->serializeXML( rSerializer, nFlags );
                    }
                }
                rSerializer.endElement();
            }
        }
    }
}

#endif

