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


WT_Result read_text_halign(WT_Text_HAlign& /*text_halign*/, WT_File& /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_text_halign(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Color *black = classFactory->Create_Color(0,0,0,255);
    
    whip_file.desired_rendition().color() = *black;

    // set rendition attributes for the text
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(60);

	WT_Text *title = classFactory->Create_Text(WT_Logical_Point(0,1340),"Horizontal Text Aliginment"); 
   
    WD_CHECK (title->serialize(whip_file));

  
	WT_Color *grey = classFactory->Create_Color(128,128,128,255);
    whip_file.desired_rendition().color() = *grey;    

    // Serialize out the Grid on which to display the Text Alignment
    WT_Logical_Point points[2];
    points[0].m_x = points[1].m_x = 250;
    points[0].m_y = 1300;
    points[1].m_y = 1000;
    WT_Polyline *vline = classFactory->Create_Polyline(2,points,WD_True);
    WD_CHECK (vline->serialize(whip_file));
    

    points[0].m_x = 0;
    points[1].m_x = 500;
    points[0].m_y = points[1].m_y = 1300;
    for (int i = 0; i < 4; i++)
    {
		WT_Polyline *line = classFactory->Create_Polyline(2,points,WD_True);
        WD_CHECK (line->serialize(whip_file));
        points[0].m_y = points[1].m_y -= 100; 

		classFactory->Destroy(line);
    }
    
    // Serialize out the text color.
	WT_Color *green = classFactory->Create_Color(0,128,0,255);
    whip_file.desired_rendition().color() = *green;
      
    // set the font size 
    whip_file.desired_rendition().font().height().set(40);
    
	WT_Text_HAlign *old_align = classFactory->Create_Text_HAlign(whip_file.rendition().text_halign().hAlign());

    // left Alignemnt
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Left;
	WT_Text *text_left = classFactory->Create_Text(WT_Logical_Point(250,1250),"Left Alignment");
    WD_CHECK (text_left->serialize(whip_file));

    // Center Alignment
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Center;
	WT_Text *text_center = classFactory->Create_Text(WT_Logical_Point(250,1150),"Center Alignment");
    WD_CHECK (text_center->serialize(whip_file));

    // Right Alignment
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Right;
	WT_Text *text_right = classFactory->Create_Text(WT_Logical_Point(250,1050),"Right Alignment");
    WD_CHECK (text_right->serialize(whip_file));

    //restore the left default setting
    whip_file.desired_rendition().text_halign() = *old_align;

	classFactory->Destroy(text_right);
	classFactory->Destroy(text_center);
	classFactory->Destroy(text_left);
	classFactory->Destroy(old_align);
	classFactory->Destroy(green);
	classFactory->Destroy(vline);
	classFactory->Destroy(grey);
	classFactory->Destroy(title);
	classFactory->Destroy(black);
	
	return WT_Result::Success;
}

// End of file.