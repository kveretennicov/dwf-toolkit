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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/macro.cpp#1 $

#ifdef _DWFCORE_WIN32_SYSTEM
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
WT_Result _write_Macro_Definition_Polygons(WT_File & whip_file, int x_start, 
                                                                int y_start, 
                                                                int& out_endX, 
                                                                int& out_endY);

WT_Result _write_Macro_Definition_Ellipse( WT_File & whip_file, int x_start, 
                                                                int y_start, 
                                                                int& out_endX, 
                                                                int& out_endY);

WT_Result _write_Macro_Definition_Text( WT_File & whip_file, int x_start, 
                                                             int y_start, 
                                                             int& out_endX, 
                                                             int& out_endY);


WT_Result read_macro_definition(WT_Macro_Definition& /*userdata*/, WT_File& /*whip_file*/)
{
    return WT_Result::Success;
}


WT_Result write_macro_definition(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Color *black = classFactory->Create_Color(0,0,0,255);
    
    int x_start   = 800;
    int y_start   = 1340;
    int x_end = 0;
    int y_end = 0;
    int text_offset = 90;
    whip_file.desired_rendition().color() = *black;
    // set rendition attributes for the text
    whip_file.desired_rendition().text_halign().hAlign() = WT_Text_HAlign::Left;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(60);
    
    // Serilaize out the first Macro test.
    
    WT_Text *title = classFactory->Create_Text(WT_Logical_Point(x_start,y_start),"Macro: Variable scale with Poygons"); 
    WD_CHECK (title->serialize(whip_file));
    y_start -= text_offset;
    WD_CHECK( _write_Macro_Definition_Polygons(whip_file,x_start, y_start, x_end, y_end) );
    y_start = y_end;
    

    // Serialize out the second Macro Test 
    y_start -= (int)(text_offset * 1.5);
    whip_file.desired_rendition().color() = *black;
	WT_Text *title2 = classFactory->Create_Text(WT_Logical_Point(x_start,y_start),"Macro: Line & Fill with Ellipse"); 
    WD_CHECK (title2->serialize(whip_file));
    y_start -= (int)(text_offset * 0.5);
    WD_CHECK( _write_Macro_Definition_Ellipse(whip_file, x_start, y_start, x_end, y_end));
    y_start = y_end;

    // Serialize out the third Macro test.
    y_start -= text_offset;
    WD_CHECK( _write_Macro_Definition_Text(whip_file,x_start, y_start, x_end, y_end));

	classFactory->Destroy(title2);
	classFactory->Destroy(title);
	classFactory->Destroy(black);

    return WT_Result::Success;
} 


WT_Result _write_Macro_Definition_Polygons(WT_File & whip_file, int x_start, 
                                                                int y_start, 
                                                                int& out_endX, 
                                                                int& out_endY)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    // Notes size of this playback is 900 wide , 600 height
    int width = 400;
    int height = 300;
    out_endX = x_start + width;
    out_endY = y_start - height;
    
    int numbers[] = {0,0,60,300,250,300,300,0,165,0,180,60,240,60,150,270,60,
                        60,120,60,135,0,0,0};
    WT_Logical_Point points[12];
    for ( int i = 0; i < 12 ; i++)
    {
        points[i].m_x = numbers[(i* 2)];
        points[i].m_y = numbers[(i* 2) +1];
    }
    // create a new macro 
    WT_Macro_Definition *macro_def =
		classFactory->Create_Macro_Definition(
			0,		// Index
			10);	// Scale

    // create the shapes for the polyline 
    WT_Polygon*  polygon = classFactory->Create_Polygon(12,points,WD_True);
    
    macro_def->add(*polygon); //macro_def then owns the polygon, we don't have the class factory destroy it 
       
    // serilaize out the macro 
    WD_CHECK (macro_def->serialize(whip_file));
	classFactory->Destroy(macro_def);

    // playback 
	WT_Color *black = classFactory->Create_Color(0,0,0,255);
	WT_Color *red = classFactory->Create_Color(255,0,0,255);
	WT_Color *green = classFactory->Create_Color(0,255,0,255);
	WT_Color *blue = classFactory->Create_Color(0,0,255,255);

    // set the color 
    whip_file.desired_rendition().color() = *black;
	classFactory->Destroy(black);
    
    // set the macro_scale
	WT_Macro_Scale *macroScale5 = classFactory->Create_Macro_Scale(5);
    whip_file.desired_rendition().macro_scale() = *macroScale5;
	classFactory->Destroy(macroScale5);

    // play back position 
    WT_Logical_Point play_pos(x_start + width/3 ,y_start - height);
	WT_Macro_Draw *macro_play = classFactory->Create_Macro_Draw(1,&play_pos, WD_True);
    
    // serialize the macro drawable
    WD_CHECK (macro_play->serialize(whip_file));
	classFactory->Destroy(macro_play);
    
    // Line one 
    WT_Logical_Point positions[5];
    whip_file.desired_rendition().color() = *red;
	classFactory->Destroy(red);
	WT_Macro_Scale *macroScale_36 = classFactory->Create_Macro_Scale(-36);
    whip_file.desired_rendition().macro_scale() = *macroScale_36;
    for (int i = 0; i < 5 ; i++)
    {
        positions[i].m_x = x_start + i* 100;//(width/3);
        positions[i].m_y = y_start - height;
    }
	WT_Macro_Draw *macro_play1 = classFactory->Create_Macro_Draw(5,positions, WD_True);
    WD_CHECK (macro_play1->serialize(whip_file));
	classFactory->Destroy(macro_play1);

    // line two 
    whip_file.desired_rendition().color() = *green;
	classFactory->Destroy(green);
    whip_file.desired_rendition().macro_scale() = *macroScale_36;
    for (int i = 0; i < 5 ; i++)
    {
        positions[i].m_x = x_start + i* 100;//(width/4);
        positions[i].m_y = y_start - 150; // height/2;
    }
    WT_Macro_Draw *macro_play2 = classFactory->Create_Macro_Draw(5,positions, WD_True);
    WD_CHECK (macro_play2->serialize(whip_file));
	classFactory->Destroy(macro_play2);
    
    // line three 
    whip_file.desired_rendition().color() = *blue;
	classFactory->Destroy(blue);
    whip_file.desired_rendition().macro_scale() = *macroScale_36;
    for (int i = 0; i < 5 ; i++)
    {
        positions[i].m_x = x_start + i* 100;//(width/3);
        positions[i].m_y = y_start ; // height/2;
    }
    WT_Macro_Draw *macro_play3 = classFactory->Create_Macro_Draw(5,positions, WD_True);
    WD_CHECK (macro_play3->serialize(whip_file));
	classFactory->Destroy(macro_play3);

	classFactory->Destroy(macroScale_36);

	return WT_Result::Success;
}   





WT_Result _write_Macro_Definition_Text( WT_File & /*whip_file*/, int /*x_start*/, 
                                                             int /*y_start*/, 
                                                             int& /*out_endX*/, 
                                                             int& /*out_endY*/)
{
        
    return WT_Result::Success;

}

WT_Result _write_Macro_Definition_Ellipse( WT_File & whip_file, int x_start, 
                                                                int y_start, 
                                                                int& out_endX, 
                                                                int& out_endY)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
    
    int height = 300;
    int width =  360;

    out_endX = x_start + width;
    out_endY = y_start - height;

	WT_Color *black = classFactory->Create_Color(0,0,0);
	WT_Color *c2 = classFactory->Create_Color(0,128,0,255);
	WT_Color *c3 = classFactory->Create_Color(128,128,0,255);
	WT_Color *c4 = classFactory->Create_Color(0,0,128,255);
	WT_Color *c5 = classFactory->Create_Color(128,0,128,255);
	WT_Color *c6 = classFactory->Create_Color(0,128,128,255);

    // draw out the grid 
    WT_Logical_Point pts[2];
    pts[0].m_x = pts[1].m_x = x_start + width/2;
    pts[0].m_y = y_start;
    pts[1].m_y = y_start - height;
    
    whip_file.desired_rendition().color() = *black;
	classFactory->Destroy(black);
    WT_Polyline *line1 = classFactory->Create_Polyline(2,pts,WD_True);
    WD_CHECK (line1->serialize(whip_file));
	classFactory->Destroy(line1);
    
    pts[0].m_x = x_start;
    pts[1].m_x = x_start + width;
    pts[0].m_y = pts[1].m_y = y_start - height/3;
    
    WT_Polyline *line2 = classFactory->Create_Polyline(2,pts,WD_True);
    WD_CHECK (line2->serialize(whip_file));
	classFactory->Destroy(line2);

    pts[0].m_y = pts[1].m_y = y_start - (height/3 * 2);
	WT_Polyline *line3 = classFactory->Create_Polyline(2,pts,WD_True);
    WD_CHECK (line3->serialize(whip_file));
	classFactory->Destroy(line3);
    
    // create Macro definition  : 0
	WT_Macro_Definition *macro1 = classFactory->Create_Macro_Definition(0,1);
    // construct an ellipse on the heap to add to the macro
	WT_Ellipse*  pEllipse = classFactory->Create_Filled_Ellipse(
												   WT_Logical_Point (0,0) 
                                                  ,75
                                                  ,40
                                                  ,(WT_Unsigned_Integer16)0
                                                  ,(WT_Unsigned_Integer16)0
                                                  ,(WT_Unsigned_Integer16)0);
	WT_Fill* pFill = classFactory->Create_Fill(WD_True);
    
    // add the attrbute and drawable to the macro 
    WD_CHECK (macro1->add(*pFill)); //owns pFill
    WD_CHECK (macro1->add(*pEllipse)); //owns pEllipse
    
    // serialize 
    WD_CHECK (macro1->serialize(whip_file));
	classFactory->Destroy(macro1);

    // create Macro definition : 1
    WT_Macro_Definition *macro2 = classFactory->Create_Macro_Definition(1,1);
	pEllipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point (0,0) 
                                                  ,75
                                                  ,40
                                                  ,(WT_Unsigned_Integer16)0
                                                  ,(WT_Unsigned_Integer16)0
                                                  ,(WT_Unsigned_Integer16)0);
	pFill = classFactory->Create_Fill(WD_False);
    WT_Line_Weight* pLineWeight = classFactory->Create_Line_Weight(5);
    WD_CHECK (macro2->add(*pFill)); //owns pFill
    WD_CHECK (macro2->add(*pLineWeight)); //owns pLineWeight
    WD_CHECK (macro2->add(*pEllipse)); //owns pEllipse
    WD_CHECK (macro2->serialize(whip_file));
	classFactory->Destroy(macro2);

    // create Macro definition : 2
    
    WT_Macro_Definition *macro3 = classFactory->Create_Macro_Definition(2,1);
    pEllipse = classFactory->Create_Filled_Ellipse(WT_Logical_Point (0,0) 
                                    ,75
                                    ,40
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0);
    pFill = NULL;
    pFill = classFactory->Create_Fill(WD_True);
	WT_Color* pColor = classFactory->Create_Color( *c2 );
    WD_CHECK (macro3->add(*pColor)); //owned
    WD_CHECK (macro3->add(*pFill)); //owned
    WD_CHECK (macro3->add(*pEllipse)); //owned
    
    pEllipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point (0,0) 
                                    ,75
                                    ,40
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0);
    pFill = classFactory->Create_Fill(WD_False);
    pColor = classFactory->Create_Color( *c5 );
    WD_CHECK (macro3->add(*pFill)); //owned
    WD_CHECK (macro3->add(*pColor)); //owned
    WD_CHECK (macro3->add(*pEllipse)); //owned
    WD_CHECK (macro3->serialize(whip_file));
	classFactory->Destroy(macro3);

    // create macro definition : 3
    WT_Macro_Definition *macro4 = classFactory->Create_Macro_Definition(3,1);
    pEllipse = classFactory->Create_Filled_Ellipse(WT_Logical_Point (0,0) 
                                    ,75
                                    ,40
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0);
    pFill = classFactory->Create_Fill(WD_True);
    pColor = classFactory->Create_Color( *c3 );
    WD_CHECK (macro4->add(*pColor)); //owned
    WD_CHECK (macro4->add(*pFill)); //owned
    WD_CHECK (macro4->add(*pEllipse)); //owned   
    
    pEllipse = classFactory->Create_Outline_Ellipse(WT_Logical_Point (0,0) 
                                    ,75
                                    ,40
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0
                                    ,(WT_Unsigned_Integer16)0);
    pFill = classFactory->Create_Fill(WD_False);
    pColor = classFactory->Create_Color( *c6 );
    WD_CHECK (macro4->add(*pFill)); //owned
    WD_CHECK (macro4->add(*pColor)); //owned
    WD_CHECK (macro4->add(*pEllipse)); //owned
    WD_CHECK (macro4->serialize(whip_file));
	classFactory->Destroy(macro4);
    
   
    // playback of macro 
   
    int off_X = 90;
    int off_y = 50;
    
    // playback 1 
    whip_file.desired_rendition().color() = *c2;
	WT_Macro_Scale *macroScale1 = classFactory->Create_Macro_Scale(1);
    whip_file.desired_rendition().macro_scale() = *macroScale1;
	classFactory->Destroy(macroScale1);

    WT_Logical_Point position( x_start + width/2 - off_X, 
                               y_start - height/3 + off_y); 
    
    WT_Macro_Draw *draw1 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw1->serialize(whip_file));
	classFactory->Destroy(draw1);

    // playback 2 
    whip_file.desired_rendition().color() = *c3;
    position.m_x =  x_start + width/2 + off_X;
    WT_Macro_Draw *draw2 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw2->serialize(whip_file));
	classFactory->Destroy(draw2);

    // playback 3 
    whip_file.desired_rendition().color() = *c5;
	WT_Macro_Index *macroIndex1 = classFactory->Create_Macro_Index(1);
    whip_file.desired_rendition().macro_index() = *macroIndex1;
	classFactory->Destroy(macroIndex1);
    position.m_x = x_start + width/2 - off_X;
    position.m_y = y_start - height/3 - off_y; 
    WT_Macro_Draw *draw3 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw3->serialize(whip_file));
	classFactory->Destroy(draw3);

    // playback 4
    whip_file.desired_rendition().color() = *c6;
    position.m_x = x_start + width/2 + off_X;
    WT_Macro_Draw *draw4 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw4->serialize(whip_file));
	classFactory->Destroy(draw4);

    //playback 5 
	WT_Macro_Index *macroIndex2 = classFactory->Create_Macro_Index(2);
    whip_file.desired_rendition().macro_index() = *macroIndex2;
	classFactory->Destroy(macroIndex2);
    position.m_x = x_start + width/2 - off_X;
    position.m_y = y_start - height/3*2 - off_y; 
    WT_Macro_Draw *draw5 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw5->serialize(whip_file));
	classFactory->Destroy(draw5);

    //playback 6  
	WT_Macro_Index *macroIndex3 = classFactory->Create_Macro_Index(3);
    whip_file.desired_rendition().macro_index() = *macroIndex3;
	classFactory->Destroy(macroIndex3);
    position.m_x = x_start + width/2 + off_X;
    WT_Macro_Draw *draw6 = classFactory->Create_Macro_Draw(1,&position,WD_True);
    WD_CHECK (draw6->serialize(whip_file));
	classFactory->Destroy(draw6);

	classFactory->Destroy(c2);
	classFactory->Destroy(c3);
	classFactory->Destroy(c4);
	classFactory->Destroy(c5);
	classFactory->Destroy(c6);

    return WT_Result::Success;
}

WT_Result read_macro_draw(WT_Macro_Draw & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_macro_draw(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 7, WT_String("Macro_Draw"));
    whip_file.desired_rendition().layer() = *layer;

    WT_Logical_Point    min, max;

    int x = -2300;
    int y = -1700;

    min.m_x = x;
    min.m_y = y-350;
    max.m_x = x+200;
    max.m_y = y+50;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Macro_Draw");
    WD_CHECK(named_view->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
	WT_Macro_Scale *macroScale32 = classFactory->Create_Macro_Scale(32);
	WT_Macro_Index *macroIndex1 = classFactory->Create_Macro_Index(1);
    whip_file.desired_rendition().macro_scale() = *macroScale32;
    whip_file.desired_rendition().macro_index() = *macroIndex1;
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y), WT_String("Macro_Draw"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().height().set(15);

    WT_Logical_Point vP[5];
    for(int i=0;i<5;i++)
    {
        vP[i].m_x = x+(i*25);
        vP[i].m_y = y-50;
		WT_Macro_Draw *macro_draw = classFactory->Create_Macro_Draw(1, &vP[i], WD_True);
        WD_CHECK(macro_draw->serialize(whip_file));

		classFactory->Destroy(macro_draw);
    }

	classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+150, y-50), WT_String("Pixel"));
    WD_CHECK(text->serialize(whip_file));

	classFactory->Destroy(macroIndex1);
	classFactory->Destroy(macroScale32);
	classFactory->Destroy(text);
	classFactory->Destroy(named_view);
	classFactory->Destroy(layer);

    return WT_Result::Success;
}

