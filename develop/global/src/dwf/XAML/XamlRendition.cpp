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

#define CHECK_ATTRIBUTE_URL( X ) \
    if (m_xaml_attribute_url.m_attribute_id == X) { \
        result = m_xaml_attribute_url.sync(file); \
        m_xaml_attribute_url.clear(); \
        if (result != WT_Result::Success) \
            break; }

WT_XAML_Rendition::WT_XAML_Rendition(void)
{
}

WT_XAML_Rendition::~WT_XAML_Rendition(void)
{
}

WT_Result WT_XAML_Rendition::sync(WT_File & file, WT_Integer32 required)
{
	if(required)
	{
		//Viewport needs to start separate canvas element so need to serialize that first.
        {
            WT_Integer32   needed = WT_Rendition::Viewport_Bit & m_changed_flags;
            WD_CHECK(m_rendering_options.sync(file));
            if (needed)
            {
                m_changed_flags &= ~needed;
                sync_parts(file, needed);
            }
        }

        required &= ~WT_Rendition::Viewport_Bit;

        WT_Integer32   needed = required & m_changed_flags;
        WD_CHECK(m_rendering_options.sync(file));
        if (needed)
        {
            m_changed_flags &= ~needed;
            return sync_parts(file, needed);
        }
	}

	return WT_Result::Success;
}

WT_Result WT_XAML_Rendition::sync_parts(WT_File & file, WT_Integer32 needed)
{
    WT_Result               result;
    while (needed)
    {
        WT_Integer32   this_case = needed & -needed;

        needed &= ~this_case;
        switch (this_case)
        {
        case Color_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Color_ID );
                result = m_xaml_color.sync(file);
                break;
        case Dash_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Dash_Pattern_ID );
                result = m_xaml_dash_pattern.sync(file);
                break;
        case Fill_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Fill_ID );
                result = m_xaml_fill.sync(file);
                break;
        case Fill_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Fill_Pattern_ID );             
                result = m_xaml_fill_pattern.sync(file);
                break;
        case Merge_Control_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Merge_Control_ID );
                result = m_xaml_merge_control.sync(file);
                break;
        case Font_Extension_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Font_Extension_ID );
                result = m_xaml_font_extension.sync(file);
                break;
        case BlockRef_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::BlockRef_ID );
                result = m_blockref.sync(file);
                break;
        case Pen_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Pen_Pattern_ID );
                result = m_xaml_pen_pattern.sync(file);
                break;
        case URL_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::URL_ID );
                result = m_xaml_url.sync(file);
                break;
        case Visibility_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Visibility_ID );
                result = m_xaml_visibility.sync(file);
                break;
        case Layer_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Layer_ID );
                result = m_xaml_layer.sync(file);
                break;
        case Line_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Line_Pattern_ID );
                result = m_xaml_line_pattern.sync(file);
                break;
        case Line_Style_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Line_Style_ID );
                result = m_xaml_line_style.sync(file);
                break;
        case Line_Weight_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Line_Weight_ID );
                result = m_xaml_line_weight.sync(file);
                break;
        case Macro_Scale_Bit: // Marker_Size_Bit 
                CHECK_ATTRIBUTE_URL( WT_Object::Macro_Scale_ID );
                if (file.heuristics().target_version() >= REVISION_WHEN_MACRO_IS_SUPPORTED)
                    result = m_xaml_macro_scale.sync(file);
                else
                    result = m_marker_size.sync(file);
                break;
        case Macro_Index_Bit: // Marker_Symbol_Bit
                CHECK_ATTRIBUTE_URL( WT_Object::Macro_Index_ID  );
                if (file.heuristics().target_version() >= REVISION_WHEN_MACRO_IS_SUPPORTED)
                    result = m_xaml_macro_index.sync(file);
                else
                    result = m_marker_symbol.sync(file);
                break;
        case Color_Map_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Color_Map_ID );
                result = m_xaml_color_map.sync(file);
                break;
        case Font_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Font_ID );
                result = m_xaml_font.sync(file);
                break;
        case Viewport_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Viewport_ID );
                result = m_xaml_viewport.sync(file);
                break;
        case Code_Page_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Code_Page_ID );
                result = m_xaml_code_page.sync(file);
                break;
        case Object_Node_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Object_Node_ID );
                result = m_xaml_object_node.sync(file);
                break;
        case Text_Background_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Text_Background_ID );
                result = m_xaml_text_background.sync(file);
                break;
        case Text_HAlign_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Text_HAlign_ID );
                result = m_xaml_text_halign.sync(file);
                break;
        case Text_VAlign_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Text_VAlign_ID );
                result = m_xaml_text_valign.sync(file);
                break;
        case Delineate_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Delineate_ID );
                result = m_xaml_delineate.sync(file);
                break;
         case User_Fill_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::User_Fill_Pattern_ID );
                result = m_xaml_user_fill_pattern.sync(file);
                break;
         case User_Hatch_Pattern_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::User_Hatch_Pattern_ID );
                result = m_xaml_user_hatch_pattern.sync(file);
                break;
         case Contrast_Color_Bit:
                CHECK_ATTRIBUTE_URL( WT_Object::Contrast_Color_ID );
                result = m_xaml_contrast_color.sync(file);
                break;
         case Attribute_URL_Bit:
                 CHECK_ATTRIBUTE_URL( WT_Object::Attribute_URL_ID );
                 result = m_xaml_attribute_url.sync(file);
                 break;
        } // switch

        if (result != WT_Result::Success)
            return result;
    } // while

    return WT_Result::Success;
}

