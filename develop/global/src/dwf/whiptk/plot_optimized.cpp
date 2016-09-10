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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/plot_optimized.cpp 1     9/12/04 8:55p Evansg $

#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Plot_Optimized::object_id() const
{
    return WT_Object::Optimized_For_Plot_ID;
}

///////////////////////////////////////////////////////////////////////////
void WT_Plot_Optimized::set_plot_optimized(WT_Boolean const & plot_optimized)
{
    m_plot_optimized = plot_optimized;
}

///////////////////////////////////////////////////////////////////////////
WT_Boolean WT_Plot_Optimized::get_plot_optimized() const
{
    return m_plot_optimized;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Plot_Optimized::serialize(WT_File & file) const
{
    WD_CHECK (file.dump_delayed_drawable());

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK (file.write_tab_level());
    WD_CHECK (file.write("(PlotOptimized "));
    WD_CHECK (file.write_ascii((WT_Integer16)((abs)(m_plot_optimized))));
    WD_CHECK (file.write(")"));
    return WT_Result::Success;
}
#else
WT_Result WT_Plot_Optimized::serialize(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Boolean    WT_Plot_Optimized::operator== (WT_Attribute const & attrib) const
{
    if (attrib.object_id() == Optimized_For_Plot_ID &&
        m_plot_optimized == ((WT_Plot_Optimized const &)attrib).m_plot_optimized)
    {
        return WD_True;
    }
    return WD_False;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Plot_Optimized::sync(WT_File & file) const
{
    WD_Assert( (file.file_mode() == WT_File::File_Write) ||
               (file.file_mode() == WT_File::Block_Append) ||
               (file.file_mode() == WT_File::Block_Write));

    if (*this != file.rendition().rendering_options().plot_optimized() )
    {
      file.rendition().rendering_options().plot_optimized() = *this;
        return serialize(file);
    }
    return WT_Result::Success;
}
#else
WT_Result WT_Plot_Optimized::sync(WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Plot_Optimized::materialize(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)

    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        {
            WT_Integer32 iTmp;
            WD_CHECK (file.read_ascii(iTmp));
            if( iTmp != 0 )
                m_plot_optimized = WD_True;
            else
                m_plot_optimized = WD_False;
            WD_CHECK (opcode.skip_past_matching_paren(file));
        } break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        {
            return WT_Result::Opcode_Not_Valid_For_This_Object;
        } break;
    } // switch

    m_materialized = WD_True;
    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}


WT_Result WT_Plot_Optimized::skip_operand(WT_Opcode const & opcode, WT_File & file)
{
#if DESIRED_CODE(WHIP_INPUT)
    switch (opcode.type())
    {
    case WT_Opcode::Extended_ASCII:
        WD_CHECK (opcode.skip_past_matching_paren(file));
        break;
    case WT_Opcode::Extended_Binary:
    case WT_Opcode::Single_Byte:
    default:
        return WT_Result::Opcode_Not_Valid_For_This_Object;
    } // switch

    return WT_Result::Success;

#else
    return WT_Result::Success;
#endif  // DESIRED_CODE()
}

WT_Result WT_Plot_Optimized::process(WT_File & file)
{
    WD_Assert(file.plot_optimized_action());
    return (file.plot_optimized_action())(*this, file);
}

WT_Result WT_Plot_Optimized::default_process(WT_Plot_Optimized & item, WT_File & file)
{
    file.rendition().rendering_options().plot_optimized() = item;
    return WT_Result::Success;
}
