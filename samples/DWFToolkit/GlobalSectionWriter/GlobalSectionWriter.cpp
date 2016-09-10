//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

// SimpleEnumReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;


int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" file.dwf" << endl;
        return ( 0 );
    }

    try
    {
        //
        // simple test case for adding a global section to a dwf
        //

        //
        // add a phony eplot section
        //
        DWFEPlotSection* pPage = DWFCORE_ALLOC_OBJECT( DWFEPlotSection(L"", L"", 0, DWFSource(L"",L"",L""), 0x00ffffff, NULL) );

        if (pPage == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate section" );
        }

        //
        // we could add a source & title to this section if necessary
        //
        DWFEPlotGlobalSection* pGlobal = DWFCORE_ALLOC_OBJECT( DWFEPlotGlobalSection );

        if (pGlobal == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate section" );
        }

        //
        // add a property
        //
        DWFProperty oProp( L"pi_short", L"3.14159" );
        pGlobal->addProperty( &oProp, false );

        //
        // add a set of properties
        //
        DWFPropertyContainer* pSet = DWFCORE_ALLOC_OBJECT( DWFPropertyContainer );
        
        oProp.setName( L"Dad" );
        oProp.setValue( L"Homer" );
        oProp.setCategory( L"Simpsons" );
        pSet->addProperty( &oProp, false );

        oProp.setName( L"Son" );
        oProp.setValue( L"Bart" );
        oProp.setCategory( L"Simpsons" );
        pSet->addProperty( &oProp, false );

        oProp.setName( L"Daughter" );
        oProp.setValue( L"Lisa" );
        oProp.setCategory( L"Simpsons" );
        pSet->addProperty( &oProp, false );

        pGlobal->addPropertyContainer( pSet );

        //
        // create another set
        //
        DWFPropertyContainer* pSet2 = DWFCORE_ALLOC_OBJECT( DWFPropertyContainer );
        
        oProp.setName( L"Name" );
        oProp.setValue( L"Value" );
        oProp.setCategory( L"" );
        pSet2->addProperty( &oProp, false );

        pGlobal->addPropertyContainer( pSet2 );


        //
        // create two objects
        //
        DWFDefinedObject* pGlobalObject = DWFCORE_ALLOC_OBJECT( DWFDefinedObject(L"") );
        DWFDefinedObject* pGlobalObject2 = DWFCORE_ALLOC_OBJECT( DWFDefinedObject(L"Custom ID") );

        //
        // add some properties
        //
        pGlobalObject->referencePropertyContainer( *pSet );
        pGlobalObject->referencePropertyContainer( *pSet2 );

        //
        // override set prop
        //
        oProp.setName( L"Daughter" );
        oProp.setValue( L"Maggie" );
        oProp.setCategory( L"Simpsons" );

        pGlobalObject->addProperty( &oProp, false );

        //
        // object defintion
        //
        DWFObjectDefinitionResource* pGlobalObjectDef = 
            DWFCORE_ALLOC_OBJECT( DWFObjectDefinitionResource(DWFXML::kzElement_GlobalObjectDefinition, DWFXML::kzRole_ObjectDefinition) );

        pGlobalObjectDef->addObject( pGlobalObject );
        pGlobalObjectDef->addObject( pGlobalObject2 );

        //
        // add to section
        //
        pGlobal->addResource( pGlobalObjectDef, true, true, true, NULL );

        //
        // add a simple bookmark tree
        //
        DWFBookmark* pRoot = DWFCORE_ALLOC_OBJECT( DWFBookmark(L"autodesk",L"http://www.autodesk.com") );
        DWFBookmark* pMark = DWFCORE_ALLOC_OBJECT( DWFBookmark(L"dwf",L"http://www.autodesk.com/dwf") );
        pRoot->addChildBookmark( pMark );

        DWFBookmark* pMark2 = DWFCORE_ALLOC_OBJECT( DWFBookmark(L"viewers",L"http://www.autodesk.com/viewers") );
        pMark->addChildBookmark( pMark2 );

        pMark = DWFCORE_ALLOC_OBJECT( DWFBookmark(L"adv",L"http://www.autodesk.com/dwfviewer") );
        pMark2->addChildBookmark( pMark );

        pMark = DWFCORE_ALLOC_OBJECT( DWFBookmark(L"composer",L"http://www.autodesk.com/dwfcomposer") );
        pMark2->addChildBookmark( pMark );

        pGlobal->provideBookmark( pRoot );

        //
        // add a global image resource
        //
        DWFImageResource* pImage = 
            DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Oceanarium",
                                                    L"Image",
                                                     DWFMIME::kzMIMEType_PNG) );

        if (pImage == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        //
        // configure the resource
        //
        double anTransform2[4][4] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        double anExtents2[4] = { 0, 0, 220, 170 };

        pImage->configureGraphic( (const double*)anTransform2,
                                  (const double*)anExtents2 );

        //
        // extras for the image
        // here we note the image has 24 bpp (color depth)
        //
        pImage->configureImage( 24 );

        DWFFile oImageFilename( L"ocean_thumbnail.png" );
        DWFStreamFileDescriptor* pImageFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oImageFilename, L"rb") );

        if (pImageFile == NULL)
        {
            DWFCORE_FREE_OBJECT( pImage );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file descriptor" );
        }

        DWFFileInputStream* pImageFilestream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );

        if (pImageFilestream == NULL)
        {
            DWFCORE_FREE_OBJECT( pImage );
            DWFCORE_FREE_OBJECT( pImageFile );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file stream" );
        }

        pImageFile->open();
        pImageFilestream->attach( pImageFile, true );

        pImage->setInputStream( pImageFilestream );

        //
        // drop the resource into the global section
        //
        pGlobal->addResource( pImage, true, true, true, NULL );


        //
        // let's also add a custom private section to this dwf
        //

        DWFCustomSection* pCustomSection = 
            DWFCORE_ALLOC_OBJECT( DWFCustomSection(L"Custom.Private.Type", L"Custom Private Title", L"BOND-007", 0.0, 1, DWFSource(L"", L"", L"")) );

        if (pCustomSection == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate section" );
        }

        //
        // add a private resource to the section
        //
        DWFResource* pCustomResource = DWFCORE_ALLOC_OBJECT( DWFResource(L"GlobalSectionWriter.cpp", L"Source File", DWFMIME::kzMIMEType_TXT, L"GlobalSectionWriter.cpp") );

        if (pCustomResource == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        DWFFile oCustomFilename( L"GlobalSectionWriter.cpp" );
        DWFStreamFileDescriptor* pCustomFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oCustomFilename, L"rb") );
        if (pCustomFile == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file descriptor" );
        }

        DWFFileInputStream* pCustomFilestream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
        if (pCustomFilestream == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file stream" );
        }

        pCustomFile->open();
        pCustomFilestream->attach( pCustomFile, true );
        pCustomResource->setInputStream( pCustomFilestream );

        //
        // drop the resource into the global section
        //
        pCustomSection->addResource( pCustomResource, true, true, true, NULL );


        DWFFile oDWF( argv[1] );
        DWF6PackageWriter oWriter( oDWF );

        oWriter.addSection( pPage );
        oWriter.addSection( pCustomSection );
        oWriter.addGlobalSection( pGlobal );
        oWriter.write( L"Autodesk", L"GlobalSectionWriter.cpp", L"", L"Autodesk", _DWFTK_VERSION_STRING );

        wcout << L"OK\n";
    }
    catch (DWFException& ex)
    {
        wcout << ex.type() << L": " << ex.message() << endl;
        wcout << L"(function) " << ex.function() << endl;
        wcout << L"(file) " << ex.file() << endl;
        wcout << L"(line) " << ex.line() << endl;
    }

	return 0;
}


