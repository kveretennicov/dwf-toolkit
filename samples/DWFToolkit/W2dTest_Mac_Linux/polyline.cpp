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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/polyline.cpp#1 $

WT_Result read_polyline(WT_Polyline & /*polyline*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_polyline(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 2, WT_String("Polylines"));
    whip_file.desired_rendition().layer() = *layer;
	classFactory->Destroy(layer);

	WT_Line_Style *default_line_style = classFactory->Create_Line_Style( whip_file.desired_rendition().line_style() );

    char caption_buf[256];
    WT_Logical_Point    vP[4];

    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().line_style().line_join() =        WT_Line_Style::Miter_Join;
    whip_file.desired_rendition().line_style().line_start_cap() =   WT_Line_Style::Butt_Cap;
    whip_file.desired_rendition().line_style().line_end_cap() =     WT_Line_Style::Butt_Cap;
    whip_file.desired_rendition().line_style().dash_start_cap() =   WT_Line_Style::Butt_Cap;
    whip_file.desired_rendition().line_style().dash_end_cap() =     WT_Line_Style::Butt_Cap;

    //6 yellow 7 white, omitted due to visibility
    const int color_list[] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};

    int x = -1300;
    int y = 1100;
    int delta = 100;

    vP[0].m_x = x + 2*delta;
    vP[1].m_x = x + 12*delta;

    // line weights and colors
    WT_Logical_Point    min, max;
    min.m_x = x;
    min.m_y = y-900;
    max.m_x = x+1500;
    max.m_y = y;
    WT_Logical_Box        box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Polylines - Lineweights");
    WD_CHECK(named_view->serialize(whip_file));
	classFactory->Destroy(named_view);

    int i;
    for (i = 0; i < 10; i++) 
    {
        whip_file.desired_rendition().color().set(color_list[i], whip_file.desired_rendition().color_map());
        whip_file.desired_rendition().line_weight() = i*5;
        vP[0].m_y = vP[1].m_y = y - (delta*i);

        WT_Polyline *p = classFactory->Create_Polyline(2, vP, WD_True);
        WD_CHECK(p->serialize (whip_file));
		classFactory->Destroy(p);

        whip_file.desired_rendition().font().font_name().set("Arial");
        whip_file.desired_rendition().font().height().set(25);

        sprintf(caption_buf, "Weight=%d", i*5);
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(vP[0].m_x, vP[0].m_y + 25), WT_String(caption_buf));
        WD_CHECK(caption->serialize(whip_file));
		classFactory->Destroy(caption);
    }

    x = -1300;
    y = 0;
    delta = 100;

    // line patterns
    min.m_x = x;
    min.m_y = y-800;
    max.m_x = x+1200;
    max.m_y = 100;
    box = WT_Logical_Box (min, max);
    named_view = classFactory->Create_Named_View(box, "Polylines - Line Patterns");
    WD_CHECK(named_view->serialize(whip_file));
	classFactory->Destroy(named_view);

    vP[0].m_x = x + 2*delta;
    vP[1].m_x = x + 12*delta;

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());

    i=0;
    for(WT_Line_Pattern::WT_Pattern_ID index = WT_Line_Pattern::Solid ;
        index < WT_Line_Pattern::Count;
		(index = (WT_Line_Pattern::WT_Pattern_ID)(long(index) + 1)), i++)
    {
        switch (index)
        {
        case WT_Line_Pattern::Solid: 
            strcpy(caption_buf, "Solid"); break;

        case WT_Line_Pattern::Dashed:
            strcpy(caption_buf, "Dashed"); break;

        case WT_Line_Pattern::Dotted:
            strcpy(caption_buf, "Dotted"); break;

        case WT_Line_Pattern::Dash_Dot:
            strcpy(caption_buf, "Dash_Dot"); break;

        case WT_Line_Pattern::Short_Dash:
            strcpy(caption_buf, "Short_Dash"); break;

        case WT_Line_Pattern::Medium_Dash:
            strcpy(caption_buf, "Medium_Dash"); break;

        case WT_Line_Pattern::Long_Dash:
            strcpy(caption_buf, "Long_Dash"); break;

        case WT_Line_Pattern::Short_Dash_X2:
            strcpy(caption_buf, "Short_Dash_X2"); break;

        case WT_Line_Pattern::Medium_Dash_X2:
            strcpy(caption_buf, "Medium_Dash_X2"); break;

        case WT_Line_Pattern::Long_Dash_X2:
            strcpy(caption_buf, "Long_Dash_X2"); break;

        case WT_Line_Pattern::Medium_Long_Dash:
            strcpy(caption_buf, "Medium_Long_Dash"); break;

        case WT_Line_Pattern::Medium_Dash_Short_Dash_Short_Dash:
            strcpy(caption_buf, "Medium_Dash_Short_Dash_Short_Dash"); break;

        case WT_Line_Pattern::Long_Dash_Short_Dash:
            strcpy(caption_buf, "Long_Dash_Short_Dash"); break;

        case WT_Line_Pattern::Long_Dash_Dot_Dot:
            strcpy(caption_buf, "Long_Dash_Dot_Dot"); break;

        case WT_Line_Pattern::Long_Dash_Dot:
            strcpy(caption_buf, "Long_Dash_Dot"); break;

        case WT_Line_Pattern::Medium_Dash_Dot_Short_Dash_Dot:
            strcpy(caption_buf, "Medium_Dash_Dot_Short_Dash_Dot"); break;

        case WT_Line_Pattern::Sparse_Dot:
            strcpy(caption_buf, "Sparse_Dot"); break;

        case WT_Line_Pattern::ISO_Dash:
            strcpy(caption_buf, "ISO_Dash"); break;

        case WT_Line_Pattern::ISO_Dash_Space:
            strcpy(caption_buf, "ISO_Dash_Space"); break;

        case WT_Line_Pattern::ISO_Long_Dash_Dot:
            strcpy(caption_buf, "ISO_Long_Dash_Dot"); break;

        case WT_Line_Pattern::ISO_Long_Dash_Double_Dot:
            strcpy(caption_buf, "ISO_Long_Dash_Double_Dot"); break;

        case WT_Line_Pattern::ISO_Long_Dash_Triple_Dot:
            strcpy(caption_buf, "ISO_Long_Dash_Triple_Dot"); break;

        case WT_Line_Pattern::ISO_Dot:
            strcpy(caption_buf, "ISO_Dot"); break;

        case WT_Line_Pattern::ISO_Long_Dash_Short_Dash:
            strcpy(caption_buf, "ISO_Long_Dash_Short_Dash"); break;

        case WT_Line_Pattern::ISO_Long_Dash_Double_Short_Dash:
            strcpy(caption_buf, "ISO_Long_Dash_Double_Short_Dash"); break;

        case WT_Line_Pattern::ISO_Dash_Dot:
            strcpy(caption_buf, "ISO_Dash_Dot"); break;

        case WT_Line_Pattern::ISO_Double_Dash_Dot:
            strcpy(caption_buf, "ISO_Double_Dash_Dot"); break;

        case WT_Line_Pattern::ISO_Dash_Double_Dot:
            strcpy(caption_buf, "ISO_Dash_Double_Dot"); break;

        case WT_Line_Pattern::ISO_Double_Dash_Double_Dot:
            strcpy(caption_buf, "ISO_Double_Dash_Double_Dot"); break;

        case WT_Line_Pattern::ISO_Dash_Triple_Dot:
            strcpy(caption_buf, "ISO_Dash_Triple_Dot"); break;

        case WT_Line_Pattern::ISO_Double_Dash_Triple_Dot:
            strcpy(caption_buf, "ISO_Double_Dash_Triple_Dot"); break;
        }

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(vP[0].m_x, (y-delta*i) + 25), WT_String(caption_buf));
        WD_CHECK(caption->serialize(whip_file));
		classFactory->Destroy(caption);

        whip_file.desired_rendition().line_pattern()= (WT_Line_Pattern::WT_Pattern_ID)index;
        vP[0].m_y = vP[1].m_y = (y-delta*i);
        whip_file.desired_rendition().line_weight() = 1;

        whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());
		WT_Polyline *p1 = classFactory->Create_Polyline(2, vP, WD_True);
        WD_CHECK(p1->serialize (whip_file));
		classFactory->Destroy(p1);

        whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
        vP[0].m_y = vP[1].m_y -= 15;
        whip_file.desired_rendition().line_style().adapt_patterns() = WD_True;
        p1 = classFactory->Create_Polyline(2, vP, WD_True);
        WD_CHECK(p1->serialize (whip_file));
		classFactory->Destroy(p1);
        whip_file.desired_rendition().line_style().adapt_patterns() = WD_False;


        whip_file.desired_rendition().color().set(6, whip_file.desired_rendition().color_map());
        vP[0].m_y = vP[1].m_y -= 15;
        whip_file.desired_rendition().line_style().pattern_scale() = 50.0;
        p1 = classFactory->Create_Polyline(2, vP, WD_True);
        WD_CHECK(p1->serialize (whip_file));
		classFactory->Destroy(p1);
        whip_file.desired_rendition().line_style().pattern_scale() = 0.0;

    }

    x = 0;
    y = 175;

    // caps and joins
    for (int k=0; k<2; k++,y-=1400)
    {
        min.m_x = x;
        min.m_y = y-400;
        max.m_x = x+800;
        max.m_y = y+1000;
        box = WT_Logical_Box (min, max);
        named_view = !k ?
						classFactory->Create_Named_View(box, "Polylines - Caps and Joins 1") :
						classFactory->Create_Named_View(box, "Polylines - Caps and Joins 2");
        WD_CHECK(named_view->serialize(whip_file));
		classFactory->Destroy(named_view);

        //Note, if these values are ever different, we need to recode the below section
        const int cap_count = 4;
        const int join_count = 4;

        delta = 1000 / 16;

        whip_file.desired_rendition().line_pattern() = k ? WT_Line_Pattern::Short_Dash : WT_Line_Pattern::Solid;
        whip_file.desired_rendition().line_style().pattern_scale() = 50.0;

        for (i = cap_count-1; i >=0; i--)
		{
            if (!k)
            {
                switch(i)
                {
                case WT_Line_Style::Butt_Cap:
                    strcpy(caption_buf, "Butt+"); break;
                case WT_Line_Style::Square_Cap:
                    strcpy(caption_buf, "Square+"); break;
                case WT_Line_Style::Round_Cap:
                    strcpy(caption_buf, "Round+"); break;
                case WT_Line_Style::Diamond_Cap:
                    strcpy(caption_buf, "Diamond+"); break;
                }
            }
            else
            {
                whip_file.desired_rendition().font().height().set(15);
                switch(i)
                {
                case WT_Line_Style::Butt_Cap:
                    strcpy(caption_buf, "Dash Butt+"); break;
                case WT_Line_Style::Square_Cap:
                    strcpy(caption_buf, "Dash Square+"); break;
                case WT_Line_Style::Round_Cap:
                    strcpy(caption_buf, "Dash Round+"); break;
                case WT_Line_Style::Diamond_Cap:
                    strcpy(caption_buf, "Dash Diamond+"); break;
                }
            }

            for (int j = 0; j < join_count; j++) 
            {
                vP[0].m_x = x + (1 + 5 * j) * delta;
                vP[1].m_x = vP[0].m_x + 4 * delta;
                vP[2].m_x = vP[0].m_x;
                vP[3].m_x = vP[1].m_x;

                vP[0].m_y = y + 5 * (3 - i) * delta;
                vP[1].m_y = vP[0].m_y;
                vP[2].m_y = vP[0].m_y - 4 * delta;
                vP[3].m_y = vP[2].m_y;

                whip_file.desired_rendition().line_style().line_start_cap() = (WT_Line_Style::WT_Capstyle_ID) i;
                whip_file.desired_rendition().line_style().dash_start_cap() = (WT_Line_Style::WT_Capstyle_ID) i;
                whip_file.desired_rendition().line_style().line_end_cap()   = (WT_Line_Style::WT_Capstyle_ID) i;
                whip_file.desired_rendition().line_style().dash_end_cap()   = (WT_Line_Style::WT_Capstyle_ID) i;
                whip_file.desired_rendition().line_style().line_join()      = (WT_Line_Style::WT_Joinstyle_ID) j;
                whip_file.desired_rendition().line_style().adapt_patterns() = /*(!k) ? WD_False :*/ WD_True;

                whip_file.desired_rendition().color().set(
                    color_list[3*i + j], whip_file.desired_rendition().color_map());

                whip_file.desired_rendition().line_weight() = 30;
                WT_Polyline *p1 = classFactory->Create_Polyline(4, vP, WD_True);
                WD_CHECK(p1->serialize (whip_file));
				classFactory->Destroy(p1);

                whip_file.desired_rendition().color().set(255,255,255);

                whip_file.desired_rendition().line_weight() = 0;
                p1 = classFactory->Create_Polyline(4, vP, WD_True);
                WD_CHECK(p1->serialize (whip_file));
				classFactory->Destroy(p1);

                char caption_cell[256];
                strcpy(caption_cell, caption_buf);

                if (!k)
                {
                    switch(j)
                    {
                    case WT_Line_Style::Miter_Join:
                        strcat(caption_cell, "Miter"); break;
                    case WT_Line_Style::Bevel_Join:
                        strcat(caption_cell, "Bevel"); break;
                    case WT_Line_Style::Round_Join:
                        strcat(caption_cell, "Round"); break;
                    case WT_Line_Style::Diamond_Join:
                        strcat(caption_cell, "Diamond"); break;
                    }
                }
                else
                {
                    switch(j)
                    {
                    case WT_Line_Style::Butt_Cap:
                        strcat(caption_cell, "Butt Miter"); break;
                    case WT_Line_Style::Square_Cap:
                        strcat(caption_cell, "Square Bevel"); break;
                    case WT_Line_Style::Round_Cap:
                        strcat(caption_cell, "Round Round"); break;
                    case WT_Line_Style::Diamond_Cap:
                        strcat(caption_cell, "Diamond Diamond"); break;
                    }
                }
                whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
				WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(vP[0].m_x, vP[0].m_y+25), WT_String(caption_cell));
                WD_CHECK(caption->serialize(whip_file));
				classFactory->Destroy(caption);
            }
        }
    }

    //Degenerate Polylines/Polygons

    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().line_weight() = 10;
    whip_file.desired_rendition().line_style().line_start_cap() =   WT_Line_Style::Round_Cap;
    whip_file.desired_rendition().line_style().line_end_cap() =     WT_Line_Style::Round_Cap;
    whip_file.desired_rendition().line_style().dash_start_cap() =   WT_Line_Style::Round_Cap;
    whip_file.desired_rendition().line_style().dash_end_cap() =     WT_Line_Style::Round_Cap;

    x = 50;
    y = -1600;
    min.m_x = x-100;
    min.m_y = y-225;
    max.m_x = x+800;
    max.m_y = y+100;
    box = WT_Logical_Box(min, max);
    named_view = classFactory->Create_Named_View(box, "Degenerate Polylines/Polygons");
    WD_CHECK(named_view->serialize(whip_file));
	classFactory->Destroy(named_view);

    whip_file.desired_rendition().font().height().set(25);
    sprintf(caption_buf, "Degenerate Polylines/Polygons (lw=10, round start/end cap)");
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    WT_Text *caption = classFactory->Create_Text(WT_Logical_Point(x, y-25), WT_String(caption_buf));
    WD_CHECK(caption->serialize(whip_file));
	classFactory->Destroy(caption);

    whip_file.desired_rendition().font().height().set(15);

    vP[0] = WT_Logical_Point(x, y-75);
    //one-point polyline
    sprintf(caption_buf, "1pt polyline");
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    caption = classFactory->Create_Text(vP[0], WT_String(caption_buf));
    WD_CHECK(caption->serialize(whip_file));
    classFactory->Destroy(caption);
    vP[0].m_x += 100;
    WT_Polyline *dpolyline = classFactory->Create_Polyline(1, vP, WD_True);
    whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
    WD_CHECK(dpolyline->serialize(whip_file));
	classFactory->Destroy(dpolyline);

    vP[0] = WT_Logical_Point(x, y-100);
	
	// the polygon should be at lease three-point
    //three-point polygon
    sprintf(caption_buf, "1pt polygon");
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    caption = classFactory->Create_Text(vP[0], WT_String(caption_buf));
    WD_CHECK(caption->serialize(whip_file));
	classFactory->Destroy(caption);
    vP[0].m_x += 100;
    WT_Polygon *dpolygon = classFactory->Create_Polygon(3, vP, WD_False);
    whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
    WD_CHECK(dpolygon->serialize(whip_file));
	classFactory->Destroy(dpolygon);

    vP[0] = WT_Logical_Point(x, y-125);
    vP[1] = WT_Logical_Point(x+500, y-125);
    //four-point polygon
    sprintf(caption_buf, "2pt polygon");
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    caption = classFactory->Create_Text(vP[0], WT_String(caption_buf));
    WD_CHECK(caption->serialize(whip_file));
    classFactory->Destroy(caption);
    vP[0].m_x += 100;
    dpolygon = classFactory->Create_Polygon(4, vP, WD_False);
    whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
    WD_CHECK(dpolygon->serialize(whip_file));
	classFactory->Destroy(dpolygon);

    whip_file.desired_rendition().line_style() = *default_line_style;
	classFactory->Destroy(default_line_style);

    return WT_Result::Success;
}
