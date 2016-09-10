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
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/text_valign.cpp 9     7/26/05 11:00a T_royaj $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
const char* WT_Text_VAlign::str_VAlign_Descentline = "Descentline";
const char* WT_Text_VAlign::str_VAlign_Baseline = "Baseline";
const char* WT_Text_VAlign::str_VAlign_Halfline = "Halfline";
const char* WT_Text_VAlign::str_VAlign_Capline = "Capline";
const char* WT_Text_VAlign::str_VAlign_Ascentline = "Ascentline";

///////////////////////////////////////////////////////////////////////////
WT_Text_VAlign::WT_Text_VAlign()
:m_eAlignment(WT_Text_VAlign::Baseline)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Text_VAlign::WT_Text_VAlign(WT_VAlign vAlign)
:m_eAlignment(vAlign)
{
}


///////////////////////////////////////////////////////////////////////////
WT_Text_VAlign::~WT_Text_VAlign()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Text_VAlign::operator==(WT_Attribute const & attrib) const
{

    return ( attrib.object_id() == Text_VAlign_ID ) ?
        ( ( WT_Text_VAlign& ) attrib ).m_eAlignment == m_eAlignment: false;

}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_VAlign::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {

        case WT_Opcode::Extended_ASCII:
            {
                char * pchar = NULL;
                WD_CHECK(file.eat_whitespace()); // move to the Enum
                WD_CHECK(file.read(pchar, 256)); // read enum text string upto 256

                // convert enum string to enum value
                string_to_enum(pchar, this->m_eAlignment);

                // pchar is allocated as a char[]
                if(pchar)
                    delete[] pchar; // cleanup

                // Go to the end of the opcode
                WD_CHECK(opcode.skip_past_matching_paren(file));
                result = WT_Result::Success;
            }
            break;
        case WT_Opcode::Extended_Binary:
            {
                WT_Byte eValue;
                WD_CHECK(file.read(eValue));

                // if enum is invalid assign default value
                if((unsigned int)eValue < 5)
                {
                    m_eAlignment = (WT_VAlign)eValue;
                }
                else
                {
                    m_eAlignment = WT_Text_VAlign::Baseline;
                }

                WD_CHECK(file.read(close)); //read EXBO close
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
WT_Object::WT_ID WT_Text_VAlign::object_id() const
{
    return WT_Object::Text_VAlign_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_VAlign::process(WT_File & file)
{
    WD_Assert(file.text_valign_action());
    return (file.text_valign_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_VAlign::serialize(WT_File & file) const
{

   WD_CHECK (file.dump_delayed_drawable());
   file.desired_rendition().blockref();
   WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

   if (file.heuristics().allow_binary_data())
   {
       // Extended Binary Output
       WD_CHECK (file.write((WT_Byte)'{'));                     // Write out EXB opener

       WD_CHECK (file.write((WT_Integer32)(                     // Write out 4 byte packer
                            sizeof(WT_Integer16) +              // opcode
                            sizeof(WT_Byte)      +              // eAlignment
                            sizeof(WT_Byte))));                 // EXBO closer '}'

       WD_CHECK (file.write((WT_Integer16)WD_EXBO_TEXT_VALIGN));// Write out opcode
       WD_CHECK (file.write((WT_Byte)m_eAlignment));            // Write out enum value
       WD_CHECK (file.write((WT_Byte)'}'));                     // Write out EXBO closer
   }
   else
   {
        // Extended ASCII output
       WD_CHECK (file.write_tab_level());
       WD_CHECK (file.write("(TextVAlign "));              // Write out opcode
       WD_CHECK (file.write(enum_to_string(m_eAlignment))); // Write out enum value
       WD_CHECK (file.write((WT_Byte)')'));
   }
   return WT_Result::Success;

}
#else
WT_Result WT_Text_VAlign::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_VAlign::skip_operand(WT_Opcode const & , WT_File & )
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_VAlign::sync(WT_File & file) const
{

    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().text_valign())
    {
        file.rendition().text_valign() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Text_VAlign::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_VAlign::default_process( WT_Text_VAlign & item,
                                           WT_File & file )
{
    file.rendition().text_valign() = item;
    return WT_Result::Success;
}


///////////////////////////////////////////////////////////////////////////
const char* WT_Text_VAlign::enum_to_string( WT_VAlign eVAlign )
{
    const char* pStr = str_VAlign_Baseline;
    switch (eVAlign)
    {
    case WT_Text_VAlign::Descentline:
        pStr = str_VAlign_Descentline;
        break;
    case WT_Text_VAlign::Halfline:
        pStr = str_VAlign_Halfline;
        break;
    case WT_Text_VAlign::Capline:
        pStr = str_VAlign_Capline;
        break;
    case WT_Text_VAlign::Ascentline:
        pStr = str_VAlign_Ascentline;
        break;
    case WT_Text_VAlign::Baseline:
    default:
        break;
    }

    return pStr;
}


///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Text_VAlign::string_to_enum(const char* pStr, WT_VAlign& outEnum)
{
    // Assign the default value;
    outEnum = WT_Text_VAlign::Baseline;

    switch (pStr[0])
    {
    case 'A':
        if(strncmp(pStr, str_VAlign_Ascentline, 10) == 0)
        {
            outEnum = WT_Text_VAlign::Ascentline;
        }
        break;
    case 'B':
        if(strncmp(pStr, str_VAlign_Baseline, 8) == 0)
        {
            outEnum = WT_Text_VAlign::Baseline;
        }
        break;
    case 'C':
        if(strncmp(pStr, str_VAlign_Capline, 7) == 0)
        {
            outEnum = WT_Text_VAlign::Capline;
        }
        break;
    case 'D':
        if(strncmp(pStr, str_VAlign_Descentline, 11) == 0)
        {
            outEnum = WT_Text_VAlign::Descentline;
        }
        break;
    case 'H':
        if(strncmp(pStr, str_VAlign_Halfline, 8) == 0)
        {
            outEnum = WT_Text_VAlign::Halfline;
        }
        break;
    default:
        // there is no need for this code but Linux complains if the default
        // parameter is not set in the switch statement.( strict compiler settings)
         outEnum = WT_Text_VAlign::Baseline;
    }
    return true;
}

// End of file.
