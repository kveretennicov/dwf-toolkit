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
// $Header:


#include "whiptk/pch.h"

#ifdef WD_NEED_DEBUG_PRINTF
    extern int WD_dprintf (char const * control, ...);
#endif


///////////////////////////////////////////////////////////////////////////
WT_UserData::WT_UserData(
            const WT_String           & data_description,
            WT_Integer32                data_size,
            WT_Byte *                   data,
            WT_Boolean                  copy) throw(WT_Result)
            : m_data_description(data_description)
            , m_data_size(data_size)
            , m_data(WD_Null)
            , m_local_data_copy(copy)
            , m_stage(Starting)
{
    WD_Assert (data_size && data);

    if (copy)
    {
        m_local_data_copy = copy;
        m_data = new WT_Byte[data_size];

        if (m_data)
            memcpy(m_data, data, data_size);
        else
            throw WT_Result::Out_Of_Memory_Error;
    }  // if (copy)
    else
    {
        m_data = data;
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_UserData::set_data(WT_Integer32 size,WT_Byte* data)
{
    if((m_local_data_copy) && (m_data))
        delete [] m_data;
    
    if(size)
    {
        m_data_size = size;
        m_local_data_copy = WD_True;
        m_data = new WT_Byte[m_data_size];
        if(!m_data)
            return WT_Result::Out_Of_Memory_Error;

        WD_COPY_MEMORY(data,m_data_size,m_data);
    }
    else
    {
        m_data_size = 0;
        m_local_data_copy = WD_False;
        m_data = WD_Null;
    }

    return WT_Result::Success;

}
///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_UserData::object_id() const
{
    return UserData_ID;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_UserData::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    WT_Integer32    parts_to_sync = //  WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::BlockRef_Bit        |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit;

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().allow_binary_data())
    {
        WD_CHECK (file.write((WT_Byte) '{'));
        WD_CHECK (file.write((WT_Integer32) (sizeof(WT_Unsigned_Integer16) + // for the opcode
                                             sizeof(WT_Byte)  +              // The open quote
                                             m_data_description.length() +   // data description
                                             sizeof(WT_Byte)  +              // The close quote
                                             sizeof(WT_Integer32) +          // the data size
                                             m_data_size +                   // the user data
                                             sizeof(WT_Byte)                 // The closing "}"
                                                )));

        WD_CHECK (file.write((WT_Unsigned_Integer16)  WD_EXBO_USERDATA));
        WD_CHECK (file.write_quoted_string(m_data_description.ascii(), WD_True));
        WD_CHECK (file.write(m_data_size));
        if(m_data_size) {
            WD_CHECK (file.write(m_data_size, m_data));
        }
        WD_CHECK (file.write((WT_Byte) '}'));

    }
    else
    {
        // Extended ASCII output

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(UserData "));

        WD_CHECK (file.write_quoted_string(m_data_description.ascii(), WD_True));
        WD_CHECK (file.write((WT_Byte) ' '));

        WD_CHECK (file.write_ascii(m_data_size));
        WD_CHECK (file.write((WT_Byte) ' '));

        if(m_data_size) {
            WD_CHECK (file.write_hex(m_data_size, m_data));
        }
        WD_CHECK (file.write((WT_Byte) ' '));

        WD_CHECK (file.write(")"));
    }

    return WT_Result::Success;
}
#else
WT_Result WT_UserData::serialize(WT_File &) const
{
   return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_UserData::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
    case WT_Opcode::Extended_Binary:
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Data_Description;
            // No break;

        case Getting_Data_Description:
            char *  a_string;
            a_string = NULL;
            WD_CHECK (file.read(a_string, 65535));
            m_data_description = a_string;
            if (a_string)
                delete[] a_string;
            m_stage = Getting_Data_Size;
            //No Break

        case Getting_Data_Size:
            WD_CHECK (file.read(m_data_size));
            m_stage = Getting_Data;
            // No break

        case Getting_Data:
            if ((!m_data) && (m_data_size > 0))
            {
                m_data = new WT_Byte[m_data_size];
                if (!m_data)
                    return WT_Result::Out_Of_Memory_Error;
                m_local_data_copy = WD_True;
            }
            if(m_data_size > 0) {
                WD_CHECK (file.read(m_data_size, m_data));
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

    case WT_Opcode::Extended_ASCII:

        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Data_Description;
            // No break;

        case Getting_Data_Description:
            WD_CHECK(file.eat_whitespace());
            char *  a_string;
            a_string = NULL;
            WD_CHECK (file.read(a_string, 65535));
            m_data_description = a_string;
            if (a_string)
                delete[] a_string;
            m_stage = Getting_Data_Size;
            //No Break

        case Getting_Data_Size:
            WD_CHECK(file.eat_whitespace());
            WD_CHECK (file.read_ascii(m_data_size));
            m_stage = Getting_Data;

            // No break
        case Getting_Data:
            WD_CHECK(file.eat_whitespace());
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

            // There will be two close parens: one for the data, one for the opcode
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
WT_Result WT_UserData::skip_operand(WT_Opcode const &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_UserData::process(WT_File & file)
{
    WD_Assert (file.userdata_action());
    return (file.userdata_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_UserData::default_process(WT_UserData &, WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type WT_UserData::object_type() const
{
    return Wrapper;
}
