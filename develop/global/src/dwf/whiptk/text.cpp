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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/text.cpp 10    8/21/05 7:36p Gopalas $


#include "whiptk/pch.h"
#include <stddef.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////
WT_Text::WT_Text (WT_Text const &in)
    : WT_Drawable()
    , m_position(in.m_position)
    , m_string(in.m_string)
    , m_option_bounds(in.m_option_bounds)
    , m_option_overscore(in.m_option_overscore)
    , m_option_underscore(in.m_option_underscore)
    , m_option_reserved(in.m_option_reserved)
    , m_stage(in.m_stage)
    , m_transformed(in.m_transformed)
    , m_relativized(in.m_relativized)
    , m_optioncode(in.m_optioncode)
    , m_obsolete_font_holder(in.m_obsolete_font_holder)
    , m_obsolete_length_holder(in.m_obsolete_length_holder)
    , m_obsolete_msg_holder(in.m_obsolete_msg_holder)
{

}

///////////////////////////////////////////////////////////////////////////
WT_Text const & WT_Text::operator= (WT_Text const & in)
{
    m_position = in.m_position;
    m_string = in.m_string;
    m_option_bounds = in.m_option_bounds;
    m_option_overscore = in.m_option_overscore;
    m_option_underscore = in.m_option_underscore;
    m_option_reserved = in.m_option_reserved;
    m_stage = in.m_stage;
    m_transformed = in.m_transformed;
    m_relativized = in.m_relativized;
    m_optioncode = in.m_optioncode;
    m_obsolete_font_holder = in.m_obsolete_font_holder;
    m_obsolete_length_holder = in.m_obsolete_length_holder;
    m_obsolete_msg_holder = in.m_obsolete_msg_holder;
    
    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Text::WT_Text ()
    : m_position (WT_Logical_Point (0,0))
    , m_stage (Getting_Started)
    , m_transformed (WD_False)
    , m_relativized (WD_False)
    , m_obsolete_font_holder (WD_Null)
{ }

///////////////////////////////////////////////////////////////////////////
WT_Text::~WT_Text()
{
    if (m_obsolete_font_holder)
        delete m_obsolete_font_holder;
}

///////////////////////////////////////////////////////////////////////////
WT_Text::WT_Text (WT_Logical_Point const &      position,
                  WT_String const &             string,
                  WT_Logical_Point const *      bounds,
                  WT_Unsigned_Integer16         overscore_count,
                  WT_Unsigned_Integer16 const * overscore_pos,
                  WT_Unsigned_Integer16         underscore_count,
                  WT_Unsigned_Integer16 const * underscore_pos)
    : m_position (position)
    , m_string(string)
    , m_stage (Getting_Started)
    , m_transformed (WD_False)
    , m_relativized (WD_False)
    , m_obsolete_font_holder (WD_Null)
{
    m_option_bounds.set(bounds);
    if (overscore_count)
        m_option_overscore.set(overscore_count, overscore_pos);
    if (underscore_count)
        m_option_underscore.set(underscore_count, underscore_pos);
}

///////////////////////////////////////////////////////////////////////////
WT_Text::WT_Text (WT_Logical_Point const &  position,
                  WT_String const &         string)
    : m_position (position)
    , m_string(string)
    , m_stage (Getting_Started)
    , m_transformed (WD_False)
    , m_relativized (WD_False)
    , m_obsolete_font_holder (WD_Null)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Text::object_id() const
{
    return Text_ID;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    // BPM: This is a hack, but is the best I can do for now.
    // Here's the issue: if Tahoe has set the heuristic to flip landscape into portrait,
    // then we actually need the Font attribute to have a different rotation.
    // The rotation code will do the adjustment, but we need to make sure that
    // the rendition sync code "thinks" the font attribute might need updating.
    file.desired_rendition().font().rotation() = file.desired_rendition().font().rotation();


    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                    //  WT_Rendition::Fill_Pattern_Bit    |
                                    //  WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Font_Extension_Bit  |
                                        WT_Rendition::Font_Bit            |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::Text_Background_Bit |
                                        WT_Rendition::Text_HAlign_Bit     |
                                        WT_Rendition::Text_VAlign_Bit     |
                                        WT_Rendition::Contrast_Color_Bit;
                                    //  WT_Rendition::Delineate_Bit            

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
        ((WT_Text *)this)->transform (file.heuristics().transform());


    if (file.heuristics().allow_binary_data())
    {
        ((WT_Text *)this)->relativize(file);

        if (m_option_bounds.bounds()    ||
            m_option_overscore.count()  ||
            m_option_underscore.count())
        {
            // Advanced binary text

            WD_CHECK (file.write((WT_Byte) 0x18));  // CTRL-X
            WD_CHECK (file.write(1, &m_position));
            WD_CHECK (m_string.serialize(file, WD_True));
            WD_CHECK (m_option_overscore.serialize(*this, file));
            WD_CHECK (m_option_underscore.serialize(*this, file));
            WD_CHECK (m_option_bounds.serialize(*this, file));
            if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
                WD_CHECK (m_option_reserved.serialize(*this, file));
        }
        else
        {
            // Basic binary text
            WD_CHECK (file.write((WT_Byte) 'x'));
            WD_CHECK (file.write(1, &m_position));
            WD_CHECK (m_string.serialize(file, WD_True));
        }
    } // if (allow binary data)
    else
    {
        // Extended ASCII
        // Don't relativize ASCII objects
        WD_CHECK (file.write_geom_tab_level());
        WD_CHECK (file.write("(Text "));
        WD_CHECK (file.write_ascii(m_position));
        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (m_string.serialize(file));
        WD_CHECK (m_option_overscore.serialize(*this, file));
        WD_CHECK (m_option_underscore.serialize(*this, file));
        WD_CHECK (m_option_bounds.serialize(*this, file));
        if (file.heuristics().target_version() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
            WD_CHECK (m_option_reserved.serialize(*this, file));
        WD_CHECK (file.write((WT_Byte) ')'));
    } // else (extended ASCII output)

    return WT_Result::Success;
}
#else
WT_Result WT_Text::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_DRAW_TEXT_ATTRIBUTES_REMOVED)
                return materialize_obsolete_form(opcode, file);   // Old obsolete version of DrawText opcode
            else
            {
                // Current version of DrawText opcode
                switch (opcode.token()[0])
                {
                case 0x18: // Ctrl-X
                    {
                        switch (m_stage)
                        {
                        case Getting_Started:
                            m_stage = Getting_Position;

                            // No break;
                        case Getting_Position:
                            WD_CHECK (file.read(1, &m_position));
                            m_stage = Getting_String;

                            // No break
                        case Getting_String:
                            WD_CHECK (m_string.materialize(file));
                            m_stage = Getting_Overscore;

                            // No break
                        case Getting_Overscore:
                            WD_CHECK (m_option_overscore.materialize(*this, m_optioncode, file));
                            m_stage = Getting_Underscore;

                            // No break
                        case Getting_Underscore:
                            WD_CHECK (m_option_underscore.materialize(*this, m_optioncode, file));
                            m_stage = Getting_Bounds;

                            // No break
                        case Getting_Bounds:
                            WD_CHECK (m_option_bounds.materialize(*this, m_optioncode, file));
                            m_stage = Getting_Reserved;

                            // No break
                        case Getting_Reserved:
                            if (file.rendition().drawing_info().decimal_revision() >= REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
                                WD_CHECK (m_option_reserved.materialize(*this, m_optioncode, file));
                            m_stage = Completed;
                            break;

                        default:
                            return WT_Result::Internal_Error;
                        }
                    }   // case Ctrl-X

                    break;
                case 'x':
                    {
                        switch (m_stage)
                        {
                        case Getting_Started:
                            m_stage = Getting_Position;

                            // No break
                        case Getting_Position:
                            WD_CHECK (file.read(1, &m_position));
                            m_stage = Getting_String;

                            // No break
                        case Getting_String:
                            WD_CHECK (m_string.materialize(file));
                            m_stage = Completed;

                            break;
                        default:
                            return WT_Result::Internal_Error;
                        } // switch (m_stage)

                    } break;  // case 'x'

                default:
                    return WT_Result::Opcode_Not_Valid_For_This_Object;  // Illegal opcode token
                } // switch (opcode token)
            } // else (current version of DrawText)

            m_relativized = WD_True;
            de_relativize(file);

        } break;
    case WT_Opcode::Extended_ASCII:
        {
            switch (m_stage)
            {
            case Getting_Started:
                m_stage = Getting_Position;

                // No break
            case Getting_Position:
                WD_CHECK (file.read_ascii(m_position));
                m_stage = Getting_String;

                // No break
            case Getting_String:
                WD_CHECK (m_string.materialize(file));
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
                case WT_Text_Optioncode::Overscore_Option:
                    WD_CHECK(m_option_overscore.materialize(*this, m_optioncode, file));
                    break;
                case WT_Text_Optioncode::Underscore_Option:
                    WD_CHECK(m_option_underscore.materialize(*this, m_optioncode, file));
                    break;
                case WT_Text_Optioncode::Bounds_Option:
                    WD_CHECK(m_option_bounds.materialize(*this, m_optioncode, file));
                    break;
                case WT_Text_Optioncode::Reserved_Option:
                    WD_CHECK(m_option_reserved.materialize(*this, m_optioncode, file));
                    break;
                case WT_Text_Optioncode::Unknown_Option:
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

        } break; // ASCII
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text::materialize_obsolete_form(WT_Opcode const & opcode, WT_File & file)
{
    WT_Integer16    tmp_int16;
    WT_Integer32    tmp_int32;

    switch (opcode.token()[0])
    {
    case 'x':
        {
            switch (m_stage)
            {
            case Getting_Started:
                WD_Assert(!m_obsolete_font_holder);

                m_obsolete_font_holder = new WT_Font;
                if (!m_obsolete_font_holder)
                    return WT_Result::Out_Of_Memory_Error;

                m_obsolete_font_holder->set_fields_defined( WT_Font::FONT_HEIGHT_BIT         |
                                                            WT_Font::FONT_ROTATION_BIT       |
                                                            WT_Font::FONT_OBLIQUE_BIT        |
                                                            WT_Font::FONT_WIDTH_SCALE_BIT    |
                                                            WT_Font::FONT_SPACING_BIT        |
                                                            WT_Font::FONT_FLAGS_BIT           );
                m_stage = Getting_Width_Scale;

                // No break
            case Getting_Width_Scale:

                WD_CHECK (file.read(tmp_int16));
                m_obsolete_font_holder->width_scale().set(tmp_int16);
                m_stage = Getting_Spacing;

            case Getting_Spacing:
                WD_CHECK (file.read(tmp_int16));
                m_obsolete_font_holder->spacing().set(tmp_int16);
                m_stage = Getting_Flags;

            case Getting_Flags:
                WD_CHECK (file.read(tmp_int32));
                m_obsolete_font_holder->flags().set(tmp_int32);
                m_stage = Getting_Oblique_Angle;

                // No break
            case Getting_Oblique_Angle:
                WD_CHECK (file.read(tmp_int16));
                m_obsolete_font_holder->oblique().set(tmp_int16);
                m_stage = Getting_Overscore;

                // No break
            case Getting_Overscore:
                WD_CHECK (m_option_overscore.materialize(*this, m_optioncode, file));
                m_stage = Getting_Underscore;

                // No break
            case Getting_Underscore:
                WD_CHECK (m_option_underscore.materialize(*this, m_optioncode, file));
                m_stage = Getting_Rotation;

                // No break
            case Getting_Rotation:
                WD_CHECK (file.read(tmp_int16));
                m_obsolete_font_holder->rotation().set(tmp_int16);
                m_stage = Getting_Height;

                // No break
            case Getting_Height:
                WD_CHECK (file.read(tmp_int32));
                if (file.heuristics().apply_transform())
                tmp_int32 = (WT_Integer32) (tmp_int32 * file.heuristics().transform().m_y_scale);
                m_obsolete_font_holder->height().set(tmp_int32);
                m_stage = Getting_Position;

                // No break
            case Getting_Position:
                WD_CHECK (file.read(1, &m_position));
                m_stage = Getting_BBox_Deltas;

                // No break
            case Getting_BBox_Deltas:
                {
                    WT_Logical_Point    tmp_bbox[4];

                    WD_CHECK (file.read(4, tmp_bbox));

                    // convert from relative to absolute
                    m_position = file.de_update_current_point(m_position);
                    m_relativized = WD_False;

                    WT_Integer32    delta[8];

                    delta[0] = tmp_bbox[0].m_x;
                    delta[1] = tmp_bbox[0].m_y;
                    delta[2] = tmp_bbox[1].m_x;
                    delta[3] = tmp_bbox[1].m_y;
                    delta[4] = tmp_bbox[2].m_x;
                    delta[5] = tmp_bbox[2].m_y;
                    delta[6] = tmp_bbox[3].m_x;
                    delta[7] = tmp_bbox[3].m_y;

                    tmp_bbox[0].m_x = m_position.m_x  + delta[4];
                    tmp_bbox[0].m_y = m_position.m_y  + delta[6];
                    tmp_bbox[1].m_x = tmp_bbox[0].m_x + delta[7];
                    tmp_bbox[1].m_y = tmp_bbox[0].m_y + delta[5];
                    tmp_bbox[2].m_x = tmp_bbox[1].m_x + delta[2];
                    tmp_bbox[2].m_y = tmp_bbox[1].m_y + delta[3];
                    tmp_bbox[3].m_x = tmp_bbox[0].m_x + delta[0];
                    tmp_bbox[3].m_y = tmp_bbox[0].m_y + delta[1];

                    m_option_bounds.set(tmp_bbox);
                    m_stage = Getting_Str_Length;
                }

                // No break
            case Getting_Str_Length:
                WD_CHECK (file.read_count(m_obsolete_length_holder));

                m_obsolete_msg_holder = new unsigned short[m_obsolete_length_holder + 1];
                if (!m_obsolete_msg_holder)
                    return WT_Result::Out_Of_Memory_Error;

                m_stage = Getting_Msg;

                // No break
            case Getting_Msg:
                WD_CHECK (file.read( 2 * m_obsolete_length_holder, (WT_Byte *)m_obsolete_msg_holder));
                m_obsolete_msg_holder[m_obsolete_length_holder] = 0;
                m_string.set(m_obsolete_length_holder, m_obsolete_msg_holder);
                delete []m_obsolete_msg_holder;

                m_stage = Completed;
                break;

            default:
                return WT_Result::Internal_Error;
            } // switch (m_stage)

            if (file.heuristics().apply_transform())
                transform(file.heuristics().transform());

        } break;

    case 0x18:
        {
            WT_Integer32    tmp_int32;
            WT_Integer16    tmp_int16;

            switch (m_stage)
            {
            case Getting_Started:
                WD_Assert(!m_obsolete_font_holder);

                m_obsolete_font_holder = new WT_Font;
                if (!m_obsolete_font_holder)
                    return WT_Result::Out_Of_Memory_Error;

                m_obsolete_font_holder->set_fields_defined( WT_Font::FONT_HEIGHT_BIT         |
                                                            WT_Font::FONT_ROTATION_BIT        );
                m_stage = Getting_Rotation;

            case Getting_Rotation:
                WD_CHECK (file.read(tmp_int16));
                m_obsolete_font_holder->rotation().set(tmp_int16);
                m_stage = Getting_Height;

            case Getting_Height:
                WD_CHECK (file.read(tmp_int32));
                if (file.heuristics().apply_transform())
                tmp_int32 = (WT_Integer32) (tmp_int32 * file.heuristics().transform().m_y_scale);

                m_obsolete_font_holder->height().set(tmp_int32);
                m_stage = Getting_Position;

            case Getting_Position:
                WD_CHECK (file.read(1, &m_position));
                m_relativized = WD_True;
                m_stage = Getting_Str_Length;

            case Getting_Str_Length:
                WD_CHECK (file.read_count(m_obsolete_length_holder));

                m_obsolete_msg_holder = new unsigned short[m_obsolete_length_holder + 1];
                if (!m_obsolete_msg_holder)
                    return WT_Result::Out_Of_Memory_Error;

                m_stage = Getting_Msg;

                // No break
            case Getting_Msg:
                WD_CHECK (file.read( 2 * m_obsolete_length_holder, (WT_Byte *)m_obsolete_msg_holder));
                m_obsolete_msg_holder[m_obsolete_length_holder] = 0;
                m_string.set(m_obsolete_length_holder, m_obsolete_msg_holder);
                delete []m_obsolete_msg_holder;
                m_stage = Completed;

                break;
            default:
                return WT_Result::Internal_Error;
            } // switch (m_stage)

        } break;
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    de_relativize(file);

    if (file.heuristics().apply_transform())
        transform(file.heuristics().transform());

    m_materialized = WD_True;

    // Now send these WT_Font attributes off for processing...
    if (m_obsolete_font_holder)
    {
        m_obsolete_font_holder->process(file);
        delete m_obsolete_font_holder;
        m_obsolete_font_holder = WD_Null;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Single_Byte:
        {
            return materialize(opcode, file);
        } break;
    case WT_Opcode::Extended_ASCII:
            WD_CHECK (opcode.skip_past_matching_paren(file));
        break;
    case WT_Opcode::Extended_Binary:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Text::transform(WT_Transform const & transform)
{
    if (!m_transformed)
    {
        WT_Logical_Point    tmp_pt;

        tmp_pt = m_position;
        m_position = tmp_pt * transform;

        m_option_bounds.transform(transform);
        m_transformed = WD_True;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text::process(WT_File & file)
{
    WD_Assert (file.text_action());
    return (file.text_action())(*this, file);
}

WT_Result WT_Text::default_process(WT_Text &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_Text::relativize(WT_File & file)
{
    if (!m_relativized)
    {
        m_position = file.update_current_point(m_position);
        m_option_bounds.relativize(file);
        m_relativized = WD_True;
    } // If (!relativized)
}
#else
void WT_Text::relativize(WT_File &)
{
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_Text::de_relativize(WT_File & file)
{
    if (m_relativized)
    {
        m_position = file.de_update_current_point(m_position);
        m_option_bounds.de_relativize(file);
        m_relativized = WD_False;
    } // If (!relativized)
}

///////////////////////////////////////////////////////////////////////////

// FIXME: WT_Text::update_bounds is completely Win32 specific, and must be ported.
#ifdef WD_WIN32_SYSTEM

#include <tchar.h>
#pragma comment (lib, "gdi32.lib") //for font bounds approximation

void WT_Text::update_bounds(WT_File * file)
{
    if (m_option_bounds.bounds() != WD_Null)
    {
        WT_Drawable::update_bounds(m_option_bounds.bounds(), 2);
        return;
    }

    WT_Drawable::update_bounds(m_position); //base point

    if (m_string.length()==0)
        return;

    WT_Integer32 height = file->rendition().font().height();
    int rotation = (int)((float)TWO_PI * (float)file->rendition().font().rotation() / 65536.0f);
    int oblique = (int)((float)TWO_PI * (float)file->rendition().font().oblique() / 65536.0f);
    BYTE charset = file->rendition().font().charset().charset();
    BYTE pitch_and_family = file->rendition().font().pitch().pitch() | file->rendition().font().family().family();
    UINT string_length = m_string.length();
    BYTE italic = file->rendition().font().style().italic();
    BYTE underline = file->rendition().font().style().underlined();
    BYTE bold = file->rendition().font().style().bold();
    //
    // this is dangerous since it assumes wchar_t is always 16-bit uchar
    //
    wchar_t* facename = new wchar_t[file->rendition().font().font_name().name().length()+1];
    const wchar_t* cwstr = (const wchar_t*)file->rendition().font().font_name().name().unicode();
    if (cwstr)
    {
        wcscpy(facename, cwstr);
    }
    else
    {
        facename[0] = 0;
    }


    wchar_t* wstr = new wchar_t[m_string.length()+1];
    wcscpy(wstr, (const wchar_t*)m_string.unicode());

    HWND hwnd = NULL;
    HDC hdc = NULL;

    do
    {
        hwnd = ::GetDesktopWindow();
        if (!hwnd || hwnd == INVALID_HANDLE_VALUE)
            break;
        hdc = ::GetDC(hwnd);
        if (!hdc || hdc == INVALID_HANDLE_VALUE)
            break;

        LOGFONTW logfont;
        logfont.lfHeight = 2048; //we'll get the right value down below
        logfont.lfWidth = 0; //force aspect ratio of font
        logfont.lfEscapement = oblique;
        logfont.lfOrientation = rotation;
        logfont.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
        logfont.lfItalic = italic;
        logfont.lfUnderline = underline;
        //logfont.lfStrikeOut;
        logfont.lfCharSet = charset;
        logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
        logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logfont.lfQuality = DEFAULT_QUALITY;
        logfont.lfPitchAndFamily = pitch_and_family;
        memset(logfont.lfFaceName,0,LF_FACESIZE*sizeof(wchar_t));
        wcsncpy(logfont.lfFaceName,facename,LF_FACESIZE);

        HFONT hfont = ::CreateFontIndirectW(&logfont);
        if (!hfont || hfont == INVALID_HANDLE_VALUE)
            break;

        //we want to get the width based on font design units, probably 2048, maybe 1024
        //query and figure that out.  use that as the height

        HGDIOBJ hobj = ::SelectObject(hdc, hfont);

        int size = ::GetOutlineTextMetricsW(hdc, 0, NULL);
        if (size != 0)
        {
            OUTLINETEXTMETRICW *otm = (OUTLINETEXTMETRICW *)new char [size];
            WD_Assert(otm != NULL);

            UINT status = GetOutlineTextMetricsW (hdc, size, otm);
            WD_Assert(status != 0);

            if (status != 0)
                logfont.lfHeight = otm->otmEMSquare;

            delete [] otm;
        }
        else
        {
            logfont.lfHeight = 1000;
        }

        ::SelectObject(hdc, hobj);
        ::DeleteObject(hfont);
        hfont = ::CreateFontIndirectW(&logfont);
        if (!hfont || hfont == INVALID_HANDLE_VALUE)
            break;

        hobj = ::SelectObject(hdc, hfont);

        SIZE fontsize;
        ::GetTextExtentPointW(hdc, wstr, string_length, &fontsize);
        ::SelectObject(hdc, hobj);
        ::DeleteObject(hfont);

        //now we have the width in font design units.  Now translate this based on font height.

        int deltaX = (int)((double)height * (double)fontsize.cx / (double)logfont.lfHeight);
        int deltaY = (int)((double)height * (double)fontsize.cy / (double)logfont.lfHeight);

        bool overflowX = (double)((m_position.m_x + deltaX) - (double)m_position.m_x) != deltaX;
        bool overflowY = (double)((m_position.m_y + deltaY) - (double)m_position.m_y) != deltaY;

        int ptx = overflowX //exceeded intmax?
                ? 0x7FFFFFFF
                : m_position.m_x + deltaX;

        int pty = overflowY //exceeded intmax?
                ? 0x7FFFFFFF
                : m_position.m_y + deltaY;

        WT_Drawable::update_bounds(WT_Logical_Point(ptx,pty));

    } while (false);

    if (hdc && hdc != INVALID_HANDLE_VALUE && hwnd && hwnd != INVALID_HANDLE_VALUE)
        ::ReleaseDC(hwnd, hdc);

    delete [] facename;
    delete [] wstr;
}

#else // WD_WIN32_SYSTEM

void WT_Text::update_bounds(WT_File *) {
    WD_Complain ("WT_Text::update_bounds is not implemented on this system");
}

#endif // WD_WIN32_SYSTEM
