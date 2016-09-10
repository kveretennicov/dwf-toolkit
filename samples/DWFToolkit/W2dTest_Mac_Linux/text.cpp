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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/text.cpp#1 $

WT_Result read_text(WT_Text & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_text(WT_File & whip_file)
{
    WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    WT_Layer *layer = classFactory->Create_Layer(whip_file, 4, WT_String("Text"));
    whip_file.desired_rendition().layer() = *layer;

    WT_Logical_Point    min, max;

    int x = 1500;
    int y = -600;

    min.m_x = x-50;
    min.m_y = y-1500;
    max.m_x = x+2500;
    max.m_y = y-200;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Text");
    WD_CHECK(named_view->serialize(whip_file));


    WT_Unsigned_Integer16 score_pos[6] = { 0, 1, 2, 3, 4, 5};

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y-20), WT_String("Underscored, overscored, height=200"));
    WD_CHECK(text->serialize(whip_file));

    WT_Logical_Point bounds[4];
    bounds[0].m_x = x;
    bounds[0].m_y = y-200;
    bounds[1].m_x = x+1190;
    bounds[1].m_y = y-200;
    bounds[2].m_x = x+1190;
    bounds[2].m_y = y;
    bounds[3].m_x = x;
    bounds[3].m_y = y;

    classFactory->Destroy(text);
    text = classFactory->Create_Text(
        WT_Logical_Point(x, y-200), 
        WT_String("Text 1"),
        bounds,
        6, 
        score_pos, 
        6, 
        score_pos
        );

    whip_file.desired_rendition().font().height().set(200);

    //Add 2 hyperlinks to views on this page
    WT_URL_Item             url1(0, "?docType=ePlot&view='Text'", "Jump to Text view");
    WT_URL_Item             url2(1, "?docType=ePlot&?view='Images'", "Jump to Images view");
    whip_file.desired_rendition().url().add_url_optimized(url1, whip_file);
    whip_file.desired_rendition().url().add_url_optimized(url2, whip_file);
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().url().clear();

    //add the caret to show the insertion position
    whip_file.desired_rendition().color().set(255,0,0);
    whip_file.desired_rendition().fill() = WD_False;
    WT_Logical_Point pt[3];
    pt[0].m_x = x-5;
    pt[1].m_x = pt[0].m_x + 5;
    pt[2].m_x = pt[1].m_x + 5;
    pt[0].m_y = y-225;
    pt[1].m_y = y-200;
    pt[2].m_y = y-225;
    WT_Polyline *p = classFactory->Create_Polyline(3, pt, WD_False);
    WD_CHECK(p->serialize(whip_file));

    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x, y-320), WT_String("Default settings, height=200"));
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().font().height().set(200);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x, y-500), WT_String("Text 2"));
    WT_URL_Item             url3(2, "http://www.autodesk.com/viewers", "Autodesk Viewers Page");
    whip_file.desired_rendition().url().add_url_optimized(url3, whip_file);
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().url().clear();

    //style: bold, italic, underlined
    whip_file.desired_rendition().font().height().set(100);
    whip_file.desired_rendition().font().style().set_bold(WD_True);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x, y-650), WT_String("Bold"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().style().set_bold(WD_False);

    whip_file.desired_rendition().font().style().set_italic(WD_True);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+300, y-650), WT_String("Italic"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().style().set_italic(WD_False);

    whip_file.desired_rendition().font().style().set_underlined(WD_True);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-650), WT_String("Underlined"));
    WT_URL_Item             url4(3, "?section=ePlotGlobal&resource=SampleWordDoc.doc", "Sample Word Doc");
    whip_file.desired_rendition().url().add_url_optimized(url4, whip_file);
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().url().clear();
    whip_file.desired_rendition().font().style().set_underlined(WD_False);

    //rotation
    whip_file.desired_rendition().font().height().set(25);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+125, y-900), WT_String("Text rotation"));
    WD_CHECK(text->serialize(whip_file));

    whip_file.desired_rendition().color().set(1, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().height().set(50);
    const int degree = (int)(65636/360);
    char cbuf[256];
    int i;
    for (i=0; i<360; i+=15)
    {
        sprintf(cbuf, "          %d", i);
        whip_file.desired_rendition().font().rotation().set((WT_Unsigned_Integer16)(degree*i));
        classFactory->Destroy(text);
        text = classFactory->Create_Text(WT_Logical_Point(x+200, y-900), WT_String(cbuf));
        WD_CHECK(text->serialize(whip_file));
    }
    whip_file.desired_rendition().font().rotation().set(0);

    //width scale
    whip_file.desired_rendition().color().set(2, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().width_scale().set(512);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-750), WT_String("Width scale=512 (.5x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().width_scale().set(1024);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-800), WT_String("Width scale=1024 (1x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().width_scale().set(2048);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-850), WT_String("Width scale=2048 (2x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().width_scale().set(1024);

    //spacing
    whip_file.desired_rendition().color().set(3, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().spacing().set(512);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-900), WT_String("Spacing=512 (.5x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().spacing().set(1024);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-950), WT_String("Spacing=1024 (1x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().spacing().set(2048);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-1000), WT_String("Spacing=2048 (2x)"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().spacing().set(1024);

    //oblique
    whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().font().oblique().set(10*degree);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-1050), WT_String("Oblique=10 deg"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().oblique().set(25*degree);
    classFactory->Destroy(text);
    text = classFactory->Create_Text(WT_Logical_Point(x+600, y-1100), WT_String("Oblique=25 deg"));
    WD_CHECK(text->serialize(whip_file));
    whip_file.desired_rendition().font().oblique().set(0);


    //flags
    whip_file.desired_rendition().color().set(5, whip_file.desired_rendition().color_map());
    int flags = 0;
    char type[256];
    for (i=0;i<11;i++)
    {
        flags = 1<<i;
        whip_file.desired_rendition().font().flags().set(flags); 
        *type = 0;
        switch (flags)
        {
            case 0x0001: strcpy(type, "VERTICAL"); break;
            case 0x0002: strcpy(type, "MIRROR_X"); break;
            case 0x0004: strcpy(type, "MIRROR_Y"); break;
            case 0x0008: strcpy(type, "UNDERSCORE"); break;
            case 0x0010: strcpy(type, "OVERSCORE"); break;
            case 0x0020: strcpy(type, "MTEXT_START"); break;
            case 0x0040: strcpy(type, "MTEXT_END"); break;
            case 0x0080: strcpy(type, "MTEXT"); break;
            case 0x0100: strcpy(type, "GREEK_TEXT"); break;
            case 0x0200: strcpy(type, "PATH_TEXT"); break;
            case 0x0400: strcpy(type, "OUTLINE_TEXT"); break;
        }
        sprintf(cbuf, "Flags=0x%04X %s", flags, type);
        classFactory->Destroy(text);
        text = classFactory->Create_Text(WT_Logical_Point(x+600, y-(1150+(i*50))), WT_String(cbuf));
        WD_CHECK(text->serialize(whip_file));
    }
    whip_file.desired_rendition().font().flags().set(0); 


    //flags:
    //   VERTICAL        0x0001      // TVERT
    //   MIRROR_X        0x0002      // TXMIR
    //   MIRROR_Y        0x0004      // TYMIR
    //   UNDERSCORE      0x0008      // TUNDER
    //   OVERSCORE       0x0010      // TOVER
    //   MTEXT_START     0x0020      // TMSTART
    //   MTEXT_END       0x0040      // TMEND
    //   MTEXT           0x0080      // TMTEXT
    //   GREEK_TEXT      0x1000      // Whip and GDI font engine only
    //   PATH_TEXT       0x2000      // Whip and GDI font engine only
    //   OUTLINE_TEXT    0x4000      // Outline Text Only


    // Create an invisible polygon which has a hyperlink.  Stick something on top of it.
    {
        //Store the current visibility and color.
        WT_Boolean previous_visibility = whip_file.desired_rendition().visibility().visible();

        WT_Color *previous_color = classFactory->Create_Color();
        *previous_color = whip_file.desired_rendition().color();

        //Define a polygon to be used as an invisible link target.
        x+=1500;
        y-=500;
        WT_Logical_Point vP[4];
        vP[0] = WT_Logical_Point(x-100,y-100);
        vP[1] = WT_Logical_Point(x-100,y+100);
        vP[2] = WT_Logical_Point(x+100,y+100);
        vP[3] = WT_Logical_Point(x+100,y-100);
        WT_Polygon *polygon = classFactory->Create_Polygon(4, vP, WD_True);
        
        //Set the current visibility to off, and the color to transparent black.
        whip_file.desired_rendition().visibility() = WD_False;
        whip_file.desired_rendition().color().set(0,0,0,0);
        //Add a hyperlink and serialize the invisible polygon.
        WT_URL_Item  url(0, "http://www.autodesk.com/viewers", "Autodesk Viewers Page");
        whip_file.desired_rendition().url().add_url_optimized(url, whip_file);
        WD_CHECK(polygon->serialize(whip_file));
        whip_file.desired_rendition().url().clear();

        //Draw something on top of the invisible polygon.
        //Set the visibility and color back.
        whip_file.desired_rendition().visibility() = previous_visibility;
        whip_file.desired_rendition().color() = *previous_color;
        //Create a text object.
        whip_file.desired_rendition().font().height().set(25);
        WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x-75, y-12), WT_String("Hidden Link"));
        WD_CHECK(text->serialize(whip_file));

        classFactory->Destroy(text);
        classFactory->Destroy(polygon);
        classFactory->Destroy(previous_color);
    }

    classFactory->Destroy(p);
    classFactory->Destroy(text);
    classFactory->Destroy(named_view);
    classFactory->Destroy(layer);

    return WT_Result::Success;
}