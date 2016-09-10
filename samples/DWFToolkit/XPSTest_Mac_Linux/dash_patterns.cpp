#include "StdAfx.h"
#include "TestFramework.h"

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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/XPSTest_Mac_Linux/dash_patterns.cpp#1 $

WT_Result read_dash_pattern(WT_Dash_Pattern & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_dash_pattern(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 8, "Dash Patterns");
    whip_file.desired_rendition().layer() = *layer;

    WT_Line_Style *default_line_style = classFactory->Create_Line_Style();
	*default_line_style = whip_file.desired_rendition().line_style();

    WT_Logical_Point    min, max;

    int x = -2300;
    int y = -2200;

    min.m_x = x;
    min.m_y = y-350;
    max.m_x = x+200;
    max.m_y = y+50;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Dash Pattern");
    WD_CHECK(named_view->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y), WT_String("User-defined Dash Pattern"));
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().font().height().set(15);
    
    WT_Integer16 vN[12];

	int i;
    for (i=0; i<6; i+=2) //dot dot dot
    {   vN[i] = 1;
        vN[i+1] = 15; }
    for (i=6; i<12; i+=2) //dash dash dash
    {   vN[i] = 15;
        vN[i+1] = 15; }

    WT_Dash_Pattern *previous_pattern = classFactory->Create_Dash_Pattern(); 
	*previous_pattern = whip_file.desired_rendition().dash_pattern();

    WT_Dash_Pattern *pattern = classFactory->Create_Dash_Pattern(
        WT_Line_Pattern::Count+100, //ID
        12, 
        vN);

    whip_file.desired_rendition().dash_pattern() = *pattern;

    WT_Logical_Point vP[2];
    vP[0].m_x = x;
    vP[0].m_y = vP[1].m_y = y-50;
    vP[1].m_x = x+500;

    whip_file.desired_rendition().line_style().adapt_patterns() = WD_False;
    whip_file.desired_rendition().line_style().pattern_scale() = 0.0;
    WT_Polyline *pline = classFactory->Create_Polyline(2, vP, WD_False);
    WD_CHECK(pline->serialize(whip_file));

    vP[0].m_y = vP[1].m_y = y-75;
    whip_file.desired_rendition().line_style().adapt_patterns() = WD_True;
    classFactory->Destroy(pline);
	pline = classFactory->Create_Polyline(2, vP, WD_False);
    WD_CHECK(pline->serialize(whip_file));

    vP[0].m_y = vP[1].m_y = y-100;
    whip_file.desired_rendition().line_style().pattern_scale() = 50.0;
    classFactory->Destroy(pline);
	pline = classFactory->Create_Polyline(2, vP, WD_False);
    WD_CHECK(pline->serialize(whip_file));

    whip_file.desired_rendition().line_style() = *default_line_style;

    whip_file.desired_rendition().dash_pattern() = *previous_pattern;

	classFactory->Destroy(pline);
	classFactory->Destroy(pattern);
	classFactory->Destroy(previous_pattern);
	classFactory->Destroy(text);
	classFactory->Destroy(named_view);
	classFactory->Destroy(default_line_style);
	classFactory->Destroy(layer);

    return WT_Result::Success;
}

