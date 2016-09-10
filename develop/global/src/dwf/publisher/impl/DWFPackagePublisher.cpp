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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/DWFPackagePublisher.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"

#include "dwf/package/Constants.h"
#include "dwf/package/DataSection.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/CustomSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/publisher/impl/DWFPackagePublisher.h"
#include "dwf/publisher/impl/PublishedDefinedObject.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
#include "dwf/publisher/impl/DefinedObjectPropertyVisitor.h"
#include "dwf/publisher/impl/ContentElementPropertyVisitor.h"
#include "dwf/publisher/impl/PublishedContentElementListener.h"
#include "dwf/presentation/ContentPresentationResource.h"
#include "dwf/presentation/ContentPresentationReferenceNode.h"

using namespace DWFToolkit;

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

//DNT_Start
const wchar_t* const DWFPackagePublisher::kz_PropName_PolygonHandedness =    L"_PolygonHandedness";
const wchar_t* const DWFPackagePublisher::kz_PropName_UseDefaultLighting =   L"_UseDefaultLighting";
const wchar_t* const DWFPackagePublisher::kz_PropName_UseSilhouetteEdges =   L"_UseSilhouetteEdges";
const wchar_t* const DWFPackagePublisher::kz_PropName_EdgeColor =            L"_EdgeColor";
const wchar_t* const DWFPackagePublisher::kz_PropName_DisplayMode =          L"_DisplayMode";

const wchar_t* const DWFPackagePublisher::kz_NamedView_Default =             L"Initial";
const wchar_t* const DWFPackagePublisher::kz_PropSetName_DefaultView =       L"_DefaultView";
const wchar_t* const DWFPackagePublisher::kz_PropName_CameraProjection =     L"_CameraProjection";
const wchar_t* const DWFPackagePublisher::kz_PropName_CameraPosition =       L"_CameraPosition";
const wchar_t* const DWFPackagePublisher::kz_PropName_CameraTarget =         L"_CameraTarget";
const wchar_t* const DWFPackagePublisher::kz_PropName_CameraUpVector =       L"_CameraUpVector";
const wchar_t* const DWFPackagePublisher::kz_PropName_CameraField =          L"_CameraField";

const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeShowCompass =            L"_ViewCubeShowCompass";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeAngleOfNorth =           L"_ViewCubeAngleOfNorth";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeHomeCameraProjection =   L"_ViewCubeHomeCameraProjection";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeHomeCameraPosition =     L"_ViewCubeHomeCameraPosition";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeHomeCameraTarget =       L"_ViewCubeHomeCameraTarget";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeHomeCameraUpVector =     L"_ViewCubeHomeCameraUpVector";
const wchar_t* const DWFPackagePublisher::kz_PropName_ViewCubeHomeCameraField =        L"_ViewCubeHomeCameraField";
const wchar_t* const DWFPackagePublisher::kz_PropName_FrontView               =		   L"_FrontView";

const wchar_t* const DWFPackagePublisher::kz_PropName_FrontViewCameraProjection =   L"_FrontViewCameraProjection";
const wchar_t* const DWFPackagePublisher::kz_PropName_FrontViewCameraPosition =     L"_FrontViewCameraPosition";
const wchar_t* const DWFPackagePublisher::kz_PropName_FrontViewCameraTarget =       L"_FrontViewCameraTarget";
const wchar_t* const DWFPackagePublisher::kz_PropName_FrontViewCameraUpVector =     L"_FrontViewCameraUpVector";
const wchar_t* const DWFPackagePublisher::kz_PropName_FrontViewCameraField =        L"_FrontViewCameraField";
//DNT_End                                 

#endif


//DNT_Start

_DWFTK_API
DWFPackagePublisher::DWFPackagePublisher( const DWFString& zSourceProductVendor,
                                          const DWFString& zSourceProductName,
                                          const DWFString& zSourceProductVersion,
                                          const DWFString& zPublisherProductVendor,
                                          const DWFString& zPublisherProductVersion )
throw( DWFException )
                   : DWFEmbeddedFontVisitor( NULL )
				   , _pCurrentSection( NULL )
				   , _pCurrentGraphicResource( NULL )
                   , _nSections( 0 )
                   , _zSourceProductVendor( zSourceProductVendor )
                   , _zSourceProductName( zSourceProductName )
                   , _zSourceProductVersion( zSourceProductVersion )
                   , _zPublisherProductVendor( zPublisherProductVendor )
                   , _zPublisherProductVersion( zPublisherProductVersion )
                   , _oEmbeddedFonts()
                   , _pResourceVisitor( NULL )
                   , _pPublishedObjectVisitor( NULL )
                   , _pPropertyVisitor( NULL )
                   , _pPublishedElementListener( NULL )
                   , _eMetaDataVersion( DWFPublisher::ePublishContentDefinition )
                   , _bCreateContentNavigationPresentation( true )
{
    setVersion( _eMetaDataVersion );
}

_DWFTK_API
DWFPackagePublisher::~DWFPackagePublisher()
throw()
{
    if (_pPublishedObjectVisitor)
    {
        DWFCORE_FREE_OBJECT( _pPublishedObjectVisitor );
    }
    if (_pPropertyVisitor)
    {
        DWFCORE_FREE_OBJECT( _pPropertyVisitor );
    }
    if (_pPublishedElementListener)
    {
        DWFCORE_FREE_OBJECT( _pPublishedElementListener );
    }
}

_DWFTK_API
void
DWFPackagePublisher::setVersion( DWFPublisher::teMetadataVersion eVersion )
    throw( DWFException )
{
    if (eVersion != DWFPublisher::ePublishContentDefinition &&
        eVersion != DWFPublisher::ePublishObjectDefinition)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The provided metadata version is not recognized by the package publisher" );
    }

    if (eVersion == _eMetaDataVersion)
    {
        if (_pPublishedObjectVisitor == NULL)
        {
            if (eVersion == DWFPublisher::ePublishContentDefinition)
            {
                _pPublishedObjectVisitor = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement::Visitor );
            }
            else if (eVersion == DWFPublisher::ePublishObjectDefinition)
            {
                _pPublishedObjectVisitor = DWFCORE_ALLOC_OBJECT( DWFPublishedDefinedObject::Visitor );
            }

        }

        if (_pPropertyVisitor == NULL)
        {
            if (eVersion == DWFPublisher::ePublishContentDefinition)
            {
                _pPropertyVisitor = DWFCORE_ALLOC_OBJECT( DWFContentElementPropertyVisitor );
            }
            else if (eVersion == DWFPublisher::ePublishObjectDefinition)
            {
                _pPropertyVisitor = DWFCORE_ALLOC_OBJECT( DWFDefinedObjectPropertyVisitor );
            }
        }
    }
    else
    {
        DWFCORE_FREE_OBJECT( _pPublishedObjectVisitor );
        DWFCORE_FREE_OBJECT( _pPropertyVisitor );

        if (eVersion == DWFPublisher::ePublishContentDefinition)
        {
            _pPublishedObjectVisitor = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement::Visitor );
            _pPropertyVisitor = DWFCORE_ALLOC_OBJECT( DWFContentElementPropertyVisitor );
        }

        else if (eVersion == DWFPublisher::ePublishObjectDefinition)
        {
            _pPublishedObjectVisitor = DWFCORE_ALLOC_OBJECT( DWFPublishedDefinedObject::Visitor );
            _pPropertyVisitor = DWFCORE_ALLOC_OBJECT( DWFDefinedObjectPropertyVisitor );
        }

        _eMetaDataVersion = eVersion;
    }
}

_DWFTK_API
void
DWFPackagePublisher::publish()
throw( DWFException )
{
    getPackageWriter()->write( _zSourceProductVendor,
                           _zSourceProductName,
                           _zSourceProductVersion,
                           _zPublisherProductVendor,
                           _zPublisherProductVersion );
}

_DWFTK_API
DWFContent*
DWFPackagePublisher::getContent()
throw( DWFException )
{
    return getPackageWriter()->getContentManager()->getContent();
}

_DWFTK_API
void
DWFPackagePublisher::attachContentManager( DWFContentManager* pContentManager, 
                                           bool bTakeOwnership )
throw()
{
    return getPackageWriter()->attachContentManager( pContentManager, bTakeOwnership );
}

_DWFTK_API
void
DWFPackagePublisher::visitPublishedObject( DWFPublishedObject& rPublishedObject )
throw( DWFException )
{
    //
    //  Objects and instances are inserted directly into the section there had better be one
    //
    if (_pCurrentSection == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current section" );
    }

    //
    //  Ensure the published object visitor is indeed allocated
    //
    if (_pPublishedObjectVisitor == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The published object visitor was not allocated" );
    }

    //
    //  Perform the visitation.
    //
    _pPublishedObjectVisitor->visitPublishedObject( rPublishedObject );

        //
        //  Invoke base publishers implementation published object visitor to ensure
        //  that composite visitors get a chance
        //
    DWFPublishedObject::Visitor* pBaseVisitor = DWFPublisher::getPublishedObjectVisitor();
    if (pBaseVisitor)
    {
        pBaseVisitor->visitPublishedObject( rPublishedObject );
    }
}

_DWFTK_API
void
DWFPackagePublisher::visitProperty( DWFProperty& rProperty )
throw( DWFException )
{
        //
        //  invoke base implementation for composites
        //
    DWFPropertyVisitor* pBaseVisitor = DWFPublisher::getPropertyVisitor();
    if (pBaseVisitor)
    {
        pBaseVisitor->visitProperty( rProperty );
    }
}

_DWFTK_API
void
DWFPackagePublisher::visitPropertyContainer( DWFPropertyContainer& rPropertyContainer )
throw( DWFException )
{
    //
    //  The property visitor is not allocated for the new version of the 
    //  object definition model.
    //
    if (_pPropertyVisitor)
    {
        _pPropertyVisitor->visitPropertyContainer( rPropertyContainer );
    }

        //
        // invoke base implementation for composites
        //
    DWFPropertyVisitor* pBaseVisitor = DWFPublisher::getPropertyVisitor();
    if (pBaseVisitor)
    {
        pBaseVisitor->visitPropertyContainer( rPropertyContainer );
    }
}

_DWFTK_API
void
DWFPackagePublisher::visitEmbeddedFont( const DWFEmbeddedFont& rFont )
throw( DWFException )
{
    //
    // blindly cache for now
    // TODO: be smart about which sections use which fonts
    //
    _oEmbeddedFonts.push_back( const_cast<DWFEmbeddedFont*>(&rFont) );

        //
        // invoke base implementation for composites
        //
    DWFEmbeddedFontVisitor* pBaseVisitor = DWFPublisher::getEmbeddedFontVisitor();
    if (pBaseVisitor)
    {
        pBaseVisitor->visitEmbeddedFont( rFont );
    }
}


_DWFTK_API
void
DWFPackagePublisher::postprocess( DWFPublishable* /*pContent*/ )
throw( DWFException )
{
    //
    // reset
    //
    _pCurrentSection = NULL;
    if (_eMetaDataVersion == DWFPublisher::ePublishObjectDefinition)
    {
        DWFPublishedDefinedObject::Visitor* pObjectVisitor =
            dynamic_cast<DWFPublishedDefinedObject::Visitor*>(_pPublishedObjectVisitor);
        DWFDefinedObjectPropertyVisitor* pPropertyVisitor =
            dynamic_cast<DWFDefinedObjectPropertyVisitor*>(_pPropertyVisitor);
        
        if (pObjectVisitor == NULL || pPropertyVisitor == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The visitors for publishing were incorrectly initialized" );
        }

        pObjectVisitor->setObjectDefinitionResource( NULL );
        pPropertyVisitor->setObjectDefinitionResource( NULL );
    }
}

void
DWFPackagePublisher::preprocessSection( DWFPublishableSection* pSection )
throw( DWFException )
{
    //
    // create a new package section object from the publishable section
    //
    DWFSource oSource( pSection->getSourceHRef(),
                       pSection->getSource(),
                       pSection->getSourceID() );


    DWFCustomSection* pCustomSection = 
        DWFCORE_ALLOC_OBJECT( DWFCustomSection(pSection->getType(),
                                               pSection->getTitle(), 
                                               /*NOXLATE*/L"",
                                              (double)_nSections++, 
                                              pSection->getVersion(),
                                               oSource) );

    DWFString zSectionName( pSection->name() );
    if (zSectionName.chars() > 0)
    {
        //
        //  Set the predefined name
        //

        pCustomSection->rename( zSectionName );
        //
        //  Prevent any renaming
        //
        DWFSection::tBehavior behavior = pCustomSection->behavior();
        behavior.bRenameOnPublish = false;
        pCustomSection->applyBehavior( behavior );
    }
        //
        // add the section to the writer
        //
    if (pCustomSection)
    {
        getPackageWriter()->addSection( pCustomSection );
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    //
    // going to need this...
    //
    _pCurrentSection = pCustomSection;

}

void
DWFPackagePublisher::postprocessSection( DWFPublishableSection* pSection )
throw( DWFException )
{
    if (_pCurrentSection == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current section" );
    }

    //
    // set the section label
    //
    _pCurrentSection->setLabel( pSection->getLabel() );

    //
    // set the initial URI
    //
    _pCurrentSection->setInitialURI( pSection->initialURI() );

    //
    // Publish associated resources
    //

    //
    // get the list of resources from the section
    //

    DWFPublishableResource* pResource = NULL;
    DWFIterator<DWFPublishableResource*>* piResources = pSection->getResources();

    DWFImage* pImage = NULL;

    if (piResources)
    {
        for (; piResources->valid(); piResources->next())
        {
            pResource = piResources->get();
			
            //
            // first try images (thumbnails, previews...)
            //
            pImage = dynamic_cast<DWFImage*>(pResource);
			 
			if (pImage)
            {
                //
                // for previews, thumbnails and textures we'll set the current
                // graphic resource as the parent.
                //
                DWFResource* pParentResource = NULL;
                DWFString zRole(DWFXML::kzRole_Preview);

                //
                // Re-coded as a switch statement, with no default, so that if
                // we add a new enum to teResourceType, gcc will warn us if we
                // don't handle it in here.
                //
                // But we'll not crash, as we initialized zRole to 'preview',
                // above - which is equivalent to how the if's were coded before.
                //
                switch(pImage->type())
                {
                case DWFImage::ePreview:
                    zRole = DWFXML::kzRole_Preview;
                    pParentResource = _pCurrentGraphicResource;
                    break;

                case DWFImage::eOverlayPreview:
                    zRole = DWFXML::kzRole_OverlayPreview;
                    break;

                case DWFImage::eMarkupPreview:
                    zRole = DWFXML::kzRole_MarkupPreview;
                    break;

                case DWFImage::eTexture:
                    zRole = DWFXML::kzRole_Texture;
                    pParentResource = _pCurrentGraphicResource;
                    break;

                case DWFImage::eThumbnail:
                    zRole = DWFXML::kzRole_Thumbnail;
                    pParentResource = _pCurrentGraphicResource;
                    break;

                case DWFImage::eIcon:
                    zRole = DWFXML::kzRole_Icon;
                    break;

                case DWFImage::eRasterOverlay:
                    zRole = DWFXML::kzRole_RasterOverlay;
                    break;

                case DWFImage::eRasterMarkup:
                    zRole = DWFXML::kzRole_RasterMarkup;
                    break;
                }

                //
                // If the image is a texture, capture the title
                //
                DWFString zTitle;
                DWFTexture* pTexture = dynamic_cast<DWFTexture*>(pImage);
                if (pTexture)
                {
                    zTitle.assign( pTexture->name() );
                }
                DWFImageResource* pImageRes = DWFCORE_ALLOC_OBJECT( DWFImageResource(zTitle, zRole, pImage->getMIMEType()) );

                if (pImageRes == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate image resource" );
                }

                double anExtents[4] = { 0.0, 0.0, pImage->width(), pImage->height() };

                pImageRes->configureGraphic( NULL, anExtents, pImage->clip() );
                pImageRes->configureImage( pImage->depth() );
                pImageRes->setInputStream( pImage->getInputStream() );

                //
                // give the resource visitor a chance to modify the resource 
                //
                DWFResource* pVisitedResource = NULL;
                if (_pResourceVisitor)
                {
                    pVisitedResource =_pResourceVisitor->visitResource( *_pCurrentSection, pImageRes );
                }

                if ( pVisitedResource == NULL )
                {
                    pVisitedResource = pImageRes;
                }

                //
                // add image to section
                // 1. section owns the resource (true)
                // 2. replace with the resource if applicable (true)
                // 3. deleted replaced resource if applicable (true)
                //
                _pCurrentSection->addResource( pVisitedResource, true, true, true, pParentResource );

                //
                // if the image is of type icon then set it as the section label icon
                //
                if (pImage->type() == DWFImage::eIcon)
                {
                    _pCurrentSection->setLabelIconResource( *pVisitedResource );
                }
            }
        }

        DWFCORE_FREE_OBJECT( piResources );
    }

    //
    // Embed Fonts
    //

    unsigned long iFont = 0;
    unsigned long nFonts = static_cast<unsigned long>(_oEmbeddedFonts.size());
    DWFEmbeddedFont* pFont = NULL;

        //
        // embed each font as a section resource
        //
    for (; iFont < nFonts; iFont++)
    {
        pFont = const_cast<DWFEmbeddedFont*>(_oEmbeddedFonts[iFont]);

        //
        // make sure the data is available
        //
        pFont->embed();

        //
        // create a new font resource
        //
        DWFFontResource* pFontRes = 
            DWFCORE_ALLOC_OBJECT( DWFFontResource(pFont->getRequest(),
                                                  pFont->getPrivilege(),
                                                  pFont->getCharacterCode(),
                                                  pFont->getFaceName(),
                                                  pFont->getLogfontName()) );

        if (pFontRes == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate font resource" );
        }

        //
        // set the data stream
        //
        pFontRes->setInputStream( pFont->getInputStream() );

        //
        // give the resource visitor a chance to modify the resource 
        //
        DWFResource* pVisitedResource = NULL;
        if (_pResourceVisitor)
        {
            pVisitedResource =_pResourceVisitor->visitResource( *_pCurrentSection, pFontRes );
        }

        if ( pVisitedResource == NULL )
        {
            pVisitedResource = pFontRes;
        }

        //
        // add font to section
        //
        _pCurrentSection->addResource( pVisitedResource, true );

    }

    //
    // prepare for next section
    //
    _oEmbeddedFonts.clear();

    //
    // MOVE all resources from the DWFPublishableSection to the DWFSection
    //
    DWFResource* pDWFResource = NULL;
    DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

    if (piAllResources)
    {
        for (; piAllResources->valid(); piAllResources->next())
        {
            pDWFResource = piAllResources->value();

            //
            // give the resource visitor a chance to modify the resource 
            //
            DWFResource* pVisitedResource = NULL;
            if (_pResourceVisitor)
            {
                pVisitedResource =_pResourceVisitor->visitResource( *_pCurrentSection, pDWFResource );
            }

            if ( pVisitedResource == NULL )
            {
                pVisitedResource = pDWFResource;
            }

            //
            // add resource to section
            //
            _pCurrentSection->addResource( pVisitedResource, false );

        }

        DWFCORE_FREE_OBJECT( piAllResources );
    }

    //
    // Section Properties
    //
    // MOVE all properties from the DWFPublishableSection to the DWFSection
    //
    _pCurrentSection->copyProperties( *pSection, true );

}

void
DWFPackagePublisher::preprocessModel( DWFModel* pW3DModel )
throw( DWFException )
{
    //
    // create a new EModelSection from the DWFModel
    //
    DWFSource oModelSource( pW3DModel->getSourceHRef(),
                            pW3DModel->getSource(),
                            pW3DModel->getSourceID() );

    DWFUnits oModelUnits( pW3DModel->getUnits() );

    DWFEModelSection* pModelSection = 
        DWFCORE_ALLOC_OBJECT( DWFEModelSection(pW3DModel->getTitle(), 
                                               /*NOXLATE*/L"",
                                              (double)_nSections++, 
                                               oModelSource,
                                              &oModelUnits) );

    DWFString zSectionName( pW3DModel->name() );
    if (pModelSection)
    {
        if (zSectionName.chars() > 0)
        {
            //
            //  Set the predefined name
            //

            pModelSection->rename( zSectionName );
            //
            //  Prevent any renaming
            //
            DWFSection::tBehavior behavior = pModelSection->behavior();
            behavior.bRenameOnPublish = false;
            pModelSection->applyBehavior( behavior );
        }

        //
        // add the section to the writer
        //
        getPackageWriter()->addSection( pModelSection );
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    //
    // going to need this...
    //
    _pCurrentSection = pModelSection;

    //
    //  For publishing object/content definition initialize the visitors
    //
    if (_eMetaDataVersion == DWFPublisher::ePublishContentDefinition)
    {
        DWFPublishedContentElement::Visitor* pCEVisitor =
            dynamic_cast<DWFPublishedContentElement::Visitor*>(_pPublishedObjectVisitor);

        //
        //  The visitor needs to use the same content that is associated with the model
        //  to create the metadata.
        //
        DWFContent* pContent = pW3DModel->getContent();
        pCEVisitor->setContent( pContent );

        DWFSectionContentResource* pResource = DWFCORE_ALLOC_OBJECT( DWFSectionContentResource( pContent ) );
        if (pResource == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate section content resource" );
        }

        //
        //  The visitor needs to know the content resource being used. It will
        //  use the ID of the content resource to generate the instances.
        //
        pCEVisitor->setContentResource( pResource );

        //
        //  To publish the default navigation presentation we need a listener
        //  track the instances getting generated and the order in which they are
        //  generated.
        //
        if (_pPublishedElementListener == NULL)
        {
            _pPublishedElementListener = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElementListener );
        }
        pCEVisitor->setNotificationSink( _pPublishedElementListener );
        _pPublishedElementListener->clear();
    }

    else if (_eMetaDataVersion == DWFPublisher::ePublishObjectDefinition)
    {
        DWFPublishedDefinedObject::Visitor* pDOVisitor =
            dynamic_cast<DWFPublishedDefinedObject::Visitor*>(_pPublishedObjectVisitor);
        DWFDefinedObjectPropertyVisitor* pDOPropertyVisitor =
            dynamic_cast<DWFDefinedObjectPropertyVisitor*>(_pPropertyVisitor);
        
        if (pDOVisitor == NULL || pDOPropertyVisitor == NULL)
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The visitors for publishing were incorrectly initialized" );
        }

        DWFObjectDefinitionResource* pObjectDefinition = 
            DWFCORE_ALLOC_OBJECT( DWFObjectDefinitionResource(DWFXML::kzElement_SpaceObjectDefinition, 
                                                              DWFXML::kzRole_ObjectDefinition) );
        if (pObjectDefinition == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate object definition resource" );
        }

		// Create a container for instances who are instanced from referenced objects.
		DWFObjectDefinitionResource* pReferencedObjectDefinition = 
			DWFCORE_ALLOC_OBJECT( DWFObjectDefinitionResource(L"Referenced", 
			L"ReferencedObjectDefinition") );
		if (pReferencedObjectDefinition == NULL)
		{
			_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate referenced object definition resource" );
		}

        pDOVisitor->setObjectDefinitionResource( pObjectDefinition );
		pDOVisitor->setReferencedObjectDefinitionResource( pReferencedObjectDefinition);
        pDOPropertyVisitor->setObjectDefinitionResource( pObjectDefinition );
    }

    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The metadata version information was not recognized" );
    }
}

void
DWFPackagePublisher::preprocessPlot( DWFPlot* pW2DPlot )
throw( DWFException )
{
    //
    // create a new EPlotSection from the DWFPlot
    //
    DWFSource oPlotSource( pW2DPlot->getSourceHRef(),
                            pW2DPlot->getSource(),
                            pW2DPlot->getSourceID() );

    DWFPaper* pPaper = pW2DPlot->getPaper();

    DWFEPlotSection* pPlotSection = 
        DWFCORE_ALLOC_OBJECT( DWFEPlotSection(pW2DPlot->getTitle(), 
                                               /*NOXLATE*/L"",
                                              (double)_nSections++,
                                              oPlotSource,
                                               0, // not used
                                              pPaper) );

    DWFString zSectionName( pW2DPlot->name() );
    if (zSectionName.chars() > 0)
    {
        //
        //  Set the predefined name
        //

        pPlotSection->rename( zSectionName );
        //
        //  Prevent any renaming
        //
        DWFSection::tBehavior behavior = pPlotSection->behavior();
        behavior.bRenameOnPublish = false;
        pPlotSection->applyBehavior( behavior );
    }
        //
        // add the section to the writer
        //
    if (pPlotSection)
    {
        getPackageWriter()->addSection( pPlotSection );
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    //
    // going to need this...
    //
    _pCurrentSection = pPlotSection;

    //
    // create a new object definition resource for this section
    //
    /*
    _pCurrentObjectDefinition = 
        DWFCORE_ALLOC_OBJECT( DWFObjectDefinitionResource(DWFXML::kzElement_SpaceObjectDefinition, DWFXML::kzRole_ObjectDefinition) );

    if (_pCurrentObjectDefinition == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate object definition resource" );
    }
    */
}

void
DWFPackagePublisher::preprocessData( DWFData* pData )
throw( DWFException )
{
    //
    // create a new DataSection from the DWFData
    //
    DWFSource oDataSource( /*NOXLATE*/L"",
                            pData->getSource(),
                            pData->getSourceID() );


    DWFDataSection* pDataSection = 
        DWFCORE_ALLOC_OBJECT( DWFDataSection(pData->getTitle(), 
                                             /*NOXLATE*/L"",
                                             (double)_nSections++,
                                             oDataSource ) );

    DWFString zSectionName( pData->name() );
    if (zSectionName.chars() > 0)
    {
        //
        //  Set the predefined name
        //

        pDataSection->rename( zSectionName );
        //
        //  Prevent any renaming
        //
        DWFSection::tBehavior behavior = pDataSection->behavior();
        behavior.bRenameOnPublish = false;
        pDataSection->applyBehavior( behavior );
    }
        //
        // add the section to the writer
        //
    if (pDataSection)
    {
        getPackageWriter()->addSection( pDataSection );
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new section" );
    }

    //
    // going to need this...
    //
    _pCurrentSection = pDataSection;

}

void
DWFPackagePublisher::postprocessModel( DWFModel* pW3DModel )
throw( DWFException )
{
	DWFGraphicResource * pGraphicResource = getGraphicResource();
    DWFEModelSection* pModelSection = dynamic_cast<DWFEModelSection*>(_pCurrentSection);

    if (pModelSection == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current section" );
    }

    double anTransform[16] = {0.0f};
    pW3DModel->getTransform( anTransform );

    _DWFTK_STD_VECTOR(double) anExtents;
    pW3DModel->getBoundingVolume( anExtents );
    double* pExtents = NULL;

    if (anExtents.size() > 0)
    {
        pExtents = DWFCORE_ALLOC_MEMORY( double, anExtents.size() );
        
        if (pExtents == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for extents" );
        }

        unsigned long i = 0;
        for (; i < (unsigned long)anExtents.size(); i++)
        {
            pExtents[i] = anExtents[i];
        }
    }
    
    pGraphicResource->configureGraphic( anTransform, pExtents );

    if (pExtents)
    {
        DWFCORE_FREE_MEMORY( pExtents );
    }

    //
    // Properties
    //
    //
    DWFProperty oModelProp( /*NOXLATE*/L"", /*NOXLATE*/L"", DWFPublishedDefinedObject::Visitor::kz_PropCategory_Hidden );

    //
    // tell the viewer how to light the scene
    //
    oModelProp.setName( kz_PropName_UseDefaultLighting );
    oModelProp.setValue( (pW3DModel->useModelLighting() ? /*NOXLATE*/L"1" : /*NOXLATE*/L"0") );
    pGraphicResource->addProperty( &oModelProp, false );

    //
    // tell the viewer about our polygons
    //
    {
        const wchar_t* zValue = NULL;
        switch (pW3DModel->getPolygonHandedness())
        {
            case DWFModel::eHandednessLeft:
            {
                zValue = /*NOXLATE*/L"left";
                break;
            }
            case DWFModel::eHandednessRight:
            {
                zValue = /*NOXLATE*/L"right";
                break;
            }
            default:
            {
                zValue = /*NOXLATE*/L"none";
            }
        };

        oModelProp.setName( kz_PropName_PolygonHandedness );
        oModelProp.setValue( zValue );
        pGraphicResource->addProperty( &oModelProp, false );
    }

    //
    // tell the viewer about silhouette edges
    //
    oModelProp.setName( kz_PropName_UseSilhouetteEdges );
    oModelProp.setValue( (pW3DModel->useSilhouetteEdges() ? /*NOXLATE*/L"1" : /*NOXLATE*/L"0") );
    pGraphicResource->addProperty( &oModelProp, false );

        //
        // tell the viewer about any edge color
        //
    const float* const pEdgeColor = pW3DModel->getEdgeColor();
    if (pEdgeColor)
    {
        wchar_t zValues[64] = {0};
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", pEdgeColor[0], pEdgeColor[1], pEdgeColor[2] );

        oModelProp.setName( kz_PropName_EdgeColor );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );
    }

    //
    // tell the viewer about the state of compass: show or hide
    //
    {
        const wchar_t* zValue = NULL;
        switch (pW3DModel->getViewCubeCompassState())
        {
            case DWFModel::eViewCubeCompassOff:
            {
                zValue = /*NOXLATE*/L"off";
                break;
            }
            case DWFModel::eViewCubeCompassOn:
            {
                zValue = /*NOXLATE*/L"on";
                break;
            }
            default:
                ;
        };

        if( zValue != NULL )
        {
            oModelProp.setName( kz_PropName_ViewCubeShowCompass );
            oModelProp.setValue( zValue );
            pGraphicResource->addProperty( &oModelProp, false );
        }
    }


    //
    // tell the viewer about the angle of north
    //
    const float* const pAngle = pW3DModel->getViewCubeAngleOfNorth();
    if( pAngle != NULL )
    {
        wchar_t zValue[64] = {0};
        _DWFCORE_SWPRINTF( zValue, 64, /*NOXLATE*/L"%.7G\0", *pAngle);

        oModelProp.setName( kz_PropName_ViewCubeAngleOfNorth );
        oModelProp.setValue( zValue );
        pGraphicResource->addProperty( &oModelProp, false );
    }

    //
    // tell the viewer about the view cube home
    //
    W3DCamera oCamera;
    DWFString zName;
    if( pW3DModel->getDefinedView( DWFModel::eViewCubeHome, zName, oCamera ) )
    {
        wchar_t zValues[64];
        float anValues[3];

        //
        // projection
        //
        oModelProp.setName( kz_PropName_ViewCubeHomeCameraProjection );
        oModelProp.setValue( (oCamera.getProjection() == W3DCamera::ePerspective ? /*NOXLATE*/L"1" : /*NOXLATE*/L"0") );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // position
        //
        oCamera.getPosition( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_ViewCubeHomeCameraPosition );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // target
        //
        oCamera.getTarget( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_ViewCubeHomeCameraTarget );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // up vector
        //
        oCamera.getUpVector( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_ViewCubeHomeCameraUpVector );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // field
        //
        oCamera.getField( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_ViewCubeHomeCameraField );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );
    }

        //
        // tell the viewer about the initial display mode
    int nDisplayMode = pW3DModel->getDisplayMode();
    if ( nDisplayMode > 0)
    {
        wchar_t zValues[32] = {0};
        _DWFCORE_SWPRINTF( zValues, 32, /*NOXLATE*/L"%d", nDisplayMode );

        oModelProp.setName( kz_PropName_DisplayMode );
        oModelProp.setValue( zValues );
        pGraphicResource->addProperty( &oModelProp, false );
    }
        //
        // define the default view in the XML if it's not in the stream
        //
    const W3DCamera* pCamera = pW3DModel->getDefaultView();
    if (pCamera)
    {
        wchar_t zValues[64];
        float anValues[3];

        //
        // projection
        //
        oModelProp.setName( kz_PropName_CameraProjection );
        oModelProp.setValue( (pCamera->getProjection() == W3DCamera::ePerspective ? /*NOXLATE*/L"1" : /*NOXLATE*/L"0") );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // position
        //
        pCamera->getPosition( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_CameraPosition );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // target
        //
        pCamera->getTarget( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_CameraTarget );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // up vector
        //
        pCamera->getUpVector( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_CameraUpVector );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // field
        //
        pCamera->getField( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_CameraField );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );
    }

	W3DCamera FrontCamera;	
    if (pW3DModel->getDefinedView(DWFModel::eFrontView, zName, FrontCamera))
    {
        wchar_t zValues[64];
        float anValues[3];

        //
        // projection
        //
        oModelProp.setName( kz_PropName_FrontViewCameraProjection );
        oModelProp.setValue( (FrontCamera.getProjection() == W3DCamera::ePerspective ? /*NOXLATE*/L"1" : /*NOXLATE*/L"0") );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // position
        //
        FrontCamera.getPosition( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_FrontViewCameraPosition );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // target
        //
        FrontCamera.getTarget( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_FrontViewCameraTarget );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // up vector
        //
        FrontCamera.getUpVector( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_FrontViewCameraUpVector );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );

        //
        // field
        //
        FrontCamera.getField( anValues );
        _DWFCORE_SWPRINTF( zValues, 64, /*NOXLATE*/L"%.7G %.7G %.7G\0", anValues[0], anValues[1], anValues[2] );

        oModelProp.setName( kz_PropName_FrontViewCameraField );
        oModelProp.setValue( DWFString::RepairDecimalSeparators(zValues) );
        pGraphicResource->addProperty( &oModelProp, false );
    }

    //
    // bind the streams
    //
    pGraphicResource->setInputStream( pW3DModel->getInputStream() );

    //
    // add any coordinate systems
    //
    DWFCoordinateSystem::tList::Iterator* piCSs = pW3DModel->getCoordinateSystems();

    for ( ; piCSs && piCSs->valid(); piCSs->next() )
    {
        DWFCoordinateSystem* pCS = piCSs->get();
        pGraphicResource->addCoordinateSystem( pCS );
        pW3DModel->removeCoordinateSystem( pCS, false );
    }

    DWFCORE_FREE_OBJECT( piCSs );

    //
    // give the resource visitor a chance to modify the resource 
    //
    DWFResource* pVisitedGraphicResource = NULL;
    if (_pResourceVisitor)
    {
        pVisitedGraphicResource =_pResourceVisitor->visitResource( *_pCurrentSection, pGraphicResource );
    }

    _pCurrentGraphicResource = ( pVisitedGraphicResource != NULL ) 
                             ? pVisitedGraphicResource 
                             : pGraphicResource;

    //
    // add the graphics resource to section
    //
    _pCurrentSection->addResource( _pCurrentGraphicResource, true );

    //
    // create the default views presentation
    //
    _createDefaultViewsPresentation( pW3DModel );

    //
    //  Now add the metadata as a child resource of the graphics resource
    //
    DWFResource* pMetadataResource = NULL;

    //
    //  First grab the metadata resource from the published object visitor
    //
    if (_eMetaDataVersion == DWFPublisher::ePublishContentDefinition)
    {
        DWFPublishedContentElement::Visitor* pObjectVisitor =
            dynamic_cast<DWFPublishedContentElement::Visitor*>(_pPublishedObjectVisitor);

        if (pObjectVisitor)
        {
            pMetadataResource = pObjectVisitor->getContentResource();
    
            //
            //  Cleanup the visitor
            //
            pObjectVisitor->setContentResource( NULL );

            //
            // create the content navigation presentation, if asked to
            //
            if (_bCreateContentNavigationPresentation)
            {
                DWFSortedVector<DWFInstance*>& oExcluded = pObjectVisitor->instancesExcludedFromModel();
                _createDefaultModelNavigationPresentation( pW3DModel, oExcluded );
            }

            //
            //  Reset the visitor for the next use
            //
            pObjectVisitor->reset();
        }
    }
    else if (_eMetaDataVersion == DWFPublisher::ePublishObjectDefinition)
    {
        DWFPublishedDefinedObject::Visitor* pObjectVisitor =
            dynamic_cast<DWFPublishedDefinedObject::Visitor*>(_pPublishedObjectVisitor);

        if (pObjectVisitor)
        {
			// Release the instances those won't be wrote into ObjectDefinition file.
			pMetadataResource = pObjectVisitor->getReferencedObjectDefinitionResource();
			if (pMetadataResource)
			{
				DWFCORE_FREE_OBJECT(pMetadataResource);
			}
            pMetadataResource = pObjectVisitor->getObjectDefinitionResource();
        }
    }

    //
    //  Now add it as a child of the graphics resource
    //
    if (pMetadataResource)
    {
        //
        // give the resource visitor a chance to modify the resource 
        //
        DWFResource* pVisitedResource = NULL;
        if (_pResourceVisitor)
        {
            pVisitedResource =_pResourceVisitor->visitResource( *_pCurrentSection, pMetadataResource );
        }
        if ( pVisitedResource == NULL )
        {
            pVisitedResource = pMetadataResource;
        }

        //
        // 1. section owns the resource (true)
        // 2. replace with the resource if applicable (true)
        // 3. deleted replaced resource if applicable (true)
        //
        _pCurrentSection->addResource( pVisitedResource, true, true, true, _pCurrentGraphicResource );
    }


    //
    // publish resources
    //
    postprocessSection( pW3DModel );
}


void
DWFPackagePublisher::postprocessPlot( DWFPlot* pW2DPlot )
throw( DWFException )
{
    
	//
	// Check if we have a graphics stream
	//
	
	DWFGraphicResource * pGraphicResource = getGraphicResource();
	if(pW2DPlot->getInputStream() != NULL)
	{
		DWFUUID oObjectID;
		pGraphicResource->setObjectID(oObjectID.uuid(true));

		double anTransform[16] = {0.0f};
		pW2DPlot->getTransform( anTransform );

		_DWFTK_STD_VECTOR(double) anExtents;
		pW2DPlot->getPlotExtents( anExtents );
		double* pExtents = NULL;

		if (anExtents.size() > 0)
		{
			pExtents = DWFCORE_ALLOC_MEMORY( double, anExtents.size() );
	        
			if (pExtents == NULL)
			{
				_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for extents" );
			}

			unsigned long i = 0;
			for (; i < (unsigned long)anExtents.size(); i++)
			{
				pExtents[i] = anExtents[i];
			}
		}
	    
		_DWFTK_STD_VECTOR(double) anClip;
		pW2DPlot->getPlotClip( anClip );
		double* pClip = NULL;

		if (anClip.size() > 0)
		{
			pClip = DWFCORE_ALLOC_MEMORY( double, anClip.size() );
	        
			if (pClip == NULL)
			{
				_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for Clip" );
			}

			unsigned long i = 0;
			for (; i < (unsigned long)anClip.size(); i++)
			{
				pClip[i] = anClip[i];
			}
		}
	    
		pGraphicResource->configureGraphic( anTransform, pExtents, pClip );

		if (pExtents)
		{
			DWFCORE_FREE_MEMORY( pExtents );
		}

		if (pClip)
		{
			DWFCORE_FREE_MEMORY( pClip );
		}

		//
		// bind the streams
		//
		pGraphicResource->setInputStream( pW2DPlot->getInputStream() );

		//
		// add any coordinate systems
		//
		DWFCoordinateSystem::tList::Iterator* piCSs = pW2DPlot->getCoordinateSystems();

		for ( ; piCSs && piCSs->valid(); piCSs->next() )
		{
			DWFCoordinateSystem* pCS = piCSs->get();
			pGraphicResource->addCoordinateSystem( pCS );
			pW2DPlot->removeCoordinateSystem( pCS, false );
		}

		DWFCORE_FREE_OBJECT( piCSs );
	

		//
		// give the resource visitor a chance to modify the resource 
		//
		DWFResource* pVisitedGraphicResource = NULL;
		if (_pResourceVisitor)
		{
			pVisitedGraphicResource =_pResourceVisitor->visitResource( *_pCurrentSection, pGraphicResource );
		}

		_pCurrentGraphicResource = ( pVisitedGraphicResource != NULL ) 
									? pVisitedGraphicResource 
									: pGraphicResource;

		//
		// add the graphics resource to section
		//
		_pCurrentSection->addResource( _pCurrentGraphicResource, true );
	}

    //
    // publish resources
    //
    postprocessSection( pW2DPlot );

}


void
DWFPackagePublisher::postprocessData( DWFData* pData )
throw( DWFException )
{
    //
    // publish resources
    //
    postprocessSection( pData );
}


/******************************************************************
void
DWFPackagePublisher::_preprocessGeneric( DWFGeneric* pGeneric )
throw( DWFException )
{
    ;
}

void
DWFPackagePublisher::_postprocessGeneric( DWFGeneric* )
throw( DWFException )
{
    ;
}
******************************************************************/

void 
DWFPackagePublisher::_createDefaultModelNavigationPresentation( DWFModel* pW3DModel,
                                                                DWFSortedVector<DWFInstance*>& oExcluded )
throw( DWFException )
{
    if (_pPublishedElementListener == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"The published element listener was not set." );
    }

    //
    //  Ensure that we have content presentation resource to add the default model navigation presentation
    //
    DWFContentPresentationResource* pContentPresentationResource = NULL;
    DWFResourceContainer::ResourceIterator* piContentResources = pW3DModel->findResourcesByRole(DWFXML::kzRole_ContentPresentation);
    if (piContentResources)
    {
        if (!piContentResources->valid())
        {
            //
            // a content presentation resource was not added, so lets make one up here
            //
            pContentPresentationResource = DWFCORE_ALLOC_OBJECT( DWFContentPresentationResource( DWFXML::kzRole_ContentPresentation ) );        
            ((DWFResourceContainer*)pW3DModel)->addResource(pContentPresentationResource, true);
        }
        else
        {
            //
            // we should have only one, get it
            //
            pContentPresentationResource = dynamic_cast<DWFContentPresentationResource*>(piContentResources->get());
        }

        DWFCORE_FREE_OBJECT( piContentResources );
    }

    if (pContentPresentationResource == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"The ContentPresentationResource could not be allocated." );
    }


    //
    // create the model presentation
    //
    DWFContentPresentation* pPresentation = DWFCORE_ALLOC_OBJECT( DWFContentPresentation( _zContentNavigationPresentationLabel, DWFContentPresentation::kzID_ContentNavigation ) );
    pContentPresentationResource->addPresentation( pPresentation );

    //
    // create the default view
    //
    DWFContentPresentationView* pDefaultView = DWFCORE_ALLOC_OBJECT( DWFContentPresentationView() );
    pPresentation->addView( pDefaultView );


    typedef std::map<DWFObject*, DWFContentPresentationReferenceNode*> _tObjectNodeMap;
    //
    //  A map to hold a collection of object to reference node mappings. Only objects with
    //  children should be stored in this map.
    //
    _tObjectNodeMap oObjectNode;

    //
    // run through the instances of this model, get their renderables and add them to
    // the model presentation
    //
    DWFInstance::tIterator* piInstance = _pPublishedElementListener->getInstances();

    if (piInstance)
    {
        for(; piInstance->valid(); piInstance->next())
        {
            DWFInstance* pInstance = piInstance->get();
            //
            //  If the instance is in the exclusion list, skip it.
            //
            if (oExcluded.count( pInstance ))
            {
                continue;
            }

            DWFRenderable* pRenderable = pInstance->getRenderedElement();
            DWFContentPresentationReferenceNode* pParentNode = NULL;

            //
            // Objects can have heirarchies
            //
            DWFObject* pObject = dynamic_cast<DWFObject*>(pRenderable);
            if (pObject)
            {
                //
                // For now I'm allowing for the possibility that subassemblies are
                // published skipping in between objects.
                //
                DWFObject* pParent = pObject->getParent();
                while (pParent && 
                       pParentNode == NULL)
                {
                    _tObjectNodeMap::iterator iParent = oObjectNode.find( pParent );
                    if (iParent != oObjectNode.end())
                    {
                        pParentNode = iParent->second;
                    }
                    else
                    {
                        pParent = pParent->getParent();
                    }
                }
            }

            DWFContentPresentationReferenceNode* pReferenceNode = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode() );
            pReferenceNode->setContentElement( *pRenderable );
            pReferenceNode->setLabel( pRenderable->getLabel() );
            if (pParentNode)
            {
                pParentNode->addChild( pReferenceNode );
            }
            else
            {
                pDefaultView->addNode( pReferenceNode );
            }

            //
            //  For objects save the mapping.
            //
            if (pObject)
            {
                oObjectNode.insert( std::make_pair(pObject, pReferenceNode) );
            }
        }

        DWFCORE_FREE_OBJECT( piInstance );
    }
}

void
DWFPackagePublisher::_createDefaultViewsPresentation( DWFModel* pW3DModel )
throw( DWFException )
{
    //
    //  Ensure that we have content presentation resource to add the default views presentation
    //
    DWFContentPresentationResource* pContentPresentationResource = NULL;
    DWFResourceContainer::ResourceIterator* piContentResources = pW3DModel->findResourcesByRole(DWFXML::kzRole_ContentPresentation);
    bool bAddResource = false;
    if (piContentResources)
    {
        if (!piContentResources->valid())
        {
            //
            // a content presentation resource was not added, so lets make one up here
            //
            pContentPresentationResource = DWFCORE_ALLOC_OBJECT( DWFContentPresentationResource( DWFXML::kzRole_ContentPresentation ) );        
            bAddResource = true;
        }
        else
        {
            //
            // we should have only one, get it
            //
            pContentPresentationResource = dynamic_cast<DWFContentPresentationResource*>(piContentResources->get());
        }

        DWFCORE_FREE_OBJECT( piContentResources );
    }

    if (pContentPresentationResource == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"The ContentPresentationResource could not be allocated." );
    }


    //
    // create the model presentation
    //
    DWFContentPresentation* pPresentation = DWFCORE_ALLOC_OBJECT( DWFContentPresentation( _zContentViewsPresentationLabel, DWFContentPresentation::kzID_Views ) );
    pContentPresentationResource->addPresentation( pPresentation );

    //
    // create the default view
    //
    DWFContentPresentationView* pDefaultView = DWFCORE_ALLOC_OBJECT( DWFContentPresentationView() );
    pPresentation->addView( pDefaultView );

    //
    // create a view node for the default camera
    //
    W3DCamera oCamera;
    DWFString zName;
    bool bViewDefined = false;
    if (pW3DModel->getDefinedView( DWFModel::eInitialView, zName, oCamera ))
    {
        DWFContentPresentationModelViewNode* pViewNode = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode( zName ) );
        pViewNode->setCamera( oCamera, true );
        pDefaultView->addNode( pViewNode );
        bViewDefined = true;
    }

    //
    // create a view node for the view cube home
    //
    if (pW3DModel->getDefinedView( DWFModel::eViewCubeHome, zName, oCamera ))
    {
        DWFContentPresentationModelViewNode* pViewNode = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode( zName ) );
        pViewNode->setCamera( oCamera, true );
        pDefaultView->addNode( pViewNode );
        bViewDefined = true;
    }

    if (bAddResource && bViewDefined)
    {
        ((DWFResourceContainer*)pW3DModel)->addResource(pContentPresentationResource, true);
    }
}

//DNT_End

#endif
