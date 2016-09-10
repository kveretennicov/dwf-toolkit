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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/fillpat.cpp 3     5/25/05 1:53p Hainese $
#include "whiptk/pch.h"

const char* WT_Fill_Pattern::m_names[Count] =
{
    "",            // Index zero is not a legal fill pattern
    "Solid",
    "Checkerboard",
    "Crosshatch",
    "Diamonds",
    "Horizontal Bars",
    "Slant Left",
    "Slant Right",
    "Square Dots",
    "Vertical Bars",
    "User Defined"
};

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Fill_Pattern::object_id() const
{   return Fill_Pattern_ID; }


///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Fill_Pattern::operator== ( WT_Attribute const & atref ) const
{   if( atref.object_id() != Fill_Pattern_ID )
        return WD_False;
    WT_Fill_Pattern const & r =
        (WT_Fill_Pattern const &) atref;
    return (*this == r );
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Fill_Pattern::operator== ( WT_Fill_Pattern const & r ) const
{   if( m_fields_defined != r.m_fields_defined
        || m_pattern_scale != r.pattern_scale() )
        return WD_False;
    if(r.m_id != m_id)
        return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Fill_Pattern::operator!= ( WT_Fill_Pattern const & r ) const
{
    return !((*this) == r);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Fill_Pattern::sync( WT_File& file ) const
{

    WD_Assert( (file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write) );

    if( *this != file.rendition().fill_pattern() )
    {   return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Fill_Pattern::sync( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill_Pattern::process( WT_File& file )
{
    WD_Assert( file.fill_pattern_action() );
    return (file.fill_pattern_action())(*this,file);
}

///////////////////////////////////////////////////////////////////////////
void WT_Fill_Pattern::merge( WT_Fill_Pattern const & style )
{
    if( style.m_fields_defined & PATTERN_SCALE_BIT )
        m_pattern_scale = style.pattern_scale();
}

WT_Result WT_Fill_Pattern::default_process(
    WT_Fill_Pattern& item,
    WT_File& file )
{
    file.rendition().fill_pattern().merge(item);
    file.rendition().fill_pattern() = item;
    return WT_Result::Success;
}

WT_Result WT_Fill_Pattern::skip_operand(
    WT_Opcode const & opcode,
    WT_File& file )
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill_Pattern::materialize(
    WT_Opcode const & opcode,
    WT_File& file )
{
    switch( opcode.type() )
    {
    case WT_Opcode::Extended_ASCII:
        return materialize_ascii(opcode,file);
    case WT_Opcode::Single_Byte:
    case WT_Opcode::Extended_Binary:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill_Pattern::materialize_ascii(
    WT_Opcode const & opcode,
    WT_File& file )
{
    char* string = 0;
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Pattern_ID;
        // No Break Here

    case Getting_Pattern_ID:
        WD_CHECK( file.read(string,65536) );
        WD_CHECK( interpret(string,m_id) );
        delete [] string;
        m_stage = Checking_For_Attribute_End;
        // No Break here
    case Checking_For_Attribute_End:
        WT_Byte a_byte;
        WD_CHECK( file.read(a_byte) );
        file.put_back(a_byte);
        if( a_byte == ')' )
        {   m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Getting_Optioncode;
        // No Break Here

next_option:
    case Getting_Optioncode:
        WD_CHECK( m_optioncode.get_optioncode(file) );
        if( m_optioncode.type() == WT_Opcode::Null_Optioncode )
        {
            m_stage = Eating_End_Whitespace;
            goto end_ascii;
        }
        m_stage = Materializing_Option;
        // No Break Here

    case Materializing_Option:
        switch( m_optioncode.option_id() )
        {
        case WT_Fill_Pattern_Option_Code::Pattern_Scale_Option:
            m_fields_defined |= PATTERN_SCALE_BIT;
            WD_CHECK( m_pattern_scale.materialize(*this,m_optioncode,file) );
            break;
        default:
            return WT_Result::Internal_Error;
        } // switch (m_optioncode.option_id())
        m_stage = Getting_Optioncode;
        goto next_option;
        // No Break Here

end_ascii:
    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Fill_Pattern::serialize( WT_File& file ) const
{
    WD_CHECK( file.dump_delayed_drawable() );

    WD_Assert (m_id > 0);

    // Determine required parts of this Attribute object.
    WT_Fill_Pattern &       style   = file.rendition().fill_pattern();
    WT_Unsigned_Integer32   fields  = 0;

    if( m_fields_defined & PATTERN_SCALE_BIT &&
        m_pattern_scale != style.pattern_scale() )
            fields |= PATTERN_SCALE_BIT;

    // Serialize this pattern's attributes if this pattern has never been serialized,
    // or if one this pattern's attributes is different from its previous instance in
    // the file.
    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(FillPattern "));
    const char* pName = m_names[m_id];
    WD_CHECK( file.write_quoted_string(pName) );

    if( fields & PATTERN_SCALE_BIT )
    {
        WD_CHECK( file.write((WT_Byte)' ') );
        WD_CHECK( m_pattern_scale.serialize(*this,file) );
    }

    file.rendition().fill_pattern() = *this;

    WD_CHECK( file.write(")") );
    return WT_Result::Success;
}
#else
WT_Result WT_Fill_Pattern::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Fill_Pattern::interpret(
    char const *   string,
    WT_Pattern_ID& value)
{
    // Match the text string defining the fill
    // pattern with the names (normal and alternate)
    // in our lists of valid pattern names.
    for (int i = 0; i < Count; i++)
    {
        if (!strcmp(string,m_names[i]))
        {
            value = (WT_Pattern_ID)i;
            return WT_Result::Success;
        }
    }

    //We don't return WT_Result::Corrupt_File_Error
    //because, we want to skip in future the patterns
    //that gets newly added (if at all a need arise
    //to include new fill patterns).

    value = (WT_Pattern_ID)1;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
int WT_Fill_Pattern::WT_Fill_Pattern_Option_Code::option_id_from_optioncode()
{
    if( !strcmp((char const *)token(), "(FillPatternScale") )
        return WT_Fill_Pattern::WT_Fill_Pattern_Option_Code::Pattern_Scale_Option;
    return 0;
}
