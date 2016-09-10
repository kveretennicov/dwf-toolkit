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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/DWFXPackagePublisher.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"
#include "dwf/package/Constants.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/package/DataSection.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/CustomSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/publisher/impl/DWFXPackagePublisher.h"
#include "dwf/publisher/impl/PublishedDefinedObject.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
#include "dwf/publisher/impl/DefinedObjectPropertyVisitor.h"
#include "dwf/publisher/impl/ContentElementPropertyVisitor.h"
#include "dwf/publisher/impl/PublishedContentElementListener.h"
#include "dwf/presentation/ContentPresentationResource.h"
#include "dwf/presentation/ContentPresentationReferenceNode.h"

using namespace DWFToolkit;

//DNT_Start

_DWFTK_API
DWFXPackagePublisher::DWFXPackagePublisher( const DWFFile&   rFile,
										    const DWFString& zPassword,
                                            const DWFString& zSourceProductVendor,
                                            const DWFString& zSourceProductName,
                                            const DWFString& zSourceProductVersion,
                                            const DWFString& zPublisherProductVendor,
                                            const DWFString& zPublisherProductVersion, 
											bool  bNoPasswordSalting)
throw( DWFException )
                   : DWFPackagePublisher( zSourceProductVendor
                                        , zSourceProductName
                                        , zSourceProductVersion
                                        , zPublisherProductVendor
                                        , zPublisherProductVersion)
                   , _oPackageWriter( rFile, zPassword, NULL, bNoPasswordSalting)
{
    ;
}

_DWFTK_API
DWFXPackagePublisher::~DWFXPackagePublisher()
throw()
{
    ;
}

_DWFTK_API
void DWFXPackagePublisher::addProxyGraphicsSection( DWFPlot* pProxyPlot, teProxyGraphicsRulesType eRules, bool bAsFirstPage )
    throw( DWFException )
{
    if (pProxyPlot == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"No section provided" );
    }
	
	//
	//Create a new Package writer rules type
	//

	DWFXPackageWriter::teProxyGraphicsRulesType rulesType = DWFXPackageWriter::eAlways;

	if( eRules == DWFXPackagePublisher::eAlways )
	{
		rulesType = DWFXPackageWriter::eAlways;
	}

	else if( eRules == DWFXPackagePublisher::eIfNoPageOutput )
	{
		rulesType = DWFXPackageWriter::eIfNoPageOutput;
	}

	else if(eRules == DWFXPackagePublisher::eIfOnlyPartialPageOutput)
	{
		rulesType = DWFXPackageWriter::eIfOnlyPartialPageOutput;
	}
	
	//
    // create a new EPlotSection from the DWFPlot
    //

    DWFSource oPlotSource( pProxyPlot->getSourceHRef(),
                            pProxyPlot->getSource(),
                            pProxyPlot->getSourceID() );

    DWFPaper* pPaper = pProxyPlot->getPaper();

    DWFEPlotSection* pPlotSection = 
        DWFCORE_ALLOC_OBJECT( DWFEPlotSection(pProxyPlot->getTitle(), 
                                               /*NOXLATE*/L"",
                                              (double)_nSections++,
                                              oPlotSource,
                                               0, // not used
                                              pPaper) );

    DWFString zSectionName( pProxyPlot->name() );
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
		_pCurrentSection = pPlotSection;
    }
	
	//
	// Add the proxy graphics section using the package writer
	//
	 if (pPlotSection)
    {
		//
		// call post process plot
		//
		
		postprocessPlot( pProxyPlot );
		//
		// Collect all the resources
		//

		DWFPackagePublisher::postprocessSection( pProxyPlot );

        _oPackageWriter.addProxyGraphicsSection( pPlotSection, rulesType, bAsFirstPage );
    }
    else
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to add the new proxy plot" );
    }
	
}

_DWFTK_API
void DWFXPackagePublisher::addProxyPage( const DWFString& zProxyPageXPSPath  )
    throw( DWFException )
{
	_oPackageWriter.addProxyPage(zProxyPageXPSPath);
}

void DWFXPackagePublisher::postprocessPlot(DWFToolkit::DWFPlot *pW2DPlot)
	throw( DWFException )
{
	DWFEPlotSection* pPlotSection = dynamic_cast<DWFEPlotSection*>(_pCurrentSection);

    if (pPlotSection == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current section" );
    }

	//
	// Check if we have a XAML stream
	//
	
	if(pW2DPlot->getInputStream() != NULL)
	{
		DWFGraphicResource* pGraphicResource = 
			DWFCORE_ALLOC_OBJECT( DWFGraphicResource(pW2DPlot->getTitle(),
													 DWFXML::kzRole_Graphics2d,
													 DWFMIME::kzMIMEType_FIXEDPAGEXML) );
		_pGraphicResource = pGraphicResource;
		DWFPackagePublisher::postprocessPlot(pW2DPlot);
	}

	DWFResource* pDWFResource = NULL;
    DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = _pCurrentSection->getResourcesByRole();
	
    if (piAllResources)
    {
        for (; piAllResources->valid(); piAllResources->next())
        {
            //
			// add relationships to the resources in the DWF Plot
			//
			
			pDWFResource = piAllResources->value();
			if( pDWFResource->role() == DWFXML::kzRole_Graphics2dExtension )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_Graphics2dExtensionResource  );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_Graphics2dDictionary )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_Graphics2dDictionaryResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_ObservationMesh )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_ObservationMeshResource);
			}
			else if( pDWFResource->role() == DWFXML::kzRole_GeographicCoordinateSystem )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_GeographicCoordinateSystemResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_RasterReference )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_RasterReferenceResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_Font )
			{
				_pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_FontResource );
			}
		}
	}
}

void DWFXPackagePublisher::preprocess( DWFPublishable *pContent)
	throw( DWFException )
{
	//
    // 3D
    //
    DWFModel* pW3DModel = dynamic_cast<DWFModel*>(pContent);
    if (pW3DModel)
    {
		DWFPackagePublisher::preprocessModel( pW3DModel );
    }
	else
	{
		//
		// 2D
		//

		DWFPlot* pW2DPlot = dynamic_cast<DWFPlot*>(pContent);
		if (pW2DPlot)
		{
			DWFPackagePublisher::preprocessPlot( pW2DPlot );
		}

		 else
		{
			//
			// Data
			//
			DWFData* pData = dynamic_cast<DWFData*>(pContent);
			if (pData)
			{
				DWFPackagePublisher::preprocessData( pData );
			}
			else
			{
				//
				// Any other
				//
				DWFPublishableSection* pSection = dynamic_cast<DWFPublishableSection*>(pContent);
				if (pSection)
				{
					DWFPackagePublisher::preprocessSection( pSection );
				}
			}
		}
	}
}

void DWFXPackagePublisher::postprocess( DWFPublishable *pContent)
	throw( DWFException )
{
	//
    // clear this if in case its set from a previous postprocess call
    //
    _pCurrentGraphicResource = NULL;

    //
    // 3D
    //
    DWFModel* pW3DModel = dynamic_cast<DWFModel*>(pContent);
    if (pW3DModel)
    {
        postprocessModel( pW3DModel );
    }
	else
	{
        //
        // 2D
        //
        DWFPlot* pW2DPlot = dynamic_cast<DWFPlot*>(pContent);
        if (pW2DPlot)
        {
            postprocessPlot( pW2DPlot );
        }
        else
        {
                //
                // Data
                //
            DWFData* pData = dynamic_cast<DWFData*>(pContent);
            if (pData)
            {
				DWFPackagePublisher::postprocessData( pData );
            }
            else
            {
                //
                // Any other
                //
                DWFPublishableSection* pSection = dynamic_cast<DWFPublishableSection*>(pContent);
                if (pSection)
                {
					DWFPackagePublisher::postprocessSection( pSection );
                }
            }
        }
	}
	DWFPackagePublisher::postprocess( pContent );
}

void DWFXPackagePublisher::postprocessModel(DWFToolkit::DWFModel *pW3DModel)
	throw( DWFException )
{
	//
    // Create a DWF resource to represent our W3D graphics stream
    //
    DWFGraphicResource* pGraphicResource = 
        DWFCORE_ALLOC_OBJECT( DWFGraphicResource(pW3DModel->getTitle(),
                                                 DWFXML::kzRole_Graphics3d,
                                                 DWFMIME::kzMIMEType_W3D) );
    if (pGraphicResource == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new resource" );
    }

    if (pW3DModel->streamCompressionEnabled() && pW3DModel->vertexCompressionEnabled())
    {
        pGraphicResource->enableCustomZipMode( true, DWFZipFileDescriptor::eZipNone );
    }
	
	_pGraphicResource = pGraphicResource;
	DWFPackagePublisher::postprocessModel( pW3DModel);
}

//DNT_End

#endif
