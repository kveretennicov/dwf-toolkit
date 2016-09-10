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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/fill.cpp 3     5/10/05 7:18p Bangiav $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Fill::object_id() const
{   return Fill_ID; }

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Fill::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    if (m_fill)
        return file.write((WT_Byte) 'F');
    else
        return file.write((WT_Byte) 'f');
}
#else
WT_Result WT_Fill::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Fill::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Fill_ID &&
        m_fill == ((WT_Fill const &)attrib).m_fill)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Fill::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().fill())
    {
        file.rendition().fill() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Fill::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill::materialize(WT_Opcode const & opcode, WT_File &)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            switch (opcode.token()[0])
            {
            case 'F':
                {
                    m_fill = WD_True;
                } break;
            case 'f':
                {
                    m_fill = WD_False;
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
WT_Result WT_Fill::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill::process(WT_File & file)
{
    WD_Assert(file.fill_action());
    return (file.fill_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill::default_process(WT_Fill & item, WT_File & file)
{
    file.rendition().fill() = item;
    file.rendition().delineate() = WT_Delineate(WD_False);
    return WT_Result::Success;
}
