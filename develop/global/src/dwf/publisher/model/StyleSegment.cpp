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


#include "dwfcore/Pointer.h"
using namespace DWFCore;

#include "dwf/w3dtk/BStream.h"
#include "dwf/publisher/model/StyleSegment.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const char* const DWFStyleSegment::kz_Style_Library                = /*NOXLATE*/"?Style Library/";
const char* const DWFStyleSegment::kz_StyleSegment_PublishedEdges  = /*NOXLATE*/"?Style Library/_dwfw3d_PublishedEdges";

#endif



_DWFTK_API
DWFStyleSegment::DWFStyleSegment( DWFSegmentHandlerBuilder&     rSegmentBuilder,
                                  DWFAttributeHandlerBuilder&   rAttributeBuilder,
                                  unsigned int                  nID )
throw()
            : _bOpen( false )
            , _nID( nID )
            , _rSegmentBuilder( rSegmentBuilder )
            , _rAttributeBuilder( rAttributeBuilder )
{
    ;
}

_DWFTK_API
DWFStyleSegment::DWFStyleSegment( const DWFStyleSegment& rSegment )
throw()
            : _bOpen( rSegment._bOpen )
            , _nID( rSegment._nID )
            , _rSegmentBuilder( rSegment._rSegmentBuilder )
            , _rAttributeBuilder( rSegment._rAttributeBuilder )
{
    ;
}

_DWFTK_API
DWFStyleSegment&
DWFStyleSegment::operator=( const DWFStyleSegment& rSegment )
throw()
{
    _rSegmentBuilder = rSegment._rSegmentBuilder;
    _rAttributeBuilder = rSegment._rAttributeBuilder;
    _nID = rSegment._nID;
    _bOpen = rSegment._bOpen;

    return *this;
}

_DWFTK_API
DWFStyleSegment::~DWFStyleSegment()
throw()
{
    ;
}

_DWFTK_API
void
DWFStyleSegment::open()
throw( DWFException )
{
    if (_bOpen)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment already open" );
    }

    TK_Open_Segment& rHandler = _rSegmentBuilder.getOpenSegmentHandler();

    //
    // "name" segment with the ID
    //
    char zSegmentID[12] = {0};
    ::sprintf( zSegmentID, /*NOXLATE*/"%u", _nID );

    //
    // add the segment to the style library
    //
    _zName.assign( kz_Style_Library );
    _zName.append( zSegmentID );

    //
    // abbreviated buffer since this will be a 7-bit ASCII string
    //
    size_t nChars = _zName.chars() + 1;
    DWFPointer<ASCII_char_t> apBuffer( DWFCORE_ALLOC_MEMORY(ASCII_char_t, nChars), true );
    _zName.getUTF8( apBuffer, nChars );

    rHandler.SetSegment( (const char*)apBuffer );
    rHandler.serialize();

    _bOpen = true;
}

_DWFTK_API
void
DWFStyleSegment::close()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    _rSegmentBuilder.getCloseSegmentHandler().serialize();
}

        //
        //
        // AttributeHandlerBuilder Interface
        //
        //

_DWFTK_API
TK_Camera&
DWFStyleSegment::getCameraHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getCameraHandler();
}

_DWFTK_API
TK_Color&
DWFStyleSegment::getColorHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorHandler();
}

_DWFTK_API
TK_Color_Map&
DWFStyleSegment::getColorMapHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorMapHandler();
}

_DWFTK_API
TK_Color_RGB&
DWFStyleSegment::getColorRGBHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getColorRGBHandler();
}

_DWFTK_API
TK_Named&
DWFStyleSegment::getEdgePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getEdgePatternHandler();
}

_DWFTK_API
TK_Size&
DWFStyleSegment::getEdgeWeightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getEdgeWeightHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getFacePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getFacePatternHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getHandednessHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getHandednessHandler();
}

_DWFTK_API
TK_Heuristics&
DWFStyleSegment::getHeuristicsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getHeuristicsHandler();
}

_DWFTK_API
TK_Named&
DWFStyleSegment::getLinePatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLinePatternHandler();
}

_DWFTK_API
TK_Line_Style&
DWFStyleSegment::getLineStyleHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLineStyleHandler();
}

_DWFTK_API
TK_Size&
DWFStyleSegment::getLineWeightHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getLineWeightHandler();
}

_DWFTK_API
TK_Size&
DWFStyleSegment::getMarkerSizeHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"Segment must be open" );
    }

    return _rAttributeBuilder.getMarkerSizeHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getMarkerSymbolHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getMarkerSymbolHandler();
}

_DWFTK_API
TK_Matrix&
DWFStyleSegment::getModellingMatrixHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getModellingMatrixHandler();
}

_DWFTK_API
TK_Rendering_Options&
DWFStyleSegment::getRenderingOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getRenderingOptionsHandler();
}

_DWFTK_API
TK_Selectability&
DWFStyleSegment::getSelectabilityHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getSelectabilityHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getTextAlignmentHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextAlignmentHandler();
}

_DWFTK_API
TK_Text_Font&
DWFStyleSegment::getTextFontHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextFontHandler();
}

_DWFTK_API
TK_Point&
DWFStyleSegment::getTextPathHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextPathHandler();
}

_DWFTK_API
TK_Size&
DWFStyleSegment::getTextSpacingHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getTextSpacingHandler();
}

_DWFTK_API
TK_User_Options&
DWFStyleSegment::getUserOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getUserOptionsHandler();
}

_DWFTK_API
TK_Unicode_Options&
DWFStyleSegment::getUnicodeOptionsHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getUnicodeOptionsHandler();
}

_DWFTK_API
TK_Visibility&
DWFStyleSegment::getVisibilityHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getVisibilityHandler();
}

_DWFTK_API
TK_Window&
DWFStyleSegment::getWindowHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getWindowFrameHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowFrameHandler();
}

_DWFTK_API
TK_Enumerated&
DWFStyleSegment::getWindowPatternHandler()
throw( DWFException )
{
    if (_bOpen == false)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Segment must be open" );
    }

    return _rAttributeBuilder.getWindowPatternHandler();
}

#endif

