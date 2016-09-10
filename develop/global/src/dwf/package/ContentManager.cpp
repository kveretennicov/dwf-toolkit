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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/ContentManager.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwf/package/ContentManager.h"
#include "dwf/package/Section.h"
#include "dwf/package/SectionContentResource.h"
using namespace DWFToolkit;

_DWFTK_API
DWFContentManager::DWFContentManager( DWFPackageReader* pPackageReader )
throw()
                 : _pPrimaryContent( NULL )
                 , _oContent()
                 , _pUUIDProvider( NULL )
                 , _pPackageReader( pPackageReader )
{
    //  Initialize the UUID provider
    _pUUIDProvider = DWFCORE_ALLOC_OBJECT( DWFUUID() );
    _pUUIDProvider->uuid(true);
}

_DWFTK_API
DWFContentManager::~DWFContentManager()
throw()
{
    DWFCORE_FREE_OBJECT( _pUUIDProvider );

    DWFContent::tMap::Iterator* piContent = _oContent.iterator();
    if (piContent)
    {
        for (; piContent->valid(); piContent->next())
        {
            DWFContent* pContent = piContent->value();

            if (pContent)
            {
                if ( (pContent->owner() == this) ||
                     (pContent->owner() == NULL)    // it's open season on unowned objects
                   )
                {
                    DWFCORE_FREE_OBJECT( pContent );
                }
                else
                {
                    pContent->unobserve( *this );
                }
            }
        }

        DWFCORE_FREE_OBJECT( piContent );
    }
    _oContent.clear();

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
DWFContent*
DWFContentManager::getContent( const DWFString& zContentID )
throw( DWFException )
{
    if (zContentID.chars() == 0)
    {
        if (_pPrimaryContent)
        {
            return _pPrimaryContent;
        }
        else
        {
            if (_oContent.size()>0)
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"PrimaryContent should not be NULL if a content exists" );
            }

            //
            // if we are reading and we don't have any content return NULL
            //
            if (_pPackageReader != NULL)
            {
                return NULL;
            }

            DWFContent* pContent = addContent();
            return pContent;
        }
    }

    DWFContent** ppContent = _oContent.find( zContentID );
    if (ppContent)
    {
        return *ppContent;
    }
    else
    {
        return NULL;
    }
}

_DWFTK_API
DWFContent*
DWFContentManager::getContentByHREF( const DWFString& zHREF )
throw()
{
    DWFContent* pContent = NULL;

    DWFContent::tMap::Iterator* piContent = _oContent.iterator();
    if (piContent)
    {
        for (; piContent->valid(); piContent->next())
        {
            DWFContent* pCurrent = piContent->value();
            if (pCurrent && pCurrent->href() == zHREF)
            {
                pContent = pCurrent;
                break;
            }
        }
        DWFCORE_FREE_OBJECT( piContent );
    }

    return pContent;
}

_DWFTK_API
DWFContent*
DWFContentManager::addContent( const DWFString& zID, 
                               DWFPackageReader* pReader )
throw( DWFException )
{
    //
    //  If a content with the provided ID already exists, return that content.
    //
    if (zID.chars()>0)
    {
        //
        //  Ensure no content with the given ID already exists
        //
        DWFContent** ppContent = _oContent.find( zID );
        if (ppContent != NULL)
        {
            return *ppContent;
        }
    }

    DWFContent* pContent = NULL;
    if (pReader)
    {
        pContent = DWFCORE_ALLOC_OBJECT( DWFContent(pReader, zID) );
        pContent->setContentManager( this );
    }
    else
    {
        pContent = DWFCORE_ALLOC_OBJECT( DWFContent(this, zID) );
    }
    pContent->own( *this );
    _oContent.insert( pContent->id(), pContent );

    //
    //  The first content created is by default the primary content
    //
    if (_oContent.size() == 1)
    {
        _pPrimaryContent = pContent;
    }

    return pContent;
}

_DWFTK_API
DWFContent*
DWFContentManager::insertContent( DWFContent* pContent )
throw( DWFException )
{
    DWFContent** ppContent = _oContent.find( pContent->id() );

    //
    //  If something is found the content has already been inserted
    //
    if (ppContent == NULL)
    {
        _oContent.insert( pContent->id(), pContent );
        pContent->setContentManager( this );
        pContent->own( *this );
    }

    if (_oContent.size() == 1)
    {
        _pPrimaryContent = pContent;
    }

    return pContent;
}

_DWFTK_API
void
DWFContentManager::removeContents( DWFContent::tList& roRemovedContent )
throw()
{
    DWFContent::tMap::Iterator* piContent = _oContent.iterator();
    if (piContent)
    {
        for (; piContent->valid(); piContent->next())
        {
            DWFContent* pContent = piContent->value();
            roRemovedContent.push_back( pContent );
            if (pContent->owner() == this)
            {
                pContent->disown( *this, true );
            }
            else
            {
                pContent->unobserve( *this );
            }
        }

        _oContent.clear();

        DWFCORE_FREE_OBJECT( piContent );
    }

    _pPrimaryContent = NULL;
}

_DWFTK_API
DWFContent*
DWFContentManager::removeContent( const DWFString& zContentID,
                                  const DWFString& zNewPrimaryID,
                                  bool bDeleteIfOwned )
throw( DWFException )
{
    bool bOwned = true;

    DWFContent* pContent = NULL;

    DWFContent** ppContent = _oContent.find( zContentID );
    if (ppContent)
    {
        pContent = *ppContent;
        if (pContent->owner() == this)
        {
            pContent->disown( *this, true );
        }
        else
        {
            pContent->unobserve( *this );
            bOwned = false;
        }
        _oContent.erase( zContentID );
    }
    else
    {
        return NULL;
    }

    if (pContent == _pPrimaryContent)
    {
        if (_oContent.size()>0)
        {
            //
            //  First set up a primary content in case we throw below - we can then recover.
            //
            DWFContent::tMap::Iterator* piContent = _oContent.iterator();
            if (piContent->valid())
            {
                //
                // Get the first one available
                //
                _pPrimaryContent = piContent->value();

                DWFCORE_FREE_OBJECT( piContent );
            }

            if (zNewPrimaryID.chars()>0)
            {
                DWFContent** ppContent = _oContent.find( zNewPrimaryID );
                if (ppContent)
                {
                    _pPrimaryContent = *ppContent;
                }
                else
                {
                    _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Could not set primary content since the requested content was not found" );
                }
            }
        }
        else
        {
            _pPrimaryContent = NULL;
        }
    }

    if (bOwned && bDeleteIfOwned)
    {
        DWFCORE_FREE_OBJECT( pContent );
    }

    return pContent;
}

_DWFTK_API
void 
DWFContentManager::notifyOwnableDeletion( DWFOwnable& rOwnable )
throw( DWFException )
{
    //
    //  This is called if a content is owned by another content manager, and it 
    //  is getting deleted
    //
    DWFContent* pContent = dynamic_cast<DWFContent*>(&rOwnable);
    if (pContent)
    {
        _oContent.erase( pContent->id() );
    }
}

_DWFTK_API
void
DWFContentManager::mergeContent( bool bPrimaryHasPriority )
throw()
{
    DWFContent::tMap::Iterator* piContents = _oContent.iterator();
    if (piContents)
    {
        //
        //  The contents need to be loaded for merging
        //
        if (_pPrimaryContent->isLoaded()==false)
        {
            _pPrimaryContent->load();
        }

        std::vector<DWFContent*> oRemoveContent;

        for (; piContents->valid(); piContents->next())
        {
            DWFContent* pContent = piContents->value();

            if (pContent->isLoaded()==false)
            {
                pContent->load();
            }

            if (pContent != _pPrimaryContent)
            {
                _pPrimaryContent->mergeContent( pContent, bPrimaryHasPriority );
                oRemoveContent.push_back( pContent );
            }
        }

        std::vector<DWFContent*>::iterator iContent = oRemoveContent.begin();
        for (; iContent != oRemoveContent.end(); ++iContent)
        {
            removeContent( (*iContent)->id() );
        }

        DWFCORE_FREE_OBJECT( piContents );
    }
    
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFContentManager::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    if (nFlags & DWFPackageWriter::eManifest)
    {
        if (_oContent.size() > 0)
        {
            rSerializer.startElement( DWFXML::kzElement_Contents, DWFXML::kzNamespace_DWF );

            //
            //  First serialize the primary content
            //
            if (_pPrimaryContent)
            {
                _pPrimaryContent->serializeXML( rSerializer, nFlags );
            }
            else
            {
                _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"The content manager has content, but the primary content was not set" );
            }

            DWFContent::tMap::Iterator* piContents = _oContent.iterator();
            if (piContents)
            {
                for (; piContents->valid(); piContents->next())
                {
                    //
                    //  Let each content serialize its information into the manifest
                    //
                    DWFContent* pContent = piContents->value();
                    if (pContent != _pPrimaryContent)
                    {
                        pContent->serializeXML( rSerializer, nFlags );
                    }
                }

                DWFCORE_FREE_OBJECT( piContents );
            }

            rSerializer.endElement();
        }
    }
}

#endif


