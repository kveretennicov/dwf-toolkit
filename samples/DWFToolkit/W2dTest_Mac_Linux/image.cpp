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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/image.cpp#1 $

#ifdef _DWFCORE_WIN32_SYSTEM
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#ifndef LPVOID
#define LPVOID void*
#endif

void init_vector (WT_Logical_Point, WT_Logical_Point, WT_Logical_Point *);
WT_Result serialize_box(WT_Logical_Box& box, WT_File &whip_file);

WT_Result read_image(WT_Image & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result write_image(WT_File & whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

	WT_Layer *layer = classFactory->Create_Layer(whip_file, 5, WT_String("Images"));
    whip_file.desired_rendition().layer() = *layer;
	classFactory->Destroy(layer);

    WT_Logical_Point    min, max;

    int x = 1500;
    int y = 600;

    min.m_x = x;
    min.m_y = y-1200;
    max.m_x = x+1700;
    max.m_y = y;

    WT_Logical_Box       box(min, max);
    WT_Named_View *named_view = classFactory->Create_Named_View(box, "Images");
    WD_CHECK(named_view->serialize(whip_file));
    classFactory->Destroy( named_view );

    //24-bit JPEG
    whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
    whip_file.desired_rendition().line_weight() = 0;
    whip_file.desired_rendition().line_pattern() = WT_Line_Pattern::Solid;
    whip_file.desired_rendition().font().font_name().set("Arial");
    whip_file.desired_rendition().font().height().set(25);
    WT_Text *text = classFactory->Create_Text(WT_Logical_Point(x, y-40), WT_String("24-bit JPEG"));
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy( text );

    {
        if (0!=access("city24.jpg", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("city24.jpg", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("city24.jpg", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //city24.jpg: 367x454 96dpi 24bit
        WT_Image *image = classFactory->Create_Image(
                454,
                367,
                WT_Image::JPEG,
                1,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x, y-504),
                WT_Logical_Point(x+367, y-50),
                WD_False,
                96);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x, y-504), WT_Logical_Point(x+367, y-50));
        WD_CHECK(serialize_box(box, whip_file));
    }

    //8-bit JPEG
    text = classFactory->Create_Text(WT_Logical_Point(x+400, y-40), WT_String("8-bit JPEG"));
    WD_CHECK(text->serialize(whip_file));
	classFactory->Destroy(text);

    {
        if (0!=access("city8.jpg", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("city8.jpg", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("city8.jpg", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //city8.jpg: 367x454 96dpi 8bit depth
        WT_Image *image = classFactory->Create_Image(
                454,
                367,
                WT_Image::JPEG,
                2,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x+400, y-504),
                WT_Logical_Point(x+767, y-50),
                WD_False,
                96);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x+400, y-504), WT_Logical_Point(x+767, y-50));
        WD_CHECK(serialize_box(box, whip_file));

    }

    //32-bit PNG
    text = classFactory->Create_Text(WT_Logical_Point(x+800, y-40), WT_String("32-bit PNG"));
    WD_CHECK(text->serialize(whip_file));
	classFactory->Destroy(text);

    {
        if (0!=access("colors32.png", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("colors32.png", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("colors32.png", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //colors32.png: 252x232 96dpi 32bit depth
        WT_PNG_Group4_Image *image = classFactory->Create_PNG_Group4_Image(
                232,
                252,
                WT_PNG_Group4_Image::PNG,
                3,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x+800, y-282),
                WT_Logical_Point(x+1052, y-50),
                WD_False,
                96);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x+800, y-282), WT_Logical_Point(x+1052, y-50));
        WD_CHECK(serialize_box(box, whip_file));
    }

    //24-bit PNG
	text = classFactory->Create_Text(WT_Logical_Point(x+1100, y-40), WT_String("24-bit PNG"));
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy(text);

    {
        if (0!=access("colors24.png", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("colors24.png", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("colors24.png", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //colors24.png: 252x232 96dpi 16bit depth
        WT_PNG_Group4_Image *image = classFactory->Create_PNG_Group4_Image(
                232,
                252,
                WT_PNG_Group4_Image::PNG,
                3,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x+1100, y-282),
                WT_Logical_Point(x+1352, y-50),
                WD_False,
                96);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x+1100, y-282), WT_Logical_Point(x+1352, y-50));
        WD_CHECK(serialize_box(box, whip_file));
    }

    //8-bit PNG
    text = classFactory->Create_Text(WT_Logical_Point(x+1400, y-40), WT_String("8-bit PNG"));
    WD_CHECK(text->serialize(whip_file));
	classFactory->Destroy(text);

    {
        if (0!=access("colors8.png", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("colors8.png", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("colors8.png", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //colors16.png: 252x232 96dpi 24bit depth
        WT_PNG_Group4_Image *image = classFactory->Create_PNG_Group4_Image(
                232,
                252,
                WT_PNG_Group4_Image::PNG,
                3,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x+1400, y-282),
                WT_Logical_Point(x+1652, y-50),
                WD_False,
                96);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x+1400, y-282), WT_Logical_Point(x+1652, y-50));
        WD_CHECK(serialize_box(box, whip_file));
    }

    //Group4
    text = classFactory->Create_Text(WT_Logical_Point(x+800, y-800), WT_String("Group4 (bitonal TIFF)"));
    WD_CHECK(text->serialize(whip_file));
	classFactory->Destroy(text);
    {
        if (0!=access("g4_instrument.tif", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("g4_instrument.tif", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("g4_instrument.tif", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        //g4_instrument.tif: 3185x2276 1bit depth
        WT_PNG_Group4_Image *image = classFactory->Create_PNG_Group4_Image(
                2276,
                3185,
                WT_PNG_Group4_Image::Group4,
                4,
                NULL,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x+800, y-1038),
                WT_Logical_Point(x+1119, y-810),
                WD_False,
                300);

        WD_CHECK(image->serialize(whip_file));
		classFactory->Destroy(image);
        free(buf);

        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        WT_Logical_Box box(WT_Logical_Point(x+800, y-1038), WT_Logical_Point(x+1119, y-810));
        WD_CHECK(serialize_box(box, whip_file));
    }

    //Group4X Mapped
    text = classFactory->Create_Text(WT_Logical_Point(x, y-550), WT_String("Group4X_Mapped (bitonal), clipped via polygonal viewport"));
    WD_CHECK(text->serialize(whip_file));
    classFactory->Destroy(text);
    {
        if (0!=access("bitonal.png", 0))
            return WT_Result::No_File_Open_Error;
        
        struct stat st;
        if (0!=stat("bitonal.png", &st))
            return WT_Result::Unknown_File_Read_Error;

        LPVOID buf = malloc(st.st_size);
        if (buf==NULL)
            return WT_Result::Out_Of_Memory_Error;
     
        FILE *fp = fopen("bitonal.png", "rb");
        if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
        {
            fclose(fp);
            free(buf);
            return WT_Result::Unknown_File_Read_Error;
        }

        fclose(fp);

        WT_RGB black_white[2] = {WT_RGB(255,255,255), WT_RGB(0,0,0)};
        WT_Color_Map *color_map = classFactory->Create_Color_Map(2, black_white, whip_file);

        //bitonal.png: 3472x2727 96dpi 1bit depth
        WT_PNG_Group4_Image *image = classFactory->Create_PNG_Group4_Image(
                2727,
                3472,
                WT_PNG_Group4_Image::Group4X_Mapped,
                5,
                color_map,
                st.st_size,
                (WT_Byte*)buf,
                WT_Logical_Point(x, y-1120),
                WT_Logical_Point(x+694, y-575),
                WD_False,
                96);

        WT_Logical_Point vP[26];
        init_vector(WT_Logical_Point(x, y-1120), WT_Logical_Point(x+694, y-575), vP);

        WT_Attribute_URL *saveAttrUrl = classFactory->Create_Attribute_URL( whip_file.desired_rendition().attribute_url() );
        WT_Viewport *vport = classFactory->Create_Viewport( whip_file.desired_rendition().viewport() );
        WT_Integer32 contour_counts[2] = {21,5};
        WT_Contour_Set *contour = classFactory->Create_Contour_Set(whip_file, 2, contour_counts, 26, vP, WD_True);
        
	    WT_Viewport *viewport = classFactory->Create_Viewport(whip_file, "Image_Clip", *contour, WD_True);
        whip_file.desired_rendition().viewport() = *viewport;
        classFactory->Destroy( viewport );
        classFactory->Destroy( contour );
        
        WT_URL_Item oUrlItem1( "http://www.autodesk.com", "Autodesk" );
        WT_URL_Item oUrlItem2( "http://www.microsoft.com", "Microsoft" );
        WT_Attribute_URL *attrUrl = classFactory->Create_Attribute_URL( *vport );
        attrUrl->add_url_optimized( oUrlItem1, whip_file );
        attrUrl->add_url_optimized( oUrlItem2, whip_file );
        whip_file.desired_rendition().attribute_url() = *attrUrl;
        classFactory->Destroy( attrUrl );

        WD_CHECK(image->serialize(whip_file));
	    classFactory->Destroy(image);
	    classFactory->Destroy(color_map);
        free(buf);

        whip_file.desired_rendition().viewport() = *vport; //restore the viewport
        WT_Polyline *p = classFactory->Create_Polyline(21, vP, WD_True);
        whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
        WD_CHECK(p->serialize(whip_file));
        classFactory->Destroy(p);

        p = classFactory->Create_Polyline(5, &vP[21], WD_True);
        whip_file.desired_rendition().color().set(4, whip_file.desired_rendition().color_map());
        WD_CHECK(p->serialize(whip_file));
        classFactory->Destroy(p);

	    classFactory->Destroy(vport);

        whip_file.desired_rendition().attribute_url() = *saveAttrUrl;
	    classFactory->Destroy(saveAttrUrl);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (CTestFramework::Instance().GetClassFactoryType() != CTestFramework::XAML)
    {

        //Bitonal_Mapped 
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        text = classFactory->Create_Text(WT_Logical_Point(x+800, y-350), WT_String("Bitonal_Mapped (Group3X)"));
        WD_CHECK(text->serialize(whip_file));
	    classFactory->Destroy(text);
        {
            if (0!=access("fax.bit", 0))
                return WT_Result::No_File_Open_Error;
            
            struct stat st;
            if (0!=stat("fax.bit", &st))
                return WT_Result::Unknown_File_Read_Error;

            LPVOID buf = malloc(st.st_size);
            if (buf==NULL)
                return WT_Result::Out_Of_Memory_Error;
         
            FILE *fp = fopen("fax.bit", "rb");
            if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
            {
                fclose(fp);
                free(buf);
                return WT_Result::Unknown_File_Read_Error;
            }

            fclose(fp);

            WT_RGB black_white[2] = {WT_RGB(0,0,0), WT_RGB(255,255,255)};
            WT_Color_Map *color_map = classFactory->Create_Color_Map(2, black_white, whip_file);

            //fax.bit  712x928 1bit depth
            WT_Image *image = classFactory->Create_Image(
                    827,
                    712,
                    WT_Image::Bitonal_Mapped,
                    4,
                    color_map,
                    st.st_size,
                    (WT_Byte*)buf,
                    WT_Logical_Point(x+800, y-760),
                    WT_Logical_Point(x+1144, y-360),
                    WD_False);

            WD_CHECK(image->serialize(whip_file));
		    classFactory->Destroy(image);
		    classFactory->Destroy(color_map);
            free(buf);

            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            WT_Logical_Box box(WT_Logical_Point(x+800, y-360), WT_Logical_Point(x+1144, y-760));
            WD_CHECK(serialize_box(box, whip_file));

        }

        //RGB
        text = classFactory->Create_Text(WT_Logical_Point(x+1200, y-350), WT_String("RGB"));
        WD_CHECK(text->serialize(whip_file));
	    classFactory->Destroy(text);
        {
            if (0!=access("jeff.rgb", 0))
                return WT_Result::No_File_Open_Error;
            
            struct stat st;
            if (0!=stat("jeff.rgb", &st))
                return WT_Result::Unknown_File_Read_Error;

            LPVOID buf = malloc(st.st_size);
            if (buf==NULL)
                return WT_Result::Out_Of_Memory_Error;
         
            FILE *fp = fopen("jeff.rgb", "rb");
            if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
            {
                fclose(fp);
                free(buf);
                return WT_Result::Unknown_File_Read_Error;
            }

            fclose(fp);

            //jeff.rgb: 193x299 24bit depth
            WT_Image *image = classFactory->Create_Image(
                    299,
                    193,
                    WT_Image::RGB,
                    4,
                    NULL,
                    st.st_size,
                    (WT_Byte*)buf,
                    WT_Logical_Point(x+1200, y-659),
                    WT_Logical_Point(x+1393, y-360),
                    WD_False);

            WD_CHECK(image->serialize(whip_file));
		    classFactory->Destroy(image);
            free(buf);

            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            WT_Logical_Box box(WT_Logical_Point(x+1200, y-659), WT_Logical_Point(x+1393, y-360));
            WD_CHECK(serialize_box(box, whip_file));

        }

        //RGBA
        text = classFactory->Create_Text(WT_Logical_Point(x+1450, y-350), WT_String("RGBA"));
        WD_CHECK(text->serialize(whip_file));
	    classFactory->Destroy(text);
        {
            if (0!=access("jeff.rgba", 0))
                return WT_Result::No_File_Open_Error;
            
            struct stat st;
            if (0!=stat("jeff.rgba", &st))
                return WT_Result::Unknown_File_Read_Error;

            LPVOID buf = malloc(st.st_size);
            if (buf==NULL)
                return WT_Result::Out_Of_Memory_Error;
         
            FILE *fp = fopen("jeff.rgba", "rb");
            if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
            {
                fclose(fp);
                free(buf);
                return WT_Result::Unknown_File_Read_Error;
            }

            fclose(fp);

            //jeff.rgba: 193x299 32bit depth
            WT_Image *image = classFactory->Create_Image(
                    299,
                    193,
                    WT_Image::RGBA,
                    4,
                    NULL,
                    st.st_size,
                    (WT_Byte*)buf,
                    WT_Logical_Point(x+1450, y-659),
                    WT_Logical_Point(x+1643, y-360),
                    WD_False);

            WD_CHECK(image->serialize(whip_file));
		    classFactory->Destroy(image);
            free(buf);

            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            WT_Logical_Box box(WT_Logical_Point(x+1450, y-659), WT_Logical_Point(x+1643, y-360));
            WD_CHECK(serialize_box(box, whip_file));

        }


        //Mapped 
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        text = classFactory->Create_Text(WT_Logical_Point(x+1200, y-800), WT_String("Mapped"));
        WD_CHECK(text->serialize(whip_file));
	    classFactory->Destroy(text);

        //We'll use this in Indexed below, too
        WT_RGB pete_colors[256] = {
            WT_RGB(255,0,0),WT_RGB(251,0,0),WT_RGB(247,0,0),WT_RGB(243,0,0),WT_RGB(239,0,0),WT_RGB(215,0,0),WT_RGB(206,20,20),WT_RGB(198,24,24),
            WT_RGB(202,0,0),WT_RGB(223,0,0),WT_RGB(170,20,20),WT_RGB(138,28,28),WT_RGB(121,32,32),WT_RGB(210,0,0),WT_RGB(219,0,0),WT_RGB(182,36,36),
            WT_RGB(125,49,49),WT_RGB(85,53,53),WT_RGB(73,53,53),WT_RGB(73,40,40),WT_RGB(190,28,28),WT_RGB(227,0,0),WT_RGB(134,40,40),WT_RGB(117,85,85),
            WT_RGB(101,97,97),WT_RGB(85,85,85),WT_RGB(97,53,53),WT_RGB(231,0,0),WT_RGB(206,28,28),WT_RGB(158,53,53),WT_RGB(81,69,69),WT_RGB(138,57,57),
            WT_RGB(219,28,24),WT_RGB(174,53,53),WT_RGB(57,53,53),WT_RGB(97,69,69),WT_RGB(146,40,40),WT_RGB(206,77,77),WT_RGB(158,117,117),WT_RGB(121,69,69),
            WT_RGB(206,53,53),WT_RGB(178,93,93),WT_RGB(142,109,109),WT_RGB(65,65,65),WT_RGB(138,73,73),WT_RGB(198,32,32),WT_RGB(227,32,32),WT_RGB(194,89,89),
            WT_RGB(125,85,85),WT_RGB(134,130,130),WT_RGB(162,150,150),WT_RGB(170,109,109),WT_RGB(194,57,57),WT_RGB(227,20,20),WT_RGB(206,65,65),WT_RGB(174,125,125),
            WT_RGB(178,158,158),WT_RGB(154,150,150),WT_RGB(146,142,142),WT_RGB(158,158,158),WT_RGB(162,158,158),WT_RGB(243,20,20),WT_RGB(215,45,45),WT_RGB(190,101,101),
            WT_RGB(190,166,166),WT_RGB(190,186,186),WT_RGB(190,190,190),WT_RGB(178,178,178),WT_RGB(150,150,150),WT_RGB(170,166,166),WT_RGB(198,150,150),WT_RGB(219,117,117),
            WT_RGB(223,53,53),WT_RGB(235,20,20),WT_RGB(227,28,24),WT_RGB(210,93,93),WT_RGB(186,182,182),WT_RGB(198,194,194),WT_RGB(198,198,198),WT_RGB(202,202,202),
            WT_RGB(206,206,206),WT_RGB(206,198,198),WT_RGB(215,166,166),WT_RGB(223,125,125),WT_RGB(227,89,89),WT_RGB(235,61,61),WT_RGB(235,40,40),WT_RGB(243,32,32),
            WT_RGB(235,0,0),WT_RGB(215,65,65),WT_RGB(202,130,130),WT_RGB(190,174,174),WT_RGB(174,174,174),WT_RGB(194,194,194),WT_RGB(210,210,210),WT_RGB(202,198,198),
            WT_RGB(202,186,186),WT_RGB(206,178,178),WT_RGB(215,150,150),WT_RGB(231,146,146),WT_RGB(231,105,105),WT_RGB(235,89,89),WT_RGB(182,150,150),WT_RGB(182,166,166),
            WT_RGB(190,154,154),WT_RGB(215,198,198),WT_RGB(215,194,194),WT_RGB(223,186,186),WT_RGB(223,178,178),WT_RGB(235,53,53),WT_RGB(194,134,134),WT_RGB(170,97,97),
            WT_RGB(198,77,77),WT_RGB(194,69,69),WT_RGB(219,109,109),WT_RGB(210,130,130),WT_RGB(219,206,206),WT_RGB(215,215,215),WT_RGB(215,206,206),WT_RGB(215,85,85),
            WT_RGB(174,150,150),WT_RGB(162,105,105),WT_RGB(162,65,65),WT_RGB(190,20,20),WT_RGB(206,32,32),WT_RGB(206,138,138),WT_RGB(206,170,170),WT_RGB(210,206,206),
            WT_RGB(202,105,105),WT_RGB(215,32,32),WT_RGB(219,20,20),WT_RGB(142,121,121),WT_RGB(134,93,93),WT_RGB(182,69,69),WT_RGB(215,138,138),WT_RGB(219,219,219),
            WT_RGB(194,186,186),WT_RGB(170,0,0),WT_RGB(121,109,109),WT_RGB(97,85,85),WT_RGB(198,40,40),WT_RGB(231,77,77),WT_RGB(223,154,154),WT_RGB(194,49,49),
            WT_RGB(198,0,0),WT_RGB(125,24,24),WT_RGB(235,28,24),WT_RGB(215,186,186),WT_RGB(215,158,158),WT_RGB(158,0,0),WT_RGB(125,97,97),WT_RGB(235,32,32),
            WT_RGB(190,0,0),WT_RGB(109,28,28),WT_RGB(178,113,113),WT_RGB(215,178,178),WT_RGB(227,105,105),WT_RGB(150,0,0),WT_RGB(210,109,109),WT_RGB(210,190,190),
            WT_RGB(210,117,117),WT_RGB(223,40,40),WT_RGB(182,0,0),WT_RGB(146,130,130),WT_RGB(231,113,113),WT_RGB(142,0,0),WT_RGB(158,97,97),WT_RGB(174,65,65),
            WT_RGB(174,0,0),WT_RGB(251,20,20),WT_RGB(239,73,73),WT_RGB(170,89,89),WT_RGB(206,0,0),WT_RGB(138,20,20),WT_RGB(146,93,93),WT_RGB(178,142,142),
            WT_RGB(231,154,154),WT_RGB(97,28,28),WT_RGB(154,24,24),WT_RGB(146,24,24),WT_RGB(158,142,142),WT_RGB(61,45,45),WT_RGB(85,40,40),WT_RGB(109,49,49),
            WT_RGB(182,20,20),WT_RGB(170,158,158),WT_RGB(235,97,97),WT_RGB(223,166,166),WT_RGB(40,40,40),WT_RGB(49,49,49),WT_RGB(53,49,49),WT_RGB(154,81,81),
            WT_RGB(247,28,20),WT_RGB(158,130,130),WT_RGB(202,85,85),WT_RGB(227,117,117),WT_RGB(142,142,142),WT_RGB(57,57,57),WT_RGB(206,40,40),WT_RGB(231,130,130),
            WT_RGB(162,77,77),WT_RGB(219,215,215),WT_RGB(81,81,81),WT_RGB(178,85,85),WT_RGB(194,0,0),WT_RGB(166,45,45),WT_RGB(194,117,117),WT_RGB(231,138,138),
            WT_RGB(223,223,223),WT_RGB(93,93,93),WT_RGB(105,105,105),WT_RGB(73,73,73),WT_RGB(121,121,121),WT_RGB(170,73,77),WT_RGB(154,0,20),WT_RGB(223,73,73),
            WT_RGB(219,93,93),WT_RGB(227,227,227),WT_RGB(182,182,182),WT_RGB(154,0,0),WT_RGB(109,109,109),WT_RGB(223,65,65),WT_RGB(113,113,113),WT_RGB(138,138,138),
            WT_RGB(166,166,166),WT_RGB(182,57,57),WT_RGB(170,36,36),WT_RGB(182,178,178),WT_RGB(130,113,113),WT_RGB(138,85,85),WT_RGB(227,170,170),WT_RGB(223,146,146),
            WT_RGB(178,0,0),WT_RGB(227,97,97),WT_RGB(130,125,125),WT_RGB(223,138,138),WT_RGB(186,49,49),WT_RGB(231,231,231),WT_RGB(223,219,219),WT_RGB(227,210,210),
            WT_RGB(223,198,198),WT_RGB(166,0,0),WT_RGB(162,0,0),WT_RGB(146,0,0),WT_RGB(186,0,0),WT_RGB(227,219,219),WT_RGB(134,0,0),WT_RGB(231,178,178),
            WT_RGB(227,190,190),WT_RGB(190,36,36),WT_RGB(227,198,198),WT_RGB(231,227,227),WT_RGB(235,235,235),WT_RGB(125,0,0),WT_RGB(105,0,0),WT_RGB(109,97,97)
        };
        
        WT_Color_Map *pete_map =
		    classFactory->Create_Color_Map(256, pete_colors, whip_file);

        {
            if (0!=access("pete.256", 0))
                return WT_Result::No_File_Open_Error;
            
            struct stat st;
            if (0!=stat("pete.256", &st))
                return WT_Result::Unknown_File_Read_Error;

            LPVOID buf = malloc(st.st_size);
            if (buf==NULL)
                return WT_Result::Out_Of_Memory_Error;
         
            FILE *fp = fopen("pete.256", "rb");
            if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
            {
                fclose(fp);
                free(buf);
                return WT_Result::Unknown_File_Read_Error;
            }

            fclose(fp);

            //pete.256  170x184 8bit depth (mapped)
            WT_Image *image = classFactory->Create_Image(
                    184,
                    170,
                    WT_Image::Mapped,
                    4,
                    pete_map,
                    st.st_size,
                    (WT_Byte*)buf,
                    WT_Logical_Point(x+1200, y-994),
                    WT_Logical_Point(x+1370, y-810),
                    WD_False);

            WD_CHECK(image->serialize(whip_file));
		    classFactory->Destroy(image);
            free(buf);

            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            WT_Logical_Box box(WT_Logical_Point(x+1200, y-810), WT_Logical_Point(x+1370, y-994));
            WD_CHECK(serialize_box(box, whip_file));

        }

        //Indexed
        whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
        text = classFactory->Create_Text(WT_Logical_Point(x+1450, y-800), WT_String("Indexed"));
        WD_CHECK(text->serialize(whip_file));
	    classFactory->Destroy(text);

        {
            if (0!=access("pete.256", 0))
                return WT_Result::No_File_Open_Error;
            
            struct stat st;
            if (0!=stat("pete.256", &st))
                return WT_Result::Unknown_File_Read_Error;

            LPVOID buf = malloc(st.st_size);
            if (buf==NULL)
                return WT_Result::Out_Of_Memory_Error;
         
            FILE *fp = fopen("pete.256", "rb");
            if (st.st_size != (long)fread(buf, 1, st.st_size, fp))
            {
                fclose(fp);
                free(buf);
                return WT_Result::Unknown_File_Read_Error;
            }

            fclose(fp);

            WT_Color_Map *default_color_map = classFactory->Create_Color_Map( whip_file.desired_rendition().color_map() );
            whip_file.desired_rendition().color_map() = *pete_map;

            //pete.256  170x184 8bit depth (indexed)
            WT_Image *image = classFactory->Create_Image(
                    184,
                    170,
                    WT_Image::Indexed,
                    4,
                    NULL,
                    st.st_size,
                    (WT_Byte*)buf,
                    WT_Logical_Point(x+1450, y-994),
                    WT_Logical_Point(x+1620, y-810),
                    WD_False);

            WD_CHECK(image->serialize(whip_file));
		    classFactory->Destroy(image);
            free(buf);

            whip_file.desired_rendition().color_map() = *default_color_map;
		    classFactory->Destroy(default_color_map);

            whip_file.desired_rendition().color().set(0, whip_file.desired_rendition().color_map());
            WT_Logical_Box box(WT_Logical_Point(x+1450, y-810), WT_Logical_Point(x+1620, y-994));
            WD_CHECK(serialize_box(box, whip_file));
        }

	    classFactory->Destroy(pete_map);
    }

    return WT_Result::Success;
}

void init_vector (WT_Logical_Point min, WT_Logical_Point max, WT_Logical_Point * vP)
{
    int x = min.m_x;
    int y = max.m_y;
    int deltaX = (max.m_x - min.m_x) / 9;
    int deltaY = (max.m_y - min.m_y) / 7;

    vP[0].m_x = x + deltaX;  //positive wound viewport
    vP[0].m_y = y;
    vP[1].m_x = x + deltaX*4;
    vP[1].m_y = y;
    vP[2].m_x = x + (int)(deltaX*4.5);
    vP[2].m_y = y - deltaY;
    vP[3].m_x = x + deltaX*5;
    vP[3].m_y = y;
    vP[4].m_x = x + deltaX*8;
    vP[4].m_y = y;
    vP[5].m_x = x + deltaX*9;
    vP[5].m_y = y - deltaY;
    vP[6].m_x = x + deltaX*9;
    vP[6].m_y = y - deltaY*3;
    vP[7].m_x = x + deltaX*8;
    vP[7].m_y = y - (int)(deltaY*3.5);
    vP[8].m_x = x + deltaX*9;
    vP[8].m_y = y - deltaY*4;
    vP[9].m_x = x + deltaX*9;
    vP[9].m_y = y - deltaY*6;
    vP[10].m_x = x + deltaX*8;
    vP[10].m_y = y - deltaY*7;
    vP[11].m_x = x + deltaX*5;
    vP[11].m_y = y - deltaY*7;
    vP[12].m_x = x + (int)(deltaX*4.5);
    vP[12].m_y = y - deltaY*6;
    vP[13].m_x = x + deltaX*4;
    vP[13].m_y = y - deltaY*7;
    vP[14].m_x = x + deltaX;
    vP[14].m_y = y - deltaY*7;
    vP[15].m_x = x;
    vP[15].m_y = y - deltaY*6;
    vP[16].m_x = x;
    vP[16].m_y = y - deltaY*4;
    vP[17].m_x = x + deltaX;
    vP[17].m_y = y - (int)(deltaY*3.5);
    vP[18].m_x = x;
    vP[18].m_y = y - deltaY*3;
    vP[19].m_x = x;
    vP[19].m_y = y - deltaY;
    vP[20].m_x = vP[0].m_x;
    vP[20].m_y = vP[0].m_y;
    vP[21].m_x = x + deltaX*3; //negative wound hole
    vP[21].m_y = y - deltaY*2;
    vP[22].m_x = x + deltaX*3;
    vP[22].m_y = y - deltaY*5;
    vP[23].m_x = x + deltaX*6;
    vP[23].m_y = y - deltaY*5;
    vP[24].m_x = x + deltaX*6;
    vP[24].m_y = y - deltaY*2;
    vP[25].m_x = vP[21].m_x; 
    vP[25].m_y = vP[21].m_y;
}

WT_Result write_png_group4_image(WT_File & /*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result read_png_group4_image(WT_PNG_Group4_Image & /*text*/, WT_File &/*whip_file*/)
{
    return WT_Result::Success;
}

WT_Result serialize_box(WT_Logical_Box& box, WT_File &whip_file)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    WT_Logical_Point p[5] = {
        box.m_min, 
        WT_Logical_Point(box.m_min.m_x, box.m_max.m_y),
        box.m_max,
        WT_Logical_Point(box.m_max.m_x, box.m_min.m_y),
        box.m_min 
    };

    WT_Polyline *pline = classFactory->Create_Polyline(5, p, WD_True);
	WD_CHECK( result = pline->serialize(whip_file) );
	classFactory->Destroy(pline);

    return WT_Result::Success;
}