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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/view.cpp 1     9/12/04 8:57p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_View::object_id() const
{
    return View_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_View::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(View "));

    if (!m_set_by_name)
    {
        WT_Logical_Point        adjusted_min;
        WT_Logical_Point        adjusted_max;

        WT_Logical_Point        final_min;
        WT_Logical_Point        final_max;

        if (file.heuristics().apply_transform())
        {
            WT_Transform const &    xform        = file.heuristics().transform();
            WT_Logical_Point        orig_xformed_min_cnr;
            WT_Logical_Point        orig_xformed_max_cnr;

            orig_xformed_min_cnr = m_view.m_min * xform;
            orig_xformed_max_cnr = m_view.m_max * xform;

            switch (xform.rotation())
            {
                case 0:
                    adjusted_min = orig_xformed_min_cnr;
                    adjusted_max = orig_xformed_max_cnr;
                    break;
                case 90:
                    adjusted_min = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                    adjusted_max = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                    break;
                case 180:
                    adjusted_min = orig_xformed_max_cnr;
                    adjusted_max = orig_xformed_min_cnr;
                    break;
                case 270:
                    adjusted_min = WT_Logical_Point(orig_xformed_min_cnr.m_x, orig_xformed_max_cnr.m_y);
                    adjusted_max = WT_Logical_Point(orig_xformed_max_cnr.m_x, orig_xformed_min_cnr.m_y);
                    break;
                default:
                    throw WT_Result::Internal_Error;
            }
        }
        else
        {
            adjusted_min = m_view.m_min;
            adjusted_max = m_view.m_max;
        }

        final_min.m_x = WD_MIN(adjusted_min.m_x, adjusted_max.m_x);
        final_min.m_y = WD_MIN(adjusted_min.m_y, adjusted_max.m_y);

        final_max.m_x = WD_MAX(adjusted_min.m_x, adjusted_max.m_x);
        final_max.m_y = WD_MAX(adjusted_min.m_y, adjusted_max.m_y);

        WD_CHECK (file.write_ascii(1, &final_min));
        WD_CHECK (file.write(" "));
        WD_CHECK (file.write_ascii(1, &final_max));
    }
    else
    {
        // Force single quotes so that we don't have to parse
        //  the view when we materialize to see if it's a view
        //  or a string.
        WD_CHECK (m_name.serialize(file,WD_True));
    }
    return    file.write(")");
}
#else
WT_Result WT_View::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_View::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == View_ID &&
        m_view == ((WT_View const &)attrib).m_view)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_View::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().rendering_options().view())
    {
        // Before we copy the view, make sure that if a
        //  name is specified that
        //      1. the corresponding named view has been serialized from
        //          the rendition
        //      TODO:  2. the named view's logical box has been copied into
        //          *this
        if (m_set_by_name)
        {
            WT_Named_View * desired_named_view
             = file.desired_rendition().drawing_info().named_view_list().find_named_view_from_name (m_name);

            // Did we find an associated named view?
            if (desired_named_view)
            {   // yes
                desired_named_view->serialize(file);
            }
            else // nope
                return WT_Result::File_Inconsistency_Warning;
        }
        file.rendition().rendering_options().view() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_View::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_View::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Eating_Initial_Whitespace:
                WD_CHECK (file.eat_whitespace());
                m_stage = Determining_String_Or_View;

            case Determining_String_Or_View:
                // We need to check to see if a logical view or a quoted string
                //     follows.
                WT_Byte        a_byte;

                WD_CHECK (file.read(a_byte));

                 // If a quoted string follows (representing a named view) we need to putback
                 //     the single quote and get the string.
                if (a_byte == 0x27) // single quote
                    m_set_by_name = WD_True;
                else
                    m_set_by_name = WD_False;
                file.put_back(a_byte);   // put the byte back, whether quote or beginning of view

                m_stage = Getting_View;

                // No break
            case Getting_View:
                if (m_set_by_name)
                {
                    // Name is given
                    if (!file.rendition().drawing_info().named_view_list().is_empty())
                    {
                        // List is not empty.
                        WD_CHECK (m_name.materialize(file));
                        if (m_name != WT_String::kNull)
                        {

                            // Search named view list for view associated with this name
                            WT_Named_View * desired_named_view
                             = file.desired_rendition().drawing_info().named_view_list().find_named_view_from_name (m_name);

                            // Did we find an associated named view?
                            if (desired_named_view)
                            {   // yes
                                set ( *(desired_named_view->view()) );
                            }
                            else // no
                            {
                                // We did not find any associated view associated with this name in the
                                //      rendition so return corrupt file error.
                                return WT_Result::Corrupt_File_Error;
                            }
                        } // if name
                    } // if named_view_list is empty
                    else
                    {
                        // List is empty.
                        // We did not find any associated view associated with this name in the
                        //      rendition named view list (since it's empty) so return corrupt file error
                        return WT_Result::Corrupt_File_Error;
                    }
                    m_stage = Eating_Trailing_Whitespace;
                }
                else
                {
                    // Simply read in the logical view from the file
                    WD_CHECK(file.read_ascii(m_view));
                    m_stage = Eating_Trailing_Whitespace;
                }
                // No break

            case Eating_Trailing_Whitespace:
                WD_CHECK(opcode.skip_past_matching_paren(file));
                // No break
            } // switch (m_stage)
        } break;
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    if (file.heuristics().apply_transform())
    {
        m_view.minpt() *= file.heuristics().transform();
        m_view.maxpt() *= file.heuristics().transform();
    }

    // We have finished materializing, reset the state to the beginning.
    m_stage = Eating_Initial_Whitespace;
    m_materialized = WD_True;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_View::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_View::process(WT_File & file)
{
    WD_Assert (file.view_action());
    return (file.view_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_View::default_process(WT_View & item, WT_File & file)
{
    file.rendition().rendering_options().view() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_View::set (char const * name)
{
    m_name = (WT_Byte const *)name;
}

///////////////////////////////////////////////////////////////////////////
void WT_View::set (WT_Unsigned_Integer16 const * name)
{
    m_name = name;
}

///////////////////////////////////////////////////////////////////////////
void WT_View::set (WT_String const & name)
{
    m_name = name;
}

///////////////////////////////////////////////////////////////////////////
void WT_View::set (WT_Logical_Box const & box)
{
    m_view = box;
}

///////////////////////////////////////////////////////////////////////////
WT_View const & WT_View::operator=(WT_View const & view)
{
    m_view = view.view();
    m_name = view.name();
    return *this;
}
