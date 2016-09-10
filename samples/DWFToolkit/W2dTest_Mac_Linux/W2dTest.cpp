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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/W2dTest_Mac_Linux/W2dTest.cpp#1 $

// W2dTest.cpp : Defines the entry point for the console application.
//
#include "StdAfx.h"
#if defined(_DWFCORE_WIN32_SYSTEM)
#include <io.h>
#include <direct.h>
#endif
#include "whiptk/whip_toolkit.h"
#include "object_list.h"
#include "test_list.h"
#include "W2dTest.h"
#include "OpcHelper.h"
#include "TestFramework.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/BufferInputStream.h"
#include "XAML/XamlFile.h"

extern WT_Result 
getSubsetFontName( 
				  WT_String& zFontName, 
				  WT_Font& oFont,
				  const WT_String &zStr, 
				  bool bObfuscate );

//TODO: define additional object tests in test_list.h

//define the extern functions for each test
#if defined(__GNUC__) && (__GNUC__ > 3)
#define TEST_MACRO(class_lower, class_upper) \
extern WT_Result write_##class_lower (WT_File &); \
extern WT_Result read_##class_lower (WT_##class_upper &, WT_File &);

#else
#define TEST_MACRO(class_lower, class_upper)              \
extern WT_Result write_##class_lower##(WT_File &);        \
extern WT_Result read_##class_lower##(WT_##class_upper## &, WT_File &);
#endif

TEST_LIST
TEST_LIST_1
TEST_LIST_2
TEST_LIST_3
#undef TEST_MACRO

#ifdef _DWFCORE_WIN32_SYSTEM
#define TEST_MACRO(class_lower, class_upper) \
OutputDebugStringA(#class_lower);\
OutputDebugStringA("\n");\
if (WT_Result::Success != write_##class_lower##(*whip_file)) {\
OutputDebugStringA("ERROR in write_");\
OutputDebugStringA(#class_lower);\
OutputDebugStringA(" test!\n");\
break; }

#else
#define TEST_MACRO(class_lower, class_upper) \
wcout << #class_lower << endl; \
if (WT_Result::Success != write_##class_lower (*whip_file)) {\
wcout << "Error in write_";\
wcout << #class_lower << endl;\
break;}

#endif

#if defined(W2DTEST_APP)
OpcHelper gHelper;
#else
extern OpcHelper gHelper;
#endif

void write_test(bool binary, bool compression, const wchar_t* filename, int testList)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
	
    wcout << "\n_______________________________________WRITE TEST\n";
	
    //WRITE TEST
	WT_File *whip_file = classFactory->Create_File();
    if (whip_file == NULL)
    {
        return;
    }
	
    whip_file->set_file_mode (WT_File::File_Write);
    //Set a different target version here if you don't want a W2D file... for example:
    //whip_file.heuristics().set_target_version(42); //produce a DWF ver 00.42
	
    //Initialize and open the file
    switch( CTestFramework::Instance().GetClassFactoryType() )
    {
        case CTestFramework::XAML:
        {
            WT_XAML_File *pxFile = static_cast<WT_XAML_File *>(whip_file);
			
            DWFString oXamlFilebase( filename );
            DWFString oXamlFilename = oXamlFilebase;
            DWFString oW2xFilename = oXamlFilebase;
            oXamlFilename.append( L"_xaml.xaml" );
            oW2xFilename.append( L"_w2x.xml" );
	        DWFFile oXamlFile( oXamlFilename );
	        DWFFile oW2xFile( oW2xFilename );
			
            DWFStreamFileDescriptor *pXamlFileDescriptor = 
			DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oXamlFile, L"wb" ) );
            ERR_NULLCHECK( pXamlFileDescriptor );
			
            DWFStreamFileDescriptor *pW2xFileDescriptor = 
			DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oW2xFile, L"wb" ) );
            ERR_NULLCHECK( pW2xFileDescriptor );
			
	        DWFFileOutputStream *pXamlFileStream = DWFCORE_ALLOC_OBJECT(DWFFileOutputStream);
            ERR_NULLCHECK( pXamlFileStream );
	        pXamlFileDescriptor->open();
	        pXamlFileStream->attach(pXamlFileDescriptor, true);
			
            DWFFileOutputStream *pW2xFileStream = DWFCORE_ALLOC_OBJECT(DWFFileOutputStream);
            ERR_NULLCHECK( pW2xFileStream );
	        pW2xFileDescriptor->open();
	        pW2xFileStream->attach(pW2xFileDescriptor, true);
			
            //Initialize the file object
            pxFile->xamlStreamOut() = pXamlFileStream;
            pxFile->w2xStreamOut() = pW2xFileStream;
            pxFile->opcResourceSerializer() = &gHelper;
            pxFile->opcResourceMaterializer() = &gHelper;
			
            pxFile->obfuscate_embedded_fonts() = false;
			
            WT_Matrix2D oTransform;
            double fWidth = 9000;  //this is the width from -4000 to 5000
            double fHeight = 6000; //this is the height from -4000 to 2000
            double fScale = 0.00222;  //to scale 9000 into inches 
            oTransform(2,0) = 2500.;
            oTransform(2,1) = 4700.;
			
            ERR_CHECK( pxFile->definePageLayout(oTransform, fWidth, fHeight, fScale) );
            ERR_CHECK( whip_file->open() );
            break;
        }
        case CTestFramework::W2D:
        {
			WT_String zFileName(filename);
            char buf[128];
            sprintf(buf, "File:%-10s Compression:%-5s Binary:%-5s\n\n", zFileName.ascii(), compression?"true":"false", binary?"true":"false");
            wcout << buf;
			whip_file->set_filename (zFileName.length(), zFileName.unicode());
            switch (testList)
            {
				case 0: // Page 1
					whip_file->heuristics().set_target_version(600);
					break;
				default:
					whip_file->heuristics().set_target_version(601);
					break;
            }
            ERR_CHECK( whip_file->open() );
            whip_file->heuristics().set_allow_binary_data(binary);
            whip_file->heuristics().set_allow_data_compression(compression);
            break;
        }
    }
	
    // set foreground color
    whip_file->desired_rendition().color().set(0,0,0);
	
    // set the initial view
    // Register the fonts we need:
    if ( CTestFramework::Instance().GetClassFactoryType() == CTestFramework::XAML )
    {
        //Test characters for ANSI, Japanese, Arabic, and extended Latin
        WT_String zSrc(L"().,0123456789-=_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz \x30A4\x30B4\x30C4\x30D4 \x0682\x0692\x06A2\x06B2 \x0105\x0115\x0125\x0135" );
        WT_String zFontName(L"Arial");
        WT_String zFontPath( zFontName );
        WT_Font *pFont = classFactory->Create_Font();
        ERR_NULLCHECK( pFont );
        pFont->height().set( 20 ); // an arbitrary default
        pFont->font_name().set( zFontName );
        pFont->pitch().set( WT_Font_Option_Pitch::PITCH_VARIABLE );
        pFont->family().set( WT_Font_Option_Family::FAMILY_SWISS );
        pFont->charset().set( WT_Font_Option_Charset::CHARSET_UNICODE );
        ERR_CHECK( getSubsetFontName( zFontPath, *pFont, zSrc, false ) );
        wchar_t *zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        wchar_t *zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;
		
        zFontName = L"Times New Roman";
        zFontPath = zFontName;
        pFont->font_name().set( zFontName );
        pFont->family().set( WT_Font_Option_Family::FAMILY_ROMAN );
        ERR_CHECK( getSubsetFontName( zFontPath, *pFont, zSrc, false ) );
        zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;
		
        zFontName =  L"Courier New";
        zFontPath = zFontName;
        pFont->font_name().set( zFontName );
        pFont->pitch().set( WT_Font_Option_Pitch::PITCH_FIXED );
        pFont->family().set( WT_Font_Option_Family::FAMILY_UNKNOWN );
        ERR_CHECK( getSubsetFontName( zFontPath, *pFont, zSrc, false ) );
        zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;
        classFactory->Destroy( pFont );
    }
	
    switch (testList)
    {
		case 0:
        {
            //WT_View *view = classFactory->Create_View(WT_Logical_Box(-4000,-4000,5000,2000));
            //ERR_CHECK( view->serialize(*whip_file) );
            //classFactory->Destroy(view);
            TEST_LIST
        }break;
		case 1:
        {
            TEST_LIST_1
        }break;
		case 2:
        {
            TEST_LIST_2
        } break;
		case 3:
        {
            TEST_LIST_3
        } break;
			
    }
    ERR_CHECK( whip_file->close() );
	
	classFactory->Destroy(whip_file);
}

#undef TEST_MACRO

#if defined(__GNUC__) && (__GNUC__ > 3)
#define TEST_MACRO(class_lower, class_upper) \
whip_file->set_##class_lower##_action(read_##class_lower);
#else
#define TEST_MACRO(class_lower, class_upper) \
whip_file->set_##class_lower##_action(read_##class_lower##);
#endif

#define OBJECT_MACRO(object_name) \
id_count++; \
id = (char**) realloc(id, sizeof(char*) * id_count); \
id[id_count-1] = (char*) malloc(strlen(#object_name)+1); \
sprintf(id[id_count-1], "%s\n", #object_name);


#ifdef W2DTEST_APP

void read_test(const wchar_t *filename, int testList)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
	
    wcout << "\n_______________________________________READ TEST\n";
	WT_String zFileName(filename);
    cout << L"File: \n" << zFileName.ascii() << endl;
	
    WT_File *whip_file = classFactory->Create_File();
	
    //READ TEST
	
    whip_file->set_file_mode (WT_File::File_Read);
	
    //Initialize and open the file
    switch( CTestFramework::Instance().GetClassFactoryType() )
    {
        case CTestFramework::XAML:
        {
            WT_XAML_File *pxFile = static_cast<WT_XAML_File *>(whip_file);
			
            DWFString oXamlFilebase( filename );
            DWFString oXamlFilename = oXamlFilebase;
            DWFString oW2xFilename = oXamlFilebase;
            oXamlFilename.append( L"_xaml.xaml" );
            oW2xFilename.append( L"_w2x.xml" );
	        DWFFile oXamlFile( oXamlFilename );
	        DWFFile oW2xFile( oW2xFilename );
			
            DWFStreamFileDescriptor *pXamlFileDescriptor = 
			DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oXamlFile, L"rb" ) );
            ERR_NULLCHECK( pXamlFileDescriptor );
			
            DWFStreamFileDescriptor *pW2xFileDescriptor = 
			DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( oW2xFile, L"rb" ) );
            ERR_NULLCHECK( pW2xFileDescriptor );
			
	        DWFFileInputStream *pXamlFileStream = DWFCORE_ALLOC_OBJECT(DWFFileInputStream);
            ERR_NULLCHECK( pXamlFileStream );
	        pXamlFileDescriptor->open();
	        pXamlFileStream->attach(pXamlFileDescriptor, true);
			
            DWFFileInputStream *pW2xFileStream = DWFCORE_ALLOC_OBJECT(DWFFileInputStream);
            ERR_NULLCHECK( pW2xFileStream );
	        pW2xFileDescriptor->open();
	        pW2xFileStream->attach(pW2xFileDescriptor, true);
			
            //Initialize the file object
            pxFile->xamlStreamIn() = pXamlFileStream;
            pxFile->w2xStreamIn() = pW2xFileStream;
            pxFile->opcResourceMaterializer() = &gHelper;
			
            WT_Matrix2D oTransform;
            double fWidth = 9000;  //this is the width from -4000 to 5000
            double fHeight = 6000; //this is the height from -4000 to 2000
            double fScale = 0.00222;  //to scale 9000 into inches 
            oTransform(2,0) = 2500.;
            oTransform(2,1) = 4700.;
			
            ERR_CHECK( pxFile->definePageLayout(oTransform, fWidth, fHeight, fScale) );
			
            ERR_CHECK( whip_file->open() );
            break;
        }
        case CTestFramework::W2D:
        {            
			WT_String zFileName(filename);
			whip_file->set_filename (zFileName.length(),zFileName.unicode());
            whip_file->heuristics().set_deferred_delete( WD_True );
            ERR_CHECK( whip_file->open() );
            break;
        }
        default:
            break;
    }
	
    switch (testList)
    {
		case 0: // Page 1
        {
            TEST_LIST
        }break;
		case 1: // Page 2
        {
            TEST_LIST_1
        }break;
		case 2: // Page 3 
        {
            TEST_LIST_2
        } break;
		case 3: // page 4 
        {
            TEST_LIST_3
        } break;
    }
	
    WT_Result result;
	
    do {
        result = whip_file->process_next_object();
#ifdef _DEBUG
#ifdef _WIN32
		WT_Object* pObj = whip_file->object();
        if ((result == WT_Result::Success || result == WT_Result::End_Of_DWF_Opcode_Found)
            && pObj != NULL )
        {
            char buf[1024];
            const char *pDescription = NULL;
            switch ( pObj->object_id() )
            {
				case WT_Object::Adapt_Line_Patterns_ID: pDescription = "WT_Adapt_Line_Patterns"; break;
				case WT_Object::Author_ID: pDescription = "WT_Author"; break;
				case WT_Object::Background_ID: pDescription = "WT_Background"; break;
				case WT_Object::Code_Page_ID: pDescription = "WT_Code_Page"; break;
				case WT_Object::Color_ID: pDescription = "WT_Color"; break;
				case WT_Object::Color_Map_ID: pDescription = "WT_Color_Map"; break;
				case WT_Object::Comments_ID: pDescription = "WT_Comments"; break;
				case WT_Object::Compressed_Data_Moniker_ID: pDescription = "WT_Compressed_Data_Moniker"; break;
				case WT_Object::Contour_Set_ID: pDescription = "WT_Contour_Set"; break;
				case WT_Object::Copyright_ID: pDescription = "WT_Copyright"; break;
				case WT_Object::Creator_ID: pDescription = "WT_Creator"; break;
				case WT_Object::Creation_Time_ID: pDescription = "WT_Creation_Time"; break;
				case WT_Object::Dash_Pattern_ID: pDescription = "WT_Dash_Pattern"; break;
				case WT_Object::Description_ID: pDescription = "WT_Description"; break;
				case WT_Object::Drawing_Info_ID: pDescription = "WT_Drawing_Info"; break;
				case WT_Object::DWF_Header_ID: pDescription = "WT_DWF_Header"; break;
				case WT_Object::Embed_ID: pDescription = "WT_Embed"; break;
				case WT_Object::Embedded_Font_ID: pDescription = "WT_Embedded_Font"; break;
				case WT_Object::End_Of_DWF_ID: pDescription = "WT_End_Of_DWF"; break;
				case WT_Object::Fill_ID: pDescription = "WT_Fill"; break;
				case WT_Object::Fill_Pattern_ID: pDescription = "WT_Fill_Pattern"; break;
				case WT_Object::Filled_Ellipse_ID: pDescription = "WT_Filled_Ellipse"; break;
				case WT_Object::Gouraud_Polyline_ID: pDescription = "WT_Gouraud_Polyline"; break;
				case WT_Object::Gouraud_Polytriangle_ID: pDescription = "WT_Gouraud_Polytriangle"; break;
				case WT_Object::Image_ID: pDescription = "WT_Image"; break;
				case WT_Object::Inked_Area_ID: pDescription = "WT_Inked_Area"; break;
				case WT_Object::Keywords_ID: pDescription = "WT_Keywords"; break;
				case WT_Object::Layer_ID: pDescription = "WT_Layer"; break;
				case WT_Object::Layer_List_ID: pDescription = "WT_Layer_List"; break;
				case WT_Object::Line_Caps_ID: pDescription = "WT_Line_Caps"; break;
				case WT_Object::Line_Join_ID: pDescription = "WT_Line_Join"; break;
				case WT_Object::Line_Pattern_ID: pDescription = "WT_Line_Pattern"; break;
				case WT_Object::Line_Style_ID: pDescription = "WT_Line_Style"; break;
				case WT_Object::Line_Pattern_Scale_ID: pDescription = "WT_Line_Pattern_Scale"; break;
				case WT_Object::Line_Weight_ID: pDescription = "WT_Line_Weight"; break;
				case WT_Object::Marker_Size_ID: pDescription = "WT_Marker_Size"; break;
				case WT_Object::Marker_Symbol_ID: pDescription = "WT_Marker_Symbol"; break;
				case WT_Object::Merge_Control_ID: pDescription = "WT_Merge_Control"; break;
				case WT_Object::Miter_Angle_ID: pDescription = "WT_Miter_Angle"; break;
				case WT_Object::Miter_Length_ID: pDescription = "WT_Miter_Length"; break;
				case WT_Object::Modification_Time_ID: pDescription = "WT_Modification_Time"; break;
				case WT_Object::Named_View_ID: pDescription = "WT_Named_View"; break;
				case WT_Object::Named_View_List_ID: pDescription = "WT_Named_View_List"; break;
				case WT_Object::Trusted_Font_List_ID: pDescription = "WT_Trusted_Font_List"; break;
				case WT_Object::Object_Node_ID: pDescription = "WT_Object_Node"; break;
				case WT_Object::Object_Node_List_ID: pDescription = "WT_Object_Node_List"; break;
				case WT_Object::Option_ID: pDescription = "WT_Option"; break;
				case WT_Object::Origin_ID: pDescription = "WT_Origin"; break;
				case WT_Object::Outline_Ellipse_ID: pDescription = "WT_Outline_Ellipse"; break;
				case WT_Object::Plot_Info_ID: pDescription = "WT_Plot_Info"; break;
				case WT_Object::Polygon_ID: pDescription = "WT_Polygon"; break;
				case WT_Object::Polyline_ID: pDescription = "WT_Polyline"; break;
				case WT_Object::Polymarker_ID: pDescription = "WT_Polymarker"; break;
				case WT_Object::Polytriangle_ID: pDescription = "WT_Polytriangle"; break;
				case WT_Object::Projection_ID: pDescription = "WT_Projection"; break;
				case WT_Object::Source_Creation_Time_ID: pDescription = "WT_Source_Creation_Time"; break;
				case WT_Object::Source_Filename_ID: pDescription = "WT_Source_Filename"; break;
				case WT_Object::Source_Modification_Time_ID: pDescription = "WT_Source_Modification_Time"; break;
				case WT_Object::Subject_ID: pDescription = "WT_Subject"; break;
				case WT_Object::Text_ID: pDescription = "WT_Text"; break;
				case WT_Object::Title_ID: pDescription = "WT_Title"; break;
				case WT_Object::Units_ID: pDescription = "WT_Units"; break;
				case WT_Object::URL_ID: pDescription = "WT_URL"; break;
				case WT_Object::URL_List_ID: pDescription = "WT_URL_List"; break;
				case WT_Object::View_ID: pDescription = "WT_View"; break;
				case WT_Object::Viewport_ID: pDescription = "WT_Viewport"; break;
				case WT_Object::Viewport_Option_Viewport_Units_ID: pDescription = "WT_Viewport_Option_Viewport_Units"; break;
				case WT_Object::Visibility_ID: pDescription = "WT_Visibility"; break;
				case WT_Object::Font_ID: pDescription = "WT_Font"; break;
				case WT_Object::Font_Option_Font_Name_ID: pDescription = "WT_Font_Option_Font_Name"; break;
				case WT_Object::Font_Option_Charset_ID: pDescription = "WT_Font_Option_Charset"; break;
				case WT_Object::Font_Option_Pitch_ID: pDescription = "WT_Font_Option_Pitch"; break;
				case WT_Object::Font_Option_Family_ID: pDescription = "WT_Font_Option_Family"; break;
				case WT_Object::Font_Option_Style_ID: pDescription = "WT_Font_Option_Style"; break;
				case WT_Object::Font_Option_Height_ID: pDescription = "WT_Font_Option_Height"; break;
				case WT_Object::Font_Option_Rotation_ID: pDescription = "WT_Font_Option_Rotation"; break;
				case WT_Object::Font_Option_Width_Scale_ID: pDescription = "WT_Font_Option_Width_Scale"; break;
				case WT_Object::Font_Option_Spacing_ID: pDescription = "WT_Font_Option_Spacing"; break;
				case WT_Object::Font_Option_Oblique_ID: pDescription = "WT_Font_Option_Oblique"; break;
				case WT_Object::Font_Option_Flags_ID: pDescription = "WT_Font_Option_Flags"; break;
				case WT_Object::Text_Option_Overscore_ID: pDescription = "WT_Text_Option_Overscore"; break;
				case WT_Object::Text_Option_Underscore_ID: pDescription = "WT_Text_Option_Underscore"; break;
				case WT_Object::Text_Option_Bounds_ID: pDescription = "WT_Text_Option_Bounds"; break;
				case WT_Object::Text_Option_Reserved_ID: pDescription = "WT_Text_Option_Reserved"; break;
				case WT_Object::PNG_Group4_Image_ID: pDescription = "WT_PNG_Group4_Image"; break;
				case WT_Object::Optimized_For_Plot_ID: pDescription = "WT_Optimized_For_Plot"; break;
				case WT_Object::Group_Begin_ID: pDescription = "WT_Group_Begin"; break;
				case WT_Object::Group_End_ID: pDescription = "WT_Group_End"; break;
				case WT_Object::Block_Meaning_ID: pDescription = "WT_Block_Meaning"; break;
				case WT_Object::Encryption_ID: pDescription = "WT_Encryption"; break;
				case WT_Object::Orientation_ID: pDescription = "WT_Orientation"; break;
				case WT_Object::Alignment_ID: pDescription = "WT_Alignment"; break;
				case WT_Object::Password_ID: pDescription = "WT_Password"; break;
				case WT_Object::Guid_ID: pDescription = "WT_Guid"; break;
				case WT_Object::FileTime_ID: pDescription = "WT_FileTime"; break;
				case WT_Object::BlockRef_ID: pDescription = "WT_BlockRef"; break;
				case WT_Object::Directory_ID: pDescription = "WT_Directory"; break;
				case WT_Object::UserData_ID: pDescription = "WT_UserData"; break;
				case WT_Object::Pen_Pattern_ID: pDescription = "WT_Pen_Pattern"; break;
				case WT_Object::SignData_ID: pDescription = "WT_SignData"; break;
				case WT_Object::Guid_List_ID: pDescription = "WT_Guid_List"; break;
				case WT_Object::Font_Extension_ID: pDescription = "WT_Font_Extension"; break;
				case WT_Object::PenPat_Options_ID: pDescription = "WT_PenPat_Options"; break;
				case WT_Object::Macro_Definition_ID: pDescription = "WT_Macro_Definition"; break;
				case WT_Object::Macro_Draw_ID: pDescription = "WT_Macro_Draw"; break;
				case WT_Object::Macro_Scale_ID: pDescription = "WT_Macro_Scale"; break;
				case WT_Object::Macro_Index_ID: pDescription = "WT_Macro_Index"; break;
				case WT_Object::Text_Background_ID: pDescription = "WT_Text_Background"; break;
				case WT_Object::Text_HAlign_ID: pDescription = "WT_Text_HAlign"; break;
				case WT_Object::Text_VAlign_ID: pDescription = "WT_Text_VAlign"; break;
				case WT_Object::Overpost_ID: pDescription = "WT_Overpost"; break;
				case WT_Object::Delineate_ID: pDescription = "WT_Delineate"; break;
				case WT_Object::User_Fill_Pattern_ID: pDescription = "WT_User_Fill_Pattern"; break;
				case WT_Object::User_Hatch_Pattern_ID: pDescription = "WT_User_Hatch_Pattern"; break;
				case WT_Object::Contrast_Color_ID: pDescription = "WT_Contrast_Color"; break;
				case WT_Object::Attribute_URL_ID: pDescription = "WT_Attribute_URL"; break;
            }
			
            WD_Assert( pDescription != NULL);
            sprintf( buf, "APPLICATION: %c %s\n", pObj->materialized() ? ' ' : '*', pDescription);
            OutputDebugStringA( buf );
			
        }
#endif
#endif
    } while (result == WT_Result::Success);
	
    whip_file->close();
	
	classFactory->Destroy(whip_file);
}

void skip_test(const wchar_t *filename)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();
	
    //TODO: wchar_t
    char buf[128];
    sprintf(buf, "File:%-10s\n\n", filename);
    wcout << "\n_______________________________________SKIP TEST\n";
	WT_String zFileName(filename);
	
    wcout << "File: " << zFileName.ascii();
	
	WT_File *whip_file = classFactory->Create_File();
	
    //SKIP TEST
    whip_file->set_filename (zFileName.length(),zFileName.unicode());
    whip_file->set_file_mode (WT_File::File_Read);
    whip_file->open();
	
    char **id = NULL;
    int id_count=0;
    OBJECT_LIST
	
    WT_Result result;
    do {
        result = whip_file->get_next_object_shell();
    } while (result == WT_Result::Success);
	
	classFactory->Destroy(whip_file);
}

#ifdef _DWFCORE_WIN32_SYSTEM
int wmain(int /*argc*/, wchar_t* /*argv*/[], wchar_t* /*envp*/[])
#else
int main(int /*argc*/, char* /*argv*/[])
#endif
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//long bp = 1973; bp; _CrtSetBreakAlloc( bp );
#endif
	

    if (access("colors8.png",0)!=0)
    {
        if (access("..\\colors8.png",0)==0)
            chdir("..");
        else
            printf("Resource file(s) not found, output will not be as intended.  Run this from the W2dTest project directory.\n");
    }
	
	// Initialize the Test Framework
	if( !CTestFramework::Instance().ReadSettingsFromConfigFile("W2DTest.ini") )
	{
		// Reading from the config file failed, so set up some default settings
		// for the tests to run
		CTestFramework::Instance().DoReadTests() = true;
		CTestFramework::Instance().DoWriteTests() = true;
		CTestFramework::Instance().SetClassFactoryType( CTestFramework::W2D );
	}
	
    if( CTestFramework::Instance().DoWriteTests() ) 
    {
        if (CTestFramework::Instance().GetClassFactoryType() == CTestFramework::XAML )
        {
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___0", 0 /* page 1*/);
            printf("Page 1 Output file generated: w2dtest___0\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___1", 1 /* page 2*/);
            printf("Page 2 Output file generated: w2dtest___1\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___2", 2 /* page 3*/);
            printf("Page 3 Output file generated: w2dtest___2\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___3", 3 /* page 4*/);
            printf("Page 4 Output file generated: w2dtest___3\n");
        }
        else
        {			
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___0.w2d", 0 /* page 1*/);
            printf("Page 1 Output file generated: w2dtest___0.w2d\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___1.w2d", 1 /* page 2*/);
            printf("Page 2 Output file generated: w2dtest___1.w2d\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___2.w2d", 2 /* page 3*/);
            printf("Page 3 Output file generated: w2dtest___2.w2d\n");
            write_test(/*binary=*/false,  /*compression=*/false,  /*filename=*/L"w2dtest___3.w2d", 3 /* page 4*/);
            printf("Page 4 Output file generated: w2dtest___3.w2d\n");
			
            write_test(/*binary=*/true,   /*compression=*/false,  /*filename=*/L"w2dtest_b_0.w2d", 0 /* page 1*/);
            printf("Page 1 Output file generated: w2dtest_b_0.w2d\n"); 
            write_test(/*binary=*/true,   /*compression=*/false,  /*filename=*/L"w2dtest_b_1.w2d", 1 /* page 2*/);
            printf("Page 2 Output file generated: w2dtest_b_1.w2d\n"); 
            write_test(/*binary=*/true,   /*compression=*/false,  /*filename=*/L"w2dtest_b_2.w2d", 2 /* page 3*/);
            printf("Page 3 Output file generated: w2dtest_b_2.w2d\n"); 
            write_test(/*binary=*/true,   /*compression=*/false,  /*filename=*/L"w2dtest_b_3.w2d", 3 /* page 4*/);
            printf("Page 4 Output file generated: w2dtest_b_3.w2d\n"); 
			
            write_test(/*binary=*/true,   /*compression=*/true,   /*filename=*/L"w2dtest_bc0.w2d", 0 /* page 1*/);
            printf("Page 1 Output file generated: w2dtest_bc0.w2d\n");
            write_test(/*binary=*/true,   /*compression=*/true,   /*filename=*/L"w2dtest_bc1.w2d", 1 /* page 2*/);
            printf("Page 2 Output file generated: w2dtest_bc1.w2d\n");
            write_test(/*binary=*/true,   /*compression=*/true,   /*filename=*/L"w2dtest_bc2.w2d", 2 /* page 3*/);
            printf("Page 3 Output file generated: w2dtest_bc2.w2d\n");
            write_test(/*binary=*/true,   /*compression=*/true,   /*filename=*/L"w2dtest_bc3.w2d", 3 /* page 4*/);
            printf("Page 4 Output file generated: w2dtest_bc3.w2d\n");
        }
    }
	
    if( CTestFramework::Instance().DoReadTests() )
    {
        if (CTestFramework::Instance().GetClassFactoryType() == CTestFramework::XAML )
        {
            read_test(/*filename=*/L"w2dtest___0", 0 /* page 1*/);
            read_test(/*filename=*/L"w2dtest___1", 1 /* page 2*/);
            read_test(/*filename=*/L"w2dtest___2", 2 /* page 3*/);
            read_test(/*filename=*/L"w2dtest___3", 3 /* page 4*/);
        }
        else
        {
            read_test(/*filename=*/L"w2dtest___0.w2d",0);
            read_test(/*filename=*/L"w2dtest___1.w2d",1);
            read_test(/*filename=*/L"w2dtest___2.w2d",2);
            read_test(/*filename=*/L"w2dtest___3.w2d",3);
			
            read_test(/*filename=*/L"w2dtest_b_0.w2d",0);
            read_test(/*filename=*/L"w2dtest_b_1.w2d",1);
            read_test(/*filename=*/L"w2dtest_b_2.w2d",2);
            read_test(/*filename=*/L"w2dtest_b_3.w2d",3);
			
            read_test(/*filename=*/L"w2dtest_bc0.w2d",0);
            read_test(/*filename=*/L"w2dtest_bc1.w2d",1);
            read_test(/*filename=*/L"w2dtest_bc2.w2d",2);
            read_test(/*filename=*/L"w2dtest_bc3.w2d",3);
        }
    }
	
    //skip_test(/*filename=*/"w2dtest___.w2d");
    //skip_test(/*filename=*/"w2dtest_b_.w2d");
    //skip_test(/*filename=*/"w2dtest_bc.w2d");
	
    return 0;
}
#endif



