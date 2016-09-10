//
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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/text_background.cpp 11    7/26/05 10:46a T_royaj $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
const char* WT_Text_Background::str_Background_None = "None";
const char* WT_Text_Background::str_Background_Ghosted = "Ghosted" ;
const char* WT_Text_Background::str_Background_Solid  = "Solid";

///////////////////////////////////////////////////////////////////////////
WT_Text_Background::WT_Text_Background()
: m_eBackground(WT_Text_Background::None)
, m_offset(0)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Background::WT_Text_Background(eBackground background,
                                       WT_Integer32 offset)
: m_eBackground(background)
, m_offset(offset)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Background::~WT_Text_Background()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Text_Background::operator==(WT_Attribute const & attrib) const
{
    WT_Boolean retValue = false;
     if(attrib.object_id() == Text_Background_ID)
     {
        WT_Text_Background& background = (WT_Text_Background&) attrib;
        if( background.m_eBackground == m_eBackground &&
            background.m_offset == m_offset)
        {
            retValue = true;
        }
     }
    return retValue;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Background::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {

        case WT_Opcode::Extended_ASCII:
            {
                char* pchar = NULL;
                WD_CHECK (file.eat_whitespace());       // move to the Enum
                WD_CHECK (file.read(pchar, 256));       // read upto 256 characters

                string_to_enum(pchar,this->m_eBackground); //convert enum to string
                // pchar is allocated as an char array
                if(pchar)
                    delete[] pchar;

                WD_CHECK (file.eat_whitespace());       // eat whitespace
                WD_CHECK (file.read_ascii(m_offset));   // read offset

                // Go to the end of the opcode
                WD_CHECK (opcode.skip_past_matching_paren(file));
                result = WT_Result::Success;
            }
            break;
        case WT_Opcode::Extended_Binary:
            {
                WT_Byte eValue;

                // read Enum value
                WD_CHECK (file.read(eValue));

                // assign default if invalid
                if((unsigned int)eValue < 3)
                {
                    m_eBackground = (eBackground)eValue;
                }
                else
                {
                    m_eBackground = WT_Text_Background::None;
                }

                WD_CHECK (file.read(m_offset)); // read offset

                WD_CHECK (file.read(close));    // read EXBO close
                if( close != '}')
                {
                    result = WT_Result::Corrupt_File_Error;
                }
                else
                {
                    result = WT_Result::Success;
                }
            }
            break;
        default:
            result = WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch
    m_materialized = (result == WT_Result::Success);
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID  WT_Text_Background::object_id() const
{
    return WT_Object::Text_Background_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Background::process(WT_File & file)
{
    WD_Assert(file.text_background_action());
    return (file.text_background_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_Background::serialize(WT_File & file) const
{
   WD_CHECK (file.dump_delayed_drawable());
   file.desired_rendition().blockref();
   WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

   if (file.heuristics().allow_binary_data())
   {
        WD_CHECK (file.write((WT_Byte)'{'));                     // Write out EXBO opener

        WD_CHECK (file.write((WT_Integer32)(                     // Write out 4 byte packer
                             sizeof(WT_Integer16) +              // opcode
                             sizeof(WT_Byte)      +              // Background Enum
                             sizeof(WT_Integer32) +              // offset
                             sizeof(WT_Byte))));                 // EXBO closer '}'

        WD_CHECK (file.write((WT_Integer16)WD_EXBO_TEXT_BACKGROUND));   // Write out opcode
        WD_CHECK (file.write((WT_Byte)m_eBackground));                  // Write out enum value
        WD_CHECK (file.write(m_offset));                                // Write out offset value
        WD_CHECK (file.write((WT_Byte)'}'));                            // Write out EXBO closer
   }
   else // Extended Ascii
   {
        // ASCII only output

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(TextBackground "));            // Write out opcode
        WD_CHECK (file.write(enum_to_string(m_eBackground))); // Write out enum value
        WD_CHECK (file.write((WT_Byte)' '));                  // Write space
        WD_CHECK (file.write_ascii(m_offset));                // Write offset
        WD_CHECK (file.write((WT_Byte)')'));                  // Write close bracket
   }
   return WT_Result::Success;
}
#else
WT_Result WT_Text_Background::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_OUTPUT

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Background::skip_operand(WT_Opcode const &, WT_File & )
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_Background::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().text_background())
    {
        file.rendition().text_background() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Text_Background::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_OUTPUT

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Background::default_process( WT_Text_Background & item,
                                               WT_File & file )

{
    file.rendition().text_background() = item;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
const char* WT_Text_Background::enum_to_string(eBackground background)
{
    const char* pStr = str_Background_None;
    switch (background)
    {
    case WT_Text_Background::Ghosted:
        pStr = str_Background_Ghosted;
        break;
    case WT_Text_Background::Solid:
        pStr = str_Background_Solid;
        break;
    case WT_Text_Background::None:
    default:
        break;
    }
    return pStr;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean   WT_Text_Background::string_to_enum(const char* pStr,
                                                eBackground& outBackground)
{
    outBackground = WT_Text_Background::None;
    switch (pStr[0])
    {
    case 'G':
        if(strncmp(pStr, str_Background_Ghosted, 7) == 0)
        {
            outBackground = WT_Text_Background::Ghosted;
        }
        break;
    case 'N': // Note: left this code in here so that if we ever change the logic
              // of the required behavior, this case will be handled.
        if(strncmp(pStr, str_Background_None, 4) == 0)
        {
            outBackground = WT_Text_Background::None;
        }
        break;
    case 'S':
        if(strncmp(pStr, str_Background_Solid, 5) == 0)
        {
            outBackground = WT_Text_Background::Solid;
        }
        break;
    default:
        // there is no need for this code but Linux complains if the default
        // parameter is not set in the switch statement.( strict compiler settings)
        outBackground = WT_Text_Background::None;
    }
    return true;
}
// End of file.
