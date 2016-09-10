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
// $Header: /NewRoot/DWF Toolkit/v6/develop/global/src/dwf/whiptk/plotinfo.cpp 1     9/12/04 8:55p Evansg $
#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Plot_Info::serialize( WT_File& file ) const
{
    WD_Assert(m_ll.m_x >= 0);
    WD_Assert(m_ll.m_y >= 0);
    WD_Assert(m_width  >= m_ur.m_x);
    WD_Assert(m_height >= m_ur.m_y);

    file.desired_rendition().blockref();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::BlockRef_Bit));

    WD_CHECK( file.write_tab_level() );
    WD_CHECK( file.write("(PlotInfo ") );

    if (m_show)
        WD_CHECK( file.write("show ") );
    else
        WD_CHECK( file.write("hide ") );

    if (file.heuristics().target_version() >= REVISION_WHEN_LANDSCAPE_FLIP_ADDED_TO_PLOTINFO ||
        file.heuristics().target_version() <= WHIP31_DWF_FILE_VERSION)
    {
        WD_CHECK( file.write_ascii(file.heuristics().transform().rotation()) );
        WD_CHECK( file.write((WT_Byte)' '));
    }

    switch( m_units )
    {
    case Millimeters:
        WD_CHECK( file.write("mm ") );
        break;
    case Inches:
        WD_CHECK( file.write("in ") );
        break;
    default:
        return WT_Result::Internal_Error;
    }

    WD_CHECK( file.write_ascii(m_width) );
    WD_CHECK( file.write((WT_Byte)' ') );
    WD_CHECK( file.write_ascii(m_height) );
    WD_CHECK( file.write((WT_Byte)' ') );

    WD_CHECK( file.write_ascii(m_ll.m_x) );
    WD_CHECK( file.write((WT_Byte)' ') );
    WD_CHECK( file.write_ascii(m_ll.m_y) );
    WD_CHECK( file.write((WT_Byte)' ') );

    WD_CHECK( file.write_ascii(m_ur.m_x) );
    WD_CHECK( file.write((WT_Byte)' ') );
    WD_CHECK( file.write_ascii(m_ur.m_y) );
    WD_CHECK( file.write((WT_Byte)' ') );

    WT_Matrix2D_IO oMatrix( m_dwf2paper );
    WD_CHECK( oMatrix.serialize(file) );
    return file.write((WT_Byte)')');
}
#else
WT_Result WT_Plot_Info::serialize( WT_File&) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Plot_Info::materialize( const WT_Opcode& opcode, WT_File& file )
{
    switch( m_stage )
    {
    case Eating_Initial_Whitespace:
        WD_CHECK( file.eat_whitespace() );
        m_stage = Getting_Show_Boolean;
        // No Break Here

    case Getting_Show_Boolean:
        {
            WD_CHECK( m_temp_string.materialize(file) );

            if( m_temp_string.equals_no_case("show") )
                m_show = WD_True;
            else
                m_show = WD_False;
        }

        // If we are reading a file made during part of the beta period, it wont have the
        // rotation information in it so we'll skip this section.
        if (file.rendition().drawing_info().decimal_revision() > WHIP31_DWF_FILE_VERSION &&
            file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_LANDSCAPE_FLIP_ADDED_TO_PLOTINFO)
        {
            m_stage = Getting_Units_String;
            goto Getting_Units_String_Hop;
        }

        m_stage = Getting_Landscape_To_Portrait_Flip;
        // No Break Here

    case Getting_Landscape_To_Portrait_Flip:
        {
            // Note that during the beta period, there were two versions of the
            // plot rotation parameter: originally there was just a boolean of "normal" or "flipped".
            // Later a "rotation angle" value was added instead.

            if (file.rendition().drawing_info().decimal_revision() < REVISION_WHEN_ROTATION_ADDED_TO_PLOTINFO)
            {
                WD_CHECK( m_temp_string.materialize(file) );

                if( m_temp_string.equals_no_case("flipped"))
                    m_rotation = 90;
                else
                    m_rotation = 0;
            }
            else
            {
                WD_CHECK( file.read_ascii(m_rotation) );
                m_rotation = ((long)((m_rotation / 90.0) + 0.5)) * 90; // Only allow 90 degree rotations in this release
            }
        }
        m_stage = Getting_Units_String;
        // No Break Here

Getting_Units_String_Hop:
    case Getting_Units_String:
        {
            WD_CHECK( m_temp_string.materialize(file) );

            if( m_temp_string.equals_no_case("mm"))
                m_units = Millimeters;
            else
                m_units = Inches;
        }
        m_stage = Getting_Paper_Width;
        // No Break Here

    case Getting_Paper_Width:
        WD_CHECK( file.read_ascii(m_width) );
        m_stage = Getting_Paper_Height;
        // No Break Here

    case Getting_Paper_Height:
        WD_CHECK( file.read_ascii(m_height) );
        m_stage = Getting_Plot_Area_Left;
        // No Break Here

    case Getting_Plot_Area_Left:
        WD_CHECK( file.read_ascii(m_ll.m_x) );
        m_stage = Getting_Plot_Area_Bottom;
        // No Break Here

    case Getting_Plot_Area_Bottom:
        WD_CHECK( file.read_ascii(m_ll.m_y) );
        m_stage = Getting_Plot_Area_Right;
        // No Break Here

    case Getting_Plot_Area_Right:
        WD_CHECK( file.read_ascii(m_ur.m_x) );
        m_stage = Getting_Plot_Area_Top;
        // No Break Here

    case Getting_Plot_Area_Top:
        WD_CHECK( file.read_ascii(m_ur.m_y) );
        m_stage = Getting_To_Paper_Transform;
        // No Break Here

    case Getting_To_Paper_Transform:
        {
            WT_Matrix2D_IO oMatrix;
            WD_CHECK( oMatrix.materialize(file) );
            m_dwf2paper = oMatrix;
            m_stage = Eating_End_Whitespace;
        }
        // No Break Here

    case Eating_End_Whitespace:
        WD_CHECK( opcode.skip_past_matching_paren(file) );
        m_stage = Eating_Initial_Whitespace;
        break;

    default:
        return WT_Result::Internal_Error;
    }
    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Plot_Info::default_process(
    WT_Plot_Info& item,
    WT_File& file )
{
    file.rendition().plot_info() = item;

    // Note, the following is only a bug if the paper units are not in inches
    if (file.heuristics().broken_plotinfo() && item.paper_units() != Inches)
    {
        //AutoCAD 2000 and 2000i eplots have 2 problems in this area:
        //
        //1. they always output the paper width and height in inches
        //2. they mess up the the printable bounds badly by subtracting
        //millimeters from inches. Let's fix that here:
        //
        //plot_info.lower_left() is fine that was in millimeters and wasn't calculated
        //
        //plot_info.upper_right() is messed up:

        // Given bad paper width BPW and height BPH and bad upper right x and y BURX BURY
        // and good lower left x and y LLX LLY and bad scale BDTP (from the to_paper()
        // matrix, element 0,0), compute the good scale factor (logical units per paper unit) dpixels:
        // bdxpw = BPW - BURX - LLX
        // bdyph = BPH - BURY - LLY
        // bgs = max( bdxpw, bdyph ) * 1.1
        // BDTP = bgs / dpixels
        // dpixels = bgs / BDTP ;
        WT_Matrix2D & mtx = (WT_Matrix2D &)item.to_paper();
        double bdxpw = item.upper_right().m_x - item.lower_left().m_x;
        double bdyph = item.upper_right().m_y - item.lower_left().m_y;
        double bgs = (( bdxpw > bdyph ) ? bdxpw : bdyph ) * 1.1 ;
        // The point of all this is to get dpixels, which is the number of logical units per millimeter
        double dpixels = bgs / mtx(0,0) ;

        //get right margin in mm
        double right_margin_mm = item.paper_width()-item.upper_right().m_x;

        //convert width from inches to millimeter
        item.paper_width()*=25.4;

        //calculate plot_info.upper_right().m_x again now with both the width and margin in mm
        item.upper_right().m_x = item.paper_width()-right_margin_mm;

        //get top margin in mm
        double top_margin_mm = item.paper_height()-item.upper_right().m_y;
        //convert height from inches to millimeter
        item.paper_height()*=25.4;
        //calculate plot_info.upper_right().m_y again now with both the height and margin in mm
        item.upper_right().m_y = item.paper_height()-top_margin_mm;

        // Finally, compute the scale from scratch, using our dpixels value:
        // dxpw = PW - URX - LLX
        // dyph = PH - URY - LLY
        // gs = max( dxpw, dyph ) * 1.1
        // DTP = gs / dpixels
        double dxpw = item.upper_right().m_x - item.lower_left().m_x;
        double dyph = item.upper_right().m_y - item.lower_left().m_y;
        double gs = (( dxpw > dyph ) ? dxpw : dyph ) * 1.1 ;

        // use this correct scale
        double scale = gs / dpixels ;
        mtx(0,0) = scale;
        mtx(1,1) = scale;

        item.to_paper() = mtx ;
    }

    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Plot_Info::process( WT_File& file )
{
    WD_Assert( file.plot_info_action() );
    return (file.plot_info_action())(*this,file);
}
