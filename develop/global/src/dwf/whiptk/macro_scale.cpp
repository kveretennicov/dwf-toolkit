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


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Macro_Scale::object_id() const
{
    return Macro_Scale_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Macro_Scale::serialize(WT_File & file) const
{
    // Macro_Size is only supported in versions 6.01 and up . 
    if (file.heuristics().target_version() < REVISION_WHEN_MACRO_IS_SUPPORTED)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) 's'));
        return    file.write(m_scale);
    }
    else
    {
        // ASCII version.
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("S "));
        return    file.write_ascii(m_scale);
    }
}
#else
WT_Result WT_Macro_Scale::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Macro_Scale::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Macro_Scale_ID &&
        m_scale == ((WT_Macro_Scale const &)attrib).m_scale)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Macro_Scale::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if (*this != file.rendition().macro_scale())
    {
        file.rendition().macro_scale() = *this;
        return serialize(file);
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Macro_Scale::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Scale::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            if (opcode.token()[0] == 's')
            {
                WD_CHECK (file.read(m_scale));
            }
            else if (opcode.token()[0] == 'S')
            {
                WD_CHECK (file.read_ascii(m_scale));
            }
            else
            {
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            }
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
WT_Result WT_Macro_Scale::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Scale::process(WT_File & file)
{
    WD_Assert (file.macro_scale_action());
    return (file.macro_scale_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Scale::default_process(WT_Macro_Scale & item, WT_File & file)
{
    file.rendition().macro_scale() = item;
    return WT_Result::Success;
}
