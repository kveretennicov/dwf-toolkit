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
#include "XAML/XamlBrushes.h"

WT_Result 
WT_XAML_Contrast_Color::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Contrast_Color::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
    pW2XSerializer->startElement(XamlXML::kpzContrast_Color_Element );

    wchar_t buf[16];
    XamlBrush::Brush::PrintColor( buf, 16, color() );

    pW2XSerializer->addAttribute(XamlXML::kpzColor_Attribute, buf);
	pW2XSerializer->endElement();

    return WT_Result::Success;
}


WT_Result 
WT_XAML_Contrast_Color::consumeStroke(XamlDrawableAttributes::Stroke::Provider* pProvider )
{
    if (pProvider == NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    XamlDrawableAttributes::Stroke oStroke, *p = &oStroke;
    WD_CHECK( pProvider->provideStroke( p ) );

    if ( oStroke.brush()->type() == XamlBrush::kSolidColor )
    {
        set( static_cast< const XamlBrush::SolidColor* >( static_cast< const XamlBrush::Brush* >( oStroke.brush() ) )->color() );
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Contrast_Color::consumeFill(XamlDrawableAttributes::Fill::Provider* pProvider )
{
    if (pProvider == NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    XamlDrawableAttributes::Fill oFill, *p = &oFill;
    WD_CHECK( pProvider->provideFill( p ) );

    if ( oFill.brush()->type() == XamlBrush::kSolidColor )
    {
        set( static_cast< const XamlBrush::SolidColor* >( static_cast< const XamlBrush::Brush* >( oFill.brush() ) )->color() );
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Contrast_Color::provideStroke(XamlDrawableAttributes::Stroke*& rpStroke)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    //We don't stroke paths if we're filling them.
    if (_pSerializeFile->rendition().fill().fill() || _pSerializeFile->desired_rendition().fill().fill() )
    {
        return WT_Result::Success;
    }

	if (rpStroke == NULL)
    {
        rpStroke = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Stroke() );
        if (rpStroke == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    XamlBrush::SolidColor* pSolidBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::SolidColor() );
    pSolidBrush->set(color());

    rpStroke->brush() = pSolidBrush;

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Contrast_Color::provideFill(XamlDrawableAttributes::Fill*& rpFill)
{
	if (rpFill == NULL)
    {
        rpFill = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Fill() );
        if (rpFill == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	XamlBrush::SolidColor* pSolidBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::SolidColor() );
	pSolidBrush->set(color());

	rpFill->brush() = pSolidBrush;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Contrast_Color::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File &rFile ) const
{
    const_cast<WT_XAML_Contrast_Color*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeStroke( const_cast<WT_XAML_Contrast_Color*>(this) );
}

WT_Result 
WT_XAML_Contrast_Color::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* pConsumer, WT_XAML_File &rFile ) const
{
    const_cast<WT_XAML_Contrast_Color*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_Contrast_Color*>(this) );
}

WT_Result
WT_XAML_Contrast_Color::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	//See if color is indexed from color_map
	const char** pColor = rMap.find(XamlXML::kpzColor_Attribute);
	if(pColor != NULL && *pColor != NULL)
	{
        WD_CHECK( XamlBrush::Brush::ReadColor( color(), *pColor ) );
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;
	return WT_Result::Success;
}
