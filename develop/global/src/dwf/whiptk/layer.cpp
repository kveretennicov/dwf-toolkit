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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/layer.cpp 1     9/12/04 8:54p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Layer::object_id() const
{
    return WT_Object::Layer_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Layer::WT_Layer (WT_Layer const & layer)
  : WT_Item()
  , WT_Attribute()
  , m_stage (Gathering_Layer_Number)
{
    set (layer);
}

///////////////////////////////////////////////////////////////////////////
WT_Layer::WT_Layer(WT_File &file,WT_Integer32 layer_num, char const * layer_name)
    : m_layer_num (layer_num)
    , m_visibility (true)
    , m_stage (Gathering_Layer_Number)
{
    set(layer_name,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Layer::WT_Layer(WT_File &file,WT_Integer32 layer_num, WT_Unsigned_Integer16 const * layer_name)
    : m_layer_num (layer_num)
    , m_visibility (true)
    , m_stage (Gathering_Layer_Number)
{
    set(layer_name,file);
}

///////////////////////////////////////////////////////////////////////////
WT_Layer const & WT_Layer::operator=(WT_Layer const & layer)
{
    set(layer);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
void WT_Layer::set(char const * layer_name, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_layer_name = layer_name ? (WT_Byte*)layer_name : WT_String::kNull;
}

///////////////////////////////////////////////////////////////////////////
void WT_Layer::set(WT_Unsigned_Integer16 const * layer_name, WT_File & file)
{
    m_incarnation = file.next_incarnation();
    m_layer_name.set(WT_String::wcslen(layer_name),layer_name);
}

///////////////////////////////////////////////////////////////////////////
void WT_Layer::set(WT_Layer const & layer)
{
    m_incarnation = layer.m_incarnation;
    m_layer_num = layer.m_layer_num;
    m_visibility = layer.m_visibility;
    m_layer_name = layer.m_layer_name;
}


///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Layer::serialize(WT_File & file) const
{

    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    //WT_Boolean prior_visibility = file.rendition().visibility().visible();
    //file.desired_rendition().visibility() = m_visibility;
    //WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::Visibility_Bit));
    //file.desired_rendition().visibility() = prior_visibility;

    // Have we seen this layer before?
    if (!file.layer_list().find_layer_from_index (m_layer_num))
    {
        // We've never seen this layer before.
        // So define it and add it to the list of defined layers.
        WD_CHECK (file.write_tab_level());
        WD_CHECK (file.write("(Layer "));
        WD_CHECK (file.write_ascii( m_layer_num ));
        WD_CHECK (file.write((WT_Byte) ' '));
        WD_CHECK (m_layer_name.serialize(file));
        WD_CHECK (file.write((WT_Byte) ')'));
        file.layer_list().add_layer(*this);
    }
    else
    {
        // We have seen this layer before.

        if (file.heuristics().allow_binary_data())
        {
             WD_CHECK (file.write((WT_Byte)0xAC));
             WD_CHECK (file.write_count(m_layer_num));
        }
        else
        {
            WD_CHECK (file.write_tab_level());
            WD_CHECK (file.write("(Layer "));
            WD_CHECK (file.write_ascii( m_layer_num ));
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write((WT_Byte) ')'));
        }
    }
    //WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::Visibility_Bit));

    return WT_Result::Success;
}
#else
WT_Result WT_Layer::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Layer::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() != Layer_ID)
        return WD_False;

    // Layer ID matches
    if (m_layer_num != ((WT_Layer const &)attrib).m_layer_num)
        return WD_False;

    // Layer ID and Name match
    if (m_layer_name != ((WT_Layer const &)attrib).m_layer_name)
        return WD_False;

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Layer::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().layer())
    {
        file.rendition().layer() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Layer::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Layer::materialize(WT_Opcode const & opcode, WT_File & file)
{
    m_incarnation = file.next_incarnation();

    switch (opcode.type())
    {
        case WT_Opcode::Single_Byte:
            {
                WD_CHECK(file.read_count(m_layer_num));
                // We've finished with this opcode!
            } break;
        case WT_Opcode::Extended_ASCII:
            {
                switch (m_stage)
                {
                case Gathering_Layer_Number:
                    WD_CHECK (file.read_ascii(m_layer_num));
                    m_stage = Gathering_Layer_Name;

                    // No break
                case Gathering_Layer_Name:
                    WD_CHECK (m_layer_name.materialize(file));
                    m_stage = Eating_End_Whitespace;

                    // No Break Here
                case Eating_End_Whitespace:
                    WD_CHECK (opcode.skip_past_matching_paren(file));
                    // We finished, so get ready to start again...
                    m_stage = Gathering_Layer_Number;
                    break;

                default:
                    return WT_Result::Internal_Error;
                }
            } break; //end of ascii
        default:
            {
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            } break;
    }

    //m_visibility = file.rendition().visibility().visible();

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Layer::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
        case WT_Opcode::Single_Byte:
            {
                WT_Integer32 tmp;
                WD_CHECK(file.read_count(tmp));
            } break;
        case WT_Opcode::Extended_ASCII:
            {
                return opcode.skip_past_matching_paren(file);
            } break;
        default:
            {
                return WT_Result::Opcode_Not_Valid_For_This_Object;
            } break;
    }
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Layer::process(WT_File & file)
{
    WD_Assert (file.layer_action());
    return (file.layer_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Layer::default_process(WT_Layer & item, WT_File & file)
{
    file.rendition().layer() = item;
    file.layer_list().add_layer(item);
    return WT_Result::Success;
}
