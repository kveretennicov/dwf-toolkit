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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/polygon.cpp 7     7/05/05 8:57p Bangiav $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Polygon::object_id() const
{
    return WT_Object::Polygon_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Polygon::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    // Make sure we have a legal point set.  Consumers must deal with degenerate cases.
    WD_Assert(m_count > 0);

    WT_Integer32 parts_to_sync =        WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::Delineate_Bit       |
                                        WT_Rendition::User_Fill_Pattern_Bit  |
                                        WT_Rendition::User_Hatch_Pattern_Bit |
                                        WT_Rendition::Contrast_Color_Bit;


    if (!file.rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_True;
        parts_to_sync |= WT_Rendition::Fill_Bit;
    }

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    return WT_Point_Set::serialize(file, (WT_Byte) 'P', (WT_Byte)'p', (WT_Byte)0x10);
}
#else
WT_Result WT_Polygon::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polygon::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x10: // Ctrl-P
                {
                    WD_CHECK (WT_Point_Set::materialize_16_bit(file));
                } break;
            case 'p':
                {
                    WD_CHECK (WT_Point_Set::materialize(file));
                } break;
            case 'P':
                {    // This is a polygon in ASCII.
                    WD_Assert(file.rendition().fill().fill());
                    WD_CHECK (WT_Point_Set::materialize_ascii(file));
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
void WT_Polygon::update_bounds(WT_File * file)
{
    WT_Drawable::update_bounds(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polygon::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 0x10: // Ctrl-P
                {
                    WD_CHECK (WT_Point_Set::skip_operand_16_bit(file));
                } break;
            case 'p':
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
WT_Result WT_Polygon::process(WT_File & file)
{
    WD_Assert (file.polygon_action());
    return (file.polygon_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Polygon::default_process(WT_Polygon &, WT_File &)
{
    return WT_Result::Success;
}
