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
const char* WT_Overpost::str_AcceptMode_All = "All";
const char* WT_Overpost::str_AcceptMode_AllFit = "AllFit";
const char* WT_Overpost::str_AcceptMode_FirstFit = "FirstFit";
const char* WT_Overpost::str_True = "True";
const char* WT_Overpost::str_False = "False";


///////////////////////////////////////////////////////////////////////////
WT_Overpost::WT_Overpost()
    : m_eAcceptMode( WT_Overpost::AcceptAll)
    , m_renderEntities(true)
    , m_addExtents(true)

{
}
 
///////////////////////////////////////////////////////////////////////////
WT_Overpost::WT_Overpost( WT_AcceptMode  acceptMode,
                          WT_Boolean     renderEntities,
                          WT_Boolean     addExtents)
    : m_eAcceptMode(acceptMode)
    , m_renderEntities(renderEntities)
    , m_addExtents(addExtents)
{
}

///////////////////////////////////////////////////////////////////////////
//providing implementations for the copy constructor and 
// assignment operator

WT_Overpost::WT_Overpost(const WT_Overpost& op)
	: m_eAcceptMode( op.m_eAcceptMode )
	, m_renderEntities(op.m_renderEntities)
	, m_addExtents(op.m_addExtents)

{
}
 

///////////////////////////////////////////////////////////////////////////
void WT_Overpost::operator =(const WT_Overpost& op)
{
	m_eAcceptMode = op.m_eAcceptMode;
	m_renderEntities = op.m_renderEntities;
	m_addExtents = op.m_addExtents;
}


///////////////////////////////////////////////////////////////////////////
WT_Overpost::~WT_Overpost()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Overpost::WT_AcceptMode   WT_Overpost::acceptMode() const
{
    return this->m_eAcceptMode;
}


///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Overpost::renderEntities() const
{
    return this->m_renderEntities;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Overpost::addExtents() const
{
    return this->m_addExtents;
}

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Overpost::add( const WT_Object& object)
{
   return this->addObject(&object);
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_Type  WT_Overpost::object_type() const 
{
    return WT_Object::Definition;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID    WT_Overpost::object_id() const
{
    return WT_Object::Overpost_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Overpost::materialize(WT_Opcode const & opcode, WT_File & file)
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;
    switch (opcode.type())
    {
        
        case WT_Opcode::Extended_ASCII:
            {
                 char* pchar = WD_Null;
                // read in the class variables
                WD_CHECK(file.eat_whitespace());    // move to the enum
                WT_AcceptMode eValue;
                WD_CHECK(file.read(pchar,256));       // read the enum string 
                if (string_to_enum(pchar,eValue))    // convert string to enum    
                {
                    this->m_eAcceptMode = eValue;
                }
                // pchar is allocated as a char[]
                if (pchar)
                {
                    delete[] pchar;      
                    pchar = WD_Null;
                }
                WD_CHECK(file.eat_whitespace());   
                WT_Boolean bValue;
                WD_CHECK(file.read(pchar,256));       // read RenderEntities flag
                if(string_to_boolean(pchar,bValue))
                {
                    this->m_renderEntities = bValue;
                }
                // pchar is allocated as a char[]
                if (pchar)
                {
                    delete[] pchar;
                    pchar = WD_Null;
                }

                WD_CHECK(file.read(pchar,256));       // read addExtents flag
                if(string_to_boolean(pchar,bValue))
                {
                    this->m_addExtents = bValue;
                }
                
                // pchar is allocated as a char[]
                if (pchar)
                {
                    delete[] pchar;
                    pchar = WD_Null;
                }

                WD_CHECK (file.eat_whitespace()); // move to the start of its children
                
                // materialize the children with the Process flag set to true.
                WD_CHECK (materialize_stream(opcode, file)); 
                WD_CHECK (file.read(close)); // closing paran
                
                result = (close != ')')? WT_Result::Corrupt_File_Error:WT_Result::Success;
                
            }
            break;
      /*  case WT_Opcode::Extended_Binary:
            {
                WT_Byte flags; 
                WD_CHECK(file.read(flags));
                WD_CHECK(binary_materialize_var(flags));
                WD_CHECK(materialize_stream(opcode, file));
                WD_CHECK(file.read(close));

                result = (close != '}')? WT_Result::Corrupt_File_Error:WT_Result::Success;
            }
            break;*/
        default:
            result = WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch
    
    m_materialized = (result == WT_Result::Success);
    return result;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Overpost::process(WT_File &)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Overpost::serialize(WT_File & file) const 
{
    // Overpost will not serialize out if the file revision is incorrect
    if (file.heuristics().target_version() < REVISION_WHEN_TEXT_ALIGNMENT_IS_SUPPORTED)
        return WT_Result::Toolkit_Usage_Error;

    // sync Desired rendition so that any rendition already set will
    // be serialized. 
    // 0x7FFFFFFF is a nicer substitution since we need all the bits 
    // However the following is more human readable 
    WT_Integer32 parts_to_sync = WT_Rendition::BlockRef_Bit         |
                                 WT_Rendition::Code_Page_Bit        |
                                 WT_Rendition::Color_Bit            |
                                 WT_Rendition::Color_Map_Bit        |
                                 WT_Rendition::Dash_Pattern_Bit     |
                                 WT_Rendition::Fill_Bit             |
                                 WT_Rendition::Fill_Pattern_Bit     |
                                 WT_Rendition::Font_Bit             |
                                 WT_Rendition::Font_Extension_Bit   |
                                 WT_Rendition::Layer_Bit            |
                                 WT_Rendition::Line_Pattern_Bit     |
                                 WT_Rendition::Line_Style_Bit       |
                                 WT_Rendition::Line_Weight_Bit      |
                                 WT_Rendition::Macro_Scale_Bit      |
                                 WT_Rendition::Macro_Index_Bit      |
                                 WT_Rendition::Merge_Control_Bit    |
                                 WT_Rendition::Object_Node_Bit      |
                                 WT_Rendition::Pen_Pattern_Bit      |
                                 WT_Rendition::Projection_Bit       |
                                 WT_Rendition::URL_Bit              |
                                 WT_Rendition::Viewport_Bit         |
                                 WT_Rendition::Visibility_Bit       |
                                 WT_Rendition::Text_Background_Bit  |
                                 WT_Rendition::Text_HAlign_Bit      |
                                 WT_Rendition::Text_VAlign_Bit      |
                                 WT_Rendition::Delineate_Bit        |
                                 WT_Rendition::User_Fill_Pattern_Bit  |
                                 WT_Rendition::User_Hatch_Pattern_Bit |
                                 WT_Rendition::Contrast_Color_Bit;
                                     
    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));
    
    WT_Boolean allowsMerging = file.heuristics().allow_drawable_merging();
    // if merging or delaying is allowed stop it and dump the drawables
    if( allowsMerging )
    {
        file.heuristics().set_allow_drawable_merging(false);
        file.dump_delayed_drawable();
    }

   // Note : Extended Binary output is not supported as it writes out an incorrect offset 
   //         which doesnt take into account the size of its children.
   //if (file.heuristics().allow_binary_data()) 
   //{
   //    WD_CHECK (file.write((WT_Byte)'{')); // extended binary 

   //    // add the 4 byte packer before the opcode
   //    WD_CHECK (file.write((WT_Integer32)( 
   //         sizeof(WT_Unsigned_Integer16) + // for the opcode
   //         sizeof(WT_Byte) +               // for the flags 
   //         sizeof(WT_Byte))));             // The closing "}"

   //    // write out the opcode
   //    WD_CHECK (file.write((WT_Integer16)WD_EXBO_OVERPOST)); // Write out Macro opcode 
   //    WD_CHECK (file.write(binary_serialize_var()));       // Write out the index 
   //    WD_CHECK (this->serialize_stream(file));
   //    WD_CHECK (file.write((WT_Byte)'}')); // extended binary closer
   //}
   //else
   //{
    const char* pchRenderEntities = m_renderEntities? str_True : str_False;
    const char* pchAddExtents     = m_addExtents? str_True : str_False;
    // Extended ASCII only output
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(Overpost "));        // Overpost opcode  + space 
    WD_CHECK (file.write(enum_to_string(m_eAcceptMode))); // Write out the enum 
    WD_CHECK (file.write((WT_Byte) ' '));       // Space
    WD_CHECK (file.write(pchRenderEntities));    // Write out the Boolean  
    WD_CHECK (file.write((WT_Byte) ' '));       // Space
    WD_CHECK (file.write(pchAddExtents));        // Write out the Boolean
    WD_CHECK (file.write((WT_Byte) ' '));       // Space
    WD_CHECK (this->serialize_stream(file));     // serialize the contents 
    WD_CHECK (file.write((WT_Byte)')'));         // write the closing brackets

   //restore the previous setting 
   file.heuristics().set_allow_drawable_merging(allowsMerging);
    
   return WT_Result::Success; 
}
#else
WT_Result WT_Overpost::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif // DESIRED_OUTPUT
    
///////////////////////////////////////////////////////////////////////////
WT_Result WT_Overpost::default_process( WT_Overpost& ,WT_File&)
{
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Overpost::on_materialize( WT_Object& obj, WT_File& file )
{
    // process all objects that are materialized 
    WD_CHECK (obj.process(file));
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
const char* WT_Overpost::enum_to_string(WT_AcceptMode acceptMode)
{
    const char* pchar = WT_Overpost::str_AcceptMode_All;
    switch (acceptMode)
    {
    case  AcceptAllFit:
        pchar = WT_Overpost::str_AcceptMode_All;
        break;
    case AcceptFirstFit:
        pchar = WT_Overpost::str_AcceptMode_FirstFit;
	case AcceptAll:
		break;
    }

    return pchar;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean  WT_Overpost::string_to_enum(const char* pStr, 
                                        WT_AcceptMode& acceptMode)
{
    WT_Boolean return_val = true;
    if(strcmp(pStr,WT_Overpost::str_AcceptMode_All) == 0)
    {
        acceptMode = WT_Overpost::AcceptAll;
    }
    else if(strcmp(pStr,WT_Overpost::str_AcceptMode_AllFit) == 0)
    {
        acceptMode = WT_Overpost::AcceptAllFit;
    }
    else if(strcmp(pStr,WT_Overpost::str_AcceptMode_FirstFit) == 0)
    {
        acceptMode = WT_Overpost::AcceptFirstFit;
    }
    else 
    {
        return_val = false;
    }
    return return_val;
}


///////////////////////////////////////////////////////////////////////////
WT_Byte WT_Overpost::binary_serialize_var() const 
{
    // convert the 
    WT_Byte outval = (WT_Byte)( this->m_eAcceptMode);
            outval |= ((WT_Byte)this->m_renderEntities) << 2; 
            outval |= ((WT_Byte)this->m_addExtents) << 3;

    return outval;
    

}
    
///////////////////////////////////////////////////////////////////////////
WT_Result WT_Overpost::binary_materialize_var(WT_Byte byte)
{
    this->m_eAcceptMode = WT_AcceptMode(byte & (WT_Byte)3);
    this->m_renderEntities =  (byte & (WT_Byte)4) == 4;
    this->m_addExtents =  (byte & (WT_Byte)8 ) == 8 ;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Overpost::string_to_boolean( const char* pStr, WT_Boolean& outBool)
{
    WT_Boolean retVal = true;
    if(!strcmp(pStr,str_True))
    {
        outBool = true;
    }
    else if ( !strcmp(pStr,str_False))
    {
        outBool = false;
    }
    else
    {
        retVal = false;
    }
    return retVal;
}

// End of file
