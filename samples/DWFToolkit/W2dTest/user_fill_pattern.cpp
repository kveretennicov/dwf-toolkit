#include "stdafx.h"
#include "whip_fill_library.h"
#include "TestFramework.h"

//
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

// declaration of functions 
WT_Result _write_EMap_Fills( WT_File & whip_file);


WT_Result read_user_fill_pattern(WT_User_Fill_Pattern& /*user_fill_pattern*/, WT_File& /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_user_fill_pattern(WT_File & whip_file)
{
        
    // construct a polygon 
    WD_CHECK (_write_EMap_Fills(whip_file)); 
    
    return WT_Result::Success;
}


// EMap Fills 
WT_Result _write_EMap_Fills( WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    // 20 rows of 3 each  
     // create the shapes for the polyline 
    WT_Logical_Point point_arr[4];

    // Set text font and height 
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(100);
    
    WT_Logical_Point text_pt(0,5200); 
    // Write out title of DWF 
	WT_Text *opening = classFactory->Create_Text(text_pt, WT_String("User Fill Patterns for EMap"));

    WD_CHECK(opening->serialize(whip_file));
	classFactory->Destroy(opening);
    
    // set rendition attributes for the next 60 drawables 
	whip_file.desired_rendition().font().height().set(50);
    
    
    int cols = 0;
    int rows = 0;
    int col_step = 0 ; // increment in the x axis  by 1700
    int row_step = 0; // decrement in the y axis  by 150 
	WT_Color *black = classFactory->Create_Color(0,0,0,255);
	WT_Color *dark_green = classFactory->Create_Color(0,128,0,255);
    for ( int i = 0; i < (int)Helper_Fill_Pattern::count ; i++ )
    {
        col_step = cols * 1700;
        row_step = rows * 250;
        point_arr[0].m_x = 500  + col_step; 
        point_arr[0].m_y = 4850 - row_step;
        point_arr[1].m_x = 500  + col_step;
        point_arr[1].m_y = 5050 - row_step;
        point_arr[2].m_x = 1400 + col_step;
        point_arr[2].m_y = 5050 - row_step;
        point_arr[3].m_x = 1400 + col_step;
        point_arr[3].m_y = 4850 - row_step;
    
        text_pt.m_x = col_step;  
        text_pt.m_y = 4850 - row_step; 
        
        // Serialize out the color for the label
        WD_CHECK (black->serialize(whip_file));

        // Serialize out the label describing the user Fill Pattern 
		WT_Text *text = classFactory->Create_Text(text_pt, Helper_Fill_Pattern_Names[i]);
        WD_CHECK (text->serialize(whip_file));
		classFactory->Destroy(text);

        WT_User_Fill_Pattern::Fill_Pattern * pattern = WT_User_Fill_Pattern::Fill_Pattern::Construct(
                                                    8,          //rows
                                                    8,          //columns
                                                    8,          // data_size
                                                    Helper_Fill_Patterns[i] // data
                                                    );
		WT_User_Fill_Pattern *usr_pattern = classFactory->Create_User_Fill_Pattern((WT_Integer16)(i+1), pattern);

        // Set the desired rendition value for the user Fill Pattern 
        whip_file.desired_rendition().user_fill_pattern() = *usr_pattern;
        classFactory->Destroy( usr_pattern );
        
        // Serialize out the color for the geometry
        WD_CHECK (dark_green->serialize(whip_file));

        // Serialize out the polygon 
		WT_Polygon *poly = classFactory->Create_Polygon(4,point_arr, WD_True);
        WD_CHECK (poly->serialize(whip_file));
		classFactory->Destroy(poly);

        
        // increment mechanism.
        if (cols == 2)
        {
            rows ++;
            cols = 0;
        }
        else
        {
            cols ++;
        }
    } // end of for loop

	classFactory->Destroy(black);
	classFactory->Destroy(dark_green);

    return WT_Result::Success;
}


// Fill test