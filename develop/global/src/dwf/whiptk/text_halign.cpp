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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/text_halign.cpp 9     7/26/05 10:59a T_royaj $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
const char* WT_Text_HAlign::str_HAlign_Left = "Left";
const char* WT_Text_HAlign::str_HAlign_Right = "Right";
const char* WT_Text_HAlign::str_HAlign_Center = "Center";

///////////////////////////////////////////////////////////////////////////
WT_Text_HAlign::WT_Text_HAlign()
:m_eAlignment(WT_Text_HAlign::Left)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Text_HAlign::WT_Text_HAlign(WT_HAlign hAlign)
:m_eAlignment(hAlign)
{
}


///////////////////////////////////////////////////////////////////////////
WT_Text_HAlign::~WT_Text_HAlign()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Text_HAlign::operator==(WT_Attribute const & attrib) const
{

    return ( attrib.object_id() == Text_HAlign_ID ) ?
        ( ( WT_Text_HAlign& ) attrib ).m_eAlignment == m_eAlignment: false;

}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_HAlign::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {

        case WT_Opcode::Extended_ASCII:
            {
                WD_CHECK(file.eat_whitespace()); // move to the Enum
                char* pchar  = NULL;
                WD_CHECK(file.read(pchar, 256));    //read in the Enum upto 256 characters
                string_to_enum(pchar, this->m_eAlignment);

                //pchar is allocated as a char[]
                if(pchar)
                    delete[] pchar;
                // Go to the end of the opcode
                WD_CHECK(opcode.skip_past_matching_paren(file));
                result = WT_Result::Success;
            }
            break;
        case WT_Opcode::Extended_Binary:
            {
                WT_Byte eValue;
                WD_CHECK (file.read(eValue));
                if((unsigned int)eValue < 3) // if enum is valid
                {
                    m_eAlignment = (WT_HAlign)eValue;
                }
                else        // assign the default value;
                {
                    m_eAlignment = WT_Text_HAlign::Left;
                }

                WD_CHECK (file.read(close)); // read EXBO closer
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
WT_Object::WT_ID WT_Text_HAlign::object_id() const
{
    return WT_Object::Text_HAlign_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_HAlign::process(WT_File & file)
{
   WD_Assert(file.text_halign_action());
    return (file.text_halign_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_HAlign::serialize(WT_File & file) const
{
   WD_CHECK (file.dump_delayed_drawable());
   file.desired_rendition().blockref();
   WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

   if (file.heuristics().allow_binary_data())
   {
       // Extended Binary Output
       WD_CHECK (file.write((WT_Byte)'{'));                     // Write out EXBO opener

       WD_CHECK (file.write((WT_Integer32)(                     // Write out 4 byte packer
                            sizeof(WT_Integer16) +              // opcode
                            sizeof(WT_Byte)      +              // eAlignment
                            sizeof(WT_Byte))));                 // EXBO closer '}'

       WD_CHECK (file.write((WT_Integer16)WD_EXBO_TEXT_HALIGN));// Write out opcode
       WD_CHECK (file.write((WT_Byte)m_eAlignment));            // Write out enum value
       WD_CHECK (file.write((WT_Byte)'}'));                     // Write out EXBO closer

   }
   else // Extended Ascii output
   {

        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(TextHAlign "));              // Write out opcode
        WD_CHECK (file.write(enum_to_string(m_eAlignment))); // Write out enum value
        WD_CHECK (file.write((WT_Byte)')'));
   }
   return WT_Result::Success;
}
#else
WT_Result WT_Text_HAlign::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_HAlign::skip_operand(WT_Opcode const & , WT_File & )
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_HAlign::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().text_halign())
    {
        file.rendition().text_halign() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Text_HAlign::sync(WT_File& ) const
{
    return WT_Result::Success;
}
#endif // DESIRED_CODE

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_HAlign::default_process( WT_Text_HAlign & item, // The object to process.
                                           WT_File & file )      // The file being read.
{
    file.rendition().text_halign() = item;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
const char* WT_Text_HAlign::enum_to_string( WT_HAlign eHAlign )
{
    const char* pStr = str_HAlign_Left;
    switch (eHAlign)
    {
    case WT_Text_HAlign::Right:
        pStr = str_HAlign_Right;
        break;
    case WT_Text_HAlign::Center:
        pStr = str_HAlign_Center;
        break;
    case WT_Text_HAlign::Left:
    default:
        break;
    }
    return pStr;
}


///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Text_HAlign::string_to_enum(const char* pStr, WT_HAlign& outEnum)
{
    outEnum = WT_Text_HAlign::Left;
    switch (pStr[0])
    {
    case 'C':
        if(strncmp(pStr, str_HAlign_Center, 6) == 0)
        {
            outEnum = WT_Text_HAlign::Center;
        }
        break;
    case 'L': // this code is left in for documentation purposes.
        if(strncmp(pStr, str_HAlign_Left, 4) == 0)
        {
            outEnum = WT_Text_HAlign::Left;
        }
        break;
    case 'R':
        if( strncmp(pStr, str_HAlign_Right, 5) == 0)
        {
            outEnum = WT_Text_HAlign::Right;
        }
        break;
    default:
        // there is no need for this code but Linux complains if the default
        // parameter is not set in the switch statement.( strict compiler settings)
        outEnum = WT_Text_HAlign::Left;

    } // end Switch

    return true;
}

// End of file.
