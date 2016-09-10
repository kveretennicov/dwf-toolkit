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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/merge_control.cpp 1     9/12/04 8:55p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Merge_Control::object_id() const
{
    return WT_Object::Merge_Control_ID;
}

///////////////////////////////////////////////////////////////////////////
void WT_Merge_Control::set_merge_control(WT_Merge_Control::WT_Merge_Format merge)
{
    m_merge = merge;
}

///////////////////////////////////////////////////////////////////////////
WT_Merge_Control::WT_Merge_Format WT_Merge_Control::get_merge_control() const
{
    return m_merge;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Merge_Control::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(LinesOverwrite "));

    if(get_merge_control() == WT_Merge_Control::Opaque) {
        WD_CHECK (file.write_quoted_string("opaque", WD_True));
    }
    else if (get_merge_control() == WT_Merge_Control::Merge) {
        WD_CHECK (file.write_quoted_string("merge", WD_True));
    }
    else if (get_merge_control() == WT_Merge_Control::Transparent) {
        WD_CHECK (file.write_quoted_string("transparent", WD_True));
    }

    WD_CHECK (file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Merge_Control::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Merge_Control::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() != Merge_Control_ID)
        return WD_False;

    if (attrib.object_id() == Merge_Control_ID &&
        m_merge == ((WT_Merge_Control const &)attrib).m_merge)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Merge_Control::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().merge_control() )
    {
      file.rendition().merge_control() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Merge_Control::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Merge_Control::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            char *  format;

            WD_CHECK (file.read(format, 40));

            if (!strcmp(format, "opaque"))
                m_merge = WT_Merge_Control::Opaque;
            else if (!strcmp(format, "merge"))
                m_merge = WT_Merge_Control::Merge;
            else if (!strcmp(format, "transparent"))
                m_merge = WT_Merge_Control::Transparent;

            delete [] format;

            WD_CHECK (opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Merge_Control::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Merge_Control::process(WT_File & file)
{
    WD_Assert(file.merge_control_action());
    return (file.merge_control_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Merge_Control::default_process(WT_Merge_Control & item, WT_File & file)
{
    file.rendition().merge_control() = item;
    return WT_Result::Success;
}
