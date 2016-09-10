//  Copyright (c) 2001-2006 by Autodesk, Inc.
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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/filetime.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_FileTime::object_id() const
{
    return FileTime_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_FileTime::object_type() const
{
    return Informational;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_FileTime::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer32) + // high_date_time
                                             sizeof(WT_Unsigned_Integer32) + // low_date_time
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_FILETIME));
        WD_CHECK (file.write((WT_Unsigned_Integer32) low_date_time()));
        WD_CHECK (file.write((WT_Unsigned_Integer32) high_date_time()));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else {

        //Extended ASCII format
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Time "));
        WD_CHECK (file.write_padded_ascii(m_low_date_time));
        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_padded_ascii(m_high_date_time));
        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_FileTime::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_FileTime::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read_ascii(m_low_date_time));
            WD_CHECK (file.eat_whitespace());
            WD_CHECK (file.read_ascii(m_high_date_time));
            WD_CHECK (opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Low_Date_Time;
            // No break;

        case Getting_Low_Date_Time:
            WD_CHECK (file.read(m_low_date_time));
            m_stage = Getting_High_Date_Time;

            // No break;
        case Getting_High_Date_Time:
            WD_CHECK (file.read(m_high_date_time));
            m_stage = Getting_Close;

        // No break
        case Getting_Close:
            WT_Byte close_brace;

            WD_CHECK (file.read(close_brace));
            if (close_brace != '}')
                return WT_Result::Corrupt_File_Error;
            m_stage = Starting;
            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.
        } // switch (m_stage) for Binary
        break;

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
WT_Result WT_FileTime::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_FileTime::process(WT_File & file)
{
    WD_Assert (file.filetime_action());
    return (file.filetime_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_FileTime::default_process(WT_FileTime &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_FileTime::operator== (WT_FileTime const & time) const
{
    if(time.object_id() != FileTime_ID)
        return WD_False;

    if(time.low_date_time() != low_date_time())
        return WD_False;

    if(time.high_date_time() != high_date_time())
        return WD_False;

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_FileTime::operator!= (WT_FileTime const & attrib) const
{
    return !(*this == attrib);
}
