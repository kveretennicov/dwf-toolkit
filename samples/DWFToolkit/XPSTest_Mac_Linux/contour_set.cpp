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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/XPSTest_Mac_Linux/contour_set.cpp#1 $

void init_vector(int x, int y, WT_Logical_Point *vP, int delta, bool winding);
void init_vector2(int x, int y, WT_Logical_Point *vP, int delta, bool winding);


WT_Result read_contour_set(WT_Contour_Set & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_contour_set(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    whip_file.desired_rendition().visibility() = WD_True;

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 11, WT_String("Contour Set"));
    whip_file.desired_rendition().layer() = *layer;
    whip_file.desired_rendition().layer().set_visibility(WD_False);

    WT_Logical_Point    min, max;

    int x = -2300;
    int y = -2500;

    min.m_x = x-50;
    min.m_y = y-500;
    max.m_x = x+1200;
    max.m_y = y+50;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Contour Sets");
    WD_CHECK(named_view->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y), WT_String("Contour Sets"));
    WD_CHECK(text->serialize(whip_file));

    WT_Logical_Point vP[45];
    
    /// A diamond.
    int tempX = x;
    int tempY = y;
    x+=450;
    y-=600;

    WT_Logical_Point diamonds[12]; 
    diamonds[0] = WT_Logical_Point(x, y+200); //positive space (winds clockwise)
    diamonds[1] = WT_Logical_Point(x+200, y); 
    diamonds[2] = WT_Logical_Point(x, y-200);
    diamonds[3] = WT_Logical_Point(x-200, y);
    diamonds[4] = WT_Logical_Point(x, y+150); //negative space (winds counterclockwise)
    diamonds[5] = WT_Logical_Point(x-150, y);
    diamonds[6] = WT_Logical_Point(x, y-150);
    diamonds[7] = WT_Logical_Point(x+150, y); 
    diamonds[8] = WT_Logical_Point(x, y+100); //positive space (winds clockwise)
    diamonds[9] = WT_Logical_Point(x+100, y); 
    diamonds[10] = WT_Logical_Point(x, y-100);
    diamonds[11] = WT_Logical_Point(x-100, y);
    WT_Integer32 contour_counts[3] = {4,4,4};
    WT_Contour_Set *diamond_contour = classFactory->Create_Contour_Set(whip_file, 3, contour_counts, 12, diamonds, WD_True);
    WD_CHECK(diamond_contour->serialize(whip_file));

    x=tempX;
    y=tempY;

    int delta = 100;
    init_vector(x+200, y-200, vP, delta, true); //positive space 
    init_vector(x+200, y-150, &vP[9], delta/6, false); //negative space
    init_vector2(x+200, y-250, &vP[18], delta/6, false); //negative space
    init_vector(x+200, y-150, &vP[27], delta/12, true);
    init_vector2(x+200, y-250, &vP[36], delta/12, true);
    WT_Integer32 counts[5] = {9,9,9,9,9};

    whip_file.desired_rendition().fill() = WD_False;
    whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
    WT_Contour_Set *contours = classFactory->Create_Contour_Set(whip_file, 5, counts, 45, vP, WD_True);
    WD_CHECK(contours->serialize(whip_file));

    init_vector2(x+700, y-200, vP, delta, true); //positive space 
    init_vector(x+700, y-150, &vP[9], delta/6, false); //negative space
    init_vector2(x+700, y-250, &vP[18], delta/6, false); //negative space
    init_vector(x+700, y-150, &vP[27], delta/12, true); //positive space 
    init_vector2(x+700, y-250, &vP[36], delta/12, true); //positive space 

    whip_file.desired_rendition().color().set(3, whip_file.desired_rendition().color_map());
    classFactory->Destroy(contours);
	contours = classFactory->Create_Contour_Set(whip_file, 5, counts, 45, vP, WD_True);
    WD_CHECK(contours->serialize(whip_file));

	classFactory->Destroy(contours);
	classFactory->Destroy(diamond_contour);
	classFactory->Destroy(text);
	classFactory->Destroy(named_view);
	classFactory->Destroy(layer);

    return WT_Result::Success;
}


void init_vector(int x, int y, WT_Logical_Point *vP, int delta, bool winding)
{
    int i = winding ? 0 : 8;
    vP[i].m_x = x-2*delta;
    vP[i].m_y = y;
    i += winding ? 1 : -1;
    vP[i].m_x = x-delta/2;
    vP[i].m_y = y+delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x;
    vP[i].m_y = y+2*delta;
    i += winding ? 1 : -1;
    vP[i].m_x = x+delta/2;
    vP[i].m_y = y+delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x+2*delta;
    vP[i].m_y = y;
    i += winding ? 1 : -1;
    vP[i].m_x = x+delta/2;
    vP[i].m_y = y-delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x;
    vP[i].m_y = y-2*delta;
    i += winding ? 1 : -1;
    vP[i].m_x = x-delta/2;
    vP[i].m_y = y-delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = vP[winding?0:8].m_x;
    vP[i].m_y = vP[winding?0:8].m_y;
}

void init_vector2(int x, int y, WT_Logical_Point *vP, int delta, bool winding)
{
    int i = winding ? 0 : 8;
    vP[i].m_x = x-2*delta;
    vP[i].m_y = y;
    i += winding ? 1 : -1;
    vP[i].m_x = x-3*delta/2;
    vP[i].m_y = y+3*delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x;
    vP[i].m_y = y+2*delta;
    i += winding ? 1 : -1;
    vP[i].m_x = x+3*delta/2;
    vP[i].m_y = y+3*delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x+2*delta;
    vP[i].m_y = y;
    i += winding ? 1 : -1;
    vP[i].m_x = x+3*delta/2;
    vP[i].m_y = y-3*delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = x;
    vP[i].m_y = y-2*delta;
    i += winding ? 1 : -1;
    vP[i].m_x = x-3*delta/2;
    vP[i].m_y = y-3*delta/2;
    i += winding ? 1 : -1;
    vP[i].m_x = vP[winding?0:8].m_x;
    vP[i].m_y = vP[winding?0:8].m_y;
}