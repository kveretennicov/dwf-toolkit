
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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/XPSTest/XPSTest.cpp#1 $

// XPSTest.cpp : Defines the entry point for the console application.
//
#include "StdAfx.h"
#include "object_list.h"
#include "test_list.h"
#include "XPSTest.h"
#include "OpcHelper.h"
#include "TestFramework.h"

#include "dwf/dwfx/Constants.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;

//TODO: define additional object tests in test_list.h

//define the extern functions for each test
#define TEST_MACRO(class_lower, class_upper)              \
    extern WT_Result write_##class_lower##(WT_File &);        \
    extern WT_Result read_##class_lower##(WT_##class_upper## &, WT_File &);
TEST_LIST
TEST_LIST_1
TEST_LIST_2
TEST_LIST_3
#undef TEST_MACRO

#define TEST_MACRO(class_lower, class_upper) \
    OutputDebugStringA(#class_lower);\
    OutputDebugStringA("\n");\
    if (WT_Result::Success != write_##class_lower##(*whip_file)) {\
        OutputDebugStringA("ERROR in write_");\
        OutputDebugStringA(#class_lower);\
        OutputDebugStringA(" test!\n");\
        break; }

OpcHelper gHelper;

void write_test(bool /*binary*/, bool /*compression*/, OpcHelper& rHelper, DWFOutputStream* pXAMLStream, DWFOutputStream* pW2XStream, int testList)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    OutputDebugStringA("\n_______________________________________WRITE TEST\n");
            
    //WRITE TEST
	WT_File *whip_file = classFactory->Create_File();
    if (whip_file == NULL)
    {
        return;
    }

    whip_file->set_file_mode (WT_File::File_Write);
    //Set a different target version here if you don't want a W2D file... for example:

    WT_XAML_File *pxFile = static_cast<WT_XAML_File *>(whip_file);

    //Initialize the file object
    pxFile->xamlStreamOut() = pXAMLStream;
    pxFile->w2xStreamOut() = pW2XStream;
    pxFile->opcResourceSerializer() = &gHelper;
    pxFile->opcResourceMaterializer() = &gHelper;

    WT_Matrix2D oTransform;
    //
    // This transfrom must match the one on the graphics resource exactly
    //
    double fScale = 0.0016792611251049536;  
    oTransform(0,0) = fScale;
    oTransform(1,1) = fScale;
    oTransform(2,0) = 4.3706968933669188;
    oTransform(2,1) = 6.1601595298068847;

    //
    // The heigh and width values must match those of the section's paper
    // the scale is a factor to scale the units given in the page size to inches...
    // so if the paper size is in mm, it should be .03937
    //
    ERR_CHECK( pxFile->definePageLayout(oTransform, 11, 8.5, 1) );
    ERR_CHECK( whip_file->open() );

    // set foreground color
    whip_file->desired_rendition().color().set(0,0,0);

    // set the initial view
    // Register the fonts we need:
    if ( CTestFramework::Instance().GetClassFactoryType() == CTestFramework::XAML )
    {
        //Test characters for ANSI, Japanese, Arabic, and extended Latin
        WT_String zSrc( L"().,0123456789-=_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz \x30A4\x30B4\x30C4\x30D4 \x0682\x0692\x06A2\x06B2 \x0105\x0115\x0125\x0135" );
        WT_String zFontName( L"Arial");
        WT_String zFontPath( zFontName );
        WT_Integer32 nHeight = 20; // a default
        ERR_CHECK( rHelper.getSubsetFontName( zFontPath, nHeight, zSrc ) );
        wchar_t *zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        wchar_t *zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;

        zFontName = L"Times New Roman";
        zFontPath = zFontName;
        ERR_CHECK( rHelper.getSubsetFontName( zFontPath, nHeight, zSrc ) );
        zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;

        zFontName = L"Courier New";
        zFontPath = zFontName;
        ERR_CHECK( rHelper.getSubsetFontName( zFontPath, nHeight, zSrc ) );
        zpFontName = WT_String::to_wchar( zFontName.length(), zFontName );
        zpFontPath = WT_String::to_wchar( zFontPath.length(), zFontPath );
        ERR_NULLCHECK( zpFontName );
        ERR_NULLCHECK( zpFontPath );
        ERR_CHECK( static_cast<WT_XAML_File*>(whip_file)->registerFontUri( zpFontName, zpFontPath ) );
        delete[] zpFontName;
        delete[] zpFontPath;
    }
   
    switch (testList)
    {
    case 0:
        {
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
#define TEST_MACRO(class_lower, class_upper) \
    whip_file->set_##class_lower##_action(read_##class_lower##);

#define OBJECT_MACRO(object_name) \
    id_count++; \
    id = (char**) realloc(id, sizeof(char*) * id_count); \
    id[id_count-1] = (char*) malloc(strlen(#object_name)+1); \
    sprintf(id[id_count-1], "%s\n", #object_name);

void read_test(const wchar_t *filename, int testList)
{
	WT_Class_Factory *classFactory = CTestFramework::Instance().GetClassFactory();

    //TODO: wchar_t
    char buf[128];
    sprintf(buf, "File:%-10s\n\n", filename);
    OutputDebugStringA("\n_______________________________________READ TEST\n");
    OutputDebugStringA(buf);

    WT_File *whip_file = classFactory->Create_File();

    //READ TEST

    WT_String zFileName(filename);
    whip_file->set_filename (zFileName);
    whip_file->set_file_mode (WT_File::File_Read);
    whip_file->open();

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
    OutputDebugStringA("\n_______________________________________SKIP TEST\n");
    OutputDebugStringA(buf);

	WT_File *whip_file = classFactory->Create_File();

    //SKIP TEST
    WT_String zFileName(filename);
    whip_file->set_filename (zFileName);
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


#ifdef XPSTEST_APP
int wmain(int argc, wchar_t* argv[], wchar_t* /*envp*/[])
{
    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" file.dwfx" << endl;
        return ( 0 );
    }
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    // For mem leak debugging... Please do not delete.
    long foo = 0;
    _CrtSetBreakAlloc(foo);
	#endif

    wchar_t zCurrentDirectory[_MAX_PATH+1];
    ::GetCurrentDirectory( _MAX_PATH+1, zCurrentDirectory );
    ::SetCurrentDirectory( L"..\\W2dTest" );

    ::CreateDirectory(L"out", NULL);

	// Reading from the config file failed, so set up some default settings
	// for the tests to run
	CTestFramework::Instance().DoReadTests() = false;
	CTestFramework::Instance().DoWriteTests() = true;
    CTestFramework::Instance().SetClassFactoryType( CTestFramework::XAML );

    //
    // in this rev of this sample, assume output is always dwfx
    //
    DWFFile oDWF( argv[1] );
    DWFXPackageWriter* pPackageWriter = DWFCORE_ALLOC_OBJECT( DWFXPackageWriter( oDWF ) );

    //
    // Page 1
    //

    //
    // where did this dwf page originate?
    //
    DWFSource oSource( L"XPSTest.cpp", "DWFToolkit Samples", L"" );
    //
    // we will be adding a graphic resource to this page (namely the FixedPageXAML)
    // this dictates that we define paper for this section
    //
    double anPaperClip[4] = { 0, 0, 11, 8.5 };
    DWFPaper oPaper( 11, 8.5, DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );

    DWFEPlotSection* pPage1 = DWFCORE_ALLOC_OBJECT( DWFEPlotSection(L"Page 1", L"Page1", 1, oSource, 0x00ff00ff, &oPaper) );

    //
    // set the current section into out helper class, so that Xaml resource received there can be added to the section
    //
    gHelper.setCurrentSection( pPage1 );

    //
    // the output stream will receive the Xaml bytes
    // the output stram will be deleted by the Xaml tk
    // on deletion XamlResourceOutputStream class will
    // pass its content onto pXamlResourceInputStream
    //
    XamlResourceInputStream* pXamlResourceInputStream = DWFCORE_ALLOC_OBJECT( XamlResourceInputStream() );
    XamlResourceOutputStream* pXamlResourceOutputStream = DWFCORE_ALLOC_OBJECT( XamlResourceOutputStream(pXamlResourceInputStream) );


    //
    // define the resource - this must be allocated on the heap
    //
    DWFGraphicResource* p2Dgfx = 
        DWFCORE_ALLOC_OBJECT( DWFGraphicResource(L"XPSTest",                          // title
                                                 DWFXML::kzRole_Graphics2d,           // role
                                                 DWFMIME::kzMIMEType_FIXEDPAGEXML,    // MIME type
                                                 L"Autodesk, Inc.",                   // author
                                                 L"XPSTest Sample",                   // description
                                                 L"",                                 // creation time
                                                 L"") );                              // modification time

    if (p2Dgfx == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
    }

    //
    // configure the resource
    //

    //
    // Transform and extent values determined empirically.
    //
    double anTransform[4][4] = { 0.0016792611251049536, 0, 0, 0, 0, 0.0016792611251049536, 0, 0, 0, 0, 1, 0, 4.3706968933669188, 6.1601595298068847, 0, 1 };
    double aExtents[4] = { 0.50000000000000089, 0.40029387069689371, 10.5, 8.0997061293031063, };

    p2Dgfx->configureGraphic( (const double*)anTransform,
                               aExtents,
                               NULL );

    p2Dgfx->setInputStream( pXamlResourceInputStream );

    //
    // finally, drop the resource into the page
    //
    pPage1->addResource( p2Dgfx, true );

    //
    // do the same stream connections for w2x
    //
    XamlResourceInputStream* pW2XInputStream = DWFCORE_ALLOC_OBJECT( XamlResourceInputStream() );
    XamlResourceOutputStream* pW2XOutputStream = DWFCORE_ALLOC_OBJECT( XamlResourceOutputStream(pW2XInputStream) );

    //
    // define the resource - this must be allocated on the heap
    //
    DWFResource* pW2XExtensionResource = 
        DWFCORE_ALLOC_OBJECT( DWFResource( L"W2X Resource",                     // title
                                           DWFXML::kzRole_Graphics2dExtension,  // role
                                           DWFMIME::kzMIMEType_XML              // MIME type
                                                 ) );

    if (pW2XExtensionResource == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
    }

    pW2XExtensionResource->setInputStream( pW2XInputStream );

    //
    // finally, drop the resource into the page
    //
    pPage1->addResource( pW2XExtensionResource, true );

    p2Dgfx->addRelationship( pW2XExtensionResource, DWFXXML::kzRelationship_Graphics2dExtensionResource );

    pPackageWriter->addSection( pPage1 );

    write_test(/*binary=*/false,  /*compression=*/false, gHelper, pXamlResourceOutputStream, pW2XOutputStream, 0 /* page 1*/);

    ::SetCurrentDirectory( zCurrentDirectory );
    pPackageWriter->write( L"Autodesk", L"SimpleXPSWriter.cpp", L"", L"Autodesk", _DWFTK_VERSION_STRING );

    wcout << L"OK\n";

    DWFCORE_FREE_OBJECT( pPackageWriter );
    return 0;
}
#endif

