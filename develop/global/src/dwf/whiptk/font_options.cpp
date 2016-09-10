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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/font_options.cpp 3     5/09/05 12:42a Evansg $


#include "whiptk/pch.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Font_Name::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
        return m_font_name.serialize(file,WD_True);

    // ASCII output
    WD_CHECK (file.write(" (Name "));
    WD_CHECK (m_font_name.serialize(file));
    return    file.write((WT_Byte) ')');
}
#else
WT_Result WT_Font_Option_Font_Name::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Font_Name::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    switch (m_stage)
    {
    case Getting_Operand:
        WD_CHECK (m_font_name.materialize(file));
        m_materialized = WD_True;

        if (optioncode.type() == WT_Opcode::Extended_ASCII)
            m_stage = Getting_Close_Paren;  // ASCII
        else
            return WT_Result::Success;      // Binary

        // No break
    case Getting_Close_Paren:

        WD_CHECK(optioncode.skip_past_matching_paren(file));
        m_stage = Completed;
        break;

    default:
        WD_Assert(WD_False);
        return WT_Result::Internal_Error;
        
    } // switch (m_stage)

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Charset::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) m_charset));
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Charset "));
        switch (m_charset)
        {
        case CHARSET_ANSI:
            WD_CHECK (file.write("ANSI)"));
            break;
        case CHARSET_DEFAULT:
            WD_CHECK (file.write("DEFAULT)"));
            break;
        case CHARSET_SYMBOL:
            WD_CHECK (file.write("SYMBOL)"));
            break;
        case CHARSET_MAC:
            WD_CHECK (file.write("MAC)"));
            break;
        case CHARSET_UNICODE:
            WD_CHECK (file.write("UNICODE)"));
            break;
        case CHARSET_SHIFTJIS:
            WD_CHECK (file.write("SHIFTJIS)"));
            break;
        case CHARSET_HANGEUL:
            WD_CHECK (file.write("HANGEUL)"));
            break;
        case CHARSET_JOHAB:
            WD_CHECK (file.write("JOHAB)"));
            break;
        case CHARSET_GB2312:
            WD_CHECK (file.write("GB2312)"));
            break;
        case CHARSET_CHINESEBIG5:
            WD_CHECK (file.write("CHINESEBIG5)"));
            break;
        case CHARSET_GREEK:
            WD_CHECK (file.write("GREEK)"));
            break;
        case CHARSET_TURKISH:
            WD_CHECK (file.write("TURKISH)"));
            break;
        case CHARSET_VIETNAMESE:
            WD_CHECK (file.write("VIETNAMESE)"));
            break;
        case CHARSET_HEBREW:
            WD_CHECK (file.write("HEBREW)"));
            break;
        case CHARSET_ARABIC:
            WD_CHECK (file.write("ARABIC)"));
            break;
        case CHARSET_BALTIC:
            WD_CHECK (file.write("BALTIC)"));
            break;
        case CHARSET_RUSSIAN:
            WD_CHECK (file.write("RUSSIAN)"));
            break;
        case CHARSET_THAI:
            WD_CHECK (file.write("THAI)"));
            break;
        case CHARSET_EASTEUROPE:
            WD_CHECK (file.write("EASTEUROPE)"));
            break;
        case CHARSET_OEM:
            WD_CHECK (file.write("OEM)"));
            break;

        default:
            WD_CHECK (file.write_ascii((WT_Integer32) m_charset));
            WD_CHECK (file.write((WT_Byte)')'));
            
        } // switch (m_charset)
    } // else ASCII output

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Charset::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Charset::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        // Binary
        WD_CHECK(file.read(m_charset));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            char *   operand;

            WD_CHECK(file.read(operand, 80));

            if (!strcmp("ANSI", operand))
                m_charset = CHARSET_ANSI;
            else if (!strcmp("DEFAULT", operand))
                m_charset = CHARSET_DEFAULT;
            else if (!strcmp("SYMBOL", operand))
                m_charset = CHARSET_SYMBOL;
            else if (!strcmp("MAC", operand))
                m_charset = CHARSET_MAC;
            else if (!strcmp("UNICODE", operand))
                m_charset = CHARSET_UNICODE;
            else if (!strcmp("SHIFTJIS", operand))
                m_charset = CHARSET_SHIFTJIS;
            else if (!strcmp("HANGEUL", operand))
                m_charset = CHARSET_HANGEUL;
            else if (!strcmp("JOHAB", operand))
                m_charset = CHARSET_JOHAB;
            else if (!strcmp("GB2312", operand))
                m_charset = CHARSET_GB2312;
            else if (!strcmp("CHINESEBIG5", operand))
                m_charset = CHARSET_CHINESEBIG5;
            else if (!strcmp("GREEK", operand))
                m_charset = CHARSET_GREEK;
            else if (!strcmp("TURKISH", operand))
                m_charset = CHARSET_TURKISH;
            else if (!strcmp("VIETNAMESE", operand))
                m_charset = CHARSET_VIETNAMESE;
            else if (!strcmp("HEBREW", operand))
                m_charset = CHARSET_HEBREW;
            else if (!strcmp("ARABIC", operand))
                m_charset = CHARSET_ARABIC;
            else if (!strcmp("BALTIC", operand))
                m_charset = CHARSET_BALTIC;
            else if (!strcmp("RUSSIAN", operand))
                m_charset = CHARSET_RUSSIAN;
            else if (!strcmp("THAI", operand))
                m_charset = CHARSET_THAI;
            else if (!strcmp("EASTEUROPE", operand))
                m_charset = CHARSET_EASTEUROPE;
            else if (!strcmp("OEM", operand))
                m_charset = CHARSET_OEM;
            else
            {
                int tmp = atoi(operand);
                if (tmp < 0 || tmp > 0xFF)
                {
                    delete []operand;
                    return WT_Result::Corrupt_File_Error;
                }

                m_charset = (WT_Byte) tmp;
            }

            delete []operand;
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (extended_ASCII)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Pitch::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        return file.write(m_pitch);
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Pitch "));
        switch (m_pitch)
        {
        case PITCH_DEFAULT:
            WD_CHECK (file.write("default)"));
            break;
        case PITCH_FIXED:
            WD_CHECK (file.write("fixed)"));
            break;
        case PITCH_VARIABLE:
            WD_CHECK (file.write("variable)"));
            break;
        default:
            return WT_Result::Internal_Error;
        } // switch on pitch
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Pitch::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Pitch::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_pitch));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            char *   operand;

            WD_CHECK(file.read(operand, 80));

            if (!strcmp("default", operand))
                m_pitch = PITCH_DEFAULT;
            else if (!strcmp("fixed", operand))
                m_pitch = PITCH_FIXED;
            else if (!strcmp("variable", operand))
                m_pitch = PITCH_VARIABLE;
            else
            {
                int tmp = atoi(operand);
                if (tmp < 0 || tmp > 0xFF)
                {
                    delete []operand;
                    return WT_Result::Corrupt_File_Error;
                }

                m_pitch = (WT_Byte) tmp;
            }

            delete []operand;
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Family::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        return file.write(m_family);
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Family "));
        switch (m_family)
        {
        case FAMILY_DECORATIVE:
            WD_CHECK (file.write("decorative)"));
            break;
        case FAMILY_UNKNOWN:
            WD_CHECK (file.write("unknown)"));
            break;
        case FAMILY_MODERN:
            WD_CHECK (file.write("modern)"));
            break;
        case FAMILY_ROMAN:
            WD_CHECK (file.write("roman)"));
            break;
        case FAMILY_SCRIPT:
            WD_CHECK (file.write("script)"));
            break;
        case FAMILY_SWISS:
            WD_CHECK (file.write("swiss)"));
            break;
        default:
            return WT_Result::Internal_Error;
        }  // switch on family
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Family::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Family::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_family));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            char *   operand;

            WD_CHECK(file.read(operand, 80));

            if (!strcmp("decorative", operand))
                m_family = FAMILY_DECORATIVE;
            else if (!strcmp("unknown", operand))
                m_family = FAMILY_UNKNOWN;
            else if (!strcmp("modern", operand))
                m_family = FAMILY_MODERN;
            else if (!strcmp("roman", operand))
                m_family = FAMILY_ROMAN;
            else if (!strcmp("script", operand))
                m_family = FAMILY_SCRIPT;
            else if (!strcmp("swiss", operand))
                m_family = FAMILY_SWISS;
            else
            {
                int tmp = atoi(operand);
                if (tmp < 0 || tmp > 0xFF)
                {
                    delete []operand;
                    return WT_Result::Corrupt_File_Error;
                }

                m_family = (WT_Byte) tmp;
            }

            delete []operand;
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Style::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary
        WT_Byte style = (m_bold       ? WD_FONT_STYLE_BOLD        : 0x00) +
                        (m_italic     ? WD_FONT_STYLE_ITALIC      : 0x00) +
                        (m_underlined ? WD_FONT_STYLE_UNDERLINED  : 0x00);

        return file.write(style);
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Style"));
        if (m_bold)
            WD_CHECK (file.write(" bold"));
        if (m_italic)
            WD_CHECK (file.write(" italic"));
        if (m_underlined)
            WD_CHECK (file.write(" underlined"));

        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Style::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Style::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WT_Byte     style;

        WD_CHECK (file.read(style));
        if (style & WD_FONT_STYLE_BOLD)
            m_bold = WD_True;
        else
            m_bold = WD_False;

        if (style & WD_FONT_STYLE_ITALIC)
            m_italic = WD_True;
        else
            m_italic = WD_False;

        if (style & WD_FONT_STYLE_UNDERLINED)
            m_underlined = WD_True;
        else
            m_underlined = WD_False;
    }
    else
    {
        switch (m_stage)
        {
        case Starting:
            m_bold      = WD_False;
            m_italic    = WD_False;
            m_underlined = WD_False;
            m_stage = Getting_Next_Optioncode;

            // No break
Getting_Next_Optioncode_Hop:
            case Getting_Next_Optioncode:

                WD_CHECK(m_optioncode.get_opcode(file, WD_True));

                switch (m_optioncode.type())
                {

                case WT_Opcode::Extended_ASCII:
                    m_stage = Skipping_Optioncode;
                    goto Skipping_Optioncode_Hop;
                    

                case WT_Opcode::Unary_Optioncode:
                    if (!strcmp((const char *)m_optioncode.token(), "bold"))
                        m_bold = WD_True;
                    else if (!strcmp((const char *)m_optioncode.token(), "italic"))
                        m_italic = WD_True;
                    else if (!strcmp((const char *)m_optioncode.token(), "underlined"))
                        m_underlined = WD_True;
                    goto Getting_Next_Optioncode_Hop;
                    
                case WT_Opcode::Null_Optioncode:
                    m_stage = Getting_Close_Paren;
                    break;
                default:
                    return WT_Result::Corrupt_File_Error;
                } // switch

                // No Break;
        case Getting_Close_Paren:
            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

Skipping_Optioncode_Hop:
        case Skipping_Optioncode:
            WD_CHECK(m_optioncode.skip_past_matching_paren(file));
            goto Getting_Next_Optioncode_Hop;

            
        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Height::serialize(WT_Object const &, WT_File & file) const
{
    WT_Integer32    adjusted_height;

    if (file.heuristics().apply_transform())
        // TODO: does this make sense if we are doing Landscape flipping?
        adjusted_height = (long)(height() * file.heuristics().transform().m_y_scale);
    else
        adjusted_height = height();

    if (adjusted_height < 0)
        adjusted_height = - adjusted_height;

    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(adjusted_height);
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Height "));
        WD_CHECK (file.write_ascii(adjusted_height));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Height::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Height::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    WT_Integer32    height = 0;

    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(height));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(height));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    }

    if (file.heuristics().apply_transform())
        // TODO: Does this make sense if we are doing Landscape flipping?
        m_height = (WT_Integer32)(height * file.heuristics().transform().m_y_scale);
    else
        m_height = height;

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Rotation::serialize(WT_Object const &, WT_File & file) const
{

    WT_Unsigned_Integer16   fixed_rot = rotation();

    if(file.heuristics().apply_transform()) {
        fixed_rot = (WT_Unsigned_Integer16)((   (long)rotation() +
                        (long)WD_90_DEGREES_AS_SHORT * (file.heuristics().transform().rotation() / 90)
                    ) & 0x0000FFFF);
    }

    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(fixed_rot);
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Rotation "));
        WD_CHECK (file.write_ascii(fixed_rot));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Rotation::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Rotation::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_rotation));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(m_rotation));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Width_Scale::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(width_scale());
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Widthscale "));
        WD_CHECK (file.write_ascii(m_width_scale));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}

#else
WT_Result WT_Font_Option_Width_Scale::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Width_Scale::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_width_scale));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(m_width_scale));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Spacing::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(spacing());
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Spacing "));
        WD_CHECK (file.write_ascii(m_spacing));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Spacing::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Spacing::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_spacing));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(m_spacing));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Oblique::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(oblique());
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Oblique "));
        WD_CHECK (file.write_ascii(m_oblique));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Oblique::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Oblique::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_oblique));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(m_oblique));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font_Option_Flags::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary
        return file.write(flags());
    }
    else
    {   // ASCII output

        WD_CHECK (file.write(" (Flags "));
        WD_CHECK (file.write_ascii(m_flags));
        WD_CHECK (file.write((WT_Byte)')'));
    } // else ASCII

    return WT_Result::Success;
}
#else
WT_Result WT_Font_Option_Flags::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Font_Option_Flags::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        WD_CHECK (file.read(m_flags));
    }
    else
    {
        switch (m_stage)
        {
        case Getting_Operand:
            WD_CHECK(file.read_ascii(m_flags));
            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert(WD_False);
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // else (ascii)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
int WT_Font_Optioncode::option_id_from_optioncode()
{
    if (!strcmp((char const *)token(), "(Name"))
        m_option_id = Font_Name_Option;
    else if (!strcmp((char const *)token(), "(Charset"))
        m_option_id = Charset_Option;
    else if (!strcmp((char const *)token(), "(Pitch"))
        m_option_id = Pitch_Option;
    else if (!strcmp((char const *)token(), "(Family"))
        m_option_id = Family_Option;
    else if (!strcmp((char const *)token(), "(Style"))
        m_option_id = Style_Option;
    else if (!strcmp((char const *)token(), "(Height"))
        m_option_id = Height_Option;
    else if (!strcmp((char const *)token(), "(Rotation"))
        m_option_id = Rotation_Option;
    else if (!strcmp((char const *)token(), "(Widthscale"))
        m_option_id = Width_Scale_Option;
    else if (!strcmp((char const *)token(), "(Spacing"))
        m_option_id = Spacing_Option;
    else if (!strcmp((char const *)token(), "(Oblique"))
        m_option_id = Oblique_Option;
    else if (!strcmp((char const *)token(), "(Flags"))
        m_option_id = Flags_Option;
    else
        m_option_id = Unknown_Option;

    return m_option_id;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Font_Name::object_id() const
{
    return WT_Object::Font_Option_Font_Name_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Charset::object_id() const
{
    return WT_Object::Font_Option_Charset_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Pitch::object_id() const
{
    return WT_Object::Font_Option_Pitch_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Family::object_id() const
{
    return WT_Object::Font_Option_Family_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Style::object_id() const
{
    return WT_Object::Font_Option_Style_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Height::object_id() const
{
    return WT_Object::Font_Option_Height_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Rotation::object_id() const
{
    return WT_Object::Font_Option_Rotation_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Width_Scale::object_id() const
{
    return WT_Object::Font_Option_Width_Scale_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Spacing::object_id() const
{
    return WT_Object::Font_Option_Spacing_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Oblique::object_id() const
{
    return WT_Object::Font_Option_Oblique_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font_Option_Flags::object_id() const
{
    return WT_Object::Font_Option_Flags_ID;
}
