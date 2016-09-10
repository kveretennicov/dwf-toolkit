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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/marksymb.cpp 3     2/25/05 5:54p Bangiav $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Marker_Symbol::object_id() const
{
    return Marker_Symbol_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Marker_Symbol::serialize(WT_File & file) const
{
    // Marker_Symbol is deprecated in Tollkit version 6.01 onwards . Its opcode 
    // is reused for Macro_Index for versions 6.01 and up.
    if (file.heuristics().target_version() >= REVISION_WHEN_MACRO_IS_SUPPORTED)
        return WT_Result::Toolkit_Usage_Error;

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    char opcode_space[3];
    opcode_space[0] = WD_SBAO_SET_MARKER_SYMBOL;  
    opcode_space[1] = ' ';
    opcode_space[2] = 0;
    WD_CHECK(file.write_geom_tab_level());
    WD_CHECK(file.write(opcode_space));
    return file.write_ascii(m_symbol);
}
#else
WT_Result WT_Marker_Symbol::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Marker_Symbol::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Marker_Symbol_ID &&
        m_symbol == ((WT_Marker_Symbol const &)attrib).m_symbol)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Marker_Symbol::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if (*this != file.rendition().marker_symbol())
    {
        file.rendition().marker_symbol() = *this;
        return serialize(file);
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Marker_Symbol::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Integer32 WT_Marker_Symbol::symbol() const
{
    return m_symbol;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Marker_Symbol::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            if (opcode.token()[0] == WD_SBAO_SET_MARKER_SYMBOL)
            {
                WD_CHECK (file.read_ascii(m_symbol));
            }
            else
            {
                // There are no other single byte opcodes for marker symbol
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            }
        } break;
    case WT_Opcode::Extended_ASCII:
    case WT_Opcode::Extended_Binary:
    default:
        {
            // There are no extended ascii or binary opcodes for marker symbol
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Marker_Symbol::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return materialize(opcode, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Marker_Symbol::process(WT_File & file)
{
    WD_Assert (file.marker_symbol_action());
    return (file.marker_symbol_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Marker_Symbol::default_process(WT_Marker_Symbol & item, WT_File & file)
{
    file.rendition().marker_symbol() = item;
    return WT_Result::Success;
}
