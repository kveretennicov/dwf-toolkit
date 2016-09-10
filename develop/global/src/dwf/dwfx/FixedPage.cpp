//
//  Copyright (c) 2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/FixedPage.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//


#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/opc/Constants.h"
#include "dwf/dwfx/FixedPage.h"
#include "dwf/package/Constants.h"
#include "dwf/xps/Constants.h"
#include "dwf/package/EPlotSection.h"
using namespace DWFToolkit;

//DNT_Start

_DWFTK_API
DWFXFixedPage::DWFXFixedPage( DWFSection* pSection )
throw(DWFException)
             : _pSection( pSection )
             , _oRoleToResourceListMap()
             , _oRoleToPartListMap()
             , _oZOrderMap()
             , _pThumbnailResourcePart( NULL )
{
    DWFEPlotSection* pEPlotSection = dynamic_cast<DWFEPlotSection*>(_pSection);
    if (pEPlotSection)
    {
        DWFPaper* pPaper = pEPlotSection->paper();
        double dWidthInInches, dHeightInInches;
        DWFPaper::teUnits ePaperUnits = pPaper->units();
        if (ePaperUnits == DWFPaper::eMillimeters)
        {
            dWidthInInches = pPaper->width() / 25.4;
            dHeightInInches = pPaper->height() / 25.4;
        }
        else if (ePaperUnits == DWFPaper::eInches)
        {
            dWidthInInches = pPaper->width();
            dHeightInInches = pPaper->height();
        }
        else
        {
            _DWFCORE_THROW( DWFUnexpectedException, L"Unexpected paper units" );
        }

        setPageSize( dWidthInInches * 96.0, dHeightInInches * 96.0 );
    }
}

_DWFTK_API
DWFXFixedPage::~DWFXFixedPage()
throw()
{
    _tZOrderMap::const_iterator itOrder = _oZOrderMap.begin();
    for (; itOrder != _oZOrderMap.end(); ++itOrder)
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRoleResourceListMap::iterator itR = (pMapPair->_oResources).begin();
            for (; itR != (pMapPair->_oResources).end(); ++itR)
            {
                DWFCORE_FREE_OBJECT( itR->second );
            }

            _tRolePartListMap::iterator itP = (pMapPair->_oParts).begin();
            for (; itP != (pMapPair->_oParts).end(); ++itP)
            {
                DWFCORE_FREE_OBJECT( itP->second );
            }

            DWFCORE_FREE_OBJECT( pMapPair );
        }
    }

    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
void
DWFXFixedPage::addGraphicResource( DWFGraphicResource* pResource )
throw( DWFException )
{
    if (pResource)
    {
        DWFString zRole( pResource->role() );
        DWFString zMIME( pResource->mime() );
        if (zMIME == DWFMIME::kzMIMEType_FIXEDPAGEXML &&
            (zRole == DWFXML::kzRole_Graphics2d ||
             zRole == DWFXML::kzRole_Graphics2dOverlay ||
             zRole == DWFXML::kzRole_Graphics2dMarkup)
            )
        {
            int nZOrder = pResource->zOrder();

            _tZOrderMap::iterator itPair = _oZOrderMap.find( nZOrder );
            _tRoleMapPair* pMapPair = NULL;
            if (itPair != _oZOrderMap.end())
            {
                pMapPair = itPair->second;
            }
            else
            {
                pMapPair = DWFCORE_ALLOC_OBJECT( _tRoleMapPair );
                _oZOrderMap.insert( std::make_pair(nZOrder, pMapPair) );
            }

            if (pMapPair)
            {
                _tRoleResourceListMap::iterator itList = (pMapPair->_oResources).find( zRole );
                DWFGraphicResource::tList* pList = NULL;
                if (itList != (pMapPair->_oResources).end())
                {
                    pList = itList->second;
                }
                else
                {
                    pList = DWFCORE_ALLOC_OBJECT( DWFGraphicResource::tList );
                    (pMapPair->_oResources).insert( std::make_pair(zRole, pList) );
                }

                if (pList)
                {
                    pList->push_back( pResource );
                }
                else
                {
                    _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for DWFGraphicResource list." );
                }
            }
            else
            {
                _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for RoleMapPair." );
            }
        }
        else
        {
            _DWFCORE_THROW( DWFInvalidArgumentException, L"Either the resource role or mime-type is not compatible with FixedPage." );
        }
    }
    else
    {
        _DWFCORE_THROW( DWFNullPointerException, L"The resource pointer should not be NULL." );
    }
}

_DWFTK_API
void
DWFXFixedPage::removeGraphicResource( DWFGraphicResource* pResource )
throw( DWFException )
{
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"The resource pointer was NULL." );
    }

    bool bFound = false;

    int nZOrder = pResource->zOrder();
    DWFString zRole( pResource->role() );

    _tZOrderMap::iterator itOrder = _oZOrderMap.find( nZOrder );
    if (itOrder != _oZOrderMap.end())
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRoleResourceListMap::iterator itList = (pMapPair->_oResources).find( zRole );
            if (itList != (pMapPair->_oResources).end())
            {
                DWFGraphicResource::tList* pList = itList->second;
                if (pList && 
                    pList->erase( pResource ))
                {
                    bFound = true;

                    //
                    // Cleanup if possible
                    //
                    if (pList->size() == 0)
                    {
                        //
                        //  No more resources with zRole
                        //
                        DWFCORE_FREE_OBJECT( pList );
                        (pMapPair->_oResources).erase( itList );

                        //
                        //  If neither of the maps have anymore resource lists, i.e.,
                        //  no resources with the given zOrder - cleanup
                        //
                        if( (pMapPair->_oResources).size() == 0 &&
                            (pMapPair->_oParts).size() == 0 )
                        {
                            DWFCORE_FREE_OBJECT( pMapPair );
                            _oZOrderMap.erase( itOrder );
                        }
                    }
                }
            }
        }
    }

    if (!bFound)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The resource was not found." );
    }
}

_DWFTK_API
void
DWFXFixedPage::addRasterPart( DWFXResourcePart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, L"The part pointer was NULL." );
    }

    DWFImageResource* pResource = dynamic_cast<DWFImageResource*>(pPart->resource());
    if (pResource)
    {
        DWFString zRole( pResource->role() );
        DWFString zMIME( pResource->mime() );
        if (
            (zRole == DWFXML::kzRole_RasterOverlay ||
             zRole == DWFXML::kzRole_RasterMarkup) &&
            (zMIME == DWFMIME::kzMIMEType_JPG ||
             zMIME == DWFMIME::kzMIMEType_PNG ||
             zMIME == DWFMIME::kzMIMEType_TIFF)
           )
        {
            int nZOrder = pResource->zOrder();

            _tZOrderMap::iterator itPair = _oZOrderMap.find( nZOrder );
            _tRoleMapPair* pMapPair = NULL;
            if (itPair != _oZOrderMap.end())
            {
                pMapPair = itPair->second;
            }
            else
            {
                pMapPair = DWFCORE_ALLOC_OBJECT( _tRoleMapPair );
                _oZOrderMap.insert( std::make_pair(nZOrder, pMapPair) );
            }

            if (pMapPair)
            {
                _tRolePartListMap::iterator itList = (pMapPair->_oParts).find( zRole );
                DWFXResourcePart::tList* pList = NULL;
                if (itList != (pMapPair->_oParts).end())
                {
                    pList = itList->second;
                }
                else
                {
                    pList = DWFCORE_ALLOC_OBJECT( DWFXResourcePart::tList );
                    (pMapPair->_oParts).insert( std::make_pair(zRole, pList) );
                }

                if (pList)
                {
                    pList->push_back( pPart );
                    addRequiredResourcePart( pPart );
                }
                else
                {
                    _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for DWFXResourcePart list." );
                }
            }
            else
            {
                _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for RoleMapPair." );
            }
        }
        else
        {
            _DWFCORE_THROW( DWFInvalidArgumentException, L"Either the resource role or mime-type is not compatible with FixedPage." );
        }
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"The part for rasters does not reference a DWFImageResource." );
    }
}

_DWFTK_API
void
DWFXFixedPage::removeRasterPart( DWFXResourcePart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"The part pointer was NULL." );
    }

    DWFImageResource* pResource = dynamic_cast<DWFImageResource*>(pPart->resource());
    if (pResource == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"The part does not reference an ImageResource." );
    }

    bool bFound = false;

    int nZOrder = pResource->zOrder();
    DWFString zRole( pResource->role() );

    _tZOrderMap::iterator itOrder = _oZOrderMap.find( nZOrder );
    if (itOrder != _oZOrderMap.end())
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRolePartListMap::iterator itList = (pMapPair->_oParts).find( zRole );
            if (itList != (pMapPair->_oParts).end())
            {
                DWFXResourcePart::tList* pList = itList->second;
                if (pList && 
                    pList->erase( pPart ))
                {
                    bFound = true;
                    removeRequiredResourcePart( pPart );

                    //
                    // Cleanup if possible
                    //
                    if (pList->size() == 0)
                    {
                        //
                        //  No more part-referenced resources with zRole
                        //
                        DWFCORE_FREE_OBJECT( pList );
                        (pMapPair->_oParts).erase( itList );

                        //
                        //  If neither of the maps have anymore resource lists, i.e.,
                        //  no resources with the given zOrder - cleanup
                        //
                        if( (pMapPair->_oResources).size() == 0 &&
                            (pMapPair->_oParts).size() == 0 )
                        {
                            DWFCORE_FREE_OBJECT( pMapPair );
                            _oZOrderMap.erase( itOrder );
                        }
                    }
                }
            }
        }
    }

    if (!bFound)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The part or it's resource was not found." );
    }

    if (pResource)
    {
        DWFString zRole( pResource->role() );
        _tRolePartListMap::iterator itList = _oRoleToPartListMap.find( zRole );
        if (itList != _oRoleToPartListMap.end())
        {
            DWFXResourcePart::tList* pList = itList->second;
            if (pList->erase( pPart ))
            {
                removeRequiredResourcePart( pPart );
                //
                //  Cleanup if the list is empty
                //
                if (pList->size() == 0)
                {
                    DWFCORE_FREE_OBJECT( pList );
                    _oRoleToPartListMap.erase( itList );
                }
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, L"The resource part was not found." );
            }
        }
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A valid resource was not associated with the provided part." );
    }
}

_DWFTK_API
void
DWFXFixedPage::addRequiredResourcePart( DWFXResourcePart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A valid part must be provided" );
    }

    addRelationship( pPart, XPSXML::kzRelationship_RequiredResource );
}

_DWFTK_API
void
DWFXFixedPage::removeRequiredResourcePart( DWFXResourcePart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A valid part must be provided." );
    }

    //
    // delete the relationship to this part
    //
    deleteRelationshipsByTarget( pPart );
}

_DWFTK_API
void
DWFXFixedPage::setThumbnailResourcePart( DWFXResourcePart* pPart )
throw( DWFException )
{
    if (pPart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A valid part must be provided" );
    }

    //
    // check the resource's role and make sure its thumbnail or preview
    //
    DWFResource* pResource = pPart->resource();
    if (!(pResource->role() == DWFXML::kzRole_Thumbnail ||
          pResource->role() == DWFXML::kzRole_Preview))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A part containing a thumbnail or preview resource was expected" );
    }

    //
    // check the resource's mime and make sure its JPEG or PNG
    //
    if (!(pResource->mime() == DWFMIME::kzMIMEType_JPG ||
          pResource->mime() == DWFMIME::kzMIMEType_PNG))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"A part containing a resource with JPEG or PNG MIME type was expected" );
    }
    if (_pThumbnailResourcePart != NULL)
    {
        removeThumbnailResourcePart();
    }

    _pThumbnailResourcePart = pPart;

    addRelationship( _pThumbnailResourcePart, OPCXML::kzRelationship_Thumbnail );
}

_DWFTK_API
void
DWFXFixedPage::removeThumbnailResourcePart()
throw( DWFException )
{
    if (_pThumbnailResourcePart == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"No thumbnail has been set" );
    }

    //
    // delete the relationship to this part
    //
    deleteRelationshipsByTarget( _pThumbnailResourcePart );

    _pThumbnailResourcePart = NULL;
}

_DWFTK_API
void
DWFXFixedPage::updateImplicitRelationships()
throw()
{

}

_DWFTK_API
DWFGraphicResource::tIterator*
DWFXFixedPage::graphicResources() const
throw()
{
    DWFGraphicResource::tCachingIterator* piIter( DWFCORE_ALLOC_OBJECT( DWFGraphicResource::tCachingIterator ) );
    
    _tZOrderMap::const_iterator itOrder = _oZOrderMap.begin();
    for (; itOrder != _oZOrderMap.end(); ++itOrder)
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRoleResourceListMap::const_iterator itR = (pMapPair->_oResources).begin();
            for (; itR != (pMapPair->_oResources).end(); ++itR)
            {
                DWFGraphicResource::tList* pList = itR->second;
                if (pList)
                {
                    DWFPointer<DWFGraphicResource::tList::Iterator> piResources( pList->iterator(), false );
                    if (!piResources.isNull())
                    {
                        for (; piResources->valid(); piResources->next())
                        {
                            DWFGraphicResource* pResource = piResources->get();
                            if (pResource)
                            {
                                piIter->add( pResource );
                            }
                        }
                    }
                }
            }
        }
    }

    return piIter;
}

_DWFTK_API
DWFXResourcePart::tIterator*
DWFXFixedPage::rasterParts() const
throw()
{
    DWFXResourcePart::tCachingIterator* piIter = DWFCORE_ALLOC_OBJECT( DWFXResourcePart::tCachingIterator );
    
    _tZOrderMap::const_iterator itOrder = _oZOrderMap.begin();
    for (; itOrder != _oZOrderMap.end(); ++itOrder)
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRolePartListMap::const_iterator itP = (pMapPair->_oParts).begin();
            for (; itP != (pMapPair->_oParts).end(); ++itP)
            {
                DWFXResourcePart::tList* pList = itP->second;
                if (pList)
                {
                    DWFPointer<DWFXResourcePart::tList::Iterator> piParts( pList->iterator(), false );
                    if (!piParts.isNull())
                    {
                        for (; piParts->valid(); piParts->next())
                        {
                            DWFXResourcePart* pPart = piParts->get();
                            if (pPart)
                            {
                                piIter->add( pPart );
                            }
                        }
                    }
                }
            }
        }
    }

    return piIter;
}


_DWFTK_API
DWFGraphicResource::tIterator*
DWFXFixedPage::allResources() const
throw()
{
    DWFGraphicResource::tCachingIterator* piIter = DWFCORE_ALLOC_OBJECT( DWFGraphicResource::tCachingIterator );
    
    _tZOrderMap::const_iterator itOrder = _oZOrderMap.begin();
    for (; itOrder != _oZOrderMap.end(); ++itOrder)
    {
        _tRoleMapPair* pMapPair = itOrder->second;
        if (pMapPair)
        {
            _tRoleResourceListMap::const_iterator itR = (pMapPair->_oResources).begin();
            for (; itR != (pMapPair->_oResources).end(); ++itR)
            {
                DWFGraphicResource::tList* pList = itR->second;
                if (pList)
                {
                    DWFPointer<DWFGraphicResource::tList::Iterator> piResources( pList->iterator(), false );
                    if (!piResources.isNull())
                    {
                        for (; piResources->valid(); piResources->next())
                        {
                            DWFGraphicResource* pResource = piResources->get();
                            if (pResource)
                            {
                                piIter->add( pResource );
                            }
                        }
                    }
                }
            }

            _tRolePartListMap::const_iterator itP = (pMapPair->_oParts).begin();
            for (; itP != (pMapPair->_oParts).end(); ++itP)
            {
                DWFXResourcePart::tList* pList = itP->second;
                if (pList)
                {
                    DWFPointer<DWFXResourcePart::tList::Iterator> piParts( pList->iterator(), false );
                    if (!piParts.isNull())
                    {
                        for (; piParts->valid(); piParts->next())
                        {
                            DWFXResourcePart* pPart = piParts->get();
                            if (pPart)
                            {
                                DWFGraphicResource* pResource = dynamic_cast<DWFGraphicResource*>(pPart->resource());
                                if (pResource)
                                {
                                    piIter->add( pResource );
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return piIter;
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXFixedPage::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    rSerializer.startElement( XPSXML::kzElement_FixedPage );
    rSerializer.addAttribute( L"xmlns", XPSXML::kzNamespaceURI_XPS );
    rSerializer.addAttribute( XPSXML::kzAttribute_Height, height() );
    rSerializer.addAttribute( XPSXML::kzAttribute_Width, width() );
    rSerializer.addAttribute( XPSXML::kzAttribute_Language, L"und" );

    //
    // Fill the paper background with the specified paper color.
    //
    DWFEPlotSection* pEPlotSection = dynamic_cast<DWFEPlotSection*>(_pSection);
    if (pEPlotSection != NULL)
    {
        DWFPaper* pPaper = pEPlotSection->paper();
        unsigned int nRGB = pPaper->color();

            //
            // Don't bother if the color is white.
            //
        if(nRGB != 0xffffff)
        {
            // Mask out any alpha value.
            nRGB &= 0x00ffffff; 

            char buf[64];
            sprintf(buf, "#%6.6x", nRGB);

            rSerializer.startElement( XPSXML::kzElement_Path );
            rSerializer.addAttribute( XPSXML::kzAttribute_Fill, buf );

            //
            //  Build up the geometry - that is - just grab the whole page.
            //
            size_t nBufferSize = 32;
            DWFPointer<wchar_t> zPageWidth( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );            
			int nWidthBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zPageWidth, nBufferSize, L"%.17g", width() );
			DWFString::RepairDecimalSeparators( zPageWidth );
            
            DWFPointer<wchar_t> zPageHeight( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
			int nHeightBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zPageHeight, nBufferSize, L"%.17g", height() );
			DWFString::RepairDecimalSeparators( zPageHeight );

            DWFString zTmpString( L"M 0, 0 L " );
            zTmpString.append( zPageWidth, nWidthBytes );
            zTmpString.append( L",0 L" );
            zTmpString.append( zPageWidth, nWidthBytes );
            zTmpString.append( L"," );
            zTmpString.append( zPageHeight, nHeightBytes );
            zTmpString.append( L" L 0," );
            zTmpString.append( zPageHeight, nHeightBytes );
            zTmpString.append( L" z" );
            rSerializer.addAttribute( XPSXML::kzAttribute_Data, zTmpString );

            rSerializer.endElement();
        }
    }


    _tZOrderMap::iterator itOrder = _oZOrderMap.begin();
    for (; itOrder != _oZOrderMap.end(); ++itOrder)
    {
        _tRoleMapPair* pMapPair = itOrder->second;

        if (pMapPair)
        {
            //
            //  We have access to all resources corresponding to a zOrder.
            //  Serialize them according to the role based default order.
            //
            _serializeGraphicResources( pMapPair->_oResources, DWFXML::kzRole_Graphics2d, rSerializer );
            _serializeResourceParts( pMapPair->_oParts, DWFXML::kzRole_RasterOverlay, rSerializer );
            _serializeGraphicResources( pMapPair->_oResources, DWFXML::kzRole_Graphics2dOverlay, rSerializer );
            _serializeResourceParts( pMapPair->_oParts, DWFXML::kzRole_RasterMarkup, rSerializer );
            _serializeGraphicResources( pMapPair->_oResources, DWFXML::kzRole_Graphics2dMarkup, rSerializer );
        }
    }

    //
    //  End the FixedPage document element
    //
    rSerializer.endElement();
}

void
DWFXFixedPage::_serializeGraphicResources( _tRoleResourceListMap& rResources,
                                           const DWFString& zRole, 
                                           DWFXMLSerializer& rSerializer )
throw()
{
    _tRoleResourceListMap::iterator iter = rResources.find( zRole );
    if (iter != rResources.end())
    {
        DWFGraphicResource::tList* pList = iter->second;
        if (pList)
        {
            DWFPointer<DWFGraphicResource::tList::Iterator> piResources( pList->iterator(), false );
            if (!piResources.isNull())
            {
                for (; piResources->valid(); piResources->next())
                {
                    DWFGraphicResource* pResource = piResources->get();
                    if (pResource)
                    {
                        _serializeToCanvas( pResource, rSerializer );
                    }
                }
            }
        }
    }
}

void
DWFXFixedPage::_serializeResourceParts( _tRolePartListMap& rParts,
                                        const DWFString& zRole, 
                                        DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    _tRolePartListMap::iterator iter = rParts.find( zRole );
    if (iter != rParts.end())
    {
        DWFXResourcePart::tList* pList = iter->second;
        if (pList)
        {
            DWFPointer<DWFXResourcePart::tList::Iterator> piParts( pList->iterator(), false );
            if (!piParts.isNull())
            {
                for (; piParts->valid(); piParts->next())
                {
                    DWFXResourcePart* pPart = piParts->get();
                    if (pPart)
                    {
                        _serializeToImageBrush( pPart, rSerializer );
                    }
                }
            }
        }
    }
}

void
DWFXFixedPage::_serializeToCanvas( DWFGraphicResource* pResource, 
                                   DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    DWFPointer<DWFInputStream> apResourceStream( pResource->getInputStream(), false );
    if (apResourceStream.isNull())
    {
        _DWFCORE_THROW( DWFNullPointerException, L"The resource did not return a valid input stream" );
    }
    else
    {
        double adXPSTransform[6];
        _getGraphicsTransform( pResource, adXPSTransform );

        rSerializer.startElement( XPSXML::kzElement_Canvas );
        rSerializer.addAttribute( XPSXML::kzAttribute_Name, pResource->internalID() );

        size_t nBufferSize = 32;
        DWFPointer<wchar_t> zValue( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
        //
        //  Build up the string for the RenderTransform
        //
        size_t nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adXPSTransform[0] );
		DWFString::RepairDecimalSeparators( zValue );
        DWFString zTmpString( zValue, nBytes );
        for (int i = 1; i<6; i++)
        {
            nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adXPSTransform[i] );
			DWFString::RepairDecimalSeparators( zValue );
            zTmpString.append( L"," );
            zTmpString.append( zValue, nBytes );
        }
        rSerializer.addAttribute( XPSXML::kzAttribute_RenderTransform, zTmpString );

        rSerializer.insertXMLStream( apResourceStream );
        rSerializer.endElement();
    }
}

void
DWFXFixedPage::_serializeToImageBrush( DWFXResourcePart* pPart, 
                                       DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    DWFImageResource* pImage = dynamic_cast<DWFImageResource*>(pPart->resource());
    if (pImage == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"Expected a DWFImageResource in the ResourcePart for the image brush." );
    }

    double adViewbox[4];
    double adViewport[4];
    double adXPSTransform[6];
    _getImagePositionInfo( pImage, adViewbox, adViewport, adXPSTransform );

	rSerializer.startElement( XPSXML::kzElement_Canvas );
    rSerializer.addAttribute( XPSXML::kzAttribute_Name, pImage->internalID() );
    rSerializer.startElement( XPSXML::kzElement_Path );

    size_t nBufferSize = 32;
    //
    //  Build up the geometry - that is - just grab the whole page.
    //
    DWFPointer<wchar_t> zPageWidth( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
    int nWidthBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zPageWidth, nBufferSize, L"%.17g", width() );
	DWFString::RepairDecimalSeparators( zPageWidth );

    DWFPointer<wchar_t> zPageHeight( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );
    int nHeightBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zPageHeight, nBufferSize, L"%.17g", height() );
	DWFString::RepairDecimalSeparators( zPageHeight );

	DWFString zTmpString( L"M 0, 0 L " );
    zTmpString.append( zPageWidth, nWidthBytes );
    zTmpString.append( L",0 L" );
    zTmpString.append( zPageWidth, nWidthBytes );
    zTmpString.append( L"," );
    zTmpString.append( zPageHeight, nHeightBytes );
    zTmpString.append( L" L 0," );
    zTmpString.append( zPageHeight, nHeightBytes );
    zTmpString.append( L" z" );
    rSerializer.addAttribute( XPSXML::kzAttribute_Data, zTmpString );

    DWFPointer<wchar_t> zValue( DWFCORE_ALLOC_MEMORY(wchar_t, nBufferSize), true );

    //
    //  Build up the string for the RenderTransform
    //
    size_t nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adXPSTransform[0] );
	DWFString::RepairDecimalSeparators( zValue );
    zTmpString.assign( zValue, nBytes );
    for (int i = 1; i<6; i++)
    {
        nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adXPSTransform[i] );
		DWFString::RepairDecimalSeparators( zValue );
        zTmpString.append( L"," );
        zTmpString.append( zValue, nBytes );
    }
    rSerializer.addAttribute( XPSXML::kzAttribute_RenderTransform, zTmpString );

    //
    //  Now insert the image as a brush
    //
    {
        rSerializer.startElement( XPSXML::kzElement_Path_Fill );            
        {
            rSerializer.startElement( XPSXML::kzElement_ImageBrush );

            rSerializer.addAttribute( XPSXML::kzAttribute_ImageSource, pPart->uri() );


            //
            //  Build up the string for the Viewbox
            //
            nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adViewbox[0] );
			DWFString::RepairDecimalSeparators( zValue );
            zTmpString.assign( zValue, nBytes );
            for (int i = 1; i<4; i++)
            {
                nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adViewbox[i] );
				DWFString::RepairDecimalSeparators( zValue );
                zTmpString.append( L"," );
                zTmpString.append( zValue, nBytes );
            }
            rSerializer.addAttribute( XPSXML::kzAttribute_Viewbox, zTmpString );

            //
            //  Build up the string for the Viewport
            //
            nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adViewbox[0] );
			DWFString::RepairDecimalSeparators( zValue );
            zTmpString.assign( zValue, nBytes );
            for (int i = 1; i<4; i++)
            {
                nBytes = sizeof(wchar_t) * _DWFCORE_SWPRINTF( zValue, nBufferSize, L"%.17g", adViewport[i] );
				DWFString::RepairDecimalSeparators( zValue );
                zTmpString.append( L"," );
                zTmpString.append( zValue, nBytes );
            }
            rSerializer.addAttribute( XPSXML::kzAttribute_Viewport, zTmpString );

            rSerializer.addAttribute( XPSXML::kzAttribute_ViewboxUnits, L"Absolute" );
            rSerializer.addAttribute( XPSXML::kzAttribute_ViewportUnits, L"Absolute" );

            rSerializer.endElement();
        }
        rSerializer.endElement();
    }
    rSerializer.endElement();
	rSerializer.endElement();

}


void
DWFXFixedPage::_getImagePositionInfo( DWFImageResource* pImage,
                                      double* adViewbox,
                                      double* adViewport,
                                      double* adXPSTransform )
throw( DWFException )
{
    DWFEPlotSection* pEPlotSection = dynamic_cast<DWFEPlotSection*>(_pSection);
    if (pEPlotSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidTypeException, L"Imagebrush position info can only be calculated for EPlotSections." );
    }

    const double* adOrigExtents = pImage->originalExtents();
    if (adOrigExtents == NULL )
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"The image resource must provide original extents for the FixedPage serialization." );
    }
    
    int i = 0;
    double dDPI = pImage->scannedResolution();
    if (dDPI == 0)
    {
        dDPI = 96;
    }

    for (; i<4; i++)
    {
        if (dDPI)
        {
            adViewbox[i] = adOrigExtents[i] / dDPI * 96;
        }
        else
        {
            adViewbox[i] = adOrigExtents[i];
        }
    }

    adViewport[0] = adViewport[1] = 0.;
    adViewport[2] = adViewbox[2]-adViewbox[0];
    adViewport[3] = adViewbox[3]-adViewbox[1];

    //
    //  If the paper is in inches the conversion factor will be 96.0
    //
    double dImageToPaper = dDPI;
    DWFPaper* pPaper = pEPlotSection->paper();
    if (pPaper->units() == DWFPaper::eMillimeters)
    {
        dImageToPaper /= 25.4;
    }

    //
    //  This returns
    //   -                                      -
    //  |   Sx*Cos(theta),  Sy*Sin(theta), 0, 0  | 
    //  |  -Sx*Sin(theta),  Sy*Cos(theta), 0, 0  |
    //  |        0       ,       0       , 0, 0  |
    //  |       Tx       ,      Ty       , 0, 1  |
    //   -                                      -
    //  Sx,Sy = scaling in X,Y respectively
    //  theta = counter-clockwise rotation
    //  Tx,Ty = translation in X,Y respectively
    //  Note: DWF Paper origin is at the bottom left
    //
    const double* adTransform = pImage->transform();

    //
    //  XPS tales the following for it's RenderTransform
    //  ( M11, M12, M21, M22, OffsetX, OffsetY )
    //  Note: XPS Paper origin is at the top left
    //

    //
    //  M11 = Sx*Cos(theta)*scale_conversion
    //
    adXPSTransform[0] = adTransform[0]*dImageToPaper;
    //
    //  M12 = -Sy*Sin(theta)*scale_conversion
    //
    adXPSTransform[1] = -adTransform[1]*dImageToPaper;
    //
    //  M21 = Sx*Sin(theta)*scale_conversion
    //
    adXPSTransform[2] = -adTransform[4]*dImageToPaper;
    //
    //  M22 = Sy*Cos(theta)*scale_conversion
    //
    adXPSTransform[3] = adTransform[5]*dImageToPaper;
    //
    //  OffsetX = Tx*scale_conversion - image_height*Sy*Sin(theta)*scale_conversion
    //          = Tx*scale_conversion + image_height*M12
    //  [The second term accounts for the shift due to the fact that the image
    //   gets rotated about the origin in both cases, but the image origin is
    //   at the bottom-left in DWF, and top-left in XPS.]
    //
    double dPaperToXPS = 96;
    if (pPaper->units() == DWFPaper::eMillimeters)
    {
        dPaperToXPS /= 25.4;
    }

    adXPSTransform[4] = adTransform[12]*dPaperToXPS + (adOrigExtents[3]/dImageToPaper*dPaperToXPS)*adXPSTransform[1];
    //
    //  OffsetY = (XPS_PageHeight - image_height*Sy*Cos(theta)*scale_conversion) - Ty*scale_conversion
    //          = (XPS_PageHeight - image_height*M22) - Ty*scale_conversion
    //
    adXPSTransform[5] = (pPaper->height() - adOrigExtents[3]*adTransform[5] - adTransform[13]) * dPaperToXPS;
}

void
DWFXFixedPage::_getGraphicsTransform( DWFGraphicResource* pResource,
                                      double* adXPSTransform )
throw( DWFException )
{
    DWFEPlotSection* pEPlotSection = dynamic_cast<DWFEPlotSection*>(_pSection);
    if (pEPlotSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidTypeException, L"Logical-to-paper transform can only be calculated for EPlotSections." );
    }

    //
    //  If the paper is in inches the conversion factor will be 96.0
    //
    double dConversion = 96.0;
    DWFPaper* pPaper = pEPlotSection->paper();
    if (pPaper->units() == DWFPaper::eMillimeters)
    {
        dConversion /= 25.4;
    }

    //
    //  This returns
    //   -                                      -
    //  |   Sx*Cos(theta),  Sy*Sin(theta), 0, 0  | 
    //  |  -Sx*Sin(theta),  Sy*Cos(theta), 0, 0  |
    //  |        0       ,       0       , 0, 0  |
    //  |       Tx       ,      Ty       , 0, 1  |
    //   -                                      -
    //  Sx,Sy = scaling in X,Y respectively
    //  theta = counter-clockwise rotation
    //  Tx,Ty = translation in X,Y respectively
    //  Note: DWF Paper origin is at the bottom left
    //
    const double* adTransform = pResource->transform();

    //
    //  XPS tales the following for it's RenderTransform
    //  ( M11, M12, M21, M22, OffsetX, OffsetY )
    //  Note: XPS Paper origin is at the top left
    //

    //
    //  M11 = Sx*Cos(theta)*scale_conversion
    //
    adXPSTransform[0] = adTransform[0]*dConversion;
    //
    //  M12 = -Sy*Sin(theta)*scale_conversion
    //
    adXPSTransform[1] = -adTransform[1]*dConversion;
    //
    //  M21 = Sx*Sin(theta)*scale_conversion
    //
    adXPSTransform[2] = -adTransform[4]*dConversion;
    //
    //  M22 = Sy*Cos(theta)*scale_conversion
    //
    adXPSTransform[3] = adTransform[5]*dConversion;
    //
    //  OffsetX = Tx*scale_conversion
    //
    adXPSTransform[4] = adTransform[12] /* *dConversion*/;
    //
    // With flip, it would be:
    //  OffsetY = XPS_PageHeight - Ty*scale_conversion
    // with no flip, it's just:
    //  OffsetY = Ty*scale_conversion
    //
    adXPSTransform[5] = adTransform[13] /* *dConversion*/;

    double x = adTransform[0] ;
    double y = adTransform[1] ;
    double XPSPaperHeight = pPaper->height()*dConversion;
    double XPSPaperWidth = pPaper->width()*dConversion;
    if ( y >= fabs( x ) ) 
    {
        //90.0f;
        adXPSTransform[5] = -adXPSTransform[5] + XPSPaperHeight;
    }
    else if ( -x >= fabs(y) )
    {
        //180.0f;
        adXPSTransform[4] = -adXPSTransform[4] + XPSPaperWidth;
        adXPSTransform[5] = adXPSTransform[5] + XPSPaperHeight;
    }
    else if ( -y >= fabs(x) )
    {
        //270.0f;
        adXPSTransform[4] = adXPSTransform[4] + XPSPaperWidth;
    }
}

#endif

//DNT_End

