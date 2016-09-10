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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/gouraud_polytriangle.cpp#1 $

WT_Result read_gouraud_polytriangle(WT_Gouraud_Polytriangle & /*polytriangle*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_gouraud_polytriangle (WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 3, WT_String("Polytriangles"));
    whip_file.desired_rendition().layer() = *layer;

    WT_Logical_Point    min, max;

    int x = 1500;
    int y = 962;

    min.m_x = x-200;
    min.m_y = y-500;
    max.m_x = x+1200;
    max.m_y = y;

    WT_Logical_Box        box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Polytriangles");

    WD_CHECK(named_view->serialize(whip_file));


    const int delta = 1000 / 7;
    WT_Logical_Point  vP[8];

    for (int i = 0; i < 8; i += 2) {
        vP[i].m_x = x + (2*delta*(i/2));
        vP[i].m_y = y;

        vP[i + 1].m_x = vP[i].m_x + delta;
        vP[i + 1].m_y = y + delta;
    }

    whip_file.desired_rendition().fill() = WD_True;
    whip_file.desired_rendition().fill_pattern() = WT_Fill_Pattern::Solid;

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y+delta+20), WT_String("Standard polytriangle"));
    WD_CHECK(caption->serialize(whip_file));

    // constant color
    whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());
    WT_Polytriangle *pt = classFactory->Create_Polytriangle(8, vP, WD_True);
    WD_CHECK(pt->serialize (whip_file));

    // Gouraud shaded polytriangles
    if (whip_file.heuristics().target_version() < REVISION_WHEN_PACKAGE_FORMAT_BEGINS)
        return WT_Result::Success;

    WT_RGBA32    rgba[9];

    rgba[0] = WT_RGBA32 (0, 0, 0, 255);
    rgba[1] = WT_RGBA32 (255, 255, 0, 255);
    rgba[2] = WT_RGBA32 (255, 0, 0, 255);
    rgba[3] = WT_RGBA32 (0, 255, 0, 255);
    rgba[4] = WT_RGBA32 (0, 255, 255, 255);
    rgba[5] = WT_RGBA32 (0, 0, 255, 255);
    rgba[6] = WT_RGBA32 (255, 0, 255, 255);
    rgba[7] = WT_RGBA32 (255, 255, 255, 255);
    rgba[8] = WT_RGBA32 (0, 255, 0, 255);

    y -= 2 * delta;

    for (int i = 0; i < 8; i += 2) {
        vP[i].m_y = y;
        vP[i + 1].m_y = y + delta;
    }

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    classFactory->Destroy(caption);
	caption = classFactory->Create_Text(WT_Logical_Point(x, y+delta+20), WT_String("Gouraud shaded polytriangle"));
    WD_CHECK(caption->serialize(whip_file));

    WT_Gouraud_Polytriangle *gpt = classFactory->Create_Gouraud_Polytriangle(8, vP, rgba, WD_True);
    WD_CHECK(gpt->serialize (whip_file));

	classFactory->Destroy(gpt);
	classFactory->Destroy(pt);
	classFactory->Destroy(caption);
	classFactory->Destroy(layer);
	classFactory->Destroy(named_view);

    return WT_Result::Success;
}
