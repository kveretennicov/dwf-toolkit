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

XamlCanvas::AttributeParser::AttributeParser( XamlXML::tAttributeMap& rMap )
: _rMap( rMap )
, _pFile( NULL )
{
}

WT_Result 
XamlCanvas::AttributeParser::provideCanvasAttributes(  XamlDrawableAttributes::CanvasAttributeConsumer* p, WT_XAML_File& r) const
{
    if (p == NULL)
        return WT_Result::Toolkit_Usage_Error;

    XamlCanvas::AttributeParser* pThis = const_cast<XamlCanvas::AttributeParser*>(this);
    pThis->_pFile = &r;

    WD_CHECK( p->consumeName( pThis ) );
    WD_CHECK( p->consumeRenderTransform( pThis ) );
    WD_CHECK( p->consumeClip( pThis ) );
    WD_CHECK( p->consumeOpacity( pThis ) );
    WD_CHECK( p->consumeOpacityMask( pThis ) );
    WD_CHECK( p->consumeNavigateUri( pThis ) );
    WD_CHECK( p->consumeResources( pThis ) );

    return WT_Result::Success;
}

WT_Result 
XamlCanvas::AttributeParser::provideName( Name*& rp )
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
XamlCanvas::AttributeParser::provideRenderTransform( RenderTransform*& rp )
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
XamlCanvas::AttributeParser::provideClip( Clip*& rp )
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
XamlCanvas::AttributeParser::provideOpacity( Opacity*& rp )
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
XamlCanvas::AttributeParser::provideOpacityMask( OpacityMask*& rp )
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
XamlCanvas::AttributeParser::provideNavigateUri( NavigateUri*& rp )
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

WT_Result 
XamlCanvas::AttributeParser::provideResources( Resources*& /*rp*/ )
{
    //Canvas resources are elements, always.
    return WT_Result::Success;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

XamlCanvas::XamlCanvas( bool bNesting )
: _oName()
, _oRenderTransform()
, _oClip()
, _oOpacity()
, _oOpacityMask()
, _oNavigateUri()
, _oResources()
, _bNesting( bNesting )
{
}

XamlCanvas::~XamlCanvas()
{
}

WT_Result XamlCanvas::serialize(WT_File &file) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);

    WT_Integer32    parts_to_sync =     WT_Rendition::Attribute_URL_Bit |
                                        WT_Rendition::Viewport_Bit;

	WD_CHECK( rXamlFile.desired_rendition().sync(file, parts_to_sync) );

    WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rXamlFile.rendition());
    XamlCanvas *pCasted = const_cast<XamlCanvas *>(this);

    WD_CHECK(static_cast<WT_XAML_Viewport &>(rXamlRendition.viewport()).provideCanvasAttributes(pCasted, rXamlFile));
	WD_CHECK(static_cast<WT_XAML_Attribute_URL &>(rXamlRendition.attribute_url()).provideCanvasAttributes(pCasted, rXamlFile));
    return dump(file);
}

#define XCALLLIST_NODATA \
    XCALLLIST_MACRO(_oName) \
    XCALLLIST_MACRO(_oRenderTransform) \
    XCALLLIST_MACRO(_oClip) \
    XCALLLIST_MACRO(_oOpacity) \
    XCALLLIST_MACRO(_oOpacityMask) \
	XCALLLIST_MACRO(_oNavigateUri) \
	XCALLLIST_MACRO(_oResources) 

#define XCALLLIST \
    XCALLLIST_NODATA

#define IF_ATTR(x,y,z) if (x.validAsAttribute()) { WD_CHECK(x.serializeAttribute( y, z ) ); }
#define IF_ELEM(x,y,z) if (!x.validAsAttribute()) { WD_CHECK( x.serializeElement( y, z ) ); }

WT_Result XamlCanvas::dump(WT_File &rFile) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(rFile);

    WD_CHECK(const_cast<XamlCanvas *>(this)->consumeName(&rXamlFile));
    rXamlFile.nameIndex()++;

    DWFXMLSerializer* pXmlSerializer = rXamlFile.xamlSerializer();
    
    if (!pXmlSerializer)
    {
        return WT_Result::Internal_Error;
    }

    //
    // in case our top level canvas (the one holding the inner
    // render-transform) was not serialized, do it now
    //
    if(rXamlFile.isTopLevelCanvasWritten() == false)
    {
        WD_Assert( rXamlFile.isLayoutValid() );
        WD_CHECK( rXamlFile.writeTopLevelCanvas() );
    }

    WD_CHECK( rXamlFile.serializeRenditionSyncEndElement() );
	WD_CHECK( rXamlFile.serializeCanvasStartElement( _bNesting ) );

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

    return WT_Result::Success;
}

WT_Result XamlCanvas::consumeName( Name::Provider *pProvider )
{
    Name *pName = &_oName;
    return pProvider->provideName(pName);
}

WT_Result XamlCanvas::consumeRenderTransform( RenderTransform::Provider *pProvider )
{
    RenderTransform *pRenderTransform = &_oRenderTransform;
    return pProvider->provideRenderTransform(pRenderTransform);
}

WT_Result XamlCanvas::consumeClip( Clip::Provider *pProvider )
{
    Clip *pClip = &_oClip;
    return pProvider->provideClip(pClip);
}

WT_Result XamlCanvas::consumeOpacity( Opacity::Provider *pProvider )
{
    Opacity *pOpacity = &_oOpacity;
    return pProvider->provideOpacity(pOpacity);
}

WT_Result XamlCanvas::consumeOpacityMask( OpacityMask::Provider *pProvider )
{
    OpacityMask *pOpacityMask = &_oOpacityMask;
    return pProvider->provideOpacityMask(pOpacityMask);
}

WT_Result XamlCanvas::consumeNavigateUri( NavigateUri::Provider *pProvider )
{
    NavigateUri *pNavigateUri = &_oNavigateUri;
    return pProvider->provideNavigateUri(pNavigateUri);
}

WT_Result XamlCanvas::consumeResources( Resources::Provider *pProvider )
{
    Resources *pResources = &_oResources;
    return pProvider->provideResources(pResources);
}

WT_Result 
XamlCanvas::provideName( XamlDrawableAttributes::Name*& rp )
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
XamlCanvas::provideRenderTransform( XamlDrawableAttributes::RenderTransform*& rp )
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
XamlCanvas::provideClip( XamlDrawableAttributes::Clip*& rp )
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
XamlCanvas::provideOpacity( XamlDrawableAttributes::Opacity*& rp )
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
XamlCanvas::provideOpacityMask( XamlDrawableAttributes::OpacityMask*& rp )
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
XamlCanvas::provideNavigateUri( XamlDrawableAttributes::NavigateUri*& rp )
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

WT_Result 
XamlCanvas::provideResources( XamlDrawableAttributes::Resources*& rp )
{
    if (rp == NULL)
    {
        rp = DWFCORE_ALLOC_OBJECT( Resources );
        if (rp == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    *rp = _oResources;

    return WT_Result::Success;
}

