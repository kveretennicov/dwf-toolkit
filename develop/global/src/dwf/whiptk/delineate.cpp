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
WT_Delineate::WT_Delineate()
: m_delineate(WD_False)
{
}

    
///////////////////////////////////////////////////////////////////////////
WT_Delineate::WT_Delineate(WT_Boolean delineate_state)
: m_delineate(delineate_state)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Delineate::~WT_Delineate()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Delineate::object_id() const
{
    return WT_Object::Delineate_ID;
}
 
///////////////////////////////////////////////////////////////////////////
WT_Result  WT_Delineate::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            m_delineate = WD_True;
         
            result = WT_Result::Success;
            WD_CHECK (file.read(close));    // read EXAO close
            if( close != ')')
            {
                result = WT_Result::Corrupt_File_Error;
            }  
        } break;
    case WT_Opcode::Extended_Binary:
        {
            m_delineate = WD_True;
            result = WT_Result::Success;
            WD_CHECK (file.read(close));    // read EXBO close
            if( close != '}')
            {
                result = WT_Result::Corrupt_File_Error;
            }
        }
        break;
    default:
        {
            result = WT_Result::Opcode_Not_Valid_For_This_Object;
        } 
    } // switch

    m_materialized = (result == WT_Result::Success);
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Delineate::process(WT_File & file)
{
    WD_Assert(file.delineate_action());
    return (file.delineate_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Delineate::skip_operand(WT_Opcode const &, WT_File &)
{
     return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Delineate::serialize (WT_File & file) const
{
   // Macro_Size is only supported in versions 6.01 and up . 
   if (file.heuristics().target_version() < REVISION_WHEN_MACRO_IS_SUPPORTED)
       return WT_Result::Toolkit_Usage_Error;

   // Delineate opcode will not be written out if the value is not true.
   if (this->m_delineate)
   {     
        WD_CHECK (file.dump_delayed_drawable());
        file.desired_rendition().blockref();
        WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));  

        if (file.heuristics().allow_binary_data()) 
        {    
                WD_CHECK (file.write((WT_Byte)'{'));                     // Write out EXBO opener
                
                WD_CHECK (file.write((WT_Integer32)(                    // Write out 4 byte packer
                                    sizeof(WT_Integer16) +              // opcode
                                    sizeof(WT_Byte))));                 // EXBO closer '}'

                WD_CHECK (file.write((WT_Integer16)WD_EXBO_DELINEATE));  // Write out opcode 
                WD_CHECK (file.write((WT_Byte)'}'));                     // Write out EXBO closer
        }
        else // Extended Ascii
        {
                WD_CHECK (file.write_tab_level());
                WD_CHECK (file.write("(Delineate"));                  // Write out opcode 
                WD_CHECK (file.write((WT_Byte)')'));                  // Write close bracket
        }
   }
   return WT_Result::Success;
}
#else
WT_Result WT_Delineate::serialize (WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_OUTPUT

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Delineate::sync (WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    // write out the new value of delineate 
    if (*this != file.rendition().delineate())
    {
        file.rendition().delineate() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Delineate::sync (WT_File &) const
{
     return WT_Result::Success;
}
#endif // DESIRED_OUTPUT

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Delineate::operator== (WT_Attribute const & attrib) const
{
    WT_Boolean bReturn = WD_False;

    if  (attrib.object_id() == WT_Object::Delineate_ID)
    {
        bReturn = ((WT_Delineate&)attrib).m_delineate == this->m_delineate;
    }
    return bReturn;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Delineate::default_process(WT_Delineate & item, WT_File & file)
{
    file.rendition().delineate() = item;
    if (item.delineate())
    {
        file.rendition().fill().set(WD_False);
    }
    return WT_Result::Success;
}

// End of file
