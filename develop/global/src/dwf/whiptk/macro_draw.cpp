//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/macro_draw.cpp 6     7/05/05 8:57p Bangiav $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Macro_Draw::object_id() const
{
    return WT_Object::Macro_Draw_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Macro_Draw::serialize(WT_File & file) const
{
    // Macro_Draw is only supported in versions 6.01 and up . 
    if (file.heuristics().target_version() < REVISION_WHEN_MACRO_IS_SUPPORTED)
        return WT_Result::Toolkit_Usage_Error;


    WD_Assert(m_count > 0);

    WT_Integer32    parts_to_sync = WT_Rendition::BlockRef_Bit          |
                                    WT_Rendition::Code_Page_Bit         |
                                    WT_Rendition::Color_Bit             |
                                    WT_Rendition::Color_Map_Bit         |
                                    WT_Rendition::Dash_Pattern_Bit      |
                                    WT_Rendition::Fill_Bit              |
                                    WT_Rendition::Fill_Pattern_Bit      |
                                    WT_Rendition::Font_Bit              |
                                    WT_Rendition::Font_Extension_Bit    |
                                    WT_Rendition::Layer_Bit             |
                                    WT_Rendition::Line_Pattern_Bit      |
                                    WT_Rendition::Line_Style_Bit        |
                                    WT_Rendition::Line_Weight_Bit       |
                                    WT_Rendition::Macro_Scale_Bit       |
                                    WT_Rendition::Macro_Index_Bit       |
                                    WT_Rendition::Merge_Control_Bit     |
                                    WT_Rendition::Object_Node_Bit       |
                                    WT_Rendition::Pen_Pattern_Bit       |
                                    WT_Rendition::Projection_Bit        |
                                    WT_Rendition::URL_Bit               |
                                    WT_Rendition::Viewport_Bit          |
                                    WT_Rendition::Visibility_Bit        |
                                    WT_Rendition::Text_Background_Bit   |
                                    WT_Rendition::Text_HAlign_Bit       |
                                    WT_Rendition::Text_VAlign_Bit       |
                                    WT_Rendition::Delineate_Bit         |
                                    WT_Rendition::User_Fill_Pattern_Bit |
                                    WT_Rendition::User_Hatch_Pattern_Bit|
                                    WT_Rendition::Contrast_Color_Bit;


    if (file.desired_rendition().macro_scale().scale() > 0)
        parts_to_sync |= WT_Rendition::Macro_Index_Bit;

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    return file.merge_or_delay_drawable(*this);
}
#else
WT_Result WT_Macro_Draw::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Macro_Draw::dump(WT_File & file) const
{
    return WT_Point_Set::serialize(file, (WT_Byte) 'M', (WT_Byte)'m', (WT_Byte)0x8D);
}
#else
WT_Result   WT_Macro_Draw::dump(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Macro_Draw::delay(WT_File & file) const
{
    WT_Macro_Draw * delayed = new WT_Macro_Draw(count(), points(), WD_True);
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    file.set_delayed_drawable(delayed);

    return WT_Result::Success;
}
#else
WT_Result   WT_Macro_Draw::delay(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Boolean WT_Macro_Draw::merge(WT_Drawable const & new_one)
{
    WD_Assert (new_one.object_id() == Macro_Draw_ID);

    WT_Macro_Draw const *  current = (WT_Macro_Draw const *)(&new_one);   // Cast safe cast to a full Polymarker

    int needed = count() + current->count();

    if (needed > allocated())
    {
        WT_Logical_Point *  new_pts = new WT_Logical_Point[needed * 2];
        if (!new_pts)
            throw WT_Result::Out_Of_Memory_Error;

        WD_COPY_MEMORY(points(), count() * sizeof(WT_Logical_Point), new_pts);

        if (m_allocated)
            delete []m_points;  // We had our own local copy of the old points, so we need to delete them

        m_points = new_pts;
        m_allocated = needed * 2;
    }

    // At this point we know that:
    //   1) we have our own local copy of the data
    //   2) we have enough space to hold the old and new points

    WD_COPY_MEMORY(current->points(), current->count() * sizeof(WT_Logical_Point), &m_points[count()]);

    m_count += current->count();

    return WD_True; // Yes, they were mergable.
}
#else
WT_Boolean WT_Macro_Draw::merge(WT_Drawable const &)
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Draw::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x8D:
                {
                    WD_CHECK (WT_Point_Set::materialize_16_bit(file));
                } break;
            case 'M':
                {
                    WD_CHECK (WT_Point_Set::materialize_ascii(file));
                } break;
            case 'm':
                {
                    WD_CHECK (WT_Point_Set::materialize(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch
        } break;
    case WT_Opcode::Extended_ASCII:
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Macro_Draw::update_bounds(WT_File * file)
{
    WT_Drawable::update_bounds(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Draw::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x8D:
                {
                    WD_CHECK (WT_Point_Set::skip_operand_16_bit(file));
                } break;
            case 'M':
                {
                    WD_CHECK (WT_Point_Set::skip_operand_ascii(file));
                } break;
            case 'm':
                {
                    WD_CHECK (WT_Point_Set::skip_operand(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch
        } break;
    case WT_Opcode::Extended_ASCII:
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Draw::process(WT_File & file)
{
    WD_Assert (file.macro_draw_action());
    return (file.macro_draw_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Draw::default_process(WT_Macro_Draw &, WT_File &)
{
    return WT_Result::Success;
}

