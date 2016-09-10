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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/gouraud_polyline.cpp 1     9/12/04 8:53p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Gouraud_Polyline::object_id() const
{
    return Gouraud_Polyline_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Gouraud_Polyline::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    // Make sure we have a legal polyline
    WD_Assert(m_count > 1);

    // TODO: Check rendition incarnation

    // Attributes that polylines care about:
    //        color, line weight, line cap, line join,
    //        line pattern, fill mode, visibility, URL
    //        layer

    WT_Integer32    parts_to_sync = //  WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                    //  WT_Rendition::View_Bit            |
                                    //  WT_Rendition::Background_Bit      |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                    //  WT_Rendition::Plot_Optimized_Bit  |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                        WT_Rendition::Line_Pattern_Bit    |
                                        WT_Rendition::Dash_Pattern_Bit    |
                                        WT_Rendition::Line_Style_Bit      |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit;


    if (file.rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_False;
        parts_to_sync |= WT_Rendition::Fill_Bit;
    }

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    WD_CHECK(WT_Gouraud_Point_Set::serialize(file,
        WT_String("GourLine"),
        (WT_Byte)WD_SBBO_DRAW_GOURAUD_POLYLINE_32R,
        (WT_Byte)WD_SBBO_DRAW_GOURAUD_POLYLINE_16R));

    return WT_Result::Success;
}
#else
WT_Result WT_Gouraud_Polyline::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Polyline::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_32R:
                {
                    // This is a multi-point polyline, 32-bit coords.
                    WD_Assert(!file.rendition().fill().fill());

                    WD_CHECK (WT_Gouraud_Point_Set::materialize(file));
                } break;
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_16R:  // Ctrl-Q
                {
                    // This is a multi-point polyline, 16-bit relative coords.
                    WD_Assert(!file.rendition().fill().fill());

                    WD_CHECK (WT_Gouraud_Point_Set::materialize_16_bit(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch
        } break;
    case WT_Opcode::Extended_ASCII: //TODO: define spec and implement
        {
            switch (m_stage)
            {
                case Getting_Count:
                    WD_CHECK (WT_Gouraud_Point_Set::materialize_ascii(file));
                //No Break here
                case Getting_Close_Paren:
                    WD_CHECK(opcode.skip_past_matching_paren(file));
                    m_stage = Getting_Count; // we finished, so reset for next time.
                    break;
                default:
                    return WT_Result::Corrupt_File_Error;
            }
        } break;
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
void WT_Gouraud_Polyline::update_bounds(WT_File *)
{
    WT_Drawable::update_bounds(*this);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Polyline::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_32R:
                {
                    WD_CHECK (WT_Gouraud_Point_Set::skip_operand(file));
                } break;
            case WD_SBBO_DRAW_GOURAUD_POLYLINE_16R:  // Ctrl-Q
                {
                    WD_CHECK (WT_Gouraud_Point_Set::skip_operand_16_bit(file));
                } break;
            default:
                return WT_Result::Opcode_Not_Valid_For_This_Object;
                // break;
            } // switch


        } break;

    case WT_Opcode::Extended_ASCII:  //TODO: define spec and implement
        WD_CHECK(opcode.skip_past_matching_paren(file));
        m_stage = Getting_Count; // we finished, so reset for next time.
        break;

    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Polyline::process(WT_File & file)
{
    WD_Assert (file.gouraud_polyline_action());
    return (file.gouraud_polyline_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Gouraud_Polyline::default_process(WT_Gouraud_Polyline &, WT_File &)
{
    return WT_Result::Success;
}
