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

#include "dwf/publisher/PublishedObject.h"
#include "dwf/publisher/plot/Plot.h"
using namespace DWFToolkit;

_DWFTK_API
DWFPlot::DWFPlot( const DWFString&  zPlotTitle,
                  const DWFString&  zPlotSource,
                  const DWFString&  zPlotSourceID,
                  const DWFString&  zPlotSourceHRef,
                  const DWFString&  zPlotLabel )
throw()
        : DWFPublishableSection(_DWF_FORMAT_EPLOT_TYPE_STRING,
                                zPlotTitle,
                                zPlotSource,
                                zPlotSourceID,
                                zPlotSourceHRef,
                                zPlotLabel)
        , _zMIME( DWFMIME::kzMIMEType_W2D )
        , _oPaper()
        , _oExtents()
        , _oClip()
        , _pW2DStream( NULL )
        , _bOwnStream( false )
        , _oEmbeddedFonts()
        , _oResources()
{
    _anPaperTransform[0]  = 1.0f ;_anPaperTransform[1]  = 0.0f; _anPaperTransform[2]  = 0.0f; _anPaperTransform[3]  = 0.0f;
    _anPaperTransform[4]  = 0.0f ;_anPaperTransform[5]  = 1.0f; _anPaperTransform[6]  = 0.0f; _anPaperTransform[7]  = 0.0f;
    _anPaperTransform[8]  = 0.0f; _anPaperTransform[9]  = 0.0f; _anPaperTransform[10] = 1.0f; _anPaperTransform[11] = 0.0f;
    _anPaperTransform[12] = 0.0f; _anPaperTransform[13] = 0.0f; _anPaperTransform[14] = 0.0f; _anPaperTransform[15] = 1.0f;
}

_DWFTK_API
DWFPlot::~DWFPlot()
    throw()
{
    if (_pW2DStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pW2DStream );
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
}

_DWFTK_API
void
DWFPlot::open( const DWFPaper& rPaper,
               const double*   pTransform,
               const double*   pExtents,
               const double*   pClip )
throw( DWFException )
{
    _oPaper = rPaper;
        //
        //
        //
    if (pTransform)
    {
        DWFCORE_COPY_MEMORY( _anPaperTransform, pTransform, sizeof(double)*16 );
    }

        //
        //
        //
    if (pExtents)
    {
        size_t n = 0;
        for (; n < 4; n++)
        {
            _oExtents.push_back(pExtents[n]);
        }
    }

        //
        //
        //
    if (pClip)
    {
        size_t n = 0;
        for (; n < 4; n++)
        {
            _oClip.push_back(pClip[n]);
        }
    }
}

_DWFTK_API
void
DWFPlot::close()
throw( DWFException )
{
}

_DWFTK_API
void
DWFPlot::attach( DWFInputStream* pW2DStream, bool bOwnStream )
throw( DWFException )
{
    if (pW2DStream == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No stream provided" );
    }

    if (_pW2DStream && _bOwnStream)
    {
        DWFCORE_FREE_OBJECT( _pW2DStream );
        _pW2DStream = NULL;
    }

    _pW2DStream = pW2DStream;
    _bOwnStream = bOwnStream;
}

_DWFTK_API
const DWFString&
DWFPlot::getMIMEType()
throw( DWFException )
{
    return _zMIME;
}

_DWFTK_API
void
DWFPlot::publish( DWFPublisher& rPublisher )
throw( DWFException )
{
    //
    // 
    //
    rPublisher.preprocess( this );


    DWFEmbeddedFontVisitor*         pFontVisitor = rPublisher.getEmbeddedFontVisitor();

        //
        // publish fonts
        //
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
DWFPaper*
DWFPlot::getPaper()
throw ()
{
    return &_oPaper;
}

_DWFTK_API
void
DWFPlot::getTransform( double* pTransform )
const
throw()
{
    if (pTransform)
    {
        DWFCORE_COPY_MEMORY( pTransform, _anPaperTransform, sizeof(double)*16 );
    }
}

_DWFTK_API
void
DWFPlot::getPlotExtents( tDoubleList& rExtents )
const
throw()
{
    rExtents = _oExtents;
}


_DWFTK_API
void
DWFPlot::getPlotClip( tDoubleList& rClip )
const
throw()
{
    rClip = _oClip;
}

_DWFTK_API
DWFInputStream*
DWFPlot::getInputStream()
throw( DWFException )
{
	//
	// This is for the overlay raster only case
	//

	if( _pW2DStream == NULL )
	{
		return _pW2DStream;
	}

    //
    // Wrap the 2D Stream up in a buffer stream
    //

	return DWFCORE_ALLOC_OBJECT( DWFBufferInputStream(_pW2DStream, false) );
}

_DWFTK_API
void
DWFPlot::embedFont( DWFEmbeddedFont* pFont )
throw( DWFException )
{
    _oEmbeddedFonts.push_back( pFont );
}

_DWFTK_API
DWFIterator<DWFPublishableResource*>*
DWFPlot::getResources()
throw( DWFException )
{
    return DWFCORE_ALLOC_OBJECT( ResourceIterator(_oResources) );
}

_DWFTK_API
void
DWFPlot::addResource( DWFPublishableResource* pResource )
throw( DWFException )
{
    _oResources.push_back( pResource );
}

#endif
