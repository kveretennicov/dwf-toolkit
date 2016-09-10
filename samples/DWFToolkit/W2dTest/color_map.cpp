#include "stdafx.h"
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
// $Header: /Components/Internal/DWF Toolkit/v7/samples/W2dTest/color_map.cpp 1     12/16/04 4:24p Klugj $

WT_Result read_color_map(WT_Color_Map & /*userdata*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_color_map(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 10, WT_String("Color Map"));
    whip_file.desired_rendition().layer() = *layer;

    WT_Logical_Point    min, max;

    int x = 1500;
    int y = -2700;

    min.m_x = x;
    min.m_y = y-200;
    max.m_y = y;
    max.m_x = x+500;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Color Map");
    WD_CHECK(named_view->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;

    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y-50), WT_String("Simple color map"));
    WD_CHECK(text->serialize(whip_file));

    WT_RGB rgb[3];
    rgb[0] = WT_RGB(255,0,0); //red
    rgb[1] = WT_RGB(0,255,0); //green
    rgb[2] = WT_RGB(0,0,255); //blue

    WT_Color_Map *old_color_map = classFactory->Create_Color_Map( whip_file.desired_rendition().color_map() );
    WT_Color_Map *color_map = classFactory->Create_Color_Map(3, rgb, whip_file);
    
    whip_file.desired_rendition().color_map() = *color_map;

    whip_file.desired_rendition().font().height().set(50);
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x, y-100), WT_String("Red"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x+150, y-100), WT_String("Green"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());
    classFactory->Destroy(text);
	text = classFactory->Create_Text(WT_Logical_Point(x+350, y-100), WT_String("Blue"));
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().color_map() = *old_color_map;

	classFactory->Destroy(color_map);
	classFactory->Destroy(old_color_map);
	classFactory->Destroy(text);
	classFactory->Destroy(named_view);
	classFactory->Destroy(layer);

    return WT_Result::Success;
}

