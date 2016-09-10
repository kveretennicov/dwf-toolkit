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


WT_Result read_text_background(WT_Text_Background& /*textbackground*/, WT_File& /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_text_background(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Color *black = classFactory->Create_Color(0,0,0,255);
	WT_Color *grey = classFactory->Create_Color(128,128,128,255);
	WT_Color *green = classFactory->Create_Color(0,128,0,255);
    // set rendition attributes for the text
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(60);

    // Serilaize out the color for the title.
    whip_file.desired_rendition().color() = *black;

    // Create and Serialize  the Title 
	WT_Text *title = classFactory->Create_Text(WT_Logical_Point(0,250),"Text Background"); 
    WD_CHECK (title->serialize(whip_file));
    whip_file.desired_rendition().font().height().set(40);

    // Construct the grid 
    whip_file.desired_rendition().color() = *grey;
    WT_Logical_Point points[2];
    points[0].m_x = points[1].m_x = 250;
    points[0].m_y = 200;
    points[1].m_y = -200;
	WT_Polyline *vline = classFactory->Create_Polyline(2,points,WD_True);
    WD_CHECK (vline->serialize(whip_file));
    
    points[0].m_x = 0;
    points[1].m_x = 500;
    points[0].m_y = points[1].m_y = 200;
    
   
    for (int i = 0; i < 5; i++)
    {
        WT_Polyline *line = classFactory->Create_Polyline(2,points,WD_True);
        WD_CHECK (line->serialize(whip_file));
        points[0].m_y = points[1].m_y -= 100;

		classFactory->Destroy(line);
    }

    WT_Text_Background *previous_background = classFactory->Create_Text_Background();
	*previous_background = whip_file.desired_rendition().text_background();
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Center;

    // Serialize out the color for the text
    whip_file.desired_rendition().color() = *green;
    
    int x_val = 125;
    int x2_val = 375;
    int y_val = 150;
    const int step = 100;
    
    // Write out first Background 
	WT_Text_Background *textBackground_none_0 = classFactory->Create_Text_Background(WT_Text_Background::None,0);
    whip_file.desired_rendition().text_background() = *textBackground_none_0;
	WT_Text *t1 = classFactory->Create_Text(WT_Logical_Point(x_val,y_val),"None");
    WD_CHECK (t1->serialize(whip_file));

    // Write out its corresponding rotated version
	WT_Text *t1_rot = classFactory->Create_Text(WT_Logical_Point(x2_val,y_val),"None");
    whip_file.desired_rendition().font().rotation().set(2731);
    WD_CHECK (t1_rot->serialize(whip_file));

    
    // increment y 
    y_val -= step;
   
    // set up the contrast color for the remaining text output
	WT_Contrast_Color *cc_yellow = classFactory->Create_Contrast_Color(255,255,0,255);
    whip_file.desired_rendition().contrast_color() = *cc_yellow;
    // 2nd Background 
	WT_Text_Background *textBackground_ghosted_49152 = classFactory->Create_Text_Background(WT_Text_Background::Ghosted,49152);
    whip_file.desired_rendition().text_background() = *textBackground_ghosted_49152;
    whip_file.desired_rendition().font().rotation().set(0);
	WT_Text *t2 = classFactory->Create_Text(WT_Logical_Point(x_val,y_val),"Ghosted");
    WD_CHECK (t2->serialize(whip_file));

    // rotated version
    whip_file.desired_rendition().font().rotation().set(5461);
	WT_Text *t2_rot = classFactory->Create_Text(WT_Logical_Point(x2_val,y_val),"Ghosted");
    WD_CHECK (t2_rot->serialize(whip_file));
   
    // increment y 
    y_val -= step;
   
     // 3rd background 
	WT_Text_Background *textBackground_solid_0 = classFactory->Create_Text_Background(WT_Text_Background::Solid,0);
	whip_file.desired_rendition().text_background() = *textBackground_solid_0;
    whip_file.desired_rendition().font().rotation().set(0);
	WT_Text *t3 = classFactory->Create_Text(WT_Logical_Point(x_val,y_val),"Solid");
    WD_CHECK (t3->serialize(whip_file));

    // 3rd background rotated 
    whip_file.desired_rendition().font().rotation().set(8192);
	WT_Text *t3_rot = classFactory->Create_Text(WT_Logical_Point(x2_val,y_val),"Solid");
    WD_CHECK (t3_rot->serialize(whip_file));

    // increment y 
    y_val -= step;

	WT_Text_Background *textBackground_ghosted_0 = classFactory->Create_Text_Background(WT_Text_Background::Ghosted,0);
    whip_file.desired_rendition().text_background() = *textBackground_ghosted_0;
    whip_file.desired_rendition().font().rotation().set(0);
    /**< Bounding rectangle (left-bottom, right-bottom, right-top, left-top.) */
    WT_Logical_Point arr[4];
    arr[0].m_x = arr[3].m_x = 25;
    arr[1].m_x = arr[2].m_x = 225;
    arr[0].m_y = arr[1].m_y = y_val - 30;
    arr[2].m_y = arr[3].m_y = y_val + 30;
	WT_Text *t4 = classFactory->Create_Text(WT_Logical_Point(x_val,y_val),"Path Text",arr,0,WD_Null,0,WD_Null);
    WD_CHECK (t4->serialize(whip_file));
    
    arr[0].m_x = arr[3].m_x = x2_val;
    arr[1].m_x = arr[2].m_x = x2_val + (step * 2);
    arr[0].m_y = y_val - 30;
    arr[1].m_y = y_val ;
    arr[2].m_y = y_val + 60;  
    arr[3].m_y = y_val;
	WT_Text *t4_skew = classFactory->Create_Text(WT_Logical_Point(x2_val,y_val),"Path Text",arr,0,WD_Null,0,WD_Null);
    WD_CHECK (t4_skew->serialize(whip_file));
   
    // restore default horizontal alignment
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Left;

	classFactory->Destroy(cc_yellow);
	classFactory->Destroy(previous_background);
	classFactory->Destroy(textBackground_ghosted_0);
	classFactory->Destroy(textBackground_solid_0);
	classFactory->Destroy(textBackground_ghosted_49152);
	classFactory->Destroy(textBackground_none_0);
	classFactory->Destroy(t4_skew);
	classFactory->Destroy(t4);
	classFactory->Destroy(t3_rot);
	classFactory->Destroy(t3);
	classFactory->Destroy(t2_rot);
	classFactory->Destroy(t2);
	classFactory->Destroy(t1_rot);
	classFactory->Destroy(t1);
	classFactory->Destroy(vline);
	classFactory->Destroy(title);
	classFactory->Destroy(black);
	classFactory->Destroy(grey);
	classFactory->Destroy(green);

    return WT_Result::Success;
}   
// End of file.