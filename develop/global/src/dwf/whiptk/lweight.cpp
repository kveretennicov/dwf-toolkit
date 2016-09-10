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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/lweight.cpp 1     9/12/04 8:54p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Line_Weight::object_id() const
{
    return Line_Weight_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Line_Weight::operator== ( WT_Attribute const & attrib ) const
{
    WT_ID id = attrib.object_id();
    if( id != Line_Weight_ID )
        return WD_False;
    WT_Line_Weight const & lw =
        (WT_Line_Weight const &) attrib;
    return ( m_weight == lw.m_weight );
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Weight::sync( WT_File& file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().line_weight() )
    {   file.rendition().line_weight() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Line_Weight::sync( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Weight::process( WT_File& file )
{
    WD_Assert( file.line_weight_action() );
    return (file.line_weight_action())(*this,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Weight::default_process( WT_Line_Weight& item, WT_File& file )
{
    file.rendition().line_weight() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Line_Weight::serialize( WT_File& file ) const
{
    WD_CHECK( file.dump_delayed_drawable() );

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if( file.heuristics().allow_binary_data() &&
        file.heuristics().target_version() >= REVISION_WHEN_WIDE_LINES_ADDED)
    {   WD_CHECK( file.write((WT_Byte)0x17) );// CTRL-W
        WD_CHECK( file.write(m_weight) );
    }
    else /* ascii */
    {   WD_CHECK( file.write_tab_level() );
        WD_CHECK( file.write("(LineWeight ") );
        WD_CHECK( file.write_ascii(m_weight) );
        WD_CHECK( file.write((WT_Byte) ')') );
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Line_Weight::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Weight::materialize( WT_Opcode const & opcode, WT_File& file )
{
    switch( opcode.type() )
    {
    case WT_Opcode::Extended_ASCII:

        switch( m_stage )
        {
        case Eating_Initial_Whitespace:
            WD_CHECK( file.eat_whitespace() );
            m_stage = Getting_Weight;
            // No Break Here
        case Getting_Weight:
            WD_CHECK( file.read_ascii(m_weight) );
            m_stage = Eating_End_Whitespace;
            // No Break Here
        case Eating_End_Whitespace:
            WD_CHECK( opcode.skip_past_matching_paren(file) );
            m_stage = Eating_Initial_Whitespace;
        }
        break;

    case WT_Opcode::Single_Byte:

        if( opcode.token()[0] != 0x17/*CTRL-W*/ )
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        WD_CHECK( file.read(m_weight) );
        break;

    case WT_Opcode::Extended_Binary:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }

    m_materialized = WD_True;

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Line_Weight::skip_operand( WT_Opcode const & opcode, WT_File& file )
{
    switch( opcode.type() )
    {
    case WT_Opcode::Extended_ASCII:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        break;

    case WT_Opcode::Single_Byte:
        WD_CHECK( file.skip(sizeof(WT_Integer32)) );
        break;

    case WT_Opcode::Extended_Binary:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }

    return WT_Result::Success;
}
