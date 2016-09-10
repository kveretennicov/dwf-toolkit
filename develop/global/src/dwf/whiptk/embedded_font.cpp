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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/embedded_font.cpp 1     9/12/04 8:52p Evansg $


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Embedded_Font::object_id() const
{
    return Embedded_Font_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_Embedded_Font::object_type() const
{
    return Wrapper;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Embedded_Font::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_Assert(m_data_size  > 0);
    WD_Assert(m_data);

    if (!((m_request & WT_Embedded_Font::Raw) ||
          (m_request & WT_Embedded_Font::Subset) ||
          (m_request & WT_Embedded_Font::Compressed) ||
          (m_request & WT_Embedded_Font::Fail_If_Variations_Simulated) ||
          (m_request & WT_Embedded_Font::Eudc) ||
          (m_request & WT_Embedded_Font::Validation_Tests) ||
          (m_request & WT_Embedded_Font::Web_Object) ||
          (m_request & WT_Embedded_Font::Encrypt_Data)))
    {
        return WT_Result::Internal_Error;
    }

    switch (m_privilege)
    {
    case WT_Embedded_Font::PreviewPrint:
    case WT_Embedded_Font::Editable:
    case WT_Embedded_Font::Installable:
    case WT_Embedded_Font::Non_Embedding:
        break;
    default:
        return WT_Result::Internal_Error;
    } // switch

    switch (m_character_set_type)
    {
    case WT_Embedded_Font::Unicode:
    case WT_Embedded_Font::Symbol:
    case WT_Embedded_Font::Glyphidx:
        break;
    default:
        return WT_Result::Internal_Error;
    } // switch

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));

        WD_CHECK (file.write((WT_Integer32) (
            sizeof(WT_Unsigned_Integer16) + // for the opcode
            sizeof(WT_Integer32) +          // request type
            sizeof(WT_Byte) +               // privilege
            sizeof(WT_Byte) +               // character set type
            sizeof(WT_Integer32) +          // font type face name length
            m_font_type_face_name_length +  // for font typeface name string
            sizeof(WT_Integer32) +          // length of logfont name string
            m_font_logfont_name_length +    // for logfont name
            sizeof(WT_Integer32) +          // the data size
            m_data_size +                   // the font data
            sizeof(WT_Byte)                 // The closing "}"
            )));

        WD_CHECK (file.write((WT_Unsigned_Integer16)
            WD_EXBO_EMBEDDED_FONT ));
        WD_CHECK (file.write((WT_Integer32) request_type()));
        WD_CHECK (file.write((WT_Byte) privilege()));
        WD_CHECK (file.write((WT_Byte) character_set_type()));
        WD_CHECK (file.write((WT_Integer32) font_type_face_name_length()));
        WD_CHECK (file.write(font_type_face_name_length()
            , font_type_face_name_string()));
        WD_CHECK (file.write((WT_Integer32) font_logfont_name_length()));
        WD_CHECK (file.write(font_logfont_name_length()
            , font_logfont_name_string()));
        WD_CHECK (file.write((WT_Integer32)m_data_size));
        WD_CHECK (file.write(m_data_size, m_data));
        WD_CHECK (file.write((WT_Byte) '}'));
    }
    else
    {
        // Extended ASCII output

        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Embedded_Font "));

        WD_CHECK (file.write_ascii((WT_Integer32) request_type()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii((WT_Unsigned_Integer16) privilege()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii((WT_Unsigned_Integer16)
            character_set_type()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii((WT_Integer32)
            font_type_face_name_length()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write(font_type_face_name_length()
            , font_type_face_name_string()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write_ascii((WT_Integer32)
            font_logfont_name_length()));

        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (file.write(font_logfont_name_length()
            , font_logfont_name_string()));

        WD_CHECK (file.write(" ("));
        WD_CHECK (file.write_ascii((WT_Integer32) m_data_size));
        WD_CHECK (file.write((WT_Byte) ' '));

        // TODO: put in a switch here based on format so that color
        // endianism is accounted for
        WD_CHECK (file.write_hex(m_data_size, m_data));

        WD_CHECK (file.write("))"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_Embedded_Font::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embedded_Font::materialize(WT_Opcode const & opcode,
                                        WT_File & file)
{
    WT_Unsigned_Integer16 a_word = 0;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Request_Type;
            // No break;

        case Getting_Request_Type:
            WD_CHECK (file.read(m_request));
            m_stage = Getting_Privilege_Type;

            // No break
        case Getting_Privilege_Type:
            WD_CHECK (file.read(m_privilege));
            m_stage = Getting_Character_Set_Type;

            // No break
        case Getting_Character_Set_Type:
            WD_CHECK (file.read(m_character_set_type));
            m_stage = Getting_Font_Type_Face_Name_Length;

        case Getting_Font_Type_Face_Name_Length:
            WD_CHECK (file.read(m_font_type_face_name_length));
            m_stage = Getting_Font_Type_Face_Name_String;

            // No break

        case Getting_Font_Type_Face_Name_String:
            if (!m_font_type_face_name_string)
            {
                m_font_type_face_name_string =
                    new WT_Byte[m_font_type_face_name_length];
                if (!m_font_type_face_name_string)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read(m_font_type_face_name_length
                , m_font_type_face_name_string));
            m_stage = Getting_Font_LogFont_Name_Length;

        case Getting_Font_LogFont_Name_Length:
            WD_CHECK (file.read(m_font_logfont_name_length));
            m_stage = Getting_Font_LogFont_Name_String;

            // No break

        case Getting_Font_LogFont_Name_String:
            if (!m_font_logfont_name_string)
            {
                m_font_logfont_name_string =
                    new WT_Byte[m_font_logfont_name_length];
                if (!m_font_logfont_name_string)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read(m_font_logfont_name_length
                , m_font_logfont_name_string));
            m_stage = Getting_Data_Size;

            // No break
        case Getting_Data_Size:
            WD_CHECK (file.read(m_data_size));
            m_stage = Getting_Data;

            // No break

        case Getting_Data:
            if (!m_data)
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read(m_data_size, m_data));
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

    case WT_Opcode::Extended_ASCII:

        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Request_Type;
            // No break;

        case Getting_Request_Type:

            WD_CHECK (file.read_ascii(m_request));
            m_stage = Getting_Privilege_Type;

            // No break

        case Getting_Privilege_Type:

            WD_CHECK (file.read_ascii(a_word));
            m_privilege = (WT_Byte) a_word;

            m_stage = Getting_Character_Set_Type;

            // No break

        case Getting_Character_Set_Type:
            WD_CHECK (file.read_ascii(a_word));
            m_character_set_type = (WT_Byte) a_word;

            m_stage = Getting_Font_Type_Face_Name_Length;

            // No break

        case Getting_Font_Type_Face_Name_Length:
            WD_CHECK (file.read_ascii(m_font_type_face_name_length));
            m_stage = Getting_Font_Type_Face_Name_String;

            // No break

        case Getting_Font_Type_Face_Name_String:
            WD_CHECK (file.eat_whitespace());
            if (!m_font_type_face_name_string)
            {
                m_font_type_face_name_string =
                    new WT_Byte[m_font_type_face_name_length];
                if (!m_font_type_face_name_string)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read(m_font_type_face_name_length
                , m_font_type_face_name_string));
            m_stage = Getting_Font_LogFont_Name_Length;

        case Getting_Font_LogFont_Name_Length:
            WD_CHECK (file.read_ascii(m_font_logfont_name_length));
            m_stage = Getting_Font_LogFont_Name_String;

            // No break

        case Getting_Font_LogFont_Name_String:
            WD_CHECK (file.eat_whitespace());
            if (!m_font_logfont_name_string)
            {
                m_font_logfont_name_string =
                    new WT_Byte[m_font_logfont_name_length];
                if (!m_font_logfont_name_string)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read(m_font_logfont_name_length
                , m_font_logfont_name_string));
            m_stage = Getting_Pre_Data_Size_Whitespace;

        case Getting_Pre_Data_Size_Whitespace:
            WD_CHECK(file.eat_whitespace());
            m_stage = Getting_Pre_Data_Size_Open_Paren;

            // No break
        case Getting_Pre_Data_Size_Open_Paren:
            WT_Byte     a_paren;

            WD_CHECK(file.read(a_paren));

            if (a_paren != '(')
                return WT_Result::Corrupt_File_Error;

            file.increment_paren_count();
            m_stage = Getting_Data_Size;

            // No break
        case Getting_Data_Size:
            WD_CHECK (file.read_ascii(m_data_size));
            m_stage = Getting_Data;

            // No break
        case Getting_Data:

            if (!m_data)
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            WD_CHECK (file.read_hex(m_data_size, m_data));

            m_stage = Getting_Close;

            // No break

        case Getting_Close:

            // There will be two close parens: one for the data, one for
            // the opcode
            WD_CHECK (opcode.skip_past_matching_paren(file));

            // Yeah!! We succeeded!
            break;  // Go below to clean up...
        default:
            return WT_Result::Internal_Error;  // Illegal stage.

        } // switch (m_stage) for ASCII

        break;

    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;  // Illegal Opcode Type
    } // switch (opcode type)

    m_stage = Starting;  // We finished, so reset stage to start.

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embedded_Font::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embedded_Font::process(WT_File & file)
{
    WD_Assert (file.embedded_font_action());
    return (file.embedded_font_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Embedded_Font::default_process(WT_Embedded_Font &, WT_File &)
{
    return WT_Result::Success;
}
