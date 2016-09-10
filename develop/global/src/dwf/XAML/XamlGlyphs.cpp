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

#include "XAML/pch.h"

using namespace XamlDrawableAttributes;

XamlGlyphs::AttributeParser::AttributeParser( XamlXML::tAttributeMap& rMap )
: _rMap( rMap )
, _pFile( NULL )
{
}

WT_Result 
XamlGlyphs::AttributeParser::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* p, WT_XAML_File& r) const
{
    if (p == NULL)
        return WT_Result::Toolkit_Usage_Error;

    XamlGlyphs::AttributeParser* pThis = const_cast<XamlGlyphs::AttributeParser*>(this);
    pThis->_pFile = &r;

    WD_CHECK( p->consumeBidiLevel( pThis ) );
    WD_CHECK( p->consumeCaretStops( pThis ) );
    WD_CHECK( p->consumeDeviceFontName( pThis ) );
    WD_CHECK( p->consumeFill( pThis ) );
    WD_CHECK( p->consumeFontRenderingEmSize( pThis ) );
    WD_CHECK( p->consumeFontUri( pThis ) );
    WD_CHECK( p->consumeOriginX( pThis ) );
    WD_CHECK( p->consumeOriginY( pThis ) );
    WD_CHECK( p->consumeIndices( pThis ) );
    WD_CHECK( p->consumeIsSideways( pThis ) );
    WD_CHECK( p->consumeUnicodeString( pThis ) );
    WD_CHECK( p->consumeStyleSimulations( pThis ) );
    WD_CHECK( p->consumeRenderTransform( pThis ) );
    WD_CHECK( p->consumeClip( pThis ) );
    WD_CHECK( p->consumeOpacity( pThis ) );
    WD_CHECK( p->consumeOpacityMask( pThis ) );
    WD_CHECK( p->consumeName( pThis ) );
    WD_CHECK( p->consumeNavigateUri( pThis ) );

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideBidiLevel( BidiLevel*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzBidiLevel_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( BidiLevel );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideCaretStops( XamlDrawableAttributes::CaretStops*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzCaretStops_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( CaretStops );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideDeviceFontName( XamlDrawableAttributes::DeviceFontName*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzDeviceFontName_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( DeviceFontName );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideFill( XamlDrawableAttributes::Fill*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzFill_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Fill );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideFontRenderingEmSize( XamlDrawableAttributes::FontRenderingEmSize*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzFontRenderingEmSize_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( FontRenderingEmSize );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideFontUri( XamlDrawableAttributes::FontUri*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzFontUri_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( FontUri );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideOriginX( XamlDrawableAttributes::OriginX*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOriginX_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( OriginX );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideOriginY( XamlDrawableAttributes::OriginY*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOriginY_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( OriginY );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideIndices( XamlDrawableAttributes::Indices*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzIndices_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Indices );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideIsSideways( XamlDrawableAttributes::IsSideways*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzIsSideways_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( IsSideways );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideUnicodeString( XamlDrawableAttributes::UnicodeString*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzUnicodeString_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( UnicodeString );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideStyleSimulations( XamlDrawableAttributes::StyleSimulations*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStyleSimulations_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StyleSimulations );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideRenderTransform( XamlDrawableAttributes::RenderTransform*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzRenderTransform_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( RenderTransform );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideClip( XamlDrawableAttributes::Clip*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzClip_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Clip );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideOpacity( XamlDrawableAttributes::Opacity*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOpacity_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Opacity );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideOpacityMask( XamlDrawableAttributes::OpacityMask*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOpacityMask_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( OpacityMask );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideName( XamlDrawableAttributes::Name*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzName_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Name );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::AttributeParser::provideNavigateUri( XamlDrawableAttributes::NavigateUri*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzNavigateUri_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( NavigateUri );
            if (rp == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        return rp->materializeAttribute( *_pFile, *pValue );
    }

    return WT_Result::Success;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

XamlGlyphs::XamlGlyphs(void)
: _oBidiLevel()
, _oCaretStops()
, _oDeviceFontName()
, _oFill()
, _oFontRenderingEmSize()
, _oFontUri()
, _oOriginX()
, _oOriginY()
, _oIsSideways()
, _oIndices()
, _oUnicodeString()
, _oStyleSimulations()
, _oRenderTransform()
, _oClip()
, _oOpacity()
, _oOpacityMask()
, _oName()
, _oNavigateUri()
{
}

XamlGlyphs::~XamlGlyphs(void)
{
}

#define XCALLLIST \
    XCALLLIST_MACRO(_oName) \
    XCALLLIST_MACRO(_oBidiLevel) \
    XCALLLIST_MACRO(_oCaretStops) \
    XCALLLIST_MACRO(_oDeviceFontName) \
    XCALLLIST_MACRO(_oFill) \
    XCALLLIST_MACRO(_oFontRenderingEmSize) \
    XCALLLIST_MACRO(_oFontUri) \
    XCALLLIST_MACRO(_oOriginX) \
    XCALLLIST_MACRO(_oOriginY) \
    XCALLLIST_MACRO(_oIsSideways) \
    XCALLLIST_MACRO(_oIndices) \
    XCALLLIST_MACRO(_oUnicodeString) \
    XCALLLIST_MACRO(_oStyleSimulations) \
    XCALLLIST_MACRO(_oRenderTransform) \
    XCALLLIST_MACRO(_oClip) \
    XCALLLIST_MACRO(_oOpacity) \
    XCALLLIST_MACRO(_oOpacityMask) \
    XCALLLIST_MACRO(_oNavigateUri)

#define IF_ATTR(x,y,z) if (x.validAsAttribute()) { WD_CHECK( x.serializeAttribute( y, z ) ); }
#define IF_ELEM(x,y,z) if (!x.validAsAttribute()) { WD_CHECK( x.serializeElement( y, z ) ); }

WT_Result XamlGlyphs::serialize(WT_File & file) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);
    DWFXMLSerializer* pXmlSerializer = rXamlFile.xamlSerializer();
    if (!pXmlSerializer)
        return WT_Result::Internal_Error;

    //
    // in case our top level canvas (the one holding the inner
    // render-transform) was not serialized, do it now
    //
    if(rXamlFile.isTopLevelCanvasWritten() == false)
    {
        WD_Assert( rXamlFile.isLayoutValid() );
        WD_CHECK( rXamlFile.writeTopLevelCanvas() );
    }

    //Rendition should be synced prior to this serialize call

    // Spin through the XAML Path needs, and with special attention to precedence,
    // examine the rendition attributes to gather the necessary information:
    WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rXamlFile.rendition());
    XamlGlyphs* thisGlyphs = const_cast<XamlGlyphs*>(this);

    #define PROVIDE(type,var) static_cast<GlyphsAttributeProvider*>(static_cast<type*>(&var))->provideGlyphsAttributes( thisGlyphs, rXamlFile )
    WD_CHECK( PROVIDE( WT_XAML_URL,          rXamlRendition.url() ));
	WD_CHECK( PROVIDE( WT_XAML_Attribute_URL, rXamlRendition.attribute_url() ));
    WD_CHECK( PROVIDE( WT_XAML_Font,         rXamlRendition.font() ));
    WD_CHECK( PROVIDE( WT_XAML_Font_Extension, rXamlRendition.font_extension()) );
    WD_CHECK( PROVIDE( WT_XAML_Visibility,   rXamlRendition.visibility() ));
    #undef PROVIDE   

    pXmlSerializer->startElement(XamlXML::kpzGlyphs_Element);
    {		
        //Serialize out the attributes as attributes, if possible
        #define XCALLLIST_MACRO(attr_name) \
            IF_ATTR( attr_name, rXamlFile, pXmlSerializer );
            XCALLLIST
        #undef XCALLLIST_MACRO

        //Otherwise, they go in as elements
        #define XCALLLIST_MACRO(attr_name) \
            IF_ELEM( attr_name, rXamlFile, pXmlSerializer );
            XCALLLIST
        #undef XCALLLIST_MACRO

        pXmlSerializer->endElement(); //XamlXML::kpzGlyphs_Element
    } 

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::consumeBidiLevel( BidiLevel::Provider* pProvider )
{
    BidiLevel *pBidiLevel = &_oBidiLevel;
    return pProvider->provideBidiLevel( pBidiLevel );
}

WT_Result 
XamlGlyphs::consumeCaretStops( CaretStops::Provider* pProvider )
{
    CaretStops *pCaretStops = &_oCaretStops;
    return pProvider->provideCaretStops( pCaretStops );
}

WT_Result 
XamlGlyphs::consumeDeviceFontName( DeviceFontName::Provider* pProvider )
{
    DeviceFontName *pDeviceFontName = &_oDeviceFontName;
    return pProvider->provideDeviceFontName( pDeviceFontName );
}

WT_Result 
XamlGlyphs::consumeFill( Fill::Provider* pProvider )
{
    Fill *pFill = &_oFill;
    return pProvider->provideFill( pFill );
}

WT_Result 
XamlGlyphs::consumeFontRenderingEmSize( FontRenderingEmSize::Provider* pProvider )
{
    FontRenderingEmSize *pFontRenderingEmSize = &_oFontRenderingEmSize;
    return pProvider->provideFontRenderingEmSize( pFontRenderingEmSize );
}

WT_Result 
XamlGlyphs::consumeFontUri( FontUri::Provider* pProvider )
{
    FontUri *pFontUri = &_oFontUri;
    return pProvider->provideFontUri( pFontUri );
}

WT_Result 
XamlGlyphs::consumeOriginX( OriginX::Provider* pProvider )
{
    OriginX *pOriginX = &_oOriginX;
    return pProvider->provideOriginX( pOriginX );
}

WT_Result 
XamlGlyphs::consumeOriginY( OriginY::Provider* pProvider )
{
    OriginY *pOriginY = &_oOriginY;
    return pProvider->provideOriginY( pOriginY );
}

WT_Result 
XamlGlyphs::consumeIsSideways( IsSideways::Provider* pProvider )
{
    IsSideways *pIsSideways = &_oIsSideways;
    return pProvider->provideIsSideways( pIsSideways );
}

WT_Result 
XamlGlyphs::consumeIndices( Indices::Provider* pProvider )
{
    Indices *pIndices = &_oIndices;
    return pProvider->provideIndices( pIndices );
}

WT_Result 
XamlGlyphs::consumeUnicodeString( UnicodeString::Provider* pProvider )
{
    UnicodeString *pUnicodeString = &_oUnicodeString;
    return pProvider->provideUnicodeString( pUnicodeString );
}

WT_Result 
XamlGlyphs::consumeStyleSimulations( StyleSimulations::Provider* pProvider )
{
    StyleSimulations *pStyleSimulations = &_oStyleSimulations;
    return pProvider->provideStyleSimulations( pStyleSimulations );
}

WT_Result 
XamlGlyphs::consumeRenderTransform( RenderTransform::Provider* pProvider )
{
    RenderTransform *pRenderTransform = &_oRenderTransform;
    return pProvider->provideRenderTransform( pRenderTransform );
}

WT_Result 
XamlGlyphs::consumeClip( Clip::Provider* pProvider )
{
    Clip *pClip = &_oClip;
    return pProvider->provideClip( pClip );
}

WT_Result 
XamlGlyphs::consumeOpacity( Opacity::Provider* pProvider )
{
    Opacity *pOpacity = &_oOpacity;
    return pProvider->provideOpacity( pOpacity );
}

WT_Result 
XamlGlyphs::consumeOpacityMask( OpacityMask::Provider* pProvider )
{
    OpacityMask *pOpacityMask = &_oOpacityMask;
    return pProvider->provideOpacityMask( pOpacityMask );
}

WT_Result 
XamlGlyphs::consumeName( Name::Provider* pProvider )
{
    Name *pName = &_oName;
    return pProvider->provideName( pName );
}

WT_Result 
XamlGlyphs::consumeNavigateUri( NavigateUri::Provider* pProvider )
{
    NavigateUri *pNavigateUri = &_oNavigateUri;
    return pProvider->provideNavigateUri( pNavigateUri );
}

WT_Result 
XamlGlyphs::provideBidiLevel ( XamlDrawableAttributes::BidiLevel*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( BidiLevel );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oBidiLevel;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideCaretStops( XamlDrawableAttributes::CaretStops*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( CaretStops );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oCaretStops;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideDeviceFontName( XamlDrawableAttributes::DeviceFontName*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( DeviceFontName );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oDeviceFontName;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideFill( XamlDrawableAttributes::Fill*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Fill );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oFill;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideFontRenderingEmSize( XamlDrawableAttributes::FontRenderingEmSize*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( FontRenderingEmSize );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oFontRenderingEmSize;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideFontUri( XamlDrawableAttributes::FontUri*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( FontUri );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oFontUri;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideOriginX( XamlDrawableAttributes::OriginX*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( OriginX );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oOriginX;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideOriginY( XamlDrawableAttributes::OriginY*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( OriginY );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oOriginY;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideIndices( XamlDrawableAttributes::Indices*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Indices );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oIndices;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideIsSideways( XamlDrawableAttributes::IsSideways*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( IsSideways );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oIsSideways;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideUnicodeString( XamlDrawableAttributes::UnicodeString*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( UnicodeString );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oUnicodeString;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideStyleSimulations( XamlDrawableAttributes::StyleSimulations*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StyleSimulations );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oStyleSimulations;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideRenderTransform( XamlDrawableAttributes::RenderTransform*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( RenderTransform );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oRenderTransform;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideClip( XamlDrawableAttributes::Clip*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Clip );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oClip;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideOpacity( XamlDrawableAttributes::Opacity*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Opacity );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oOpacity;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideOpacityMask( XamlDrawableAttributes::OpacityMask*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( OpacityMask );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oOpacityMask;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideName( XamlDrawableAttributes::Name*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Name );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oName;

    return WT_Result::Success;
}

WT_Result 
XamlGlyphs::provideNavigateUri( XamlDrawableAttributes::NavigateUri*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( NavigateUri );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oNavigateUri;

    return WT_Result::Success;
}
