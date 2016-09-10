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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/blockref_defs.cpp 1     9/12/04 8:51p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Block_Meaning::object_id() const
{
    return Block_Meaning_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Block_Meaning::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
void WT_Block_Meaning::set_block_description(
                       WT_Block_Meaning::WT_Block_Description description)
{
    m_description = description;
}

///////////////////////////////////////////////////////////////////////////
WT_Block_Meaning::WT_Block_Description
            WT_Block_Meaning::get_block_description() const
{
    return m_description;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Block_Meaning::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer16) + // block description
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_BLOCK_MEANING));
        WD_CHECK (file.write((WT_Unsigned_Integer16) get_block_description()));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else {

        //Extended ASCII format
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(BlockMeaning "));

        if(get_block_description() == WT_Block_Meaning::None) {
            WD_CHECK (file.write_quoted_string("None     ", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Seal) {
            WD_CHECK (file.write_quoted_string("Seal     ", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Stamp) {
            WD_CHECK (file.write_quoted_string("Stamp    ", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Label) {
            WD_CHECK (file.write_quoted_string("Label    ", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Redline) {
            WD_CHECK (file.write_quoted_string("Redline  ", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Reserved1) {
            WD_CHECK (file.write_quoted_string("Reserved1", WD_True));
        }
        else if (get_block_description() == WT_Block_Meaning::Reserved2) {
            WD_CHECK (file.write_quoted_string("Reserved2", WD_True));
        }

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Block_Meaning::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Block_Meaning::materialize(WT_Opcode const & opcode,
                                        WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    WT_Unsigned_Integer16 description;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            char *  format;

            WD_CHECK (file.read(format, 40));

            if (!strcmp(format, "None     "))
                m_description = WT_Block_Meaning::None;
            else if (!strcmp(format, "Seal     "))
                m_description = WT_Block_Meaning::Seal;
            else if (!strcmp(format, "Stamp    "))
                m_description = WT_Block_Meaning::Stamp;
            else if (!strcmp(format, "Label    "))
                m_description = WT_Block_Meaning::Label;
            else if (!strcmp(format, "Redline  "))
                m_description = WT_Block_Meaning::Redline;
            else if (!strcmp(format, "Reserved1"))
                m_description = WT_Block_Meaning::Reserved1;
            else if (!strcmp(format, "Reserved2"))
                m_description = WT_Block_Meaning::Reserved2;

            WD_CHECK (opcode.skip_past_matching_paren(file));
            delete format;
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Description;
            // No break;

        case Getting_Description:
            WD_CHECK (file.read(description));
            switch(description)
            {
            case WT_Block_Meaning::None:
                m_description = WT_Block_Meaning::None;
                break;
            case WT_Block_Meaning::Seal:
                m_description = WT_Block_Meaning::Seal;
                break;
            case WT_Block_Meaning::Stamp:
                m_description = WT_Block_Meaning::Stamp;
                break;
            case WT_Block_Meaning::Label:
                m_description = WT_Block_Meaning::Label;
                break;
            case WT_Block_Meaning::Redline:
                m_description = WT_Block_Meaning::Redline;
                break;
            case WT_Block_Meaning::Reserved1:
                m_description = WT_Block_Meaning::Reserved1;
                break;
            case WT_Block_Meaning::Reserved2:
                m_description = WT_Block_Meaning::Reserved2;
                break;
            default:
                return WT_Result::Internal_Error;
            }
            m_stage = Getting_Close;

            // No break
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

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
WT_Boolean    WT_Block_Meaning::operator!= (WT_Block_Meaning
                                            const & meaning) const
{
    return !(*this == meaning);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Block_Meaning::operator== (WT_Block_Meaning
                                            const & meaning) const
{
    if(get_block_description() == meaning.get_block_description())
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Block_Meaning::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Block_Meaning::process(WT_File & file)
{
    WD_Assert(file.block_meaning_action());
    return (file.block_meaning_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Block_Meaning::default_process(WT_Block_Meaning &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Encryption::object_id() const
{
    return Encryption_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Encryption::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
void WT_Encryption::set_encryption_description(
                   WT_Encryption::WT_Encryption_Description description)
{
    m_description = description;
}

///////////////////////////////////////////////////////////////////////////
WT_Encryption::WT_Encryption_Description
        WT_Encryption::get_encryption_description() const
{
    return m_description;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Encryption::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer16) + // encryption description
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_ENCRYPTION));
        WD_CHECK (file.write((WT_Unsigned_Integer16)
            get_encryption_description()));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else {

        //Extended ASCII format
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Encryption "));

        if(get_encryption_description() == WT_Encryption::None) {
            WD_CHECK (file.write_quoted_string("None     "));
        }
        else if (get_encryption_description() == WT_Encryption::Reserved1) {
            WD_CHECK (file.write_quoted_string("Reserved1", WD_True));
        }
        else if (get_encryption_description() == WT_Encryption::Reserved2) {
            WD_CHECK (file.write_quoted_string("Reserved2", WD_True));
        }
        else if (get_encryption_description() == WT_Encryption::Reserved3) {
            WD_CHECK (file.write_quoted_string("Reserved3", WD_True));
        }

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Encryption::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Encryption::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    WT_Unsigned_Integer16 description;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            char *  format;

            WD_CHECK (file.read(format, 40));

            if (!strcmp(format, "None     "))
                m_description = WT_Encryption::None;
            else if (!strcmp(format, "Reserved1"))
                m_description = WT_Encryption::Reserved1;
            else if (!strcmp(format, "Reserved2"))
                m_description = WT_Encryption::Reserved2;
            else if (!strcmp(format, "Reserved3"))
                m_description = WT_Encryption::Reserved3;
            else
                return WT_Result::Internal_Error;

            WD_CHECK (opcode.skip_past_matching_paren(file));
            delete format;
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Description;
            // No break;

        case Getting_Description:
            WD_CHECK (file.read(description));
            switch(description)
            {
            case WT_Encryption::None:
                m_description = WT_Encryption::None;
                break;
            case WT_Encryption::Reserved1:
                m_description = WT_Encryption::Reserved1;
                break;
            case WT_Encryption::Reserved2:
                m_description = WT_Encryption::Reserved2;
                break;
            case WT_Encryption::Reserved3:
                m_description = WT_Encryption::Reserved3;
                break;
            default:
                return WT_Result::Internal_Error;
            }
            m_stage = Getting_Close;

            // No break
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

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
WT_Boolean    WT_Encryption::operator!= (WT_Encryption const & encryption) const
{
    return !(*this == encryption);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Encryption::operator== (WT_Encryption const & encryption) const
{
    if(get_encryption_description() == encryption.get_encryption_description())
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Encryption::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Encryption::process(WT_File & file)
{
    WD_Assert(file.encryption_action());
    return (file.encryption_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Encryption::default_process(WT_Encryption &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Orientation::object_id() const
{
    return Orientation_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Orientation::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
void WT_Orientation::set_orientation_description(WT_Orientation::WT_Orientation_Description description)
{
    m_description = description;
}

///////////////////////////////////////////////////////////////////////////
WT_Orientation::WT_Orientation_Description WT_Orientation::get_orientation_description() const
{
    return m_description;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Orientation::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer16) + // orientation description
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_ORIENTATION));
        WD_CHECK (file.write((WT_Unsigned_Integer16)
            get_orientation_description()));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else {

        //Extended ASCII format
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Orientation "));

        if(get_orientation_description() == WT_Orientation::Always_In_Sync) {
            WD_CHECK (file.write_quoted_string("Always_In_Sync  "));
        }
        else if (get_orientation_description() == WT_Orientation::Always_Different) {
            WD_CHECK (file.write_quoted_string("Always_Different", WD_True));
        }
        else if (get_orientation_description() == WT_Orientation::Decoupled) {
            WD_CHECK (file.write_quoted_string("Decoupled       ", WD_True));
        }

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Orientation::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Orientation::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    WT_Unsigned_Integer16 description;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            char *  format;

            WD_CHECK (file.read(format, 40));

            if (!strcmp(format, "Always_In_Sync  "))
                m_description = WT_Orientation::Always_In_Sync;
            else if (!strcmp(format, "Always_Different"))
                m_description = WT_Orientation::Always_Different;
            else if (!strcmp(format, "Decoupled       "))
                m_description = WT_Orientation::Decoupled;
            else
                return WT_Result::Internal_Error;

            WD_CHECK (opcode.skip_past_matching_paren(file));
            delete format;
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Description;
            // No break;

        case Getting_Description:
            WD_CHECK (file.read(description));
            switch(description)
            {
            case WT_Orientation::Always_In_Sync:
                m_description = WT_Orientation::Always_In_Sync;
                break;
            case WT_Orientation::Always_Different:
                m_description = WT_Orientation::Always_Different;
                break;
            case WT_Orientation::Decoupled:
                m_description = WT_Orientation::Decoupled;
                break;
            default:
                return WT_Result::Internal_Error;
            }
            m_stage = Getting_Close;

            // No break
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

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
WT_Boolean    WT_Orientation::operator!= (WT_Orientation
                                          const & orientation) const
{
    return !(*this == orientation);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Orientation::operator== (WT_Orientation
                                          const & orientation) const
{
    if(get_orientation_description() == orientation.get_orientation_description())
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Orientation::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Orientation::process(WT_File & file)
{
    WD_Assert(file.orientation_action());
    return (file.orientation_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Orientation::default_process(WT_Orientation &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Alignment::object_id() const
{
    return Alignment_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Alignment::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
void WT_Alignment::set_alignment_description(
                   WT_Alignment::WT_Alignment_Description description)
{
    m_description = description;
}

///////////////////////////////////////////////////////////////////////////
WT_Alignment::WT_Alignment_Description
            WT_Alignment::get_alignment_description() const
{
    return m_description;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Alignment::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Unsigned_Integer16) + // orientation description
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));
        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_ALIGNMENT));
        WD_CHECK (file.write((WT_Unsigned_Integer16)
            get_alignment_description()));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else {

        //Extended ASCII format
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Alignment "));

        if(get_alignment_description() == WT_Alignment::Align_Center) {
            WD_CHECK (file.write_quoted_string("Align_Center      "));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Title_Block) {
            WD_CHECK (file.write_quoted_string("Align_Title_Block ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Top) {
            WD_CHECK (file.write_quoted_string("Align_Top         ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Bottom) {
            WD_CHECK (file.write_quoted_string("Align_Bottom      ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Left) {
            WD_CHECK (file.write_quoted_string("Align_Left        ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Right) {
            WD_CHECK (file.write_quoted_string("Align_Right       ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Top_Left) {
            WD_CHECK (file.write_quoted_string("Align_Top_Left    ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Top_Right) {
            WD_CHECK (file.write_quoted_string("Align_Top_Right   ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Bottom_Left) {
            WD_CHECK (file.write_quoted_string("Align_Bottom_Left ", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_Bottom_Right) {
            WD_CHECK (file.write_quoted_string("Align_Bottom_Right", WD_True));
        }
        else if (get_alignment_description() == WT_Alignment::Align_None) {
            WD_CHECK (file.write_quoted_string("Align_None        ", WD_True));
        }
        else
            return WT_Result::Internal_Error;

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Alignment::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Alignment::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    WT_Unsigned_Integer16 description;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            char *  format;

            WD_CHECK (file.read(format, 40));

            if (!strcmp(format, "Align_Center      "))
                m_description = WT_Alignment::Align_Center;
            else if (!strcmp(format, "Align_Title_Block "))
                m_description = WT_Alignment::Align_Title_Block;
            else if (!strcmp(format, "Align_Top         "))
                m_description = WT_Alignment::Align_Top;
            else if (!strcmp(format, "Align_Bottom      "))
                m_description = WT_Alignment::Align_Bottom;
            else if (!strcmp(format, "Align_Left        "))
                m_description = WT_Alignment::Align_Left;
            else if (!strcmp(format, "Align_Right       "))
                m_description = WT_Alignment::Align_Right;
            else if (!strcmp(format, "Align_Top_Left    "))
                m_description = WT_Alignment::Align_Top_Left;
            else if (!strcmp(format, "Align_Top_Right   "))
                m_description = WT_Alignment::Align_Top_Right;
            else if (!strcmp(format, "Align_Bottom_Left "))
                m_description = WT_Alignment::Align_Bottom_Left;
            else if (!strcmp(format, "Align_Bottom_Right"))
                m_description = WT_Alignment::Align_Bottom_Right;
            else if (!strcmp(format, "Align_None        "))
                m_description = WT_Alignment::Align_None;
            else
                return WT_Result::Internal_Error;

            WD_CHECK (opcode.skip_past_matching_paren(file));
            delete format;
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Description;
            // No break;

        case Getting_Description:
            WD_CHECK (file.read(description));
            switch(description)
            {
            case WT_Alignment::Align_Center:
                m_description = WT_Alignment::Align_Center;
                break;
            case WT_Alignment::Align_Title_Block:
                m_description = WT_Alignment::Align_Title_Block;
                break;
            case WT_Alignment::Align_Top:
                m_description = WT_Alignment::Align_Top;
                break;
            case WT_Alignment::Align_Bottom:
                m_description = WT_Alignment::Align_Bottom;
                break;
            case WT_Alignment::Align_Left:
                m_description = WT_Alignment::Align_Left;
                break;
            case WT_Alignment::Align_Right:
                m_description = WT_Alignment::Align_Right;
                break;
            case WT_Alignment::Align_Top_Left:
                m_description = WT_Alignment::Align_Top_Left;
                break;
            case WT_Alignment::Align_Top_Right:
                m_description = WT_Alignment::Align_Top_Right;
                break;
            case WT_Alignment::Align_Bottom_Left:
                m_description = WT_Alignment::Align_Bottom_Left;
                break;
            case WT_Alignment::Align_Bottom_Right:
                m_description = WT_Alignment::Align_Bottom_Right;
                break;
            case WT_Alignment::Align_None:
                m_description = WT_Alignment::Align_None;
                break;
            default:
                return WT_Result::Internal_Error;
            }
            m_stage = Getting_Close;

            // No break
            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

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
WT_Boolean    WT_Alignment::operator!= (WT_Alignment const &
                                        alignment) const
{
    return !(*this == alignment);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Alignment::operator== (WT_Alignment const &
                                        alignment) const
{
    if(get_alignment_description() == alignment.get_alignment_description())
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Alignment::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Alignment::process(WT_File & file)
{
    WD_Assert(file.alignment_action());
    return (file.alignment_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Alignment::default_process(WT_Alignment &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Password::WT_Password(char * string)
{   set(string);    }

///////////////////////////////////////////////////////////////////////////
WT_Password::WT_Password(WT_Unsigned_Integer16 * string)
{   set(string);    }

///////////////////////////////////////////////////////////////////////////
WT_Password::WT_Password(WT_Password const & info)
    : WT_Object()
{
    memcpy(m_string, info.string(), sizeof(WT_Byte) * 32);
}
///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Password::object_id() const
{   return Password_ID; }

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Password::object_type() const
{   return Definition;  }

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Password::process(WT_File & file)
{
    WD_Assert (file.password_action());
    return (file.password_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Password::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Password::default_process(WT_Password &, WT_File &)
{    return WT_Result::Success;  }

///////////////////////////////////////////////////////////////////////////
void WT_Password::set(char const * string)
{
    memcpy(m_string, string, sizeof(WT_Byte) * 32);
}

///////////////////////////////////////////////////////////////////////////
void WT_Password::set(WT_Unsigned_Integer16 const * string)
{
    memcpy(m_string, string, sizeof(WT_Byte) * 32);
}

///////////////////////////////////////////////////////////////////////////
void WT_Password::set(WT_String const & string)
{
    WD_Assert(string.length() == 32);
    memcpy(m_string, string.ascii(), sizeof(WT_Byte) * 32);
}

///////////////////////////////////////////////////////////////////////////
WT_Byte * const     WT_Password::string(void) const
{
    return (WT_Byte * const) &m_string;
}

///////////////////////////////////////////////////////////////////////////
WT_Unsigned_Integer16 const  WT_Password::get_string_length(void)
{
    return (WT_Unsigned_Integer16) 32;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Password::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());
    if (file.heuristics().allow_binary_data())
    {
        if(!strlen((char const *) m_string))
            memset((void *) &m_string, 0, 32 * sizeof(WT_Byte));
        WD_CHECK (file.write ((WT_Byte) '{'));
        WD_CHECK (file.write ((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + (32 * sizeof(WT_Byte)) + sizeof(WT_Byte))));
        WD_CHECK (file.write ((WT_Unsigned_Integer16) WD_EXBO_PASSWORD));
        WD_CHECK (file.write (32 , (WT_Byte const *) m_string));
        return    file.write ((WT_Byte) '}');
    }
    else {
        if(!strlen((char const *) m_string))
            memset((void *) &m_string, 0, 32 * sizeof(WT_Byte));
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write ((WT_Byte) '('));
        WD_CHECK (file.write ("Psswd"));
        WD_CHECK (file.write ((WT_Byte) ' '));
        WD_CHECK (file.write ((WT_Byte) '\''));
        WD_CHECK (file.write (32, m_string));
        WD_CHECK (file.write ((WT_Byte) '\''));
        return    file.write ((WT_Byte) ')');
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Password::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        WD_CHECK (file.read(32, m_string));
        WT_Byte close_brace;
        WD_CHECK (file.read(close_brace));
        if (close_brace != '}')
            return WT_Result::Corrupt_File_Error;
        break;
    case WT_Opcode::Extended_ASCII:
        WD_CHECK (file.eat_whitespace());
        WT_Byte open_quotes;
        WD_CHECK (file.read(open_quotes));
        if (open_quotes != '\'')
            return WT_Result::Corrupt_File_Error;
        WD_CHECK (file.read(32, m_string));
        WD_CHECK (file.read(open_quotes));
        if (open_quotes != '\'')
            return WT_Result::Corrupt_File_Error;
        WD_CHECK (opcode.skip_past_matching_paren(file));
        break;
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Password::operator!= (WT_Password const & Password) const
{
    return !(*this == Password);
}

///////////////////////////////////////////////////////////////////////////
WT_Password    WT_Password::operator= (WT_Password const & Password)
{
    memcpy(m_string, Password.m_string, sizeof(WT_Byte) * 32);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Password::operator== (WT_Password const & Password) const
{
    if(memcmp((void *)string(), (void *)Password.string(), 32) == 0)
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Guid::WT_Guid(WD_GUID guid)
    : m_stage(Starting)
{   set(guid);    }

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Guid::object_id() const
{
    return Guid_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Guid::object_type() const
{
    return Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid::process(WT_File & file)
{
    WD_Assert (file.guid_action());
    return (file.guid_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid::default_process(WT_Guid &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Guid::WT_Guid(WT_Guid const & info)
    : WT_Object()
    , WT_Item()
{
    set(((WT_Guid *) &info)->get_guid());
}

///////////////////////////////////////////////////////////////////////////
void WT_Guid::set(WT_Guid guid)
{
    set((WD_GUID) guid.get_guid());
}

///////////////////////////////////////////////////////////////////////////
void WT_Guid::set(const WD_GUID guid)
{
    memcpy((void *) &m_guid, (void *)&guid, sizeof(WD_GUID));
}

///////////////////////////////////////////////////////////////////////////
WD_GUID WT_Guid::get_guid()
{
    return (WD_GUID) m_guid;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());
    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write ((WT_Byte) '{'));
        WD_CHECK (file.write ((WT_Integer32) (sizeof(WT_Unsigned_Integer16) +
            sizeof(WT_Unsigned_Integer32) + sizeof(WT_Unsigned_Integer16) +
            sizeof(WT_Unsigned_Integer16) + (sizeof(WT_Byte) * 8) +
            sizeof(WT_Byte))));
        WD_CHECK (file.write ((WT_Unsigned_Integer16) WD_EXBO_GUID));
        WD_CHECK (file.write ((WT_Unsigned_Integer32) m_guid.Data1));
        WD_CHECK (file.write ((WT_Unsigned_Integer16) m_guid.Data2));
        WD_CHECK (file.write ((WT_Unsigned_Integer16) m_guid.Data3));
        WD_CHECK (file.write (8 , (WT_Byte *) &(m_guid.Data4)));
        return    file.write ((WT_Byte) '}');
    }
    else {
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write ("(Guid "));
        WD_CHECK (file.write_ascii ((WT_Unsigned_Integer32) m_guid.Data1));
        WD_CHECK (file.write ((WT_Byte) ' '));
        WD_CHECK (file.write_ascii ((WT_Unsigned_Integer16) m_guid.Data2));
        WD_CHECK (file.write ((WT_Byte) ' '));
        WD_CHECK (file.write_ascii ((WT_Unsigned_Integer16) m_guid.Data3));
        WD_CHECK (file.write ((WT_Byte) ' '));
        WD_CHECK (file.write_hex (8 , (WT_Byte *) m_guid.Data4));
        WD_CHECK (file.write ((WT_Byte) ' '));
        return    file.write ((WT_Byte) ')');
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Guid::operator!=(WT_Guid const & in) const
{
    if(memcmp(&in.m_guid, &m_guid, sizeof(WD_GUID)) == 0)
        return WD_False;
    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Guid::operator==(WT_Guid const & in) const
{
    if(memcmp(&in.m_guid, &m_guid, sizeof(WD_GUID)) == 0)
        return WD_True;
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
WT_Guid WT_Guid::operator=(WT_Guid const & in) const
{
    memcpy((void *)&m_guid, &in.m_guid, sizeof(WD_GUID));
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Guid::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Starting:
                m_stage = Getting_Data1;
                WD_CHECK (file.eat_whitespace());
                // No break;

            case Getting_Data1:
                WD_CHECK (file.read_ascii(m_guid.Data1));
                WD_CHECK (file.eat_whitespace());
                m_stage = Getting_Data2;
                //No break;

            case Getting_Data2:
                WD_CHECK (file.read_ascii(m_guid.Data2));
                WD_CHECK (file.eat_whitespace());
                m_stage = Getting_Data3;
                //No break;

            case Getting_Data3:
                WD_CHECK (file.read_ascii(m_guid.Data3));
                WD_CHECK (file.eat_whitespace());
                m_stage = Getting_Data4;
                //No break;

            case Getting_Data4:
                WD_CHECK (file.read_hex(8, m_guid.Data4));
                WD_CHECK (file.eat_whitespace());
                m_stage = Getting_Close;
                //No break;

            case Getting_Close:
                WD_CHECK (opcode.skip_past_matching_paren(file));
                break;  // Go below to clean up...
            default:
                return WT_Result::Internal_Error;  // Illegal stage.
            } // switch (m_stage) for ASCII
        } break;
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Data1;
            // No break;

        case Getting_Data1:
            WD_CHECK (file.read(m_guid.Data1));
            m_stage = Getting_Data2;
            //No break;

        case Getting_Data2:
            WD_CHECK (file.read(m_guid.Data2));
            m_stage = Getting_Data3;
            //No break;

        case Getting_Data3:
            WD_CHECK (file.read(m_guid.Data3));
            m_stage = Getting_Data4;
            //No break;

        case Getting_Data4:
            WD_CHECK (file.read(8, (WT_Byte *)&m_guid.Data4));
            m_stage = Getting_Close;
            //No break;

            case Getting_Close:
                WT_Byte close_brace;

                WD_CHECK (file.read(close_brace));
                if (close_brace != '}')
                    return WT_Result::Corrupt_File_Error;

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
}
