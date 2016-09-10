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

XamlPath::AttributeParser::AttributeParser( XamlXML::tAttributeMap& rMap )
: _rMap( rMap )
, _pFile( NULL )
{
}

WT_Result 
XamlPath::AttributeParser::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* p, WT_XAML_File& r) const
{
    if (p == NULL)
        return WT_Result::Toolkit_Usage_Error;

    XamlPath::AttributeParser* pThis = const_cast<XamlPath::AttributeParser*>(this);
    pThis->_pFile = &r;

    WD_CHECK( p->consumeData( pThis ) );
    WD_CHECK( p->consumeName( pThis ) );
    WD_CHECK( p->consumeFill( pThis ) );
    WD_CHECK( p->consumeRenderTransform( pThis ) );
    WD_CHECK( p->consumeClip( pThis ) );
    WD_CHECK( p->consumeOpacity( pThis ) );
    WD_CHECK( p->consumeOpacityMask( pThis ) );
    WD_CHECK( p->consumeStroke( pThis ) );
    WD_CHECK( p->consumeStrokeDashArray( pThis ) );
    WD_CHECK( p->consumeStrokeDashCap( pThis ) );
    WD_CHECK( p->consumeStrokeDashOffset( pThis ) );
    WD_CHECK( p->consumeStrokeEndLineCap( pThis ) );
    WD_CHECK( p->consumeStrokeStartLineCap( pThis ) );
    WD_CHECK( p->consumeStrokeLineJoin( pThis ) );
    WD_CHECK( p->consumeStrokeMiterLimit( pThis ) );
    WD_CHECK( p->consumeStrokeThickness( pThis ) );
    WD_CHECK( p->consumeNavigateUri( pThis ) );

    return WT_Result::Success;
}

WT_Result 
XamlPath::AttributeParser::provideData( Data*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzData_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Data() );
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
XamlPath::AttributeParser::provideName( Name*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzName_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Name() );
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
XamlPath::AttributeParser::provideFill( Fill*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzFill_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Fill() );
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
XamlPath::AttributeParser::provideRenderTransform( RenderTransform*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzRenderTransform_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( RenderTransform() );
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
XamlPath::AttributeParser::provideClip( Clip*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzClip_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Clip() );
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
XamlPath::AttributeParser::provideOpacity( Opacity*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOpacity_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Opacity() );
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
XamlPath::AttributeParser::provideOpacityMask( OpacityMask*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzOpacityMask_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( OpacityMask() );
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
XamlPath::AttributeParser::provideStroke( Stroke*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStroke_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( Stroke() );
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
XamlPath::AttributeParser::provideStrokeDashArray( StrokeDashArray*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeDashArray_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeDashArray() );
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
XamlPath::AttributeParser::provideStrokeDashCap( StrokeDashCap*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeDashCap_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeDashCap() );
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
XamlPath::AttributeParser::provideStrokeDashOffset( StrokeDashOffset*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeDashOffset_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeDashOffset() );
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
XamlPath::AttributeParser::provideStrokeEndLineCap( StrokeEndLineCap*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeEndLineCap_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeEndLineCap() );
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
XamlPath::AttributeParser::provideStrokeStartLineCap(StrokeStartLineCap*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeStartLineCap_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeStartLineCap() );
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
XamlPath::AttributeParser::provideStrokeLineJoin( StrokeLineJoin*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeLineJoin_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeLineJoin() );
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
XamlPath::AttributeParser::provideStrokeMiterLimit( StrokeMiterLimit*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeMiterLimit_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeMiterLimit() );
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
XamlPath::AttributeParser::provideStrokeThickness( StrokeThickness*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzStrokeThickness_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( StrokeThickness() );
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
XamlPath::AttributeParser::provideNavigateUri( NavigateUri*& rp )
{
    const char** pValue = _rMap.find( XamlXML::kpzNavigateUri_Attribute );
    if (pValue != NULL && *pValue != NULL)
    {
    	if (rp == NULL)
        {
            rp = DWFCORE_ALLOC_OBJECT( NavigateUri() );
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

XamlPath::XamlPath()
: _oData()
, _oName()
, _oFill()
, _oRenderTransform()
, _oClip()
, _oOpacity()
, _oOpacityMask()
, _oStroke()
, _oStrokeDashArray()
, _oStrokeDashCap()
, _oStrokeDashOffset()
, _oStrokeEndLineCap()
, _oStrokeStartLineCap()
, _oStrokeLineJoin()
, _oStrokeMiterLimit()
, _oStrokeThickness()
, _oNavigateUri()
{
}


XamlPath::XamlPath(const XamlPath& s)
: _oData()
, _oName()
, _oFill()
, _oRenderTransform()
, _oClip()
, _oOpacity()
, _oOpacityMask()
, _oStroke()
, _oStrokeDashArray()
, _oStrokeDashCap()
, _oStrokeDashOffset()
, _oStrokeEndLineCap()
, _oStrokeStartLineCap()
, _oStrokeLineJoin()
, _oStrokeMiterLimit()
, _oStrokeThickness()
, _oNavigateUri()
{  
    XamlPath::operator=(s);
}

XamlPath::~XamlPath(void)
{	
}

XamlPath& XamlPath::operator=(const XamlPath& s)
{
    if (&s != this) {

        _oData = s._oData;
        _oName = s._oName;
        _oFill = s._oFill;
        _oRenderTransform = s._oRenderTransform;
        _oClip = s._oClip;
        _oOpacity = s._oOpacity;
        _oOpacityMask = s._oOpacityMask;
        _oStroke = s._oStroke;
        _oStrokeDashArray = s._oStrokeDashArray;
        _oStrokeDashCap = s._oStrokeDashCap;
        _oStrokeDashOffset = s._oStrokeDashOffset;
        _oStrokeEndLineCap = s._oStrokeEndLineCap;
        _oStrokeStartLineCap = s._oStrokeStartLineCap;
        _oStrokeLineJoin = s._oStrokeLineJoin;
        _oStrokeMiterLimit = s._oStrokeMiterLimit;
        _oStrokeThickness = s._oStrokeThickness;
        _oNavigateUri = s._oNavigateUri;
	}

    return *this;
}


void XamlPath::addDrawable(XamlGraphicsObject* pXamlDrawable)
{
    _oData.geometry().figure().addDrawable(pXamlDrawable);
}

WT_Result XamlPath::serialize(WT_File& file) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);
    return rXamlFile.merge_or_delay_path( *this );
}

WT_Result XamlPath::delay(WT_File & file) const
{
    WT_XAML_File& xFile = static_cast<WT_XAML_File&>(file);

    XamlPath * delayed = DWFCORE_ALLOC_OBJECT( XamlPath(*this) );
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    xFile.set_delayed_path(delayed);

    return WT_Result::Success;
}

#define XCALLLIST_NODATA \
    XCALLLIST_MACRO(_oName) \
    XCALLLIST_MACRO(_oFill) \
    XCALLLIST_MACRO(_oRenderTransform) \
    XCALLLIST_MACRO(_oClip) \
    XCALLLIST_MACRO(_oOpacity) \
    XCALLLIST_MACRO(_oOpacityMask) \
    XCALLLIST_MACRO(_oStroke) \
    XCALLLIST_MACRO(_oStrokeDashArray) \
    XCALLLIST_MACRO(_oStrokeDashCap) \
    XCALLLIST_MACRO(_oStrokeDashOffset) \
    XCALLLIST_MACRO(_oStrokeEndLineCap) \
    XCALLLIST_MACRO(_oStrokeStartLineCap) \
    XCALLLIST_MACRO(_oStrokeLineJoin) \
    XCALLLIST_MACRO(_oStrokeMiterLimit) \
    XCALLLIST_MACRO(_oStrokeThickness) \
    XCALLLIST_MACRO(_oNavigateUri)

#define XCALLLIST \
    XCALLLIST_NODATA \
    XCALLLIST_MACRO(_oData)

#define IF_ATTR(x,y,z) if (x.validAsAttribute()) { WD_CHECK( x.serializeAttribute( y, z ) ); }
#define IF_ELEM(x,y,z) if (!x.validAsAttribute()) { WD_CHECK( x.serializeElement( y, z ) ); }

WT_Result XamlPath::dump(WT_File & file) const
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

    pXmlSerializer->startElement(XamlXML::kpzPath_Element);
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

		pXmlSerializer->endElement(); //XamlXML::kpzPath_Element
     
    } 

    return WT_Result::Success;
}

bool XamlPath::merge(const XamlPath & new_one)
{
    //check to see of the attributes are any different
    #define XCALLLIST_MACRO(attr_name) \
        if (! (new_one.attr_name == attr_name)) \
            { return false; }
        XCALLLIST_NODATA
    #undef XCALLLIST_MACRO

    WD_Assert (new_one.xobject_id() == Path_ID);

    //FUTURE: move this merging logic into the Data class.
    _oData.geometry().figure().appendFigure( new_one._oData.geometry().figure() );

    return true;
}

WT_Result XamlPath::consumeData( Data::Provider *pProvider )
{
    Data *pData = &_oData;
    return pProvider->provideData( pData );
}

WT_Result XamlPath::consumeName( Name::Provider *pProvider )
{
    Name *pName = &_oName;
    return pProvider->provideName( pName );
}

WT_Result XamlPath::consumeFill( Fill::Provider *pProvider )
{
    Fill *pFill = &_oFill;
    return pProvider->provideFill( pFill );
}

WT_Result XamlPath::consumeRenderTransform( RenderTransform::Provider *pProvider )
{
    RenderTransform *pRenderTransform = &_oRenderTransform;
    return pProvider->provideRenderTransform( pRenderTransform );
}

WT_Result XamlPath::consumeClip( Clip::Provider *pProvider )
{
    Clip *pClip = &_oClip;
    return pProvider->provideClip( pClip );
}

WT_Result XamlPath::consumeOpacity( Opacity::Provider *pProvider )
{
    Opacity *pOpacity = &_oOpacity;
    return pProvider->provideOpacity( pOpacity );
}

WT_Result XamlPath::consumeOpacityMask( OpacityMask::Provider *pProvider )
{
    OpacityMask *pOpacityMask = &_oOpacityMask;
    return pProvider->provideOpacityMask( pOpacityMask );
}

WT_Result XamlPath::consumeStroke( Stroke::Provider *pProvider )
{
    Stroke *pStroke = &_oStroke;
    return pProvider->provideStroke( pStroke );
}

WT_Result XamlPath::consumeStrokeDashArray( StrokeDashArray::Provider *pProvider )
{
    StrokeDashArray *pStrokeDashArray = &_oStrokeDashArray;
    return pProvider->provideStrokeDashArray( pStrokeDashArray );
}

WT_Result XamlPath::consumeStrokeDashCap( StrokeDashCap::Provider *pProvider )
{
    StrokeDashCap *pStrokeDashCap = &_oStrokeDashCap;
    return pProvider->provideStrokeDashCap( pStrokeDashCap );
}

WT_Result XamlPath::consumeStrokeDashOffset( StrokeDashOffset::Provider *pProvider )
{
    StrokeDashOffset *pStrokeDashOffset = &_oStrokeDashOffset;
    return pProvider->provideStrokeDashOffset( pStrokeDashOffset );
}

WT_Result XamlPath::consumeStrokeEndLineCap( StrokeEndLineCap::Provider *pProvider )
{
    StrokeEndLineCap *pStrokeEndLineCap = &_oStrokeEndLineCap;
    return pProvider->provideStrokeEndLineCap( pStrokeEndLineCap );
}

WT_Result XamlPath::consumeStrokeStartLineCap( StrokeStartLineCap::Provider *pProvider )
{
    StrokeStartLineCap *pStrokeStartLineCap = &_oStrokeStartLineCap;
    return pProvider->provideStrokeStartLineCap( pStrokeStartLineCap );
}

WT_Result XamlPath::consumeStrokeLineJoin( StrokeLineJoin::Provider *pProvider )
{
    StrokeLineJoin *pStrokeLineJoin = &_oStrokeLineJoin;
    return pProvider->provideStrokeLineJoin( pStrokeLineJoin );
}

WT_Result XamlPath::consumeStrokeMiterLimit( StrokeMiterLimit::Provider *pProvider )
{
    StrokeMiterLimit *pStrokeMiterLimit = &_oStrokeMiterLimit;
    return pProvider->provideStrokeMiterLimit( pStrokeMiterLimit );
}

WT_Result XamlPath::consumeStrokeThickness( StrokeThickness::Provider *pProvider )
{
    StrokeThickness *pStrokeThickness = &_oStrokeThickness;
    return pProvider->provideStrokeThickness( pStrokeThickness );
}

WT_Result XamlPath::consumeNavigateUri( NavigateUri::Provider *pProvider )
{
    NavigateUri *pNavigateUri = &_oNavigateUri;
    return pProvider->provideNavigateUri( pNavigateUri );
}

WT_Result XamlPath::provideData(XamlDrawableAttributes::Data*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Data() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = data();
    return WT_Result::Success;
}

WT_Result XamlPath::provideName(XamlDrawableAttributes::Name*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Name() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = name();
    return WT_Result::Success;
}

WT_Result XamlPath::provideFill(XamlDrawableAttributes::Fill*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Fill() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = fill();
    return WT_Result::Success;
}

WT_Result XamlPath::provideRenderTransform(XamlDrawableAttributes::RenderTransform*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( RenderTransform() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = renderTransform();
    return WT_Result::Success;
}

WT_Result XamlPath::provideClip(XamlDrawableAttributes::Clip*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Clip() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = clip();
    return WT_Result::Success;
}

WT_Result XamlPath::provideOpacity(XamlDrawableAttributes::Opacity*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Opacity() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = opacity();
    return WT_Result::Success;
}

WT_Result XamlPath::provideOpacityMask(XamlDrawableAttributes::OpacityMask*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( OpacityMask() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = opacityMask();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStroke(XamlDrawableAttributes::Stroke*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Stroke() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = stroke();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeDashArray(XamlDrawableAttributes::StrokeDashArray*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeDashArray() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeDashArray();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeDashCap(XamlDrawableAttributes::StrokeDashCap*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeDashCap() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeDashCap();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeDashOffset(XamlDrawableAttributes::StrokeDashOffset*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeDashOffset() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeDashOffset();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeEndLineCap(XamlDrawableAttributes::StrokeEndLineCap*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeEndLineCap() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeEndLineCap();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeStartLineCap(XamlDrawableAttributes::StrokeStartLineCap*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeStartLineCap() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeStartLineCap();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeLineJoin(XamlDrawableAttributes::StrokeLineJoin*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeLineJoin() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeLineJoin();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeMiterLimit(XamlDrawableAttributes::StrokeMiterLimit*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeMiterLimit() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeMiterLimit();
    return WT_Result::Success;
}

WT_Result XamlPath::provideStrokeThickness(XamlDrawableAttributes::StrokeThickness*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( StrokeThickness() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = strokeThickness();
    return WT_Result::Success;
}

WT_Result XamlPath::provideNavigateUri(XamlDrawableAttributes::NavigateUri*& rp )
{
	if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( NavigateUri() );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = navigateUri();
    return WT_Result::Success;
}
