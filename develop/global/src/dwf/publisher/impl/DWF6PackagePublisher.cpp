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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/publisher/impl/DWF6PackagePublisher.cpp#1 $
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
#include "dwf/publisher/impl/DWF6PackagePublisher.h"
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
DWF6PackagePublisher::DWF6PackagePublisher( const DWFFile&   rFile,
                                            const DWFString& zPassword,
                                            const DWFString& zSourceProductVendor,
                                            const DWFString& zSourceProductName,
                                            const DWFString& zSourceProductVersion,
                                            const DWFString& zPublisherProductVendor,
                                            const DWFString& zPublisherProductVersion,
                                            bool             bNoPasswordSalting )
throw( DWFException )
                   : DWFPackagePublisher( zSourceProductVendor
                                        , zSourceProductName
                                        , zSourceProductVersion
                                        , zPublisherProductVendor
                                        , zPublisherProductVersion)
                   , _oPackageWriter( rFile, zPassword )
{
    _oPackageWriter.enablePasswordSalting( !bNoPasswordSalting );
}

_DWFTK_API
DWF6PackagePublisher::~DWF6PackagePublisher()
throw()
{
    ;
}

_DWFTK_API
void
DWF6PackagePublisher::enablePasswordSalting( bool bEnable )
throw()
{
    _oPackageWriter.enablePasswordSalting( bEnable );
}

_DWFTK_API
void DWF6PackagePublisher::addSignatureRequest( DWFSignatureRequest* pSignatureRequest )
    throw( DWFException )
{
    _oPackageWriter.addSignatureRequest( pSignatureRequest );
}

void DWF6PackagePublisher::postprocess(	DWFToolkit::DWFPublishable *pContent )
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

void DWF6PackagePublisher::preprocess( DWFToolkit::DWFPublishable *pContent )
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

void DWF6PackagePublisher::postprocessModel(DWFToolkit::DWFModel *pW3DModel)
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

void DWF6PackagePublisher::postprocessPlot(DWFToolkit::DWFPlot *pW2DPlot)
	throw( DWFException )
{
	DWFEPlotSection* pPlotSection = dynamic_cast<DWFEPlotSection*>(_pCurrentSection);

    if (pPlotSection == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No current section" );
    }

	//
	// Check if we have a graphics stream
	//
	
	if(pW2DPlot->getInputStream() != NULL)
	{
		//
		// Create a DWF resource to represent our W2D graphics stream
		//

		DWFGraphicResource* pGraphicResource = 
			DWFCORE_ALLOC_OBJECT( DWFGraphicResource(pW2DPlot->getTitle(),
													 DWFXML::kzRole_Graphics2d,
													 DWFMIME::kzMIMEType_W2D) );

		if (pGraphicResource == NULL)
		{
			_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new resource" );
		}
		
		_pGraphicResource = pGraphicResource;
		
		DWFPackagePublisher::postprocessPlot(pW2DPlot);
	}
}

//DNT_End

#endif
