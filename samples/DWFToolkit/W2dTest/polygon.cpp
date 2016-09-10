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
// $Header: /Components/Internal/DWF Toolkit/v7/samples/W2dTest/polygon.cpp 1     12/16/04 4:28p Klugj $

void init_vector (int, int, int, WT_Logical_Point *);
void init_vector2 (int, int, int, WT_Logical_Point *);

WT_Result read_polygon (WT_Polygon & /*polygon*/, WT_File & /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_polygon (WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 1, WT_String("Polygons"));
    whip_file.desired_rendition().layer() = *layer;
    classFactory->Destroy( layer );

    WT_Logical_Point    min, max;

    int x = 50;
    int y = -2200;

    min.m_x = x;
    min.m_y = y-1100;
    max.m_x = x+700;
    max.m_y = y+400;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Polygons - Faces and Edges");
    WD_CHECK(named_view->serialize(whip_file));
    classFactory->Destroy( named_view );

    const int delta = 300;
    const int major = 110;
    const int minor = 75;

    // faces (filled)
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);

    if (true)
    {
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y+(delta+25)), WT_String("Faces"));
        WD_CHECK(caption->serialize(whip_file));
        classFactory->Destroy( caption );

        whip_file.desired_rendition().fill() = WD_True;
        whip_file.desired_rendition().line_weight() = 0;

        WT_Logical_Point vP[12];
        init_vector (x, y, delta, vP);

        whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());
        WT_Polygon *pg = classFactory->Create_Polygon(12, vP, WD_True);
        WD_CHECK(pg->serialize (whip_file));
        classFactory->Destroy( pg );

        whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());

        int yc = y + delta / 2;

        WT_Filled_Ellipse *e1 = classFactory->Create_Filled_Ellipse(x+600, yc, major, minor);
        WD_CHECK(e1->serialize (whip_file));
        classFactory->Destroy( e1 );

        whip_file.desired_rendition().color().set(3, whip_file.desired_rendition().color_map());

        WT_Filled_Ellipse *e2 = classFactory->Create_Filled_Ellipse(x+900, yc, minor, major);
        WD_CHECK(e2->serialize (whip_file));
        classFactory->Destroy( e2 );
    }

    // edges (outline)

    y -= 3 * delta / 2;

    if (true)
    {
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y+(delta+25)), WT_String("Edges"));
        WD_CHECK(caption->serialize(whip_file));
        classFactory->Destroy( caption );

        whip_file.desired_rendition().fill() = WD_False;
        whip_file.desired_rendition().line_weight() = 5;

        WT_Logical_Point vP[12];
        init_vector (x, y, delta, vP);

        whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());

        WT_Polyline *pg = classFactory->Create_Polyline(12, vP, WD_True); //polygons are always filled, edges should use polylines!
        WD_CHECK(pg->serialize (whip_file));
        classFactory->Destroy( pg );

        whip_file.desired_rendition().color().set(5, whip_file.desired_rendition().color_map());

        int yc = y + delta / 2;

        WT_Outline_Ellipse *e1 = classFactory->Create_Outline_Ellipse(x+600, yc, major, minor);
        WD_CHECK(e1->serialize (whip_file));
        classFactory->Destroy( e1 );

        whip_file.desired_rendition().color().set(6, whip_file.desired_rendition().color_map());

        WT_Outline_Ellipse *e2 = classFactory->Create_Outline_Ellipse(x+900, yc, minor, major);
        WD_CHECK(e2->serialize (whip_file));
		classFactory->Destroy( e2 );
    }

    // edges and faces

    y -= 3 * delta / 2;

    if (true)
    {
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y+(delta+25)), WT_String("Faces and Edges"));
        WD_CHECK(caption->serialize(whip_file));
        classFactory->Destroy( caption );

        WT_Logical_Point vP[12];
        init_vector (x, y, delta, vP);

        whip_file.desired_rendition().fill() = WD_True;
        whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());

        WT_Polygon *pg1 = classFactory->Create_Polygon(12, vP, WD_True); 
        WD_CHECK(pg1->serialize (whip_file));
        classFactory->Destroy( pg1 );

        whip_file.desired_rendition().fill() = WD_False;
        whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());
        whip_file.desired_rendition().line_weight() = 5;

        WT_Polyline *pg2 = classFactory->Create_Polyline(12, vP, WD_True); //polygons are always filled, edges should use polylines!
        WD_CHECK(pg2->serialize (whip_file));
        classFactory->Destroy( pg2 );

        int yc = y + delta / 2;

        whip_file.desired_rendition().line_weight() = 0;
        whip_file.desired_rendition().color().set(3, whip_file.desired_rendition().color_map());
        WT_Filled_Ellipse *e1 = classFactory->Create_Filled_Ellipse(x+600, yc, major, minor);
        WD_CHECK(e1->serialize (whip_file));
        classFactory->Destroy( e1 );

        whip_file.desired_rendition().line_weight() = 5;
        whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
        WT_Outline_Ellipse *e2 = classFactory->Create_Outline_Ellipse(x+600, yc, major, minor);
        WD_CHECK(e2->serialize (whip_file));
        classFactory->Destroy( e2 );

        whip_file.desired_rendition().line_weight() = 0;
        whip_file.desired_rendition().color().set(5, whip_file.desired_rendition().color_map());
        WT_Filled_Ellipse *e3 = classFactory->Create_Filled_Ellipse(x+900, yc, minor, major);
        WD_CHECK(e3->serialize (whip_file));
        classFactory->Destroy( e3 );

        whip_file.desired_rendition().line_weight() = 5;
        whip_file.desired_rendition().color().set(6, whip_file.desired_rendition().color_map());
        WT_Outline_Ellipse *e4 = classFactory->Create_Outline_Ellipse(x+900, yc, minor, major);
        WD_CHECK(e4->serialize (whip_file));
        classFactory->Destroy( e4 );
    }

    //fill patterns
    if (true)
    {
        WT_Logical_Point    min, max;

        min.m_x = -2800;
        min.m_y = -100;
        max.m_x = -1000;
        max.m_y = 1100;

        WT_Logical_Box       box(min, max);
        WT_Named_View *named_view = classFactory->Create_Named_View(box, "Polygons - Fills");
        WD_CHECK(named_view->serialize(whip_file));
        classFactory->Destroy( named_view );

        WT_Logical_Point vP[16];
        const int delta =  200;
        int y = 900;
        int x = -2300;

        for(int index = WT_Fill_Pattern::Solid; 
            (WT_Fill_Pattern::WT_Pattern_ID)index < WT_Fill_Pattern::User_Defined;
            index++)
        {
            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            char caption_buf[256];

            switch (index)
            {
            case WT_Fill_Pattern::Solid:
                strcpy(caption_buf, "Solid Fill"); break;
            case WT_Fill_Pattern::Checkerboard:
                strcpy(caption_buf, "Checkerboard Fill"); break;
            case WT_Fill_Pattern::Crosshatch:
                strcpy(caption_buf, "Crosshatch Fill"); break;
            case WT_Fill_Pattern::Diamonds:
                strcpy(caption_buf, "Diamonds Fill"); break;
            case WT_Fill_Pattern::Horizontal_Bars:
                strcpy(caption_buf, "Horizontal_Bars Fill"); break;
            case WT_Fill_Pattern::Slant_Left:
                strcpy(caption_buf, "Slant_Left Fill"); break;
            case WT_Fill_Pattern::Slant_Right:
                strcpy(caption_buf, "Slant_Right Fill"); break;
            case WT_Fill_Pattern::Square_Dots:
                strcpy(caption_buf, "Square_Dots Fill"); break;
            case WT_Fill_Pattern::Vertical_Bars:
                strcpy(caption_buf, "Vertical_Bars Fill"); break;
            }

            WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y+(delta+25)), WT_String(caption_buf));
            WD_CHECK(caption->serialize(whip_file));
            classFactory->Destroy( caption );

            init_vector2 (x, y, delta, vP);

            whip_file.desired_rendition().fill_pattern() = (WT_Fill_Pattern::WT_Pattern_ID) index;
            whip_file.desired_rendition().fill_pattern().pattern_scale() = .75;

            whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());
            WT_Polygon *pg1 = classFactory->Create_Polygon(9, vP, WD_True);
            WD_CHECK(pg1->serialize (whip_file));
            classFactory->Destroy( pg1 );

            whip_file.desired_rendition().fill_pattern().pattern_scale() = 1.0;
            whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());
            WT_Polygon *pg2 = classFactory->Create_Polygon(4, &vP[9], WD_True);
            WD_CHECK(pg2->serialize (whip_file));
            classFactory->Destroy( pg2 );

            whip_file.desired_rendition().fill_pattern().pattern_scale() = 0.5;
            whip_file.desired_rendition().color().set(3, whip_file.desired_rendition().color_map());
            WT_Polygon *pg3 = classFactory->Create_Polygon(3, &vP[13], WD_True);
            WD_CHECK(pg3->serialize (whip_file));
            classFactory->Destroy( pg3 );

            y -= 3 * delta / 2;
        }    
    }

    return WT_Result::Success;
}

void init_vector (int x, int y, int delta, WT_Logical_Point * vP)
{
    vP[0].m_x = x;
    vP[0].m_y = y;
    vP[1].m_x = x + delta / 5;
    vP[1].m_y = y + delta;
    vP[2].m_x = x + 4 * delta / 5;
    vP[2].m_y = vP[1].m_y;
    vP[3].m_x = x + delta;
    vP[3].m_y = y;
    vP[4].m_x = x + 55 * delta / 100;
    vP[4].m_y = y;
    vP[5].m_x = x + 3 * delta / 5;
    vP[5].m_y = y + delta / 5;
    vP[6].m_x = x + 4 * delta / 5;
    vP[6].m_y = vP[5].m_y;
    vP[7].m_x = x + delta / 2;
    vP[7].m_y = y + 9 * delta / 10;
    vP[8].m_x = x + delta / 5;
    vP[8].m_y = vP[5].m_y;
    vP[9].m_x = x + 2 * delta / 5;
    vP[9].m_y = vP[5].m_y;
    vP[10].m_x = x + 45 * delta / 100;
    vP[10].m_y = y;
    vP[11].m_x = x;
    vP[11].m_y = y;
}

void init_vector2 (int x, int y, int delta, WT_Logical_Point * vP)
{
    vP[0].m_x = x;
    vP[0].m_y = y + delta / 2;
    vP[1].m_x = x + 2 * delta / 5;
    vP[1].m_y = y + 3 * delta / 5;
    vP[2].m_x = x + delta / 2;
    vP[2].m_y = y + delta;
    vP[3].m_x = x + 3 * delta / 5;
    vP[3].m_y = vP[1].m_y;
    vP[4].m_x = x + delta;
    vP[4].m_y = vP[0].m_y;
    vP[5].m_x = vP[3].m_x;
    vP[5].m_y = y + 2 * delta / 5;
    vP[6].m_x = vP[2].m_x;
    vP[6].m_y = y;
    vP[7].m_x = vP[1].m_x;
    vP[7].m_y = vP[5].m_y;
    vP[8].m_x = x;
    vP[8].m_y = y + delta / 2;

    x += 3 * delta / 2;

    vP[9].m_x = x;
    vP[9].m_y = y;
    vP[10].m_x = x;
    vP[10].m_y = y + delta;
    vP[11].m_x = x + delta;
    vP[11].m_y = vP[10].m_y;
    vP[12].m_x = vP[11].m_x;
    vP[12].m_y = y;

    x += 3 * delta / 2;

    vP[13].m_x = x;
    vP[13].m_y = y;
    vP[14].m_x = x + delta / 2;
    vP[14].m_y = y + 4 * delta / 5;
    vP[15].m_x = x + delta;
    vP[15].m_y = y;
}