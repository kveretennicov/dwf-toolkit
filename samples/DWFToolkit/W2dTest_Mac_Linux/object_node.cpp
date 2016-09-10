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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/object_node.cpp#1 $

#ifdef _DWFCORE_WIN32_SYSTEM
#include <io.h>
#endif

WT_Result read_object_node(WT_Object_Node & /*node*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_object_node(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    int x=3000, y=-3050;

    WT_Logical_Point    min, max;
    min.m_x = x-100;
    min.m_y = y-100;
    max.m_x = x+400;
    max.m_y = y+775;
    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Object Nodes");
    WD_CHECK(named_view->serialize(whip_file));
    classFactory->Destroy( named_view );

    //test WT_Object_Node usage around some graphics.
    WT_Logical_Point pt[5];

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;

    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_String str("Object Nodes");
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y+650), str);
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy( text );

    //DOOR 1
    pt[0] = WT_Logical_Point(x,y);
    pt[1] = WT_Logical_Point(x,y+600);
    pt[2] = WT_Logical_Point(x+300,y+600);
    pt[3] = WT_Logical_Point(x+300,y);
    pt[4] = WT_Logical_Point(x,y); //close it

    //Door frame 1
	WT_Object_Node *objectNode_1_door1rect = classFactory->Create_Object_Node(whip_file, 1, "door1rect");
    whip_file.desired_rendition().object_node() = *objectNode_1_door1rect;
    classFactory->Destroy( objectNode_1_door1rect );
    whip_file.desired_rendition().color().set(102,51,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    WT_Polyline *line = classFactory->Create_Polyline(5, pt, WD_True);
    WD_CHECK(line->serialize(whip_file));
    classFactory->Destroy( line );

    //Doorknob 1
	WT_Object_Node *objectNode_2_door1knob = classFactory->Create_Object_Node(whip_file, 2, "door1knob");
	whip_file.desired_rendition().object_node() = *objectNode_2_door1knob;
    classFactory->Destroy( objectNode_2_door1knob );
    whip_file.desired_rendition().color().set(255,204,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    WT_Outline_Ellipse *ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+265, y+250), 20, 20);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );
    whip_file.desired_rendition().color().set(255,204,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
	ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+265, y+250), 10, 10);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );

    //Screw object 1
	WT_Object_Node *objectNode_3_screw1 = classFactory->Create_Object_Node(whip_file, 3, "screw1");
    whip_file.desired_rendition().object_node() = *objectNode_3_screw1;
    classFactory->Destroy( objectNode_3_screw1 );
    whip_file.desired_rendition().color().set(255,0,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+280, y+250), 2, 2);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );
    pt[0] = WT_Logical_Point(x+278, y+250);
    pt[1] = WT_Logical_Point(x+282, y+250);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );
    pt[0] = WT_Logical_Point(x+280, y+248);
    pt[1] = WT_Logical_Point(x+280, y+252);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

    //Screw object 2
	WT_Object_Node *objectNode_4_screw2 = classFactory->Create_Object_Node(whip_file, 4, "screw2");
    whip_file.desired_rendition().object_node() = *objectNode_4_screw2;
    classFactory->Destroy( objectNode_4_screw2 );
    whip_file.desired_rendition().color().set(255,0,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+250, y+250), 2, 2);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );

    pt[0] = WT_Logical_Point(x+248, y+250);
    pt[1] = WT_Logical_Point(x+252, y+250);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

    pt[0] = WT_Logical_Point(x+250, y+248);
    pt[1] = WT_Logical_Point(x+250, y+252);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

    //DOOR 2
    x+= 350;
    pt[0] = WT_Logical_Point(x,y);
    pt[1] = WT_Logical_Point(x,y+600);
    pt[2] = WT_Logical_Point(x+300,y+600);
    pt[3] = WT_Logical_Point(x+300,y);
    pt[4] = WT_Logical_Point(x,y); //close it

    //Door frame 1
	WT_Object_Node *objectNode_5_door2rect = classFactory->Create_Object_Node(whip_file, 5, "door2rect");
    whip_file.desired_rendition().object_node() = *objectNode_5_door2rect;
    classFactory->Destroy( objectNode_5_door2rect );
    whip_file.desired_rendition().color().set(102,51,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    line = classFactory->Create_Polyline(5, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

    //Doorknob 1
	WT_Object_Node *objectNode_6_door2knob = classFactory->Create_Object_Node(whip_file, 6, "door2knob");
    whip_file.desired_rendition().object_node() = *objectNode_6_door2knob;
    classFactory->Destroy( objectNode_6_door2knob );
    whip_file.desired_rendition().color().set(255,204,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+265, y+250), 20, 20);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );
    whip_file.desired_rendition().color().set(255,204,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+265, y+250), 10, 10);
    WD_CHECK( ellipse->serialize(whip_file) ); 
    classFactory->Destroy( ellipse );

    //Screw object 1
	WT_Object_Node *objectNode_7_screw3 = classFactory->Create_Object_Node(whip_file, 7, "screw3");
    whip_file.desired_rendition().object_node() = *objectNode_7_screw3;
    classFactory->Destroy( objectNode_7_screw3 );
    whip_file.desired_rendition().color().set(255,0,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+280, y+250), 2, 2);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );
    pt[0] = WT_Logical_Point(x+278, y+250);
    pt[1] = WT_Logical_Point(x+282, y+250);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );
    pt[0] = WT_Logical_Point(x+280, y+248);
    pt[1] = WT_Logical_Point(x+280, y+252);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

    //Screw object 2
	WT_Object_Node *objectNode_8_screw4 = classFactory->Create_Object_Node(whip_file, 8, "screw4");
    whip_file.desired_rendition().object_node() = *objectNode_8_screw4;
    classFactory->Destroy( objectNode_8_screw4 );
    whip_file.desired_rendition().color().set(255,0,0, WT_Color::Search_666_Color_Cube_Exact_Then_Color_Map_Nearest);
    ellipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point(x+250, y+250), 2, 2);
    WD_CHECK( ellipse->serialize(whip_file) );
    classFactory->Destroy( ellipse );
    pt[0] = WT_Logical_Point(x+248, y+250);
    pt[1] = WT_Logical_Point(x+252, y+250);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );
    pt[0] = WT_Logical_Point(x+250, y+248);
    pt[1] = WT_Logical_Point(x+250, y+252);
    line = classFactory->Create_Polyline(2, pt, WD_True);
    WD_CHECK( line->serialize(whip_file) );
    classFactory->Destroy( line );

	WT_Object_Node *objectNode_0 = classFactory->Create_Object_Node(whip_file, 0);
    whip_file.desired_rendition().object_node() = *objectNode_0;
    classFactory->Destroy( objectNode_0 );

    return WT_Result::Success;
}

