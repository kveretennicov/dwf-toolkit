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
WT_Macro_Definition::WT_Macro_Definition(WT_Unsigned_Integer16 index,
                                         WT_Integer32 scale_units )
                                         : m_index( index )
                                         , m_scale_units( scale_units )
                                         , m_bFill(WD_False)
{
}

///////////////////////////////////////////////////////////////////////////
WT_Macro_Definition::~WT_Macro_Definition()
{
}

///////////////////////////////////////////////////////////////////////////
WT_Logical_Box WT_Macro_Definition::bounds() 
{
    // iterate though the collection and constrct a WT_Logical_Box 
    this->reset();
    const WT_Object* pObj = NULL;
    WT_Logical_Box extents;
    WT_File file;
    while( this->get_next() )
    {
        pObj = NULL;
        pObj = this->object();
        if( pObj != NULL && 
            pObj->object_type() == WT_Object::Drawable )
        {
            WT_Drawable* pDrawable = (WT_Drawable*) pObj;
            extents += pDrawable->bounds(&file);
        }
    }
    
    return extents;
}
///////////////////////////////////////////////////////////////////////////
// Implementation of WT_Object Interface
WT_Object::WT_Type WT_Macro_Definition::object_type() const 
{
    return WT_Object::Definition;
}

///////////////////////////////////////////////////////////////////////////
    /// Returns the WT_ID for this object.
WT_Object::WT_ID WT_Macro_Definition::object_id() const
{
    return WT_Object::Macro_Definition_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Definition::materialize(WT_Opcode const & opcode, 
                                           WT_File & file )
{
    WT_Result result = WT_Result::Unsupported_DWF_Opcode;
    WT_Byte close;

    // Macro_Definition automatically shuts off Fill when it is being materialized
    // The fill state is restored 
    WT_Boolean restoreFill = (file.rendition().fill().fill());
    if (restoreFill)
    {
        file.rendition().fill().set(WD_False);
    }
    

    switch (opcode.type())
    {
        
        case WT_Opcode::Extended_ASCII:
            {
                // read in the class variables
                WD_CHECK (file.eat_whitespace()); // move to the index
                WD_CHECK (file.read_ascii(m_index));
                WD_CHECK (file.eat_whitespace()); // move to the scaleunits
                WD_CHECK (file.read_ascii(m_scale_units));
                WD_CHECK (file.eat_whitespace()); // move to the start of its children
                WD_CHECK (materialize_stream( opcode, file )); // materialize the children
                WD_CHECK (file.read(close)); // closing paran
                
                result = (close != ')')? WT_Result::Corrupt_File_Error:WT_Result::Success;
                
                // Revert any changes made to the file rendition fill attribute
                // NOTE: THIS IS DANGEROUS AND WRONG.  Fill should never be used in this fashion
                // and it was a mistake to ever do so.  The entire reason it was misused in this
                // way was so that the Delineate opcode didn't have to be fully implemented.
                // TODO: remove special fill processing, implement Delineate properly.
                if (m_bFill)
                {
                    file.rendition().fill() = m_cached_fill;
                }
            }
            break;
        /*case WT_Opcode::Extended_Binary:
            {
                
                WD_CHECK (file.read(m_index));
                WD_CHECK (file.read(m_scale_units));
                WD_CHECK (materialize_stream(opcode, file));
                WD_CHECK (file.read(close));

                result = (close != '}')? WT_Result::Corrupt_File_Error:WT_Result::Success;
            }
            break;*/
        default:
            result = WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch
    
    if (restoreFill)
    {
        file.rendition().fill().set(WD_True);
    }

    m_materialized = (result == WT_Result::Success);

    return result;
}

///////////////////////////////////////////////////////////////////////////
/// Calls the configured action handler for this object (passes in the file reference.)
/** \warning This is used by the framework and should not be called by client code.
*/
WT_Result WT_Macro_Definition::process(WT_File &) 
{
    // todo
    
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Definition::add(const WT_Drawable & drawable)
{
    WD_CHECK (this->addObject(&drawable));
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Definition::add(const WT_Attribute & attribute)
{
    WD_CHECK (this->addObject(&attribute));
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Macro_Definition::serialize(WT_File & file) const 
{
    // Macro_Defintion will not serialize out if the file revision is incorrect
    if (file.heuristics().target_version() < REVISION_WHEN_MACRO_IS_SUPPORTED)
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
                                 WT_Rendition::User_Fill_Pattern_Bit    |
                                 WT_Rendition::User_Hatch_Pattern_Bit   |
                                 WT_Rendition::Contrast_Color_Bit;
                                 

    // Macro_Definition always has its Fill set to false.
    file.desired_rendition().fill() = WT_Fill(WD_False);

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));
    
    WT_Boolean allowsMerging = file.heuristics().allow_drawable_merging();

    // if merging or delaying is allowed stop it and dump the drawables
    if( allowsMerging )
    {
        file.heuristics().set_allow_drawable_merging(false);
        file.dump_delayed_drawable();
    }

   // Note : Macro Definition does not support Binary output correctly as 
   //        the Integer32 size is incorrect (does not take into account the size of 
   //        its children
   //if (file.heuristics().allow_binary_data()) 
   //{
   //    WD_CHECK (file.write((WT_Byte)'{')); // extended binary 

   //    // add the 4 byte packer before the opcode
   //    WD_CHECK (file.write((WT_Integer32)( 
   //         sizeof(WT_Unsigned_Integer16) + // for the opcode
   //         sizeof(WT_Integer16) +          // index
   //         sizeof(WT_Integer32) +          // scale units
   //         sizeof(WT_Byte))));             // The closing "}"

   //    // write out the opcode
   //    WD_CHECK (file.write((WT_Integer16)WD_EXBO_MACRO_DEFINITION)); // Write out Macro opcode 
   //    WD_CHECK (file.write(m_index));       // Write out the index 
   //    WD_CHECK (file.write(m_scale_units)); // Write out the scale Units 
   //    WD_CHECK (this->serialize_stream(file));
   //    WD_CHECK (file.write((WT_Byte)'}')); // extended binary closer
   // }
   
    // Extended ASCII only output
    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(Macro "));       // Macro + space 
    WD_CHECK (file.write_ascii(m_index));       // Write out the index 
    WD_CHECK (file.write((WT_Byte) ' '));       // Space
    WD_CHECK (file.write_ascii(m_scale_units)); // Write out the Scale Units 
    WD_CHECK (file.write((WT_Byte) ' '));       // Space
    WD_CHECK (this->serialize_stream(file));    // serialize the contents 
    WD_CHECK (file.write((WT_Byte)')'));        // write the closing brackets

   //restore the previous setting 
   file.heuristics().set_allow_drawable_merging(allowsMerging);
   return WT_Result::Success; 
}
#else
WT_Result WT_Macro_Definition::serialize(WT_File &) const 
{
    return WT_Result::Success;
}
#endif // DESIRED_OUTPUT

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Definition::on_materialize( WT_Object& obj, 
                                               WT_File& file )
{
    // special case handling for Fill attribute 
    // we want fill to be added to the rendition till the end of the processing 
    if (obj.object_id() == WT_Object::Fill_ID)
    {
        // NOTE: THIS IS DANGEROUS AND WRONG.  Fill should never be used in this fashion
        // and it was a mistake to ever do so.  The entire reason it was misused in this
        // way was so that the Delineate opcode didn't have to be fully implemented.
        // TODO: remove special fill processing, implement Delineate properly.
        if(!m_bFill)
        {
            m_cached_fill = file.rendition().fill();
            m_bFill = WD_True;
        }
        obj.process(file);
    }
    // special case handling for WT_Origin.
    else if (obj.object_id() == WT_Object::Origin_ID)
    {
        obj.process(file);
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Macro_Definition::default_process( WT_Macro_Definition& , /**< The attribute to process. */
                                                WT_File& ) /**< The file being read. */
{
    return WT_Result::Success;
}
// End of file.
