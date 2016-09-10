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
#include "dwf/publisher/data/Data.h"
using namespace DWFToolkit;

_DWFTK_API
DWFData::DWFData( const DWFString&  zDataTitle,
                  const DWFString&  zDataSource,
                  const DWFString&  zDataSourceID,
                  const DWFString&  zDataSourceHRef )
throw()
        : DWFPublishableSection(_DWF_FORMAT_DATA_TYPE_STRING,
                                zDataTitle,
                                zDataSource,
                                zDataSourceID,
                                zDataSourceHRef)
        , _bOpen(false)
        , _oEmbeddedFonts()
        , _oResources()
{
}

_DWFTK_API
DWFData::~DWFData()
    throw()
{
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
DWFData::open()
throw( DWFException )
{
}

_DWFTK_API
void
DWFData::close()
throw( DWFException )
{
}

_DWFTK_API
void
DWFData::publish( DWFPublisher& rPublisher )
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
void
DWFData::embedFont( DWFEmbeddedFont* pFont )
throw( DWFException )
{
    _oEmbeddedFonts.push_back( pFont );
}

_DWFTK_API
DWFIterator<DWFPublishableResource*>*
DWFData::getResources()
throw( DWFException )
{
    return DWFCORE_ALLOC_OBJECT( ResourceIterator(_oResources) );
}

_DWFTK_API
void
DWFData::addResource( DWFPublishableResource* pResource )
throw( DWFException )
{
    _oResources.push_back( pResource );
}

#endif
