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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/projection.cpp 1     9/12/04 8:56p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Projection::object_id() const
{
    return WT_Object::Projection_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Projection::object_type() const
{
    return WT_Object::Attribute;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::materialize(
    WT_Opcode const & opcode,
    WT_File & file,
    WT_String_Interpreter interpret )
{
    char* string = 0;

    if( opcode.type() == WT_Opcode::Extended_Binary )
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    WT_Integer16 value;
    if( opcode.type() == WT_Opcode::Extended_ASCII )
        switch( m_stage )
        {
        case Eating_Initial_Whitespace:
            WD_CHECK( file.eat_whitespace() );
            m_stage = Gathering_String;
            // No Break Here

        case Gathering_String:
            WD_CHECK( file.read(string,65536) );
            WD_CHECK( interpret(string,m_value) );
            delete [] string;
            m_stage = Eating_End_Whitespace;
            // No Break here

        case Eating_End_Whitespace:
            WD_CHECK( opcode.skip_past_matching_paren(file) );
            m_stage = Eating_Initial_Whitespace;
        }
    else
    {
        WD_CHECK( file.read(value) );
        m_value = (WT_Projection_Type)value;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::skip_operand(
    WT_Opcode const & opcode,
    WT_File& file )
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Projection::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write ("(Projection "));
    switch (m_value)
    {
    case Normal: WD_CHECK (file.write_quoted_string ("normal"));     break;
    case Stretch: WD_CHECK (file.write_quoted_string ("stretch"));    break;
    case Chop: WD_CHECK (file.write_quoted_string ("chop"));       break;
    default:                                                    return WT_Result::Internal_Error;
    }
    return    file.write (")");
}
#else
WT_Result WT_Projection::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::interpret_string (char const * string, WT_Projection_Type& value)
{
    if (!strcmp (string, "normal")) {
        value = Normal;
        return WT_Result::Success;
    }
    if (!strcmp (string, "stretch")) {
        value = Stretch;
        return WT_Result::Success;
    }
    if (!strcmp (string, "chop")) {
        value = Chop;
        return WT_Result::Success;
    }
    return WT_Result::Corrupt_File_Error;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::process( WT_File& file )
{
    WD_Assert (file.projection_action());
    return (file.projection_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::default_process( WT_Projection &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Projection::operator== ( WT_Attribute const & attrib ) const
{
    if( attrib.object_id() == Projection_ID
        && m_value == ((WT_Projection const &)attrib).m_value)
        return WD_True;
    else
        return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Projection::sync( WT_File& file ) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if( *this != file.rendition().projection() )
    {
        file.rendition().projection() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
