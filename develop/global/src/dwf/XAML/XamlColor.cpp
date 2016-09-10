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
#include "dwfcore/DWFXMLSerializer.h"

WT_Result 
WT_XAML_Color::serialize(WT_File& file) const
{
	WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Color::serialize( *rFile.w2dContentFile() );
    }

    bool bForceSerializeToW2X = 
        file.rendition().fill_pattern() != file.desired_rendition().fill_pattern() ||
        file.rendition().user_hatch_pattern() != file.desired_rendition().user_hatch_pattern() ||
        file.rendition().user_fill_pattern() != file.desired_rendition().user_fill_pattern();

    if (bForceSerializeToW2X)
    {
        WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

        //Serialize RenditionSync (top element) if not yet done
	    WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	    //now write whipx hint to w2xserializer
	    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	    if (!pW2XSerializer)
		    return WT_Result::Internal_Error;
    	
	    pW2XSerializer->startElement(XamlXML::kpzColor_Element);
        //
        if( index() == WD_NO_COLOR_INDEX ) 
        {
            wchar_t buf[16];
            XamlBrush::SolidColor::PrintColor( buf, 16, rgba() );
            pW2XSerializer->addAttribute(XamlXML::kpzColor_Attribute, buf);
        }
        else
        {
            pW2XSerializer->addAttribute(XamlXML::kpzIndex_Attribute, index());
        }
	    pW2XSerializer->endElement();
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Color::provideStroke(XamlDrawableAttributes::Stroke*& rpStroke)
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
    pSolidBrush->set(rgba());

    rpStroke->brush() = pSolidBrush;

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Color::consumeStroke(XamlDrawableAttributes::Stroke::Provider* pProvider )
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
        //Mapping / color map index can be resolved later using WT_XAML_File
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Color::consumeFill(XamlDrawableAttributes::Fill::Provider* pProvider )
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
        //Mapping / color map index can be resolved later using WT_XAML_File
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Color::provideFill(XamlDrawableAttributes::Fill*& rpFill)
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
	pSolidBrush->set(rgba());

	rpFill->brush() = pSolidBrush;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Color::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File &rFile ) const
{
    const_cast<WT_XAML_Color*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeStroke( const_cast<WT_XAML_Color*>(this) );
}

WT_Result 
WT_XAML_Color::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* pConsumer, WT_XAML_File &rFile ) const
{
    const_cast<WT_XAML_Color*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_Color*>(this) );
}

WT_Result WT_XAML_Color::sync_index( WT_XAML_File & rFile )
{
    int idx = rFile.rendition().color_map().exact_index( *this );
    if (idx != WD_NO_COLOR_INDEX)
    {
        set( index(), rFile.rendition().color_map() );
    }
    return WT_Result::Success;
}

WT_Result WT_XAML_Color::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_File& file)
{
	WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
	WT_Result result = WT_Result::Success;

	if(!rMap.size())
		return WT_Result::Success;

	//See if color is indexed from color_map
	const char** ppValue = rMap.find(XamlXML::kpzIndex_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
		//Indexed color
		int iIndex = atoi(*ppValue);
		if (iIndex < 0)
			return WT_Result::Corrupt_File_Error;

        // map color if possible
        WT_Color_Map *pColorMap = &rFile.rendition().color_map();

        // If there is a color map in the object list, we need to sync it first.
        std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
        std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;
        if (rFile.object_list().find_by_id(WT_Object::Color_Map_ID, iStart, iEnd))
        {
            if (iStart != iEnd && iStart->second  != NULL && iStart->second->object_id() == WT_Object::Color_Map_ID )
	        {
                pColorMap = static_cast<WT_Color_Map*>(iStart->second);
            }
        }

        if (pColorMap == NULL)
        {
            return WT_Result::Internal_Error;
        }

		if(iIndex < pColorMap->size())
        {
			set(iIndex, *pColorMap );
        }
		else
        {
			set(WT_RGBA32(0,0,0,255));
        }
	}

    ppValue = rMap.find(XamlXML::kpzColor_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        WT_RGBA32 rgba;
        if (WT_Result::Success == XamlBrush::SolidColor::ReadColor( rgba, *ppValue ) )
        {
            set( rgba );
        }
    }

    materialized() = WD_True;
	return result;
}
