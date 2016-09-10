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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/object_node.cpp 1     9/12/04 8:55p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Object_Node::object_id() const
{
    return Object_Node_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node::WT_Object_Node (WT_Object_Node const & object_node)
  : WT_Item()
  , WT_Attribute()
    , m_previous_object_node_num(-1)
  , m_stage (Gathering_Object_Node_Number)
{
    set (object_node);
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node::WT_Object_Node(WT_File &file, WT_Integer32 object_node_num, char const * object_node_name)
    : m_previous_object_node_num(-1)
    , m_stage (Gathering_Object_Node_Number)
{
    set(file, object_node_num, object_node_name);
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node::WT_Object_Node(WT_File &file, WT_Integer32 object_node_num, WT_Unsigned_Integer16 const * object_node_name)
    : m_previous_object_node_num(-1)
    , m_stage (Gathering_Object_Node_Number)
{
    set(file,object_node_num,object_node_name);
}

///////////////////////////////////////////////////////////////////////////
WT_Object_Node const & WT_Object_Node::operator=(WT_Object_Node const & object_node)
{
    set(object_node);
    return *this;
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Node::set(WT_File & file, WT_Integer32 object_node_num, char const * object_node_name)
{
    m_incarnation = file.next_incarnation();
    m_object_node_num = object_node_num;
    m_object_node_name = object_node_name ? (WT_Byte*)object_node_name : WT_String::kNull;
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Node::set(WT_File & file, WT_Integer32 object_node_num, WT_Unsigned_Integer16 const * object_node_name)
{
    m_incarnation = file.next_incarnation();
    m_object_node_num = object_node_num;
    m_object_node_name.set(WT_String::wcslen(object_node_name),object_node_name);
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Node::set(WT_Object_Node const & object_node)
{
    m_incarnation = object_node.m_incarnation;
    m_object_node_num = object_node.m_object_node_num;
    m_object_node_name = object_node.m_object_node_name;
}

///////////////////////////////////////////////////////////////////////////
void WT_Object_Node::set(WT_File &file, WT_Integer32 object_node_num)
{
    WT_Object_Node *node = file.object_node_list().find_object_node_from_index( object_node_num );
    if (node != NULL)
    {
        set( *node );
    }
    else
    {
        m_incarnation = file.next_incarnation();
        m_object_node_num = object_node_num;
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Object_Node::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    if (file.heuristics().target_version() < REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Success;

    // Have we seen this object_node before?
    if (!file.object_node_list().find_object_node_from_index (m_object_node_num))
    {
        // We've never seen this object_node before.
        // So define it and add it to the list of defined object_nodes.
        if (m_object_node_name.length()!=0)
        {
            WD_CHECK (file.write_tab_level());
            WD_CHECK (file.write("(Node "));
            WD_CHECK (file.write_ascii( m_object_node_num ));
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (m_object_node_name.serialize(file));
            WD_CHECK (file.write((WT_Byte) ')'));
            file.object_node_list().add_object_node(*this);
        }
        else //no name
        {
            if (file.heuristics().allow_binary_data())
            {
                //Determine which of the three single byte optimizations we should use.
                //Use the following algorithm:
                //
                //if the delta/offset from the previous value is 1, we can use
                //    0x0E, which indicates an auto-increment of 1  (previous + 1 = this)
                //else if the delta/offset from the previous value is <= +/- 32767, then can use
                //    0x6E, with a 16-bit signed offset value (previous + offset = this)
                //else we have to use
                //    0x4E, with a 32-bit absolute value

                WT_Integer32 delta = m_object_node_num - m_previous_object_node_num;
                if (delta == 1 && m_previous_object_node_num>=0)
                {
                    WD_CHECK (file.write((WT_Byte)0x0E));
                }
                else if (abs(delta) < 32768 && m_previous_object_node_num>=0)
                {
                    WD_CHECK (file.write((WT_Byte)0x6E));
                    WD_CHECK (file.write((WT_Integer16)delta));
                }
                else
                {
                    WD_CHECK (file.write((WT_Byte)0x4E));
                    WD_CHECK (file.write(m_object_node_num));
                }
            }
            else
            {
                WD_CHECK (file.write_tab_level());
                WD_CHECK (file.write("(Node "));
                WD_CHECK (file.write_ascii( m_object_node_num ));
                WD_CHECK (file.write((WT_Byte) ')'));
            }
        }
    }
    else
    {
        // We have seen this object_node before.
        if (file.heuristics().allow_binary_data())
        {
            //Determine which of the three single byte optimizations we should use.
            //Use the following algorithm:
            //
            //if the delta/offset from the previous value is 1, we can use
            //    0x0E, which indicates an auto-increment of 1  (previous + 1 = this)
            //else if the delta/offset from the previous value is <= +/- 32767, then can use
            //    0x6E, with a 16-bit signed offset value (previous + offset = this)
            //else we have to use
            //    0x4E, with a 32-bit absolute value

            WT_Integer32 delta = m_object_node_num - m_previous_object_node_num;
            if (delta == 1 && m_previous_object_node_num>=0)
            {
                WD_CHECK (file.write((WT_Byte)0x0E));
            }
            else if (abs(delta) < 32768 && m_previous_object_node_num>=0)
            {
                WD_CHECK (file.write((WT_Byte)0x6E));
                WD_CHECK (file.write((WT_Integer16)delta));
            }
            else
            {
                WD_CHECK (file.write((WT_Byte)0x4E));
                WD_CHECK (file.write(m_object_node_num));
            }
        }
        else
        {
            WD_CHECK (file.write_tab_level());
            WD_CHECK (file.write("(Node "));
            WD_CHECK (file.write_ascii( m_object_node_num ));
            WD_CHECK (file.write((WT_Byte) ')'));
        }
    }
    ((WT_Object_Node*)this)->m_previous_object_node_num = m_object_node_num;

    return WT_Result::Success;
}
#else
WT_Result WT_Object_Node::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Object_Node::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() != Object_Node_ID)
        return WD_False;

    // Object_Node ID matches
    if (m_incarnation != ((WT_Object_Node const &)attrib).m_incarnation)
        return WD_False;

    // Object_Node ID matches, but Incarnation differs
    if (m_object_node_num != ((WT_Object_Node const &)attrib).m_object_node_num)
        return WD_False;

    // Object_Node ID and Object_Node Number match
    if (m_object_node_name != ((WT_Object_Node const &)attrib).m_object_node_name)
        return WD_False;

    return WD_True;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Object_Node::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().object_node())
    {
        file.rendition().object_node() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Object_Node::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Node::materialize(WT_Opcode const & opcode, WT_File & file)
{
    switch (opcode.type())
    {
        case WT_Opcode::Single_Byte:
            {
                switch (opcode.token()[0])
                {
                    case WD_SBBO_OBJECT_NODE_32:
                    {
                        file.read(m_object_node_num);
                        break;
                    }
                    case WD_SBBO_OBJECT_NODE_16:
                    {
                        WT_Integer16 value;
                        file.read(value);
                        m_object_node_num = file.rendition().object_node().object_node_num() + value;
                        break;
                    }
                    case WD_SBBO_OBJECT_NODE_AUTO:
                        m_object_node_num = file.rendition().object_node().object_node_num() + 1;
                        break;
                    default:
                        return WT_Result::Opcode_Not_Valid_For_This_Object;
                }
            } break;
        case WT_Opcode::Extended_ASCII:
            {
                switch (m_stage)
                {
                case Gathering_Object_Node_Number:
                    WD_CHECK (file.read_ascii(m_object_node_num));
                    m_stage = Gathering_Object_Node_Name;

                    // No break
                case Gathering_Object_Node_Name:
                    WD_CHECK (m_object_node_name.materialize(file));
                    m_stage = Eating_End_Whitespace;

                    // No Break Here
                case Eating_End_Whitespace:
                    WD_CHECK (opcode.skip_past_matching_paren(file));
                    // We finished, so get ready to start again...
                    m_stage = Gathering_Object_Node_Number;
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

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Node::skip_operand(WT_Opcode const & opcode, WT_File & file)
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
WT_Result WT_Object_Node::process(WT_File & file)
{
    WD_Assert (file.object_node_action());
    return (file.object_node_action())(*this, file);
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Object_Node::default_process(WT_Object_Node & item, WT_File & file)
{
    file.rendition().object_node() = item;
    file.object_node_list().add_object_node(item);
    return WT_Result::Success;
}
