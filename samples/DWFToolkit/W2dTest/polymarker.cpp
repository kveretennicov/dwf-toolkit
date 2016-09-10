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
// $Header: /Components/Internal/DWF Toolkit/v7.1/samples/DWFToolkit/W2dTest/polymarker.cpp 4     3/24/05 11:12p Bangiav $


WT_Result read_polymarker(WT_Polymarker & /*polymarker*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_polymarker(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 9, WT_String("Polymarker"));
    whip_file.desired_rendition().layer() = *layer;
    classFactory->Destroy( layer );

    WT_Logical_Point    min, max;

    int x = -2300;
    int y = -1700;

    min.m_x = x;
    min.m_y = y-350;
    max.m_x = x+200;
    max.m_y = y+50;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Polymarker");
    WD_CHECK(named_view->serialize(whip_file));
    classFactory->Destroy( named_view );

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern().set( WT_Line_Pattern::Solid );
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text* text = classFactory->Create_Text( WT_Logical_Point(x, y), WT_String("Polymarker"));
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy(text);
    whip_file.desired_rendition().font().height().set(15);

    WT_Logical_Point vP[5];
    for(int i=0;i<5;i++)
    {
        vP[i].m_x = x+(i*25);
        vP[i].m_y = y-50;
        WT_Polymarker* pMarker = classFactory->Create_Polymarker(1, &vP[i], WD_True);
        WD_CHECK(pMarker->serialize(whip_file));
        classFactory->Destroy(pMarker);
    }

    text = classFactory->Create_Text(WT_Logical_Point(x+150, y-50), WT_String("Pixel"));
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy(text);

    return WT_Result::Success;
}
