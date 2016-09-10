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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/font.cpp 3     5/09/05 12:42a Evansg $


#include "whiptk/pch.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////
WT_Font::WT_Font (WT_String             name,
                  WT_Boolean            bold,
                  WT_Boolean            italic,
                  WT_Boolean            underlined,
                  WT_Byte               charset,
                  WT_Byte               pitch,
                  WT_Byte               family,
                  WT_Integer32          height,
                  WT_Unsigned_Integer16 rotation,
                  WT_Unsigned_Integer16 width_scale,
                  WT_Unsigned_Integer16 spacing,
                  WT_Unsigned_Integer16 oblique,
                  WT_Integer32          flags)
        : m_option_font_name(name)
        , m_option_charset (charset)
        , m_option_pitch (pitch)
        , m_option_family (family)
        , m_option_style (bold, italic, underlined)
        , m_option_height(height)
        , m_option_rotation(rotation)
        , m_option_width_scale(width_scale)
        , m_option_spacing(spacing)
        , m_option_oblique(oblique)
        , m_option_flags(flags)
        , m_name_length (0) // Obsolete, but needed to read older files
        , m_stage (Getting_Name_Length)
        , m_fields_defined(FONT_ALL_FIELDS)
{ }

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Font::object_id() const
{
    return Font_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    file.desired_rendition().font_extension();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::Font_Extension_Bit));

    // BPM: this could be improved when time allows...
    if(file.heuristics().apply_transform() && !m_rotation_checked && file.heuristics().transform().rotation())
    {
        // The first time we start up the file, if the application wants an unrotated
        // font, and we need to flip landscape to portrait, then the above test will think
        // that the rotation doesn't need to be output the first time.  Here we force it...
        ((WT_Font &)(*this)).m_fields_defined |= FONT_ROTATION_BIT;
        ((WT_Font *)this)->m_rotation_checked = WD_True;  // Cast from const to alterable.
    }

    if (file.heuristics().allow_binary_data())
    {
        // Binary
        WD_CHECK (file.write((WT_Byte) 0x06));  // CTRL-F
        WD_CHECK (file.write(m_fields_defined));
    }
    else
    {
        // ASCII Output
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(Font"));
    }

    if (m_fields_defined & FONT_NAME_BIT)
        WD_CHECK (font_name().serialize(*this, file));
    if (m_fields_defined & FONT_CHARSET_BIT)
        WD_CHECK (charset().serialize(*this, file));
    if (m_fields_defined & FONT_PITCH_BIT)
        WD_CHECK (pitch().serialize(*this, file));
    if (m_fields_defined & FONT_FAMILY_BIT)
        WD_CHECK (family().serialize(*this, file));
    if (m_fields_defined & (FONT_STYLE_BIT))
        WD_CHECK (style().serialize(*this, file));
    if (m_fields_defined & FONT_HEIGHT_BIT)
        WD_CHECK (height().serialize(*this,file));
    if (m_fields_defined & FONT_ROTATION_BIT)
        WD_CHECK (rotation().serialize(*this,file));
    if (m_fields_defined & FONT_WIDTH_SCALE_BIT)
        WD_CHECK (width_scale().serialize(*this,file));
    if (m_fields_defined & FONT_SPACING_BIT)
        WD_CHECK (spacing().serialize(*this,file));
    if (m_fields_defined & FONT_OBLIQUE_BIT)
        WD_CHECK (oblique().serialize(*this,file));
    if (m_fields_defined & FONT_FLAGS_BIT)
        WD_CHECK (flags().serialize(*this,file));

    if (!file.heuristics().allow_binary_data())
        WD_CHECK (file.write((WT_Byte)')'));

    return WT_Result::Success;
}
#else
WT_Result WT_Font::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Font const &    WT_Font::operator= (WT_Font const & font)
{
    if (font.m_fields_defined & FONT_NAME_BIT)
        m_option_font_name = (WT_String const &)font.font_name();
    if (font.m_fields_defined & FONT_CHARSET_BIT)
        m_option_charset = font.charset();
    if (font.m_fields_defined & FONT_PITCH_BIT)
        m_option_pitch = font.pitch();
    if (font.m_fields_defined & FONT_FAMILY_BIT)
        m_option_family = font.family();
    if (font.m_fields_defined & FONT_STYLE_BIT)
        m_option_style = font.style();
    if (font.m_fields_defined & FONT_HEIGHT_BIT)
        m_option_height = font.height();
    if (font.m_fields_defined & FONT_ROTATION_BIT)
        m_option_rotation = font.rotation();
    if (font.m_fields_defined & FONT_WIDTH_SCALE_BIT)
        m_option_width_scale = font.width_scale();
    if (font.m_fields_defined & FONT_SPACING_BIT)
        m_option_spacing = font.spacing();
    if (font.m_fields_defined & FONT_OBLIQUE_BIT)
        m_option_oblique = font.oblique();
    if (font.m_fields_defined & FONT_FLAGS_BIT)
        m_option_flags = font.flags();

    //
    // Note: Prior to 7.4, this assignment was a "|=".
    // I think that was like pouring wine into a barrel of sewage.
    //
    m_fields_defined = font.m_fields_defined;

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Font::operator!= (WT_Attribute const & attrib) const
{
    return (operator==( attrib )) ? WD_False : WD_True;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Font::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id()  == Font_ID                                  &&
        style()             == ((WT_Font const &)attrib).style()        &&
        charset()           == ((WT_Font const &)attrib).charset()      &&
        pitch()             == ((WT_Font const &)attrib).pitch()        &&
        family()            == ((WT_Font const &)attrib).family()       &&
        font_name()         == ((WT_Font const &)attrib).font_name()    &&
        height()            == ((WT_Font const &)attrib).height()       &&
        rotation()          == ((WT_Font const &)attrib).rotation()     &&
        width_scale()       == ((WT_Font const &)attrib).width_scale()  &&
        spacing()           == ((WT_Font const &)attrib).spacing()      &&
        oblique()           == ((WT_Font const &)attrib).oblique()      &&
        flags()             == ((WT_Font const &)attrib).flags()           )
        return WD_True;
    else
        return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Font::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().font())
    {
        // Find out which fields of the font definition have changed and only
        // output those sub-fields.

        ((WT_Font &)(*this)).m_fields_defined = 0;

        if (font_name()   != file.rendition().font().font_name())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_NAME_BIT;
        if (charset()     != file.rendition().font().charset())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_CHARSET_BIT;
        if (pitch()       != file.rendition().font().pitch())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_PITCH_BIT;
        if (family()      != file.rendition().font().family())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_FAMILY_BIT;
        if (style()       != file.rendition().font().style())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_STYLE_BIT;
        if (height()      != file.rendition().font().height())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_HEIGHT_BIT;
        if (width_scale() != file.rendition().font().width_scale())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_WIDTH_SCALE_BIT;
        if (spacing()     != file.rendition().font().spacing())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_SPACING_BIT;
        if (oblique()     != file.rendition().font().oblique())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_OBLIQUE_BIT;
        if (flags()       != file.rendition().font().flags())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_FLAGS_BIT;
        if (rotation()    != file.rendition().font().rotation())
            ((WT_Font &)(*this)).m_fields_defined |= FONT_ROTATION_BIT;

        WD_CHECK (serialize(file));
        file.rendition().font() = *this;
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Font::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_SET_FONT_STRING_FIXED)
            {
              WT_Byte bold_italic;

                // Before revision 00.31, the Set Font font name didn't use
                // the proper string notation.  This code supports the old
                // file format.
                switch (m_stage)
                {
                case Getting_Name_Length:
                    WD_CHECK (file.read_count(m_name_length));
                    m_stage = Getting_Name;
                    // No break
                case Getting_Name:
                    {
                        WT_Byte *  tmp = new WT_Byte[m_name_length + 1];
                        if (!tmp)
                            return WT_Result::Out_Of_Memory_Error;

                        WT_Result   result;
                        result = file.read(m_name_length, tmp);
                        if (result != WT_Result::Success)
                        {
                            delete []tmp;
                            return result;
                        }

                        tmp[m_name_length] = 0x00;    // null terminate
                        m_option_font_name.set(tmp);
                        delete []tmp;

                        m_stage = Getting_Bold_Italic;
                    }

                    // No break. Note: No support for underlined option in older files
                case Getting_Bold_Italic:
                    WD_CHECK (file.read(bold_italic));
                    m_option_style.set_bold (bold_italic & WD_FONT_STYLE_BOLD);
                    m_option_style.set_italic (bold_italic & WD_FONT_STYLE_ITALIC);
                    m_stage = Getting_Charset;
                    // No break
                case Getting_Charset:
                    WT_Integer32    cs;
                    WD_CHECK (file.read(cs));
                    if ((cs & 0xFF) != cs)
                        return WT_Result::Corrupt_File_Error;
                    m_option_charset.set((WT_Byte)cs);
                    m_stage = Getting_Pitch_and_Family;
                    // No break
                case Getting_Pitch_and_Family:
                    WT_Integer32    pitch_and_family;

                    WD_CHECK (file.read(pitch_and_family));
                    m_option_pitch.set ((WT_Byte)(pitch_and_family & WT_Font_Option_Pitch::PITCH_BITS));
                    m_option_family.set((WT_Byte)(pitch_and_family & WT_Font_Option_Family::FAMILY_BITS));
                    m_stage = Completed;
                    break;
                default:
                    return WT_Result::Internal_Error;
                } // switch (m_stage)

                m_fields_defined =  FONT_NAME_BIT       |
                                    FONT_STYLE_BIT      |
                                    FONT_CHARSET_BIT    |
                                    FONT_PITCH_BIT      |
                                    FONT_FAMILY_BIT;

            } // If (old version of DWF Set Font)
            else
            {
                // Binary and Current
                // We have a file that uses the current definition of Set Font
                switch (m_stage)
                {
                case Getting_Name_Length:
                    m_stage = Getting_Fields;

                case Getting_Fields:
                    WD_CHECK(file.read(m_fields_defined));
                    m_stage = Getting_Name;

                    // No break
                case Getting_Name:

                    if (m_fields_defined & FONT_NAME_BIT)
                        WD_CHECK (m_option_font_name.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Charset;

                    // No break
                case Getting_Charset:

                    if (m_fields_defined & FONT_CHARSET_BIT)
                        WD_CHECK (m_option_charset.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Pitch;

                    // No break
                case Getting_Pitch:

                    if (m_fields_defined & FONT_PITCH_BIT)
                        WD_CHECK (m_option_pitch.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Family;

                    // No break
                case Getting_Family:

                    if (m_fields_defined & FONT_FAMILY_BIT)
                        WD_CHECK (m_option_family.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Style;

                    // No break
                case Getting_Style:

                    if (m_fields_defined & FONT_STYLE_BIT)
                        WD_CHECK (m_option_style.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Height;

                    // No break
                case Getting_Height:

                    if (m_fields_defined & FONT_HEIGHT_BIT)
                        WD_CHECK (m_option_height.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Rotation;

                    // No break
                case Getting_Rotation:

                    if (m_fields_defined & FONT_ROTATION_BIT)
                        WD_CHECK (m_option_rotation.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Width_Scale;

                    // No break
                case Getting_Width_Scale:

                    if (m_fields_defined & FONT_WIDTH_SCALE_BIT)
                        WD_CHECK (m_option_width_scale.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Spacing;

                    // No break
                case Getting_Spacing:

                    if (m_fields_defined & FONT_SPACING_BIT)
                        WD_CHECK (m_option_spacing.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Oblique;

                    // No break
                case Getting_Oblique:

                    if (m_fields_defined & FONT_OBLIQUE_BIT)
                        WD_CHECK (m_option_oblique.materialize(*this, m_optioncode, file));

                    m_stage = Getting_Flags;

                    // No break
                case Getting_Flags:

                    if (m_fields_defined & FONT_FLAGS_BIT)
                        WD_CHECK (m_option_flags.materialize(*this, m_optioncode, file));

                    m_stage = Completed;
                    break;
                default:
                    return WT_Result::Internal_Error;
                } // switch (m_stage)
            } // newer version of Set Font
        } break; // Case (single byte opcode)
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Getting_Name_Length:
                m_fields_defined = 0;
                m_stage = Getting_Next_Optioncode;

                // No break
Getting_Next_Optioncode_Hop:
            case Getting_Next_Optioncode:

                WD_CHECK(m_optioncode.get_optioncode(file));
                if (m_optioncode.type() == WT_Opcode::Null_Optioncode)
                {
                    m_stage = Skipping_Last_Paren;
                    goto Skipping_Last_Paren_Hop;
                }
                m_stage = Materializing_Option;

                // No Break;
            case Materializing_Option:

                switch (m_optioncode.option_id())
                {
                case WT_Font_Optioncode::Font_Name_Option:
                    m_fields_defined |= FONT_NAME_BIT;
                    WD_CHECK(m_option_font_name.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Charset_Option:
                    m_fields_defined |= FONT_CHARSET_BIT;
                    WD_CHECK(m_option_charset.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Pitch_Option:
                    m_fields_defined |= FONT_PITCH_BIT;
                    WD_CHECK(m_option_pitch.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Family_Option:
                    m_fields_defined |= FONT_FAMILY_BIT;
                    WD_CHECK(m_option_family.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Style_Option:
                    m_fields_defined |= FONT_STYLE_BIT;
                    WD_CHECK(m_option_style.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Height_Option:
                    m_fields_defined |= FONT_HEIGHT_BIT;
                    WD_CHECK(m_option_height.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Rotation_Option:
                    m_fields_defined |= FONT_ROTATION_BIT;
                    WD_CHECK(m_option_rotation.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Width_Scale_Option:
                    m_fields_defined |= FONT_WIDTH_SCALE_BIT;
                    WD_CHECK(m_option_width_scale.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Spacing_Option:
                    m_fields_defined |= FONT_SPACING_BIT;
                    WD_CHECK(m_option_spacing.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Oblique_Option:
                    m_fields_defined |= FONT_OBLIQUE_BIT;
                    WD_CHECK(m_option_oblique.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Flags_Option:
                    m_fields_defined |= FONT_FLAGS_BIT;
                    WD_CHECK(m_option_flags.materialize(*this, m_optioncode, file));
                    break;
                case WT_Font_Optioncode::Unknown_Option:
                    WD_CHECK(m_optioncode.skip_past_matching_paren(file));
                    break;
                default:
                    return WT_Result::Internal_Error;
                } // switch (m_sub_option.option_id())

                m_stage = Getting_Next_Optioncode;
                goto Getting_Next_Optioncode_Hop;

Skipping_Last_Paren_Hop:
            case Skipping_Last_Paren:
                WD_CHECK (opcode.skip_past_matching_paren(file));
                m_stage = Completed;
                break;

            default:
                return WT_Result::Internal_Error;
            } // switch (m_stage)

        } break;
    case WT_Opcode::Extended_Binary:
    default:
            return WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch (opcode type)

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            return materialize(opcode, file);
        } break;
    case WT_Opcode::Extended_ASCII:
        {
            WD_CHECK (opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch

    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font::process(WT_File & file)
{
    WD_Assert(file.font_action());
    return (file.font_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Font::default_process(WT_Font & item, WT_File & file)
{
    file.rendition().font() = item;
    return WT_Result::Success;
}
