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

WT_Result read_text_valign(WT_Text_VAlign& /*text_valign*/, WT_File& /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_text_valign(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Color *black = classFactory->Create_Color(0,0,0,255);
    WT_Color *green = classFactory->Create_Color(0,128,0,255);
    // set rendition attributes for the text
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(60);

    // Serilaize out the color for the title.
    whip_file.desired_rendition().color() = *black;

    // Create and Serialize  the Title 
	WT_Text *title = classFactory->Create_Text(WT_Logical_Point(0,800),"Vertical Text Aliginment"); 
    WD_CHECK (title->serialize(whip_file));
    whip_file.desired_rendition().font().height().set(40);

    // Serialize Grid here
    WT_Color *grey = classFactory->Create_Color(128,128,128,255);
    WT_Color *light_grey = classFactory->Create_Color(200,200,200,255);
    whip_file.desired_rendition().color() = *grey;
    // Serialize out the Grid on which to display the Text Alignment
    WT_Logical_Point points[2];
    points[0].m_x = points[1].m_x = 50;
    points[0].m_y = 750;
    points[1].m_y = 350;
	WT_Polyline *vline = classFactory->Create_Polyline(2,points,WD_True);
    WD_CHECK (vline->serialize(whip_file));
    

    points[0].m_x = 50;
    points[1].m_x = 500;
    points[0].m_y = points[1].m_y = 750;
    WT_Polyline *line = classFactory->Create_Polyline(2,points,WD_True);
    WD_CHECK (line->serialize(whip_file));
    for (int i = 0; i < 4; i++)
    {
        whip_file.desired_rendition().color() = *light_grey;
        points[0].m_y = points[1].m_y -= 25;
        for ( int j = 0; j < 3 ; j++)
        {
			WT_Polyline *line_inner = classFactory->Create_Polyline(2,points,WD_True);
            WD_CHECK (line_inner->serialize(whip_file));
            points[0].m_y = points[1].m_y -= 25;

			classFactory->Destroy(line_inner);
        }
        whip_file.desired_rendition().color() = *grey;
        WT_Polyline *line = classFactory->Create_Polyline(2,points,WD_True);
        WD_CHECK (line->serialize(whip_file));
    
		classFactory->Destroy(line);
    }
    int xstart = 50;
    int ystart = 700;
	WT_Text_VAlign *previous_vAlign = classFactory->Create_Text_VAlign(whip_file.desired_rendition().text_valign().vAlign());
    
    // serilaize out the color for the text 
    whip_file.desired_rendition().color() = *green;

    //Serialize the text with different Horizontal Alignment settings 
    whip_file.desired_rendition().text_valign().vAlign() = WT_Text_VAlign::Ascentline;
	WT_Text *text1 = classFactory->Create_Text(WT_Logical_Point(xstart,ystart),"Ascentline");
    WD_CHECK (text1->serialize(whip_file));
    
    ystart -=100;
    whip_file.desired_rendition().text_valign().vAlign() = WT_Text_VAlign::Baseline;
	WT_Text *text2 = classFactory->Create_Text(WT_Logical_Point(xstart,ystart),"Baseline");
    WD_CHECK (text2->serialize(whip_file));

    ystart -=100;
    whip_file.desired_rendition().text_valign().vAlign() = WT_Text_VAlign::Capline;
    WT_Text *text3 = classFactory->Create_Text(WT_Logical_Point(xstart,ystart),"Capline");
    WD_CHECK (text3->serialize(whip_file));

    ystart -=100;
    whip_file.desired_rendition().text_valign().vAlign() = WT_Text_VAlign::Halfline;
    WT_Text *text4 = classFactory->Create_Text(WT_Logical_Point(xstart,ystart),"Halfline");
    WD_CHECK (text4->serialize(whip_file));

    //restore the old settings
    whip_file.desired_rendition().text_valign() = *previous_vAlign;

	classFactory->Destroy(text4);
	classFactory->Destroy(text3);
	classFactory->Destroy(text2);
	classFactory->Destroy(text1);
	classFactory->Destroy(previous_vAlign);
	classFactory->Destroy(line);
	classFactory->Destroy(vline);
	classFactory->Destroy(grey);
	classFactory->Destroy(light_grey);
	classFactory->Destroy(title);
	classFactory->Destroy(black);
	classFactory->Destroy(green);

    return WT_Result::Success;
}

// End of file.