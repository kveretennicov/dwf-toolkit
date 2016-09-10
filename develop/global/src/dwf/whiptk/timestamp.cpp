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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/timestamp.cpp 3     5/09/05 12:41a Evansg $


#include "whiptk/pch.h"

IMPLEMENT_TIMESTAMP_CLASS_FUNCTIONS (Creation_Time,            creation_time,            "Created")
IMPLEMENT_TIMESTAMP_CLASS_FUNCTIONS (Modification_Time,        modification_time,        "Modified")
IMPLEMENT_TIMESTAMP_CLASS_FUNCTIONS (Source_Creation_Time,     source_creation_time,     "SourceCreated")
IMPLEMENT_TIMESTAMP_CLASS_FUNCTIONS (Source_Modification_Time, source_modification_time, "SourceModified")

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Timestamp::object_type(void) const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Timestamp::serialize(WT_File & file, char const * command) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write ("("));
    WD_CHECK (file.write (command));
    WD_CHECK (file.write (" "));
    WD_CHECK (file.write_ascii (m_seconds));
    WD_CHECK (file.write (" "));
    WD_CHECK (m_string.serialize(file));
    WD_CHECK (file.write (" "));
    WD_CHECK (m_guid.serialize(file));
    return    file.write (")");
}
#else
WT_Result WT_Timestamp::serialize(WT_File &, char const *) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Timestamp::materialize(WT_Opcode const & opcode, WT_File & file)
{

    if (opcode.type() != WT_Opcode::Extended_ASCII)
        return WT_Result::Opcode_Not_Valid_For_This_Object;

    switch (m_stage)
    {
    case Eating_Initial_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_Seconds;
        // No Break here

    case Gathering_Seconds:
        WD_CHECK (file.read_ascii (m_seconds));
        m_stage = Eating_Middle_Whitespace;
        // No Break here

    case Eating_Middle_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_String;
        // No Break here

    case Gathering_String:
        WD_CHECK (m_string.materialize(file));
        m_stage = Eating_Next_Whitespace;
        // No Break here

    case Eating_Next_Whitespace:
        WD_CHECK (file.eat_whitespace());
        m_stage = Gathering_GUID;
        // No Break here

    case Gathering_GUID:
        WD_CHECK (m_guid.materialize(file));
        m_stage = Eating_End_Whitespace;
        // No Break here

    case Eating_End_Whitespace:
        WD_CHECK (opcode.skip_past_matching_paren(file));
        m_stage = Eating_Initial_Whitespace;
    }

    m_materialized = WD_True;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Timestamp::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    return opcode.skip_past_matching_paren(file);
}

///////////////////////////////////////////////////////////////////////////
WT_Timestamp::WT_Timestamp(WT_Integer32 seconds, char const * string)
    : m_stage (Eating_Initial_Whitespace)
{
    set(seconds, string);
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(char const * string)
{
    m_string = (WT_Byte*) string;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set_guid(WT_String const & guid)
{
    m_guid = guid;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_Unsigned_Integer16 const * string)
{
    m_string = string;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_String const & string)
{
    m_string = string;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_Integer32 seconds, char const * string)
{
    set(string);
    m_seconds = seconds;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_Integer32 seconds, WT_String const & string)
{
    set(string);
    m_seconds = seconds;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_Integer32 seconds, WT_Unsigned_Integer16 const * string)
{
    set(string);
    m_seconds = seconds;
}

///////////////////////////////////////////////////////////////////////////
void WT_Timestamp::set(WT_Timestamp const & ts)
{
    set(ts.seconds(), ts.string());
    set_guid(ts.guid());
}
