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


#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"

#include "dwf/publisher/Image.h"
#include "dwf/publisher/impl/DWF6PackagePublisher.h"
#include "dwf/publisher/impl/DWFXPackagePublisher.h"
#include "dwf/publisher/impl/PublishedDefinedObject.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
#include "dwf/publisher/model/Model.h"
#include "dwf/package/ContentManager.h"
#include "dwf/w3dtk/Version.h"

using namespace DWFToolkit;

#include <iostream>
using namespace std;


#define _DWFMODEL_MIN_VERION_FOR_CONTENT    _DWF_FORMAT_VERSION_CURRENT



_DWFTK_API
DWFModel::DWFModel( DWFOutputStream&  rModelStream,
                    const DWFString&  zModelTitle,
                    const DWFString&  zModelSource,
                    const DWFString&  zModelSourceID,
                    const DWFString&  zModelSourceHRef,
                    const DWFString&  zModelLabel )
throw()
        : DWFPublishableSection(_DWF_FORMAT_EMODEL_TYPE_STRING,
                                zModelTitle,
                                zModelSource,
                                zModelSourceID,
                                zModelSourceHRef,
                                zModelLabel)
        , _eOutputMode( eBinary )
        , _bOpen( false )
        , _bCompressStream( true )
        , _bCompressVertices( false )
        , _bStreamDefaults( true )
        , _bBoundingVolumeSet( false )
        , _bUseDefaultLighting( true )
        , _bModelLightsUsed( false )
        , _bUseSilhouetteEdges( true )
        , _bSetEdgeColor( false )
        , _bOpenLocalLightAttributes( false )
        , _bSetNorthAngle( false )
        , _fNorthAngle( 90.0 )
        , _eUnits( DWFUnits::eMillimeters )
        , _eHandedness( eHandednessNone )
        , _eCompassState( eViewCubeCompassNone )
        , _nStyleSegments( 0 )
        , _zMIME( DWFMIME::kzMIMEType_W3D )
        , _pW3DFile( NULL )
        , _pW3DFileStream( NULL )
        , _pVersionBuffer( NULL )
        , _pW3DStreamWriter( NULL )
        , _pDefaultViewCamera( NULL )
        , _pInitialViewCamera( NULL )
        , _oBoundingVolume()
        , _oEmbeddedFonts()
        , _oResources()
        , _pPublishedObjectFactory( NULL )
        , _pContent( NULL )
        , _zTempID()
        , _eMetadataVersion( DWFPublisher::ePublishContentDefinition )
        , _nDisplayMode( 0 )
        , _pViewCubeHome( NULL )
		, _pFrontView (NULL)
{
    _anEdgeColor[0] = _anEdgeColor[1] = _anEdgeColor[2] = 0.0f;

    _anUnitsTransform[0] = _anUnitsTransform[1] = _anUnitsTransform[2] = _anUnitsTransform[3] = 0.0f;
    _anUnitsTransform[4] = _anUnitsTransform[5] = _anUnitsTransform[6] = _anUnitsTransform[7] = 0.0f;
    _anUnitsTransform[8] = _anUnitsTransform[9] = _anUnitsTransform[10] = _anUnitsTransform[11] = 0.0f;
    _anUnitsTransform[12] = _anUnitsTransform[13] = _anUnitsTransform[14] = _anUnitsTransform[15] = 0.0f;

    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );

    _pW3DStreamWriter = DWFCORE_ALLOC_OBJECT( W3DStreamWriter(rModelStream, _oToolkit) );
    
    _pPublishedObjectFactory = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement::Factory );
}


_DWFTK_API
DWFModel::DWFModel( const DWFString&  zModelTitle,
                    const DWFString&  zModelSource,
                    const DWFString&  zModelSourceID,
                    const DWFString&  zModelSourceHRef,
                    const DWFString&  zModelLabel )
throw()
        : DWFPublishableSection(_DWF_FORMAT_EMODEL_TYPE_STRING,
                                 zModelTitle,
                                 zModelSource,
                                 zModelSourceID,
                                 zModelSourceHRef,
                                 zModelLabel)
        , _eOutputMode( eBinary )
        , _bOpen( false )
        , _bCompressStream( true )
        , _bCompressVertices( false )
        , _bStreamDefaults( true )
        , _bBoundingVolumeSet( false )
        , _bUseDefaultLighting( true )
        , _bModelLightsUsed( false )
        , _bUseSilhouetteEdges( true )
        , _bSetEdgeColor( false )
        , _bOpenLocalLightAttributes( false )
        , _bSetNorthAngle( false )
        , _fNorthAngle( 90.0 )
        , _eUnits( DWFUnits::eMillimeters )
        , _eHandedness( eHandednessNone )
        , _eCompassState( eViewCubeCompassNone )
        , _nStyleSegments( 0 )
        , _zMIME( DWFMIME::kzMIMEType_W3D )
        , _pW3DFile( NULL )
        , _pW3DFileStream( NULL )
        , _pVersionBuffer( NULL )
        , _pW3DStreamWriter( NULL )
        , _pDefaultViewCamera( NULL )
        , _pInitialViewCamera( NULL )
        , _oBoundingVolume()
        , _oEmbeddedFonts()
        , _oResources()
        , _pPublishedObjectFactory( NULL )
        , _pContent( NULL )
        , _zTempID()
        , _eMetadataVersion( DWFPublisher::ePublishContentDefinition )
        , _nDisplayMode( 0 )
        , _pViewCubeHome( NULL )
		, _pFrontView (NULL)
{
    _anEdgeColor[0] = _anEdgeColor[1] = _anEdgeColor[2] = 0.0f;
    
    _anUnitsTransform[0] = _anUnitsTransform[1] = _anUnitsTransform[2] = _anUnitsTransform[3] = 0.0f;
    _anUnitsTransform[4] = _anUnitsTransform[5] = _anUnitsTransform[6] = _anUnitsTransform[7] = 0.0f;
    _anUnitsTransform[8] = _anUnitsTransform[9] = _anUnitsTransform[10] = _anUnitsTransform[11] = 0.0f;
    _anUnitsTransform[12] = _anUnitsTransform[13] = _anUnitsTransform[14] = _anUnitsTransform[15] = 0.0f;

    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );
    _oBoundingVolume.push_back( 0.0f );

    _pPublishedObjectFactory = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement::Factory );

}

_DWFTK_API
DWFModel::~DWFModel()
throw()
{
    if (_bOpen)
    {
        try { DWFModel::close(); } catch (...) {;}
    }

    DWFEmbeddedFont::tList::iterator iFont = _oEmbeddedFonts.begin();
    for (; iFont != _oEmbeddedFonts.end(); iFont++)
    {
        DWFCORE_FREE_OBJECT( *iFont );
    }

    DWFPublishableResource::tList::iterator iResource = _oResources.begin();
    for (; iResource != _oResources.end(); iResource++)
    {
        DWFCORE_FREE_OBJECT( *iResource );
    }

    if (_pPublishedObjectFactory)
    {
        DWFCORE_FREE_OBJECT( _pPublishedObjectFactory );
    }

    if (_pDefaultViewCamera)
    {
        DWFCORE_FREE_OBJECT( _pDefaultViewCamera );
    }

    if (_pInitialViewCamera)
    {
        DWFCORE_FREE_OBJECT( _pInitialViewCamera );
    }

    if (_pW3DStreamWriter)
    {
        DWFCORE_FREE_OBJECT( _pW3DStreamWriter );
    }

    if (_pW3DFile)
    {
        DWFCORE_FREE_OBJECT( _pW3DFile );
    }

    if (_pViewCubeHome)
    {
        DWFCORE_FREE_OBJECT( _pViewCubeHome );
    }

	if( _pFrontView )
	{
		 DWFCORE_FREE_OBJECT( _pFrontView );
	}
}

_DWFTK_API
void
DWFModel::open( DWFContent*                     pContent,
                tePolygonHandedness             ePolygonHandedness,
                DWFUnits::teType                eModelUnits,
                double*                         pTransform,
                bool                            bUseDefaultLighting,
                bool                            bUsePublishedEdges,
                bool                            bUseSilhouetteEdges,
                unsigned int                    nTargetW3DVersion,
                DWFPublisher::teMetadataVersion eMetadataVersion )
throw( DWFException )
{
#ifdef  _DWF_MODEL_ENABLE_LOGGING
    _oToolkit.OpenLogFile( /*NOXLATE*/"model_w3d.log", /*NOXLATE*/"w" );
    _oToolkit.SetLogging( true );
    _oToolkit.SetLoggingOptions( 0xFF );
#endif

    _bOpen = true;

    //
    // polygon handedness
    //
    _eHandedness = ePolygonHandedness;

    //
    // model graphics units 
    //
    _eUnits = eModelUnits;

        //
        //
        //
    if (pTransform)
    {
        DWFCORE_COPY_MEMORY( _anUnitsTransform, pTransform, sizeof(double)*16 );
    }
        //
        // identity
        //
    else
    {
        _anUnitsTransform[0] = 1.0;
        _anUnitsTransform[1] = 0.0;
        _anUnitsTransform[2] = 0.0;
        _anUnitsTransform[3] = 0.0;
        _anUnitsTransform[4] = 0.0;
        _anUnitsTransform[5] = 1.0;
        _anUnitsTransform[6] = 0.0;
        _anUnitsTransform[7] = 0.0;
        _anUnitsTransform[8] = 0.0;
        _anUnitsTransform[9] = 0.0;
        _anUnitsTransform[10] = 1.0;
        _anUnitsTransform[11] = 0.0;
        _anUnitsTransform[12] = 0.0;
        _anUnitsTransform[13] = 0.0;
        _anUnitsTransform[14] = 0.0;
        _anUnitsTransform[15] = 1.0;
    }

        //
        // create the stream writer if necessary
        //
    if (_pW3DStreamWriter == NULL)
    {
        //
        // first create a temporary file for this W3D
        //
        DWFString zTemplate( /*NOXLATE*/"model_w3d_" );
        _pW3DFile = DWFTempFile::Create( zTemplate, true );

        _pW3DStreamWriter = DWFCORE_ALLOC_OBJECT( W3DStreamWriter(_pW3DFile->getOutputStream(), _oToolkit) );

        if (_pW3DStreamWriter == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate W3D stream writer" );
        }
    }

    //
    // initialize the writer
    //
    _pW3DStreamWriter->open( nTargetW3DVersion );

    //
    // prepare the toolkit with default settings
    //
    _oToolkit.SetWriteFlags( TK_Full_Resolution             |   // full fidelity for normals and vertices
                             TK_Disable_Tristrips           |   // this must be explicitly turned on per shell
                             TK_Disable_Instancing          |   // instancing nor used
                             TK_Suppress_LOD                |   // disable this for now
                             TK_Connectivity_Compression );     // always enable - nice to have

    //
    // global quantization is always disabled by default due to the limitation of the hsf toolkit
    // to always correctly account of the world coordinates of the quantized shells
    //

    //
    // vertex coordinate quantization disabled by default
    // vertex normal quantization enabled by default
    //
    enableNormalsCompression();

    //
    // add the header to stream - this must be first
    // and it must be created explicitly, if you try
    // and use the one in the toolkit, you will get
    // an empty header
    //
    TK_Header* pHeader = new TK_Header;
    pHeader->setObserver( _pW3DStreamWriter );
    pHeader->serialize();
    delete pHeader;

    //
    // add the version comment
    //
    TK_Comment* pComment = new TK_Comment( _W3D_FILE_VERSION_STRING );
    pComment->setObserver( _pW3DStreamWriter );
    pComment->serialize();
    delete pComment;

    //
    // add the file information to the stream - 
    // again this must be created explicitly
    //
    TK_File_Info* pFileInfo = new TK_File_Info;
    pFileInfo->setObserver( _pW3DStreamWriter );
    pFileInfo->serialize();
    delete pFileInfo;

        //
        // Enable toolkit compression
        //
    if (_bCompressStream)
    {
        _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Start_Compression) );
    }

    if (_bCompressVertices)
    {
        enableVertexCompression();
    }


        //
        // Create the style segment that scene segments
        // containing published edges will reference
        //
    if (bUsePublishedEdges)
    {
        TK_Open_Segment& rHandler = getOpenSegmentHandler();
        rHandler.SetSegment( DWFStyleSegment::kz_StyleSegment_PublishedEdges );
        rHandler.serialize();

        getCloseSegmentHandler().serialize();
    }

    //
    // Model lighting preference
    //
    _bUseDefaultLighting = bUseDefaultLighting;

    //
    // Edges
    //
    _bUseSilhouetteEdges = bUseSilhouetteEdges;

    _pContent = pContent;

    if (eMetadataVersion != _eMetadataVersion)
    {
        _eMetadataVersion = eMetadataVersion;

        DWFCORE_FREE_OBJECT( _pPublishedObjectFactory );

        if (_eMetadataVersion == DWFPublisher::ePublishContentDefinition)
        {
            _pPublishedObjectFactory = DWFCORE_ALLOC_OBJECT( DWFPublishedContentElement::Factory );
        }
        else if (_eMetadataVersion == DWFPublisher::ePublishObjectDefinition)
        {
            _pPublishedObjectFactory = DWFCORE_ALLOC_OBJECT( DWFPublishedDefinedObject::Factory );
        }
        else
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unrecognized metadata version information" );
        }
    }

    if (_eMetadataVersion == DWFPublisher::ePublishContentDefinition)
    {
        if (_pContent)
        {
            _pContent = pContent;
            _zTempID = _pContent->getIDProvider()->next(true);
        }
        else
        {
            _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"A valid content is required for content definition" );
        }
    }
}

_DWFTK_API
unsigned int
DWFModel::close()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

        //
        // Disable toolkit compression
        //
    if (_bCompressStream)
    {
        _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Stop_Compression) );
    }

    //
    // add required terminating op-code to the stream
    //
    _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Termination) );

     //
    // finalize the writer
    //
    unsigned int nRequiredVersion = _pW3DStreamWriter->close();

    //
    // get the writer out of the picture for good
    //
    DWFCORE_FREE_OBJECT( _pW3DStreamWriter );

    //
    // get the input stream of the w3d file
    //
    _pW3DFileStream = _pW3DFile->getInputStream();

        //
        // here we surgically repair the w3d file with the minimum required version
        // we do this for maximum backwards compatability. a version requirement greater
        // than zero indicates the required version is less than the current format version
        // and can be downgraded.
        //
    if ( _eOutputMode == eBinary )
    {
        if (nRequiredVersion > 0)
        {
               //
               // allocate a buffer for the header opcode data
               //
             _pVersionBuffer = DWFCORE_ALLOC_MEMORY( char, 16 );
             if (_pVersionBuffer == NULL)
             {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate read buffer" );
             }

               //
               // read in enough to capture the stream version
               //
            if (16 != _pW3DFileStream->read(_pVersionBuffer, 16))
            {
                _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to read temporary file stream" );
            }

               //
               // "fix" the version
               //
            ::sprintf( (char*)&_pVersionBuffer[8], /*NOXLATE*/"%02d.%02d ", nRequiredVersion/100, nRequiredVersion%100 );
        }
    }
    _bOpen = false;

#ifdef  _DWF_MODEL_ENABLE_LOGGING
    _oToolkit.CloseLogFile();
#endif

    return nRequiredVersion;
}

_DWFTK_API 
void 
DWFModel::setW3DOutputMode( teOutputMode eMode )
throw()
{
    _eOutputMode = eMode ;

    switch ( _eOutputMode )
    {
        case eBinary:
            _oToolkit.SetAsciiMode( false );
            break;
        case eASCII:
            _oToolkit.SetAsciiMode( true );
            break;
    }
}

_DWFTK_API
void
DWFModel::enableW3DCompression( bool bEnableStreamCompression,
                                bool bEnableConnectivityCompression,
                                bool bEnableGlobalQuantizationCompression )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

        //
        // toggle LZ stream compression with op-code
        //
    if (_bCompressStream && (bEnableStreamCompression == false))
    {
        //
        // turn off current compression
        //
        _bCompressStream = false;
        _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Stop_Compression) );
    }
    else if ((_bCompressStream == false) && bEnableStreamCompression)
    {
        //
        // turn on compression
        //
        _bCompressStream = true;
        _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Start_Compression) );
    }

        //
        // toggle connectivity compression with toolkit flag
        //
    if (bEnableConnectivityCompression)
    {
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() | TK_Connectivity_Compression );
    }
    else
    {
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() & ~TK_Connectivity_Compression );
    }

        //
        // toggle global quantization with toolkit flag
        //
    if (bEnableGlobalQuantizationCompression)
    {
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() | TK_Global_Quantization );
    }
    else
    {
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() & ~TK_Global_Quantization );
    }
}

_DWFTK_API
void
DWFModel::enableNormalsCompression( char nTotalNormalBits )
throw()
{
    if (nTotalNormalBits == keNormal_FullResolution)
    {
        //
        // switch to full resolution normals
        //
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() | TK_Full_Resolution_Normals );
    }
    else
    {
        //
        // disable full resolution normals
        //
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() & ~TK_Full_Resolution_Normals );

            //
            // upper quantization bit depth
            //
        if (nTotalNormalBits > keNormal_MaximumResolution)
        {
            nTotalNormalBits = keNormal_MaximumResolution;
        }

        //
        // set compression for vertex normals (i,j,k)
        //
        _oToolkit.SetNumNormalBits( (int)nTotalNormalBits );
    }
}

_DWFTK_API
void
DWFModel::enableVertexCompression( char nTotalVertexBits,
                                   char nTotalParameterBits )
throw()
{
    if (nTotalVertexBits == keVertex_FullResolution)
    {
        //
        // switch to full resolution vertices
        //
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() | TK_Full_Resolution_Vertices );
        _bCompressVertices = false;
    }
    else
    {
        //
        // disable full resolution vertices
        //
        _oToolkit.SetWriteFlags( _oToolkit.GetWriteFlags() & ~TK_Full_Resolution_Vertices );
        _bCompressVertices = true;

            //
            // upper quantization bit depth
            //
        if (nTotalVertexBits > keVertex_MaximumResolution)
        {
            nTotalVertexBits = keVertex_MaximumResolution;
        }

        //
        // set compression for vertex localtions (x,y,z)
        //
        _oToolkit.SetNumVertexBits( (int)nTotalVertexBits );

            //
            // if requested, compress texture parameter coordinates (u,v,w)
            //
        if (nTotalParameterBits != keTextureParameter_FullResolution)
        {
                //
                // upper quantization bit depth
                //
            if (nTotalVertexBits > keTextureParameter_MaximumResolution)
            {
                nTotalVertexBits = keTextureParameter_MaximumResolution;
            }

            _oToolkit.SetNumParameterBits( (int)nTotalParameterBits );
        }
    }
}

_DWFTK_API
void
DWFModel::disableAllCompression()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

        //
        // turn off LZ
        //
    if (_bCompressStream)
    {
        _bCompressStream = false;
        _pW3DStreamWriter->notify( _oToolkit.GetOpcodeHandler(TKE_Stop_Compression) );
    }

    //
    // turn off...
    //
    int nFlags = (_oToolkit.GetWriteFlags() & ~(TK_Connectivity_Compression | TK_Global_Quantization));

    //
    // turn on...
    //
    nFlags |= (TK_Full_Resolution | TK_Disable_Global_Compression);
    _bCompressVertices = false;
    _oToolkit.SetWriteFlags( nFlags );
}

_DWFTK_API
void
DWFModel::publish( DWFPublisher& rPublisher )
throw( DWFException )
{
    //
    //  If we are using a DWF6PackagePublisher or DWFXPackagePublisher, ensure
    //  its version is set before doing anything
    //
    DWF6PackagePublisher* pDWF6Publisher = dynamic_cast<DWF6PackagePublisher*>(&rPublisher);
    if (pDWF6Publisher)
    {
        pDWF6Publisher->setVersion( _eMetadataVersion );

        if (_pContent != NULL)
        {
            DWFContentManager* pManager = _pContent->getContentManager();
            pDWF6Publisher->attachContentManager( pManager, pManager->owner() == NULL );
        }
    }
    else
    {
        DWFXPackagePublisher* pDWFXPublisher = dynamic_cast<DWFXPackagePublisher*>(&rPublisher);
        if (pDWFXPublisher)
        {
            pDWFXPublisher->setVersion( _eMetadataVersion );

            if (_pContent != NULL)
            {
                DWFContentManager* pManager = _pContent->getContentManager();
                pDWFXPublisher->attachContentManager( pManager, pManager->owner() == NULL );
            }
        }
    }

    rPublisher.preprocess( this );    

    if (_eMetadataVersion == DWFPublisher::ePublishContentDefinition)
    {
        _visitPublishedContentElements( rPublisher );
    }
    else
    {
        _visitPublishedObjects( rPublisher );
    }

        //
        // publish fonts
        //
    DWFEmbeddedFontVisitor* pFontVisitor = rPublisher.getEmbeddedFontVisitor();

    if (pFontVisitor)
    {
        unsigned long i;

            //
            // visit each embedded font
            //
        for (i=0; i<_oEmbeddedFonts.size(); i++)
        {
            pFontVisitor->visitEmbeddedFont( *(_oEmbeddedFonts[i]) );
        }
    }

    //
    //
    //
    rPublisher.postprocess( this );
}

_DWFTK_API
void 
DWFModel::_visitPublishedContentElements( DWFPublisher& rPublisher )
throw( DWFException )
{
    DWFPublishedContentElement::Factory* pFactory = 
        dynamic_cast<DWFPublishedContentElement::Factory*>( _pPublishedObjectFactory );
    if (pFactory == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The published object factory type is incorrect" );
    }

    DWFPublishedObject::Visitor* pObjectVisitor = rPublisher.getPublishedObjectVisitor();

    //
    // walk the object tree with the visitors provided
    //
    DWFPublishedContentElement::tMap::Iterator* piObject = pFactory->getPublishedObjects();

    if (piObject)
    {
        for (; piObject->valid(); piObject->next())
        {
            DWFPublishedContentElement* pElement = piObject->value();

            _ensurePublishedContentElementIndex( pElement );

            if (pObjectVisitor)
            {
                pElement->accept( *pObjectVisitor );
            }
        }

        DWFCORE_FREE_OBJECT( piObject );
    }
}

void
DWFModel::_ensurePublishedContentElementIndex( DWFPublishedContentElement* pElement )
throw()
{
    DWFPublishedObject::tIndex  iObj = -1;

    //
    // resolve the scene segment index from the key
    //
    _oToolkit.KeyToIndex( (ID_Key)pElement->key(), iObj );

    //
    // assign the scene index to the object
    //
    pElement->setIndex( iObj );

    //
    // resolve the scene segment indices from the reference keys
    //

    DWFPublishedObject::tReference* pRef = pElement->getNamedSegmentIncludeRef();
    if (pRef)
    {
        _oToolkit.KeyToIndex( (ID_Key)pRef->nKey, iObj );
        pRef->nIndex = iObj;

        DWFPublishedContentElement* pRefElem = dynamic_cast<DWFPublishedContentElement*>(pRef->pObj);
        if (pRefElem)
        {
            _ensurePublishedContentElementIndex( pRefElem );
        }
    }

    DWFPublishedObject::tReferenceList& rRefs = pElement->references();
    DWFPublishedObject::tReferenceList::iterator iRef = rRefs.begin();
    for (; iRef != rRefs.end(); ++iRef)
    {
        pRef = *iRef;

        _oToolkit.KeyToIndex( (ID_Key)pRef->nKey, iObj );
        pRef->nIndex = iObj;

        DWFPublishedContentElement* pRefElem = dynamic_cast<DWFPublishedContentElement*>(pRef->pObj);
        if (pRefElem)
        {
            _ensurePublishedContentElementIndex( pRefElem );
        }
    }
}


_DWFTK_API
void
DWFModel::_visitPublishedObjects( DWFPublisher& rPublisher )
throw( DWFException )
{
    unsigned int                iRef = 0;
    DWFPublishedObject*         pObj = NULL;
    DWFPublishedObject::tIndex  iObj = 0;

    DWFPublishedObject::Visitor*    pObjectVisitor = rPublisher.getPublishedObjectVisitor();
    DWFPropertyVisitor*             pPropertyVisitor = rPublisher.getPropertyVisitor();

    DWFPublishedDefinedObject::Factory* pFactory = 
        dynamic_cast<DWFPublishedDefinedObject::Factory*>( _pPublishedObjectFactory );
    if (pFactory == NULL)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The published object factory type is incorrect" );
    }

    //
    // walk the object tree with the visitors provided
    //
    DWFPublishedDefinedObject::tMap::Iterator* piObject = pFactory->getPublishedObjects();

    if (piObject)
    {
        for (; piObject->valid(); piObject->next())
        {
            iObj = -1;
            pObj = piObject->value();

            //
            // resolve the scene segment index from the key
            //
            _oToolkit.KeyToIndex( (ID_Key)pObj->key(), iObj );

            //
            // assign the scene index to the object
            //
            pObj->setIndex( iObj );

            //
            // resolve the scene segment indices from the reference keys
            //
            DWFPublishedObject::tReference* pRef = NULL;
            DWFPublishedObject::tReferenceList& rRefs = pObj->references();
            for (iRef = 0; iRef < rRefs.size(); iRef++)
            {
                pRef = rRefs[iRef];

                _oToolkit.KeyToIndex( (ID_Key)pRef->nKey, iObj );
                pRef->nIndex = iObj;
            }

                //
                // the publish object visitor will publish any structure
                // derived from this graph
                //
            if (pObjectVisitor)
            {
                pObj->accept( *pObjectVisitor );
            }

                //
                // properties are available on the published object
                // however, we cannot make assumptions about the nature
                // of how the published objects are processed.
                // so the object is provided with a visitor to interact with
                //
            if (pPropertyVisitor)
            {
                pObj->accept( *pPropertyVisitor );
            }
        }

        DWFCORE_FREE_OBJECT( piObject );
    }
}


_DWFTK_API
DWFSegment
DWFModel::openSegment()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    //
    // once a segment is opened in the model,
    // the default (view, bounds, etc) characteristics
    // are not writen to the W3D steam put only to the XML
    //
    _bStreamDefaults = false;

    bool bUseOldObjectModel = (_eMetadataVersion==DWFPublisher::ePublishObjectDefinition);

    return DWFSegment( *this,                           // SegmentBuilder
                       *this,                           // GeometryBuilder
                       *this,                           // FeatureBuilder
                       *this,                           // AttributeBuilder
                       *_pPublishedObjectFactory,       // PublishedObject Factory
                       NULL,                            // No parents
                       bUseOldObjectModel );
}

_DWFTK_API
DWFIncludeSegment
DWFModel::openIncludeSegment()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    //
    // once a segment is opened in the model,
    // the default (view, bounds, etc) characteristics
    // are not writen to the W3D steam put only to the XML
    //
    _bStreamDefaults = false;

    bool bUseOldObjectModel = (_eMetadataVersion==DWFPublisher::ePublishObjectDefinition);

    return DWFIncludeSegment( *this,                            // SegmentBuilder
                              *this,                            // GeometryBuilder
                              *this,                            // FeatureBuilder
                              *this,                            // AttributeBuilder
                              *_pPublishedObjectFactory,        // PublishedObject Factory
                              bUseOldObjectModel );
}

_DWFTK_API
DWFStyleSegment
DWFModel::openStyleSegment()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    //
    // once a segment is opened in the model,
    // the default (view, bounds, etc) characteristics
    // are not writen to the W3D steam put only to the XML
    //
    _bStreamDefaults = false;

    return DWFStyleSegment( *this,                 // SegmentBuilder
                            *this,                 // AttributeBuilder
                            _nStyleSegments++ );
}

_DWFTK_API
void
DWFModel::createView( const char*   zName,
                      W3DCamera&    rViewCamera )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    if (_bStreamDefaults)
    {
        float anValues[3] = {0.0, 0.0, 0.0};

        TK_Camera& rCamera = getViewHandler();

        rCamera.SetView( zName );
        rCamera.SetProjection( rViewCamera.getProjection() );
        rCamera.SetPosition( rViewCamera.getPosition(anValues) );
        rCamera.SetTarget( rViewCamera.getTarget(anValues) );
        rCamera.SetUpVector( rViewCamera.getUpVector(anValues) );
        rCamera.SetField( rViewCamera.getField(anValues) );

        rCamera.serialize();
    }
        //
        // cache this information for later publishing
        // only worried about one (default) camera for now
        //
    else
    {
        if (_pDefaultViewCamera == NULL)
        {
            _pDefaultViewCamera = DWFCORE_ALLOC_OBJECT( W3DCamera );
            if (_pDefaultViewCamera == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for camera" );
            }
        }

        *_pDefaultViewCamera = rViewCamera;
    }
}

_DWFTK_API
void
DWFModel::createView( teViewType        eCameraType,
                      const DWFString&  zName,
                      W3DCamera&        rViewCamera )
throw( DWFException )
{
    switch( eCameraType )
    {
    case eInitialView:
        createView( /*NOXLATE*/"default", rViewCamera );

        //
        // cache this view for serialization into the views presentation
        //
        if (_pInitialViewCamera == NULL)
        {
            _pInitialViewCamera = DWFCORE_ALLOC_OBJECT( W3DCamera );
            if (_pInitialViewCamera == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for camera" );
            }
        }
        *_pInitialViewCamera = rViewCamera;
        _zDefaultViewName = zName;
        break;

    case eViewCubeHome:
        if (_pViewCubeHome == NULL)
        {
            _pViewCubeHome = DWFCORE_ALLOC_OBJECT( W3DCamera );
            if (_pViewCubeHome == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for camera" );
            }
        }

        *_pViewCubeHome = rViewCamera;
        _zViewCubeHomeName = zName;
        break;

	case eFrontView:
		if(!_pFrontView)
		{
			 _pFrontView = DWFCORE_ALLOC_OBJECT( W3DCamera );
            if (_pFrontView == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory for camera" );
            }
		}
		*_pFrontView = rViewCamera;
        _zFrontViewName = zName;
        break;

    default:
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Unknown view type specified" );
    }
}

_DWFTK_API
void
DWFModel::setBoundingCube( float nMinX,   float nMinY,    float nMinZ,
                           float nMaxX,   float nMaxY,    float nMaxZ )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    if (_bStreamDefaults)
    {
        float anCube[6] = { nMinX, nMinY, nMinZ,
                            nMaxX, nMaxY, nMaxZ };

        //
        // this will invoke the SetBoundingxxx() method in the toolkit
        //
        TK_Bounding* pHandler = new TK_Bounding( TKE_Bounding_Info, anCube, &anCube[3] );

        _pW3DStreamWriter->notify( pHandler );

        delete pHandler;

        _bBoundingVolumeSet = true;
    }

    _oBoundingVolume.push_back( nMinX );
    _oBoundingVolume.push_back( nMinY );
    _oBoundingVolume.push_back( nMinZ );
    _oBoundingVolume.push_back( nMaxX );
    _oBoundingVolume.push_back( nMaxY );
    _oBoundingVolume.push_back( nMaxZ );
}

_DWFTK_API
void
DWFModel::setBoundingSphere( float nCenterX, float nCenterY, float nCenterZ,
                             float nRadius )
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    if (_bStreamDefaults)
    {
        float anPoint[3] = { nCenterX, nCenterY, nCenterZ };

        //
        // this will invoke the SetBoundingxxx() method in the toolkit
        //
        TK_Bounding* pHandler = new TK_Bounding( TKE_Bounding_Info, anPoint, nRadius );

        _pW3DStreamWriter->notify( pHandler );

        delete pHandler;

        _bBoundingVolumeSet = true;
    }

    _oBoundingVolume.push_back( nCenterX );
    _oBoundingVolume.push_back( nCenterY );
    _oBoundingVolume.push_back( nCenterZ );
    _oBoundingVolume.push_back( nRadius );
}

_DWFTK_API
void
DWFModel::setEdgeColor( float anRGB[3] )
throw()
{
    _bSetEdgeColor = true;
    _anEdgeColor[0] = anRGB[0];
    _anEdgeColor[1] = anRGB[1];
    _anEdgeColor[2] = anRGB[2];
}

_DWFTK_API
void
DWFModel::setViewCubeCompassState( teViewCubeCompassState eCompassState )
throw()
{
    _eCompassState = eCompassState;
}

_DWFTK_API
void
DWFModel::setViewCubeNorthAngle( float fAngle )
throw()
{
    _bSetNorthAngle = true;
    _fNorthAngle = fAngle;
}

_DWFTK_API
const W3DCamera*
DWFModel::getDefaultView()
const
throw()
{
    return _pDefaultViewCamera;
}

_DWFTK_API
bool
DWFModel::getDefinedView( teViewType teViewType, DWFString& zName, W3DCamera& rView )
const
throw( DWFException )
{
    switch( teViewType )
    {
    case eInitialView:
        //
        // if no default view was defined, return false
        //
        if (_pInitialViewCamera == NULL)
        {
            return false;
        }

        zName = _zDefaultViewName;
        rView = *_pInitialViewCamera;
        break;

    case eViewCubeHome:
        //
        // if no view cube home was defined, return false
        //
        if (_pViewCubeHome == NULL)
        {
            return false;
        }

        zName = _zViewCubeHomeName;
        rView = *_pViewCubeHome;
        break;

	case eFrontView:
		if(_pFrontView == NULL)
		{
			return false;
		}
		zName = _zFrontViewName;
		rView = *_pFrontView;
		break;

    default:
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid view type specified" );
        break;
    }

    return true;
}

_DWFTK_API
void
DWFModel::getBoundingVolume( tDoubleList& rBounds )
const
throw()
{
    rBounds.clear();

    size_t n = 0;
    for (; n < _oBoundingVolume.size(); n++)
    {
        rBounds.push_back( _oBoundingVolume[n] );
    }
}

_DWFTK_API
bool
DWFModel::useModelLighting()
const
throw()
{
    return (_bUseDefaultLighting || (!_bUseDefaultLighting && !_bModelLightsUsed));
}

_DWFTK_API
bool
DWFModel::useSilhouetteEdges() const
throw()
{
    return _bUseSilhouetteEdges;
}

_DWFTK_API
const float* const
DWFModel::getEdgeColor() const
throw()
{
    return (_bSetEdgeColor ? _anEdgeColor : NULL);
}

_DWFTK_API
const float* const
DWFModel::getViewCubeAngleOfNorth() const
throw()
{
    return (_bSetNorthAngle ? &_fNorthAngle : NULL);
}

_DWFTK_API
DWFUnits::teType
DWFModel::getUnits()
const
throw()
{
    return _eUnits;
}

_DWFTK_API
void
DWFModel::getTransform( double* pTransform )
const
throw()
{
    if (pTransform)
    {
        DWFCORE_COPY_MEMORY( pTransform, _anUnitsTransform, sizeof(double)*16 );
    }
}

_DWFTK_API
DWFModel::tePolygonHandedness
DWFModel::getPolygonHandedness()
const
throw()
{
    return _eHandedness;
}

_DWFTK_API
DWFModel::teViewCubeCompassState 
DWFModel::getViewCubeCompassState()
const
throw()
{
    return _eCompassState;
}

_DWFTK_API
void
DWFModel::setDisplayMode( unsigned int nDisplayMode )
throw()
{
    _nDisplayMode = nDisplayMode;
}

_DWFTK_API
unsigned int
DWFModel::getDisplayMode() const
throw()
{
    return _nDisplayMode;
}

_DWFTK_API
const DWFString&
DWFModel::getMIMEType()
throw( DWFException )
{
    return _zMIME;
}

_DWFTK_API
DWFInputStream*
DWFModel::getInputStream()
throw( DWFException )
{
        //
        // this should not have been called...
        //
    if (_pW3DFileStream == NULL)
    {
        _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"No input stream available for the model" );
    }
        //
        // if there is no version buffer, we can just return the file stream
        //
    else if (_pVersionBuffer == NULL)
    {
        return _pW3DFileStream;
    }
    //
        // we have both a version buffer and the rest of the w3d stream
        // so build a special buffer stream and return that
    //
    else 
    {
        return DWFCORE_ALLOC_OBJECT( _SpecialBufferedInputStream(_pVersionBuffer, 16, _pW3DFileStream) );
    } 
}

_DWFTK_API
void
DWFModel::embedFont( DWFEmbeddedFont* pFont )
throw( DWFException )
{
    _oEmbeddedFonts.push_back( pFont );
}

_DWFTK_API
void
DWFModel::addResource( DWFPublishableResource* pResource )
throw( DWFException )
{
    _oResources.push_back( pResource );

        //
        // Images for textures require a little bit more work...
        //
    DWFTexture* pTexture = dynamic_cast<DWFTexture*>(pResource);
    if (pTexture)
    {
        DWFPointer<W3D_Image> apHandler( DWFCORE_ALLOC_OBJECT(W3D_Image), false );
                              apHandler->setObserver( _pW3DStreamWriter );

        const DWFString& zName = pTexture->name();
        size_t nChars = zName.chars();

        if (nChars > 0)
        {
            char* pUTF8Name = NULL;
            zName.getUTF8( &pUTF8Name );

            apHandler->SetName( pUTF8Name );

            DWFCORE_FREE_MEMORY( pUTF8Name );
        }
        else
        {
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Texture images must be named" );
        }

        apHandler->SetSize( (int)pTexture->width(), (int)pTexture->height() );
        apHandler->SetBitDepth( (char)pTexture->depth() );
        
        apHandler->serialize();
    }
}

_DWFTK_API
DWFIterator<DWFPublishableResource*>*
DWFModel::getResources()
throw( DWFException )
{
    return DWFCORE_ALLOC_OBJECT( ResourceIterator(_oResources) );
}

DWFPublishedObject*
DWFModel::makePublishedObject( DWFPublishedObject::tKey nKey,
                               const DWFString&         zName )
throw( DWFException )
{
    ////
    //// allocate the object
    ////
    //DWFPublishedObject* pObj = DWFCORE_ALLOC_OBJECT( DWFPublishedObject(nKey, zName) );
    //
    //if (pObj == NULL)
    //{
    //    _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate object" );
    //}

    ////
    //// when this container is destroyed, it will
    //// take these objects down with it
    ////
    //_oPublishedObjects.insert( nKey, pObj );

    //return pObj;
    return _pPublishedObjectFactory->makePublishedObject( nKey, zName );
}

DWFPublishedObject&
DWFModel::findPublishedObject( DWFPublishedObject::tKey nKey )
throw( DWFException )
{
    //DWFPublishedObject** ppObject = _oPublishedObjects.find( nKey );

    //if (ppObject == NULL)
    //{
    //    _DWFCORE_THROW( DWFDoesNotExistException, L"Object not found" );
    //}

    //return **ppObject;
    return _pPublishedObjectFactory->findPublishedObject( nKey );
}

TK_Close_Segment&
DWFModel::getCloseSegmentHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Close_Segment* pHandler = (TK_Close_Segment*)_oToolkit.GetOpcodeHandler( TKE_Close_Segment );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Open_Segment&
DWFModel::getOpenSegmentHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Open_Segment* pHandler = (TK_Open_Segment*)_oToolkit.GetOpcodeHandler( TKE_Open_Segment );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Referenced_Segment&
DWFModel::getIncludeSegmentHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Referenced_Segment* pHandler = (TK_Referenced_Segment*)_oToolkit.GetOpcodeHandler( TKE_Include_Segment );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Referenced_Segment&
DWFModel::getStyleSegmentHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Referenced_Segment* pHandler = (TK_Referenced_Segment*)_oToolkit.GetOpcodeHandler( TKE_Style_Segment );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Area_Light&
DWFModel::getAreaLightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Area_Light* pHandler = (TK_Area_Light*)_oToolkit.GetOpcodeHandler( TKE_Area_Light );
    pHandler->setObserver( _pW3DStreamWriter );

    _bModelLightsUsed = true;

    return *pHandler;
}

TK_Circle&
DWFModel::getCircleHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Circle* pHandler = (TK_Circle*)_oToolkit.GetOpcodeHandler( TKE_Circle );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Circle&
DWFModel::getCircularArcHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Circle* pHandler = (TK_Circle*)_oToolkit.GetOpcodeHandler( TKE_Circular_Arc );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Circle&
DWFModel::getCircularChordHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Circle* pHandler = (TK_Circle*)_oToolkit.GetOpcodeHandler( TKE_Circular_Chord );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Circle&
DWFModel::getCircularWedgeHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Circle* pHandler = (TK_Circle*)_oToolkit.GetOpcodeHandler( TKE_Circular_Wedge );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Cutting_Plane&
DWFModel::getCuttingPlaneHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Cutting_Plane* pHandler = (TK_Cutting_Plane*)_oToolkit.GetOpcodeHandler( TKE_Cutting_Plane );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Cylinder&
DWFModel::getCylinderHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Cylinder* pHandler = (TK_Cylinder*)_oToolkit.GetOpcodeHandler( TKE_Cylinder );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Point&
DWFModel::getDistantLightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Point* pHandler = (TK_Point*)_oToolkit.GetOpcodeHandler( TKE_Distant_Light );
    pHandler->setObserver( _pW3DStreamWriter );

    _bModelLightsUsed = true;

    return *pHandler;
}

TK_Ellipse&
DWFModel::getEllipseHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Ellipse* pHandler = (TK_Ellipse*)_oToolkit.GetOpcodeHandler( TKE_Ellipse );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Ellipse&
DWFModel::getEllipticalArcHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Ellipse* pHandler = (TK_Ellipse*)_oToolkit.GetOpcodeHandler( TKE_Elliptical_Arc );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Glyph_Definition&
DWFModel::getGlyphDefinitionHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Glyph_Definition* pHandler = (TK_Glyph_Definition*)_oToolkit.GetOpcodeHandler( TKE_Glyph_Definition );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Grid&
DWFModel::getGridHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Grid* pHandler = (TK_Grid*)_oToolkit.GetOpcodeHandler( TKE_Grid );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Image&
DWFModel::getImageHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Image* pHandler = (TK_Image*)_oToolkit.GetOpcodeHandler( TKE_Image );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Line&
DWFModel::getLineHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Line* pHandler = (TK_Line*)_oToolkit.GetOpcodeHandler( TKE_Line );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Point&
DWFModel::getLocalLightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Point* pHandler = (TK_Point*)_oToolkit.GetOpcodeHandler( TKE_Local_Light );
    pHandler->setObserver( _pW3DStreamWriter );

    _bModelLightsUsed = true;

    return *pHandler;
}

TK_Point&
DWFModel::getMarkerHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Point* pHandler = (TK_Point*)_oToolkit.GetOpcodeHandler( TKE_Marker );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Mesh&
DWFModel::getMeshHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Mesh* pHandler = (TK_Mesh*)_oToolkit.GetOpcodeHandler( TKE_Mesh );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_NURBS_Curve&
DWFModel::getNURBSCurveHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_NURBS_Curve* pHandler = (TK_NURBS_Curve*)_oToolkit.GetOpcodeHandler( TKE_NURBS_Curve );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}


TK_NURBS_Surface&
DWFModel::getNURBSSurfaceHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_NURBS_Surface* pHandler = (TK_NURBS_Surface*)_oToolkit.GetOpcodeHandler( TKE_NURBS_Surface );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_PolyCylinder&
DWFModel::getPolyCylinderHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_PolyCylinder* pHandler = (TK_PolyCylinder*)_oToolkit.GetOpcodeHandler( TKE_PolyCylinder );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Polypoint&
DWFModel::getPolygonHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Polypoint* pHandler = (TK_Polypoint*)_oToolkit.GetOpcodeHandler( TKE_Polygon );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Polypoint&
DWFModel::getPolylineHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Polypoint* pHandler = (TK_Polypoint*)_oToolkit.GetOpcodeHandler( TKE_Polyline );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Shell&
DWFModel::getShellHandler( bool bTriStripsOnly,
                           bool bDisableOptimization )
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Shell* pHandler = (TK_Shell*)_oToolkit.GetOpcodeHandler( TKE_Shell );
    pHandler->setObserver( _pW3DStreamWriter );

    //
    // adjust compression options on the shell
    // based on the current model settings
    //
    pHandler->InitSubop( _oToolkit, 0, true );

        //
        // data organization - if tri-strips aren't actually used
        // setting this flag will result in garbage data
        //
    if (bTriStripsOnly)
    {
        pHandler->SetSubop( (unsigned char)(pHandler->GetSubop() | TK_Polyhedron::TKSH_TRISTRIPS) );
    }

    //
    // duplicate vertex and degenerate face optimization
    //
    pHandler->optimize( !bDisableOptimization );

    return *pHandler;
}

TK_Sphere&
DWFModel::getSphereHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Sphere* pHandler = (TK_Sphere*)_oToolkit.GetOpcodeHandler( TKE_Sphere );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Spot_Light&
DWFModel::getSpotLightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Spot_Light* pHandler = (TK_Spot_Light*)_oToolkit.GetOpcodeHandler( TKE_Spot_Light );
    pHandler->setObserver( _pW3DStreamWriter );

    _bModelLightsUsed = true;

    return *pHandler;
}

TK_Text&
DWFModel::getTextHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Text* pHandler = (TK_Text*)_oToolkit.GetOpcodeHandler( TKE_Text );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Text&
DWFModel::getTextWithEncodingHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Text* pHandler = (TK_Text*)_oToolkit.GetOpcodeHandler( TKE_Text_With_Encoding );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Texture&
DWFModel::getTextureHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Texture* pHandler = (TK_Texture*)_oToolkit.GetOpcodeHandler( TKE_Texture );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Matrix&
DWFModel::getTextureMatrixHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Matrix* pHandler = (TK_Matrix*)_oToolkit.GetOpcodeHandler( TKE_Texture_Matrix );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Camera&
DWFModel::getCameraHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Camera* pHandler = (TK_Camera*)_oToolkit.GetOpcodeHandler( TKE_Camera );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Color&
DWFModel::getColorHandler()
throw( DWFException )
{
    //
    // This is the only handler that doesn't check _bOpenLocalLightAttributes (intentionally).
    //

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Color* pHandler = (TK_Color*)_oToolkit.GetOpcodeHandler( TKE_Color );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Color_Map&
DWFModel::getColorMapHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Color_Map* pHandler = (TK_Color_Map*)_oToolkit.GetOpcodeHandler( TKE_Color_Map );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Color_RGB&
DWFModel::getColorRGBHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Color_RGB* pHandler = (TK_Color_RGB*)_oToolkit.GetOpcodeHandler( TKE_Color_RGB );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Named&
DWFModel::getEdgePatternHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Named* pHandler = (TK_Named*)_oToolkit.GetOpcodeHandler( TKE_Edge_Pattern );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Size&
DWFModel::getEdgeWeightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Size* pHandler = (TK_Size*)_oToolkit.GetOpcodeHandler( TKE_Edge_Weight );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getFacePatternHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Face_Pattern );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getHandednessHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Handedness );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Heuristics&
DWFModel::getHeuristicsHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Heuristics* pHandler = (TK_Heuristics*)_oToolkit.GetOpcodeHandler( TKE_Heuristics );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Named&
DWFModel::getLinePatternHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Named* pHandler = (TK_Named*)_oToolkit.GetOpcodeHandler( TKE_Line_Pattern );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Line_Style&
DWFModel::getLineStyleHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Line_Style* pHandler = (TK_Line_Style*)_oToolkit.GetOpcodeHandler( TKE_Line_Style );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Size&
DWFModel::getLineWeightHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Size* pHandler = (TK_Size*)_oToolkit.GetOpcodeHandler( TKE_Line_Weight );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Size&
DWFModel::getMarkerSizeHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Size* pHandler = (TK_Size*)_oToolkit.GetOpcodeHandler( TKE_Marker_Size );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getMarkerSymbolHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Marker_Symbol );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Matrix&
DWFModel::getModellingMatrixHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Matrix* pHandler = (TK_Matrix*)_oToolkit.GetOpcodeHandler( TKE_Modelling_Matrix );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Rendering_Options&
DWFModel::getRenderingOptionsHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Rendering_Options* pHandler = (TK_Rendering_Options*)_oToolkit.GetOpcodeHandler( TKE_Rendering_Options );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Selectability&
DWFModel::getSelectabilityHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Selectability* pHandler = (TK_Selectability*)_oToolkit.GetOpcodeHandler( TKE_Selectability );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getTextAlignmentHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Text_Alignment );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Text_Font&
DWFModel::getTextFontHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Text_Font* pHandler = (TK_Text_Font*)_oToolkit.GetOpcodeHandler( TKE_Text_Font );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Point&
DWFModel::getTextPathHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Point* pHandler = (TK_Point*)_oToolkit.GetOpcodeHandler( TKE_Text_Path );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Size&
DWFModel::getTextSpacingHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Size* pHandler = (TK_Size*)_oToolkit.GetOpcodeHandler( TKE_Text_Spacing );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_User_Options&
DWFModel::getUserOptionsHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_User_Options* pHandler = (TK_User_Options*)_oToolkit.GetOpcodeHandler( TKE_User_Options );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Unicode_Options&
DWFModel::getUnicodeOptionsHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Unicode_Options* pHandler = (TK_Unicode_Options*)_oToolkit.GetOpcodeHandler( TKE_Unicode_Options );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Visibility&
DWFModel::getVisibilityHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Visibility* pHandler = (TK_Visibility*)_oToolkit.GetOpcodeHandler( TKE_Visibility );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Camera&
DWFModel::getViewHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Camera* pHandler = (TK_Camera*)_oToolkit.GetOpcodeHandler( TKE_View );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Window&
DWFModel::getWindowHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Window* pHandler = (TK_Window*)_oToolkit.GetOpcodeHandler( TKE_Window );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getWindowFrameHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Window_Frame );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}

TK_Enumerated&
DWFModel::getWindowPatternHandler()
throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Must first close local lighting attribute" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Model must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Window_Pattern );
    pHandler->setObserver( _pW3DStreamWriter );

    return *pHandler;
}


void
DWFModel::openLocalLightAttributes()
    throw( DWFException )
{
    if(_bOpenLocalLightAttributes == true)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Local lighting attribute already open" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    TK_Enumerated* pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Geometry_Attributes );
    pHandler->setObserver( _pW3DStreamWriter );
    pHandler->serialize();

    _bOpenLocalLightAttributes = true;
}

void
DWFModel::closeLocalLightAttributes()
    throw( DWFException )
{
    if(_bOpenLocalLightAttributes == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Local lighting attribute not open" );
    }

    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

	TK_Enumerated* pHandler = NULL;
	if(getW3DOutputMode() == DWFModel::eBinary)
	{
		pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Termination );
	}
	else	if(getW3DOutputMode() == DWFModel::eASCII)
	{
		pHandler = (TK_Enumerated*)_oToolkit.GetOpcodeHandler( TKE_Close_Geometry_Attributes );
	}

	if(pHandler != NULL)
	{
		pHandler->setObserver( _pW3DStreamWriter );
		pHandler->serialize();
		_bOpenLocalLightAttributes = false;
	}
}


///
///
///

DWFModel::_SpecialBufferedInputStream::_SpecialBufferedInputStream( char*           pBuffer,
                                                                    unsigned int    nBufferBytes,
                                                                    DWFInputStream* pStream )
throw()
        : _nBufferBytes( nBufferBytes )
        , _nBufferBytesRead( 0 )
        , _pBuffer( pBuffer )
        , _pStream( pStream )
{
    ;
}

DWFModel::_SpecialBufferedInputStream::~_SpecialBufferedInputStream()
throw()
{
    if (_pBuffer)
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
    }

    if (_pStream)
    {
        DWFCORE_FREE_OBJECT( _pStream );
    }
}

size_t
DWFModel::_SpecialBufferedInputStream::available() const
throw( DWFException )
{
    return (_pStream->available() - (_nBufferBytesRead - _nBufferBytes));
}

size_t
DWFModel::_SpecialBufferedInputStream::read( void*  pBuffer,
                                             size_t nBytesToRead )
throw( DWFException )
{
    size_t nBytesRead = 0;
    char* pOut = (char*)pBuffer;
    char* pIn = _pBuffer;

    while ((_nBufferBytesRead < _nBufferBytes) && (nBytesRead < nBytesToRead))
    {
        *pOut = *pIn;

        pOut++;
        pIn++;
        _nBufferBytesRead++;
        nBytesRead++;
    }

    if (nBytesRead < nBytesToRead)
    {
        nBytesRead += _pStream->read( pOut, (nBytesToRead - nBytesRead) );
    }

    return nBytesRead;
}

off_t
DWFModel::_SpecialBufferedInputStream::seek( int    eOrigin,
                                             off_t  nOffset )
throw( DWFException )
{
    if ((eOrigin == SEEK_SET) && (nOffset < (off_t)_nBufferBytes))
    {
        _nBufferBytesRead = nOffset;
    }
    else
    {
        _pStream->seek( eOrigin, nOffset );
    }

    return -1;
}


_DWFTK_API
int DWFModel::keyToIndex(const DWFSegment::tKey &rKey)
    throw( DWFException )
{
    int retVal = -1;
    TK_Status status = _oToolkit.KeyToIndex( rKey, retVal );
    if(status == TK_NotFound)
    {
        _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"No index found for this key" );
    }

    return retVal;
}



#endif

