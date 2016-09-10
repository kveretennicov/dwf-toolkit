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


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID   WT_Contrast_Color::object_id() const
{
    return Contrast_Color_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Contrast_Color::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {
        case WT_Opcode::Extended_ASCII:
            {
                WD_CHECK (file.eat_whitespace()); // move to color
                WD_CHECK (file.read_ascii(m_color));
                WD_CHECK (file.eat_whitespace()); // move to offset
                // Go to the end of the opcode
                WD_CHECK (opcode.skip_past_matching_paren(file)); 
                result = WT_Result::Success;
            }
            break;
        case WT_Opcode::Extended_Binary:
            {
                WD_CHECK (file.read(m_color));  //read color
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
WT_Result   WT_Contrast_Color::process(WT_File & file)
{
    WD_Assert (file.contrast_color_action());
    return (file.contrast_color_action())(*this,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Contrast_Color::skip_operand(WT_Opcode const & , 
                                              WT_File & )
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Contrast_Color::serialize (WT_File & file) const
{
   WD_CHECK (file.dump_delayed_drawable());
   file.desired_rendition().blockref();
   WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));  

   // Extended Binary output
   if (file.heuristics().allow_binary_data()) 
   {
        WD_CHECK (file.write((WT_Byte)'{'));                            // Write out EXBO opener
        
        WD_CHECK (file.write((WT_Integer32)(                            // Write out 4 byte packer
                             sizeof(WT_Integer16) +                     // opcode
                             sizeof(WT_RGBA32)    +                     // color
                             sizeof(WT_Byte))));                        // EXBO closer '}'

        WD_CHECK (file.write((WT_Integer16)WD_EXBO_SET_CONTRAST_COLOR));  // Write out opcode 
        WD_CHECK (file.write(m_color));                                   // Write out color value 
        WD_CHECK (file.write((WT_Byte)'}'));                              // Write out EXBO closer
   }
   else // Extended Ascii output 
   {
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(ContrastColor "));                       // !!!To be changed                   // Write opcode
        WD_CHECK (file.write_ascii(m_color));                           // Write color
        WD_CHECK (file.write((WT_Byte)')'));                            // Write close bracket
   }
   return WT_Result::Success;
}
#else
WT_Result   WT_Contrast_Color::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_Contrast_Color::sync (WT_File & file) const
{
    WD_Assert ((file.file_mode() == WT_File::File_Write)   ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if( *this != file.rendition().contrast_color() )
    {   file.rendition().contrast_color() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result   WT_Contrast_Color::sync (WT_File &) const
{
    return WT_Result::Success;
}
#endif

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Contrast_Color::operator== (WT_Attribute const & attrib) const
{
    WT_Boolean result = WD_False;
    if (attrib.object_id() == WT_Object::Contrast_Color_ID)
    {
        result = this->operator ==((WT_Contrast_Color const &)attrib);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Contrast_Color::operator== (WT_Contrast_Color const & color) const
{
    return (color.m_color == this->m_color);
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Contrast_Color::operator!= (WT_Contrast_Color const & color) const
{
    return !(this->operator ==(color));
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Contrast_Color::default_process( WT_Contrast_Color & item, 
                                                WT_File & file)
{
    file.rendition().contrast_color() = item;
    return WT_Result::Success;
}
