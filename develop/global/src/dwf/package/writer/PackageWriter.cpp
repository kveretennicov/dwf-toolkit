//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
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


#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"
#include "dwfcore/Owner.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Constants.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/SignatureSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/utility/PropertyContainer.h"
#include "dwf/presentation/PackageContentPresentations.h"

using namespace DWFToolkit;




    //
    // used for temporary buffer to transfer bytes
    // from a resource to the dwf archive
    //
#define _DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES  16384

//DNT_Start

_DWFTK_API
DWFPackageWriter::DWFPackageWriter( const DWFFile& rDWFPackageFile )
throw()
                : _rDWFPackage( rDWFPackageFile )
                , _oUUID()
                , _pPackageManifest( NULL )
                , _nNextPlot( 1.0 )
                , _bEPlotGlobalAdded( false )
                , _bEModelGlobalAdded( false )
                , _nEPlotSectionCount( 0 )
                , _nEModelSectionCount( 0 )
                , _oSections()
                , _oGlobalSections()
                , _bAggregating( false )
                , _eMergeType( eNoMerge )
                , _oSectionToContentManager()
                , _oContentToContentManager()
                , _pPackageContentPresentations( NULL )
                , _pUndeclaredResourceProvider( NULL )
                , _pUndeclaredContentTypeProvider( NULL )
                , _pUndeclaredPackageRelationshipsProvider( NULL )
{
    _pPackageContentPresentations = DWFCORE_ALLOC_OBJECT( DWFPackageContentPresentations );
}

_DWFTK_API
DWFPackageWriter::~DWFPackageWriter()
throw()
{
    if (_pPackageManifest)
    {
        DWFCORE_FREE_OBJECT( _pPackageManifest );
    }
}


_DWFTK_API
void
DWFPackageWriter::enablePasswordSalting( bool bEnable )
throw()
{
    _bNoPasswordSalting = !bEnable;
}

_DWFTK_API
void
DWFPackageWriter::addSection( DWFSection*   pSection,
                              DWFInterface* pInterface )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No section provided" );
    }

    if (_bAggregating)
    {
        //
        //  Grab the associated content manager before it gets swapped. 
        //  A later call to provideManifest will change the content manager on the section
        //  to this writer's associated content manager. 
        //
        DWFContentManager* pContentManager = pSection->getContentManager();
        if (pContentManager)
        {
            _oSectionToContentManager.insert( std::make_pair(pSection, pContentManager) );
        }
    }

        //
        // trap global sections
        //
    {
        DWFGlobalSection* pGlobal = dynamic_cast<DWFGlobalSection*>(pSection);
        if (pGlobal)
        {
            return addGlobalSection( pGlobal );
        }
    }

    //
    //
    //
    _ensureManifest();

        //
        // ask the section for it's interface (it may not exist)
        //
    if (pInterface == NULL)
    {
        pInterface = pSection->buildInterface();
    }

        //
        // 
        //
    if (pInterface)
    {
        //
        // add to the manifest
        //
        _pPackageManifest->provideInterface( pInterface );
    }

    //
    // Grab the behavior from the section. In 7.1, this was on CustomSection.
    // In 7.2, we've promoted it to Section in general.
    //
    DWFSection::tBehavior tSectionBehavior = pSection->behavior();

        //
        // modify the section before caching
        //
    if (tSectionBehavior.bRenameOnPublish)
    {
        pSection->rename( _oUUID.next( false ) );
    }

        //
        // manage section descriptor resource
        //
    if (tSectionBehavior.bPublishDescriptor)
    {
        pSection->reorder( _nNextPlot++ );

        //
        // the HREF of this descriptor
        //
        DWFString zHREF( pSection->name() );
        zHREF.append( /*NOXLATE*/L"\\descriptor.xml" );

        //
        // create the descriptor resource
        //
        DWFResource* pDescriptor = DWFCORE_ALLOC_OBJECT( DWFResource(/*NOXLATE*/L"",
                                                            DWFXML::kzRole_Descriptor,
                                                            DWFMIME::kzMIMEType_XML,
                                                            zHREF) );
        if (pDescriptor == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate desriptor resource" );
        }

        //
        // remove any previous descriptor
        // and have it destroyed if the section owns it
        //
        pSection->removeResourcesByRole( DWFXML::kzRole_Descriptor, true );

        //
        // add a descriptor resource to the section
        //
        pSection->addResource( pDescriptor, true );
    }

    //
    // add the section to the manifest
    //
    _pPackageManifest->provideSection( pSection );

    //
    // cache the section
    //
    _oSections.push_back( pSection );

    //
    // increment the section specific count based on the section type
    //
    const DWFString& zSectionType = pSection->type();

    if (zSectionType == _DWF_FORMAT_EPLOT_TYPE_STRING)
    {
        _nEPlotSectionCount++;
    }
    else if (zSectionType == _DWF_FORMAT_EMODEL_TYPE_STRING)
    {
        _nEModelSectionCount++;
    }
}

_DWFTK_API
void
DWFPackageWriter::addGlobalSection( DWFGlobalSection* pSection )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No section provided" );
    }

        //
    //
    //
    _ensureManifest();

        //
        // ensure only one global section per type
        //
    if (pSection->type() == _DWF_FORMAT_EPLOT_GLOBAL_TYPE_STRING)
    {
        if (_bEPlotGlobalAdded)
        {
            DWFString zEx( /*NOXLATE*/L"Global section already exists [" );
                      zEx.append( _DWF_FORMAT_EPLOT_GLOBAL_TYPE_STRING );
                      zEx.append( /*NOXLATE*/L"]" );

            _DWFCORE_THROW( DWFUnexpectedException, zEx );
        }

        _bEPlotGlobalAdded = true;
    }
    else if (pSection->type() == _DWF_FORMAT_EMODEL_GLOBAL_TYPE_STRING)
    {
        if (_bEModelGlobalAdded)
        {
            DWFString zEx( /*NOXLATE*/L"Global section already exists [" );
                      zEx.append( _DWF_FORMAT_EMODEL_GLOBAL_TYPE_STRING );
                      zEx.append( /*NOXLATE*/L"]" );

            _DWFCORE_THROW( DWFUnexpectedException, zEx );
        }

        _bEModelGlobalAdded = true;
    }

    //
    // the HREF of this descriptor
    //
    DWFString zHREF( pSection->name() );
    zHREF.append( /*NOXLATE*/L"\\descriptor.xml" );

    //
    // create the descriptor resource
    //
    DWFResource* pDescriptor = DWFCORE_ALLOC_OBJECT( DWFResource(/*NOXLATE*/L"",
                                                        DWFXML::kzRole_Descriptor,
                                                        DWFMIME::kzMIMEType_XML,
                                                        zHREF) );
    if (pDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate desriptor resource" );
    }

    //
    // remove any previous descriptor
    // and have it destroyed if the section owns it
    //
    pSection->removeResourcesByRole( DWFXML::kzRole_Descriptor, true );

    //
    // add a descriptor resource to the section
    //
    pSection->addResource( pDescriptor, true );

    //
    // add the section to the manifest
    //
    _pPackageManifest->provideSection( pSection );

    //
    // cache the section
    //
    _oGlobalSections.push_back( pSection );
}

_DWFTK_API
DWFInterface*
DWFPackageWriter::addInterface( DWFInterface* pInterface )
throw( DWFException )
{
    if (pInterface == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Interface must be specified" );
    }

    //
    //
    //
    _ensureManifest();

    //
    // add to the manifest
    //
    return _pPackageManifest->provideInterface( pInterface );
}

_DWFTK_API
void
DWFPackageWriter::attachContentManager( DWFContentManager* pContentManager, 
                                        bool bTakesOwnership )
throw( DWFException )
{
    _ensureManifest();

    _pPackageManifest->attachContentManager( pContentManager, bTakesOwnership );
}

_DWFTK_API
DWFContentManager*
DWFPackageWriter::getContentManager()
throw( DWFException )
{
    _ensureManifest();

    DWFContentManager* pContentManager = _pPackageManifest->getContentManager();
    return pContentManager;
}

_DWFTK_API
void DWFPackageWriter::addPresentation( DWFContentPresentation* pPresentation, bool bOwnPresentation )
    throw( DWFException )
{
    _pPackageContentPresentations->addPresentation( pPresentation, bOwnPresentation );
}

_DWFTK_API
void DWFPackageWriter::setUndeclaredResourceProvider( UndeclaredResource::Provider* pProvider )
    throw( DWFException )
{
    _pUndeclaredResourceProvider = pProvider;
}

_DWFTK_API
void DWFPackageWriter::setUndeclaredContentTypeProvider( UndeclaredContentType::Provider* pProvider )
    throw( DWFException )
{
    _pUndeclaredContentTypeProvider = pProvider;
}

_DWFTK_API
void DWFPackageWriter::setUndeclaredPackageRelationshipsProvider( UndeclaredPackageRelationships::Provider* pProvider )
    throw( DWFException )
{
    _pUndeclaredPackageRelationshipsProvider = pProvider;
}


_DWFTK_API
DWFPropertyContainer&
DWFPackageWriter::getManifestProperties()
throw( DWFException )
{
    //
    //
    //
    _ensureManifest();

    return dynamic_cast<DWFPropertyContainer&>(*_pPackageManifest);
}

void
DWFPackageWriter::_ensureManifest()
throw( DWFException )
{
        //
        // create the manifest if necessary
        //
    if (_pPackageManifest == NULL)
    {
        //
        // when the manifest is created it is assigned a new UUID for it's object ID
        // all subsequent object IDs should be acquired via the next() method of the UUID generator
        //
        _pPackageManifest = DWFCORE_ALLOC_OBJECT( DWFManifest(_oUUID.uuid(false)) );
        if (_pPackageManifest == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to create manifest" );
        }
    }
}


_DWFTK_API
void
DWFPackageWriter::initializeForAggregation( teMergeContent eMergeType )
throw( DWFException )
{
    _bAggregating = true;
    _eMergeType = eMergeType;

    if (_oSections.size() > 0)
    {
        DWFContentManager* pContentManager = getContentManager();

        _DWFTK_STD_VECTOR(DWFSection*)::iterator iSection = _oSections.begin();
        for (; iSection != _oSections.end(); ++iSection)
        {
            _oSectionToContentManager.insert( std::make_pair( *iSection, pContentManager ) );
        }
    }
}

void
DWFPackageWriter::_aggregateContent()
throw( DWFException )
{
    //
    //  If we are aggregating pre 6.2 DWFs we don't need any content aggregation
    //  or merge. So lets count the number of associated contents first before we proceed.
    //
    DWFSortedVector<DWFString> oContentIDs( false );
    _tSectionToContentManager::iterator iMap = _oSectionToContentManager.begin();
    for (; iMap != _oSectionToContentManager.end();  ++iMap)
    {
        DWFSection* pSection = iMap->first;

        DWFSection::tStringMultiMap oResourceIDContentID;
        pSection->getAssociatedContentIDs( oResourceIDContentID );

        DWFSection::tStringMultiMap::iterator iResource = oResourceIDContentID.begin();
        for (; iResource != oResourceIDContentID.end(); ++iResource)
        {
            oContentIDs.insert( iResource->second );
        }
    }

    if (oContentIDs.empty())
    {
        return;
    }


    //
    //  Local content manager
    //
    DWFContentManager* pContentManager = getContentManager();

    //
    //  If we aren't merging just get all the contents from the other managers
    //  into this content manager.
    //
    if (_eMergeType == eNoMerge)
    {
        _tSectionToContentManager::iterator iMap = _oSectionToContentManager.begin();
        for (; iMap != _oSectionToContentManager.end(); ++iMap)
        {
            DWFSection* pSection = iMap->first;
            DWFContentManager* pCurrentManager = iMap->second;

            DWFSection::tStringMultiMap oResourceIDContentID;
            pSection->getAssociatedContentIDs( oResourceIDContentID );

            DWFSection::tStringMultiMap::iterator iResource = oResourceIDContentID.begin();
            for (; iResource != oResourceIDContentID.end(); ++iResource)
            {
                //
                //  First do a simple ID check - if a content with the given ID isn't in
                //  the output content manager, then pContent will be NULL. Hence
                //  grab the input content for aggregation
                //
                DWFString zContentID = iResource->second;
                DWFContent* pContent = pContentManager->getContent( zContentID );
                if (pContent == NULL)
                {
                    pContent = pCurrentManager->getContent( zContentID );
                    
                    //
                    //  Save the original owner to re-establish ownership after the write, where necessary
                    //
                    _oContentToContentManager.insert( std::make_pair( pContent, pContent->getContentManager() ) );

                    //
                    //  Now let the writer's content manager have it for the write
                    //
                    pContentManager->insertContent( pContent );
                }
            }
        }

        return;
    }

    //
    //  For merging, set whether the source or the target (primary) content element/property set
    //  has priority if a UUID or property name clashes.
    //
    bool bPrimaryHasPriority = (_eMergeType == eMergePriorityPrimary);

    //
    //  Get the primary content - this maybe empty - this is what everything will merge into
    //
    DWFContent* pPrimaryContent = pContentManager->getContent();
    if(pPrimaryContent == NULL)
    {
        //
        // Add a new, empty content.
        //
        pPrimaryContent = pContentManager->addContent();
    }

    //
    //  Don't want to revisit a content so track them in here
    //
    DWFSortedVector<DWFContent*> oVisitedContent( false );

    _tSectionVector::iterator iSection = _oSections.begin();
    for (; iSection != _oSections.end(); ++iSection)
    {
        DWFSection* pSection = (*iSection);

        //
        //  Skip this if the section has no corresponding content manager
        //
        _tSectionToContentManager::iterator iMap = _oSectionToContentManager.find( pSection );
        if (iMap == _oSectionToContentManager.end() || 
            iMap->second == NULL)
        {
            continue;
        }

        //
        //  This is the content manager to use to load section associated content
        //
        DWFContentManager* pCurrentManager = iMap->second;

        //
        //  The section no longer has the current manager as it's manager so 
        //  pSection->getContentDefinition will NOT work. 
        //
        DWFSection::tStringMultiMap oResourceIDContentID;
        pSection->getAssociatedContentIDs( oResourceIDContentID );

        DWFSection::tStringMultiMap::iterator iResource = oResourceIDContentID.begin();
        for (; iResource != oResourceIDContentID.end(); ++iResource)
        {
            DWFString zContentID = iResource->second;
            DWFContent* pContent = pCurrentManager->getContent( zContentID );

            if (pContent && 
                pContent != pPrimaryContent)
            {
                size_t iFirst, iLast;
                if (!oVisitedContent.find( pContent, iFirst, iLast ))
                {
                    pPrimaryContent->mergeContent( pContent, bPrimaryHasPriority );
                    oVisitedContent.insert( pContent );

                    //
                    //  If the content exists in the writer's content manager - delete the
                    //  content since its been merged in.
                    //
                    if (pCurrentManager == pContentManager)
                    {
                        pCurrentManager->removeContent( zContentID );
                    }
                }

                //
                //  Update the section's mapping of resource object IDs to content IDs. This will also
                //  update the corresponding resources.
                //
                pSection->updateResourceContentMapping( iResource->first, zContentID, pPrimaryContent->id() );
            }
        }
    }
}

void
DWFPackageWriter::_postAggregationCleanup()
throw()
{
    //
    //  Local content manager
    //
    DWFContentManager* pContentManager = getContentManager();

    _tContentToContentManager::iterator iContent = _oContentToContentManager.begin();
    for (; iContent != _oContentToContentManager.end(); ++iContent)
    {
        DWFContentManager* pCurrentManager = iContent->second;
        if (pCurrentManager &&
            pCurrentManager != pContentManager)
        {
            pCurrentManager->insertContent( iContent->first );
        }
    }

    _oSectionToContentManager.clear();
    _oContentToContentManager.clear();
    _bAggregating = false;
}

//DNT_End

#endif
