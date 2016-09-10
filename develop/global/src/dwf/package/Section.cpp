//
//  Copyright (c) 2003-2006 by Autodesk, Inc.  All rights reserved.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Section.cpp#2 $
//  $DateTime: 2011/10/11 01:26:00 $
//  $Author: caos $
//  $Change: 229047 $
//  $Revision: #2 $
//

#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/Pointer.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/Source.h"
#include "dwf/package/Section.h"
#include "dwf/package/Constants.h"
#include "dwf/package/FontResource.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/GraphicResource.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/package/reader/PackageManifestReader.h"
#include "dwf/package/reader/EPlotSectionDescriptorReader.h"
#include "dwf/package/reader/EModelSectionDescriptorReader.h"
#include "dwf/package/reader/GlobalSectionDescriptorReader.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const wchar_t* const DWFSection::kzProperty_Label                  = L"_Label";
const wchar_t* const DWFSection::kzProperty_LabelIconResourceID    = L"_LabelIconResourceID";
const wchar_t* const DWFSection::kzProperty_InitialURI             = L"_InitialURI";
const wchar_t* const DWFSection::kzPropertyCategory_Hidden         = L"hidden";
//DNT_End
#endif

_DWFTK_API
DWFSection::DWFSection( DWFPackageReader* pPackageReader )
throw()
          : _zType()
          , _zName()
          , _zTitle()
          , _pLabelIconResource( NULL )
          , _nVersion( 0.0 )
          , _nPlotOrder( 0.0 )
          , _pPackageReader( pPackageReader )
          , _pContentManager( NULL )
          , _oResourceToContentID()
          , _tBehavior()
          , _bContentPresentationsRead( false )
{}

_DWFTK_API
DWFSection::DWFSection( const DWFString&  zType,
                        const DWFString&  zName,
                        const DWFString&  zTitle,
                        DWFPackageReader* pPackageReader )
throw()
          : _zType( zType )
          , _zName( zName )
          , _zTitle( zTitle )
          , _pLabelIconResource( NULL )
          , _nVersion( 0.0 )
          , _nPlotOrder( -1 )
          , _pPackageReader( pPackageReader )
          , _pContentManager( NULL )
          , _oResourceToContentID()
          , _tBehavior()
          , _bContentPresentationsRead( false )
{}

_DWFTK_API
DWFSection::DWFSection( const DWFString& zType,
                        const DWFString& zTitle,
                        const DWFString& zObjectID,
                        double           nVersion,
                        double           nPlotOrder,
                        const DWFSource& rSource )
throw()
          : _zType( zType )
          , _zTitle( zTitle )
          , _pLabelIconResource( NULL )
          , _zObjectID( zObjectID )
          , _nVersion( nVersion )
          , _nPlotOrder( nPlotOrder )
          , _oSource( rSource )
          , _pPackageReader( NULL )
          , _pContentManager( NULL )
          , _oResourceToContentID()
          , _tBehavior()
          , _bContentPresentationsRead( false )
{}

_DWFTK_API
DWFSection::DWFSection( const DWFSection& rSection )
throw()
          : _zType( rSection._zType )
          , _zName( rSection._zName )
          , _zTitle( rSection._zTitle )
          , _pLabelIconResource( rSection._pLabelIconResource )
          , _zObjectID( rSection._zObjectID )
          , _nVersion( rSection._nVersion )
          , _nPlotOrder( rSection._nPlotOrder )
          , _oSource( rSection._oSource )
          , _pPackageReader( rSection._pPackageReader )
          , _pContentManager( rSection._pContentManager )
          , _oResourceToContentID( rSection._oResourceToContentID )
          , _bContentPresentationsRead( rSection._bContentPresentationsRead )
{
    DWFCORE_COPY_MEMORY( &_tBehavior, &(rSection._tBehavior), sizeof(DWFSection::tBehavior) );
}

_DWFTK_API
DWFSection&
DWFSection::operator=( const DWFSection& rSection )
throw()
{
    _zType = rSection._zType;
    _zName = rSection._zName;
    _zTitle = rSection._zTitle;
    _pLabelIconResource = rSection._pLabelIconResource;
    _zObjectID = rSection._zObjectID;
    _nVersion = rSection._nVersion;
    _nPlotOrder = rSection._nPlotOrder;
    _oSource = rSection._oSource;
    _pPackageReader = rSection._pPackageReader;
    _pContentManager = rSection._pContentManager;
    _oResourceToContentID = rSection._oResourceToContentID;
    _bContentPresentationsRead = rSection._bContentPresentationsRead;

    DWFCORE_COPY_MEMORY( &_tBehavior, &(rSection._tBehavior), sizeof(DWFSection::tBehavior) );

    return *this;
}

_DWFTK_API
DWFSection::~DWFSection()
throw()
{
    //
    // requirement of the DWFOwnable contract
    //
    DWFOwnable::_notifyDelete();
}

_DWFTK_API
void
DWFSection::addSource( const DWFSource& rSource )
throw()
{
    _oSource = rSource;
}

_DWFTK_API
void
DWFSection::rename( const DWFString& zName )
throw()
{
    _zName.assign( _zType );
    _zName.append( /*NOXLATE*/L"_" );
    _zName.append( zName );
}

_DWFTK_API
DWFResource*
DWFSection::addResource( DWFResource*       pResource,
                         bool               bOwnResource,
                         bool               bReplace,
                         bool               bDeleteReplacedIfOwned,
                         const DWFResource* pParentResource )
throw( DWFException )
{
    DWFResource::tStringIterator* piContentID = pResource->contentIDs();
    if (piContentID)
    {
        for(; piContentID->valid(); piContentID->next())
        {
            _oResourceToContentID.insert( std::make_pair(pResource, piContentID->get()) );
        }

        DWFCORE_FREE_OBJECT( piContentID );
    }

    return DWFResourceContainer::addResource( pResource, bOwnResource, bReplace, bDeleteReplacedIfOwned, pParentResource );
}

DWFResource*
DWFSection::_remove( DWFResource* pResource, bool bDeleteIfOwned )
throw()
{
    _oResourceToContentID.erase( pResource );

    return DWFResourceContainer::_remove( pResource, bDeleteIfOwned );
}

_DWFTK_API
const DWFResource&
DWFSection::readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader )
const
throw( DWFException )
{
        //
        // we need a reader at this level
        //
    if (pSectionDescriptorReader == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"A section descriptor reader interface must be provided" );
    }

    //
    // find the one resource with the role of "descriptor"
    //
    DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.find( DWFXML::kzRole_Descriptor );
    if (iResource == _oResourcesByRole.end())
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"This section does not have a descriptor" );
    }

    //
    // acquire the data feed from the resource directly
    // don't cache since hopefully the descriptor is read
    // only once and any useful data is collected by the
    // implementation of the reader
    //
    DWFPointer<DWFInputStream> apDescriptorStream( (*iResource).second->getInputStream(), false );

    //
    // parse the descriptor
    //
    readDescriptor( *pSectionDescriptorReader, *(DWFInputStream*)apDescriptorStream );

    //
    // return the resource
    //
    return *((*iResource).second);
}

_DWFTK_API
void
DWFSection::readDescriptor( DWFSectionDescriptorReader& rSectionDescriptorReader,
                            DWFResource&                rResource ) const
throw( DWFException )
{
    //
    // acquire the data feed from the resource directly
    // don't cache since hopefully the descriptor is read
    // only once and any useful data is collected by the
    // implementation of the reader
    //
    DWFPointer<DWFInputStream> apDescriptorStream( rResource.getInputStream(), false );

    //
    // parse the descriptor
    //
    readDescriptor( rSectionDescriptorReader, *(DWFInputStream*)apDescriptorStream );
}

_DWFTK_API
void
DWFSection::readDescriptor( DWFSectionDescriptorReader& rSectionDescriptorReader,
                            DWFInputStream&             rSectionDescriptorStream ) const
throw( DWFException )
{
    //
    // parse the descriptor
    //
    _parseDocument( rSectionDescriptorStream, rSectionDescriptorReader );
}

_DWFTK_API
DWFContent*
DWFSection::getContentDefinition( DWFResource* pResource, 
                                  DWFContentResourceReader* pReaderFilter,
                                  bool bLoadContent,
								  unsigned int nProviderFlags)
    throw( DWFException )
{
    DWFContent* pContent = NULL;

    if (pResource && 
        (pResource->role()==DWFXML::kzRole_ContentDefinition))
    {
        //
        // Resource with the role of ContentDefinition have only one content ID associated. Find it.
        //
        tResourceStringMultiMap::iterator iResource = _oResourceToContentID.find( pResource );
        if (iResource != _oResourceToContentID.end())
        {
            pContent = _pContentManager->getContent( iResource->second );
            if (pContent)
            {
                if (bLoadContent)
                {
                    pContent->load(NULL, nProviderFlags);
                }
                else if(!pContent->isLoaded())
                {
                    return NULL;
                }

                //
                // Don't want to create a second round of the same instances in the content
                // so check that this resource hasn't been read yet. If it has, the resource
                // ID will have be in the content's map
                //
                if (!pContent->isResourceLoaded( pResource->objectID() ))
                {
                    //
                    //  Need to cast it to the specific kind of reader - i.e. content resource reader
                    //
                    DWFContentResourceReader* pReader = pContent->getContentResourceReader();
                    pReader->setResourceObjectID( pResource->objectID() );

                    if (pReaderFilter)
                    {
                        pReader->setFilter( pReaderFilter );
                    }

                    //
                    // acquire the data feed from the resource directly and don't cache
                    // since hopefully the document is read only once and any useful data
                    // is collected by the implementation of the reader
                    //
                    DWFPointer<DWFInputStream> apObjectStream( pResource->getInputStream(), false );

                    _parseDocument( *(DWFInputStream*)apObjectStream, *pReader );

                    pReader->setResourceObjectID( /*NOXLATE*/L"" );
                }

                return pContent;
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No content found for the content resource." );
            }
        }
        else
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Could not determine the content ID associated with resource." );
        }
    }

    return NULL;
}

_DWFTK_API
DWFContent::tIterator*
DWFSection::getContentDefinition( DWFContentResourceReader* pReaderFilter,
                                  bool bLoadContent,
								  unsigned int nProviderFlags)
throw( DWFException )
{
    DWFContent::tCachingIterator* piContent = NULL;

    DWFContent::tMap oResourceObjectIDContent;
    getContentDefinition( oResourceObjectIDContent, pReaderFilter, bLoadContent, nProviderFlags);

    if (oResourceObjectIDContent.size() > 0)
    {
        DWFContent::tMap::Iterator* piMap = oResourceObjectIDContent.iterator();
        if (piMap)
        {
            piContent = DWFCORE_ALLOC_OBJECT( DWFContent::tCachingIterator );

            for (; piMap->valid(); piMap->next())
            {
                piContent->add( piMap->value() );
            }

            DWFCORE_FREE_OBJECT( piMap );
        }
    }

    return piContent;
}

_DWFTK_API
void
DWFSection::getContentDefinition( DWFContent::tMap& oResourceObjectIDContent,
                                  DWFContentResourceReader* pReaderFilter,
                                  bool bLoadContent,
								  unsigned int nProviderFlags )
throw( DWFException )
{
    //
    //  Find all resources with the role of "content definition"
    //
    DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.begin();
    for (; iResource != _oResourcesByRole.end(); iResource++)
    {
        if (iResource->second->role() == DWFXML::kzRole_ContentDefinition)
        {
            DWFResource* pResource = iResource->second;

            //
            //  Read the content resource
            //
            DWFContent* pContent = getContentDefinition( pResource, pReaderFilter, bLoadContent, nProviderFlags );

            //
            //  If successfully read, store the pointer in the map.
            //
            if (pContent)
            {
                oResourceObjectIDContent.insert( pResource->objectID(), pContent );
            }
        }
    }
}

_DWFTK_API
void
DWFSection::getContentDefinition( DWFContentResourceReader& rReader,
                                  DWFResource* pResource )
    throw( DWFException )
{
    if (pResource)
    {
        rReader.setResourceObjectID( pResource->objectID() );

        //
        // acquire the data feed from the resource directly and don't cache since hopefully the document is 
        // read only once and any useful data is collected by the implementation of the reader
        //
        DWFPointer<DWFInputStream> apObjectStream( pResource->getInputStream(), false );

        //
        // parse the descriptor
        //
        _parseDocument( *(DWFInputStream*)apObjectStream, rReader );

        rReader.setResourceObjectID( /*NOXLATE*/L"" );
    }
    else
    {
        //
        // find all resources with the role of "content definition"
        //
        DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.begin();
        for (; iResource != _oResourcesByRole.end(); iResource++)
        {
            if (iResource->second->role() == DWFXML::kzRole_ContentDefinition)
            {
                DWFResource* pResource = iResource->second;

                if (pResource)
                {
                    rReader.setResourceObjectID( pResource->objectID() );

                    //
                    // acquire the data feed from the resource directly and don't cache since hopefully the document is 
                    // read only once and any useful data is collected by the implementation of the reader
                    //
                    DWFPointer<DWFInputStream> apObjectStream( iResource->second->getInputStream(), false );

                    //
                    // parse the object definition
                    //
                    _parseDocument( *(DWFInputStream*)apObjectStream, rReader );

                    rReader.setResourceObjectID( /*NOXLATE*/L"" );
                }
            }
        }
    }
}

_DWFTK_API
void
DWFSection::getContentDefinition( DWFContentResourceReader& rReader,
                                  DWFInputStream& rContentResourceStream )
throw( DWFException )
{
    _parseDocument( rContentResourceStream, rReader );
}

_DWFTK_API
void
DWFSection::getAssociatedContentIDs( tStringMultiMap& oResourceIDContentIDMap ) const
throw()
{
    tResourceStringMultiMap::const_iterator iResource = _oResourceToContentID.begin();

    for (; iResource != _oResourceToContentID.end(); ++iResource)
    {
        oResourceIDContentIDMap.insert( std::make_pair(iResource->first->objectID(), iResource->second) );
    }
}

_DWFTK_API
void DWFSection::updateResourceContentMapping( const DWFString& zResourceObjectID,
                                               const DWFString& zOldContentID,
                                               const DWFString& zNewContentID )
throw( DWFException )
{
    DWFResource* pResource = findResourceByObjectID( zResourceObjectID );
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"A resource with the given object ID was not found." );
    }

    updateResourceContentMapping( pResource, zOldContentID, zNewContentID );
}

_DWFTK_API
void DWFSection::updateResourceContentMapping( DWFResource* pResource,
                                               const DWFString& zOldContentID,
                                               const DWFString& zNewContentID )
throw( DWFException )
{
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The resource should not be null." );
    }

    //
    // Remove references to the old content ID
    //
    if (zOldContentID.chars() > 0)
    {
        //
        //  Update the resource itself
        //
        pResource->removeContentID( zOldContentID );

        //
        //  Update the mapping that the section stores
        //
        tResourceStringMultiMap::iterator iResource = _oResourceToContentID.find( pResource );
        while (iResource != _oResourceToContentID.end() &&
               iResource->first == pResource)
        {
            if (iResource->second == zOldContentID)
            {
                tResourceStringMultiMap::iterator iThis = iResource;
                ++iResource;
                _oResourceToContentID.erase( iThis );
            }
            else
            {
                ++iResource;
            }
        }
    }

    //
    //  Add references to the new content ID
    //
    if (zNewContentID.chars() > 0)
    {
        //
        //  Update the resource itself. If it already exists then nothing happens
        //
        if (pResource->addContentID( zNewContentID ))
        {
            //
            //  Update the mapping that the section stores
            //
            _oResourceToContentID.insert( std::make_pair(pResource, zNewContentID) );
        }
    }
}

_DWFTK_API
DWFObjectDefinition*
DWFSection::getObjectDefinition(unsigned char nProviderFlags)
const
throw( DWFException )
{
    DWFObjectDefinition* pDefs = DWFCORE_ALLOC_OBJECT( DWFObjectDefinition(_pPackageReader, nProviderFlags) );

    if (pDefs == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate instance tree" );
    }

        //
        // no leaks
        //
    try
    {
        getObjectDefinition( *pDefs, NULL );
        return pDefs;
    }
    catch (...)
    {
        DWFCORE_FREE_OBJECT( pDefs );
        throw;
    }
}

_DWFTK_API
DWFObjectDefinition*
DWFSection::getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionFilter,
                                 DWFResource&               rResource )
const
throw( DWFException )
{
    DWFObjectDefinition* pDefs = DWFCORE_ALLOC_OBJECT( DWFObjectDefinition(_pPackageReader) );

    if (pDefs == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate instance tree" );
    }

    pDefs->setFilter(&rObjectDefinitionFilter);

        //
        // no leaks
        //
    try
    {
        getObjectDefinition( *pDefs, &rResource );
        return pDefs;
    }
    catch (...)
    {
        DWFCORE_FREE_OBJECT( pDefs );
        throw;
    }
}

_DWFTK_API
void
DWFSection::getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionReader,
                                 DWFResource*               pResource )
const
throw( DWFException )
{
    if (pResource)
    {
        //
        // acquire the data feed from the resource directly
        // don't cache since hopefully the document is read
        // only once and any useful data is collected by the
        // implementation of the reader
        //
        DWFPointer<DWFInputStream> apObjectStream( pResource->getInputStream(), false );

        //
        // parse the descriptor
        //
        getObjectDefinition( rObjectDefinitionReader, *(DWFInputStream*)apObjectStream );
    }
    else
    {
        //
        // find all resources with the role of "object definition"
        //
        DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.begin();
        DWFResource::tMultiMap::const_iterator iEnd = _oResourcesByRole.end();

        for (; iResource != iEnd; iResource++)
        {
            const DWFString& zRole = iResource->second->role();
            if (zRole == DWFXML::kzRole_ObjectDefinition
                || zRole == DWFXML::kzRole_MarkupObjectDefinition)
            {
                //
                // acquire the data feed from the resource directly
                // don't cache since hopefully the document is read
                // only once and any useful data is collected by the
                // implementation of the reader
                //
                DWFPointer<DWFInputStream> apObjectStream( iResource->second->getInputStream(), false );

                //
                // parse the object definition
                //
                getObjectDefinition( rObjectDefinitionReader, *(DWFInputStream*)apObjectStream );
            }
        }
    }
}

_DWFTK_API
void
DWFSection::getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionReader,
                                 DWFInputStream&            rObjectDefinitionStream )
const
throw( DWFException )
{
    //
    // parse the object definition
    //
    _parseDocument( rObjectDefinitionStream, rObjectDefinitionReader );
}

_DWFTK_API
void
DWFSection::_parseDocument( DWFInputStream& rDocumentStream,
                            DWFCore::DWFXMLCallback& rDocumentReader )
const
throw( DWFException )
{
	DWFXMLParser docParser(&rDocumentReader);
    docParser.parseDocument( rDocumentStream);
}

_DWFTK_API
void
DWFSection::parseAttributeList( const char** ppAttributeList )
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
        //
        // skip over any "dwf:" in the attribute name
        //
        pAttrib = (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0) ?
                  &ppAttributeList[iAttrib][4] :
                  &ppAttributeList[iAttrib][0];

            //
            // set the name
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x01;

            _zName.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the type
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x02;

            _zType.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the title
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Title) == 0))
        {
            nFound |= 0x04;

            _zTitle.assign( ppAttributeList[iAttrib+1] );
        }
    }
}

_DWFTK_API
void
DWFSection::readContentPresentations()
throw( DWFException )
{
    if (_bContentPresentationsRead)
    {
        return;
    }

    //
    // find all resources with the role of "content presentation"
    // or "markup content presentation"
    //
    DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.begin();
    DWFResource::tMultiMap::const_iterator iEnd = _oResourcesByRole.end();

    for (; iResource != iEnd; iResource++)
    {
        const DWFString& zRole = iResource->second->role();
        if (zRole == DWFXML::kzRole_ContentPresentation ||
            zRole == DWFXML::kzRole_MarkupContentPresentation)
        {
            //
            // acquire the data feed from the resource directly
            // don't cache since hopefully the document is read
            // only once and any useful data is collected by the
            // implementation of the reader
            //
            DWFPointer<DWFInputStream> apObjectStream( iResource->second->getInputStream(), false );

            //
            // parse the content presentation
            //
            _parseDocument( *(DWFInputStream*)apObjectStream, dynamic_cast<DWFContentPresentationResource&>(*iResource->second) );

        }
    }

    _bContentPresentationsRead = true;
}

_DWFTK_API
void
DWFSection::readContentPresentations( DWFContentPresentationReader& rContentPresentationFilter,
                                      DWFResource&                  rResource ) const
throw( DWFException )
{
    DWFContentPresentationResource* pContentPresentationResource = dynamic_cast<DWFContentPresentationResource*>(&rResource);

    if (pContentPresentationResource == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The resource must be a content presentation resource" );
    }

    pContentPresentationResource->setFilter( &rContentPresentationFilter );

    try
    {
        readContentPresentations( *pContentPresentationResource, &rResource );
    }
    catch (...)
    {
        throw;
    }

    /*
    //
    // acquire the data feed from the resource directly
    // don't cache since hopefully the document is read
    // only once and any useful data is collected by the
    // implementation of the reader
    //
    DWFPointer<DWFInputStream> apObjectStream( pContentPresentationResource->getInputStream(), false );

    //
    // parse the content presentation
    //
    _parseDocument( *(DWFInputStream*)apObjectStream, pContentPresentationResource );  
    */
}

_DWFTK_API
void
DWFSection::readContentPresentations( DWFContentPresentationReader& rContentPresentationReader,
                                      DWFResource*                  pResource ) const
throw( DWFException )
{
    if (pResource)
    {
        //
        // acquire the data feed from the resource directly
        // don't cache since hopefully the document is read
        // only once and any useful data is collected by the
        // implementation of the reader
        //
        DWFPointer<DWFInputStream> apObjectStream( pResource->getInputStream(), false );

        //
        // parse the descriptor
        //
        readContentPresentations( rContentPresentationReader, *(DWFInputStream*)apObjectStream );
    }
    else
    {
        //
        // find all resources with the role of "content presentation" or "markup content presentation"
        //
        DWFResource::tMultiMap::const_iterator iResource = _oResourcesByRole.begin();
        DWFResource::tMultiMap::const_iterator iEnd = _oResourcesByRole.end();

        for (; iResource != iEnd; iResource++)
        {
            const DWFString& zRole = iResource->second->role();
            if (zRole == DWFXML::kzRole_ContentPresentation
                || zRole == DWFXML::kzRole_MarkupContentPresentation)
            {
                //
                // acquire the data feed from the resource directly
                // don't cache since hopefully the document is read
                // only once and any useful data is collected by the
                // implementation of the reader
                //
                DWFPointer<DWFInputStream> apObjectStream( iResource->second->getInputStream(), false );

                //
                // parse the object definition
                //
                readContentPresentations( rContentPresentationReader, *(DWFInputStream*)apObjectStream );
            }
        }
    }
}

_DWFTK_API
void
DWFSection::readContentPresentations ( DWFContentPresentationReader& rContentPresentationReader,
                                       DWFInputStream&               rContentPresentationStream ) const
throw( DWFException )
{
    //
    // parse the content presentation
    //
    _parseDocument( rContentPresentationStream, rContentPresentationReader );    
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFSection::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    //
    // ensure that the resource container knows the section name
    // so that our resources have the correct HREFs
    //
    DWFResourceContainer::rename( _zName );

        //
        // detail the section in the manifest
        //
    if (nFlags & DWFPackageWriter::eManifest)
    {
        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Section, DWFXML::kzNamespace_DWF );
        {
                //
                // attributes
                //
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_Type, _zType );
                rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zName );
                rSerializer.addAttribute( DWFXML::kzAttribute_Title, _zTitle );
            }

            //
            // source
            //
            _oSource.serializeXML( rSerializer, nFlags );

                //
                // resources
                //
            if (_tBehavior.bPublishResourcesToManifest)
            {
                DWFResourceContainer::getSerializable().serializeXML( rSerializer, nFlags );
            }
        }
        rSerializer.endElement();
    }
    else if (_tBehavior.bPublishDescriptor && (nFlags & DWFPackageWriter::eDescriptor))
    {
        //
        // add the section label property
        //
        if (_zLabel.chars())
        {
            addProperty( kzProperty_Label, _zLabel, kzPropertyCategory_Hidden );
        }

        //
        // add the section label icon resource id property
        //
        if (_pLabelIconResource)
        {
            const DWFString zObjectID = _pLabelIconResource->objectID();
            if (zObjectID.chars() == 0)
            {
                _pLabelIconResource->setObjectID( rSerializer.nextUUID(true) );
            }
            addProperty( kzProperty_LabelIconResourceID, _pLabelIconResource->objectID(), kzPropertyCategory_Hidden );
        }
        else
        {
            if (_zLabelIconResourceID.chars())
            {
                addProperty( kzProperty_LabelIconResourceID, _zLabelIconResourceID, kzPropertyCategory_Hidden );
            }
        }

        if (_zInitialURI.chars())
        {
            //
            // add the initial URI property
            //
            addProperty( kzProperty_InitialURI, _zInitialURI, kzPropertyCategory_Hidden );
        }

        //
        // properties
        //
        DWFPropertyContainer::getSerializable().serializeXML( rSerializer, nFlags );

        //
        // resources
        //
        DWFResourceContainer::getSerializable().serializeXML( rSerializer, nFlags );
    }
}

#endif


///
///
///

_DWFTK_API
DWFSection::Factory::Factory( const DWFString& zType )
throw()
                   : _zType( zType )
{
    ;
}

_DWFTK_API
DWFSection::Factory::~Factory()
throw()
{
    ;
}

_DWFTK_API
const DWFString&
DWFSection::Factory::type()
const
throw()
{
    return _zType;
}

_DWFTK_API
DWFSection*
DWFSection::Factory::build( const DWFString&     zName,
                            const DWFString&     zTitle,
                            DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFSection* pSection = DWFCORE_ALLOC_OBJECT( DWFSection(_zType, zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}

_DWFTK_API
DWFSection*
DWFSection::Factory::build( const DWFString&     zType,
                            const DWFString&     zName,
                            const DWFString&     zTitle,
                            DWFPackageReader*    pPackageReader )
throw( DWFException )
{
    DWFSection* pSection = DWFCORE_ALLOC_OBJECT( DWFSection(zType, zName, zTitle, pPackageReader) );

    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    return pSection;
}

///
///
///

_DWFTK_API
DWFResource*
DWFSection::buildResource( const char**      ppAttributeList,
                           DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFResource* pResource = DWFXMLElementBuilder::buildResource( ppAttributeList, pPackageReader );

    //
    // see if this resource matches one we already have
    //
    DWFResource* pContainerResource = findResourceByHREF( pResource->href() );

        //
        // remove the old one and have it deleted
        //
    if (pContainerResource)
    {
        removeResource( *pContainerResource, true );
    }

    return pResource;
}

_DWFTK_API
DWFFontResource*
DWFSection::buildFontResource( const char**      ppAttributeList,
                               DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFFontResource* pResource = DWFXMLElementBuilder::buildFontResource( ppAttributeList, pPackageReader );

    //
    // see if this resource matches one we already have
    //
    DWFResource* pContainerResource = findResourceByHREF( pResource->href() );

        //
        // remove the old one and have it deleted
        //
    if (pContainerResource)
    {
        removeResource( *pContainerResource, true );
    }

    return pResource;
}

_DWFTK_API
DWFImageResource*
DWFSection::buildImageResource( const char**      ppAttributeList,
                                DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFImageResource* pResource = DWFXMLElementBuilder::buildImageResource( ppAttributeList, pPackageReader );

    //
    // see if this resource matches one we already have
    //
    DWFResource* pContainerResource = findResourceByHREF( pResource->href() );

        //
        // remove the old one and have it deleted
        //
    if (pContainerResource)
    {
        removeResource( *pContainerResource, true );
    }

    return pResource;
}

_DWFTK_API
DWFGraphicResource*
DWFSection::buildGraphicResource( const char**      ppAttributeList,
                                  DWFPackageReader* pPackageReader )
throw( DWFException )
{
    DWFGraphicResource* pResource = DWFXMLElementBuilder::buildGraphicResource( ppAttributeList, pPackageReader );

    //
    // see if this resource matches one we already have
    //
    DWFResource* pContainerResource = findResourceByHREF( pResource->href() );

        //
        // remove the old one and have it deleted
        //
    if (pContainerResource)
    {
        removeResource( *pContainerResource, true );
    }

    return pResource;
}

_DWFTK_API
DWFSection::tBehavior
DWFSection::behavior() const
throw()
{
    return _tBehavior;
}

_DWFTK_API
void
DWFSection::applyBehavior( const DWFSection::tBehavior& rBehavior )
throw()
{
    DWFCORE_COPY_MEMORY( &_tBehavior, &rBehavior, sizeof(DWFSection::tBehavior) );
}


