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

void setResourceStreamFromFile( DWFResource* pResource, const DWFString& zFilename )
    throw( DWFException )
{
    //
    // most importantly - bind a stream to the resource
    // in this case, we have a file on disk so we open the
    // file with a streaming descriptor.  we will also
    // create everything on the heap since the package writer
    // will not use the stream immediately, he will need to own
    // these resources.
    //
    DWFFile oResourceFilename( zFilename );
    DWFStreamFileDescriptor* pResourceFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oResourceFilename, L"rb") );

    if (pResourceFile == NULL)
    {
        DWFCORE_FREE_OBJECT( pResource );

        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file descriptor" );
    }

    DWFFileInputStream* pResourceFilestream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );

    if (pResourceFilestream == NULL)
    {
        DWFCORE_FREE_OBJECT( pResource );
        DWFCORE_FREE_OBJECT( pResourceFile );

        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file stream" );
    }

    //
    // open the file and bind it to the stream
    //
    pResourceFile->open();
    pResourceFilestream->attach( pResourceFile, true );

    //
    // hand the stream off to the resource
    // NOTE: since we don't already know the filesize (in the application space - of course we can look on disk...)
    // leave the second parameter (nBytes) default as zero, this will tell the package writer
    // to use the number of bytes it processed through the stream as the size attribute in the descriptor.
    //
    pResource->setInputStream( pResourceFilestream );
}

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
		// create the DWFPackagePublisher object
		//

		DWFFile oDWF( argv[1] );
		DWFPackagePublisher* pPackagePublisher = NULL;
		pPackagePublisher = DWFCORE_ALLOC_OBJECT( DWFXPackagePublisher( oDWF ) );
		
		//
		// create a DWFPlot
		//

		DWFPlot oPlot( L"Fixed page XAML Test", L"SimpleXPSPublisher.cpp" );


        double anPaperClip[4] = { 0, 0, 11, 8.5 };
        DWFPaper oPaper( 11.0, 8.5, DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );
		double anTransform[4][4] = { 0.0025, 0, 0, 0, 0, 0.0025, 0, 0, 0, 0, 1, 0, -5368698.81750012, 1.339999879852054, 0, 1 };
		oPlot.open( oPaper, (const double*)anTransform );

		//
		// add the graphics
		//
        
		//
        // open the FPXAML as a streaming binary file
        //
        DWFStreamFileDescriptor* p2DgfxFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( "line.xml", "rb" ));
        p2DgfxFile->open();

		//
		// create a stream and attach the descriptor
		//
		
		DWFFileInputStream *p2DgfxStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
		p2DgfxStream->attach( p2DgfxFile, false );

		//
		// attach the stream to the plot
		//
		
		oPlot.attach( p2DgfxStream, false );
		
		//
		// let us create an object and instance for the 2D node we have added
		//
		
		//DWFDefinedObject* pObject = DWFCORE_ALLOC_OBJECT( DWFDefinedObject );
        //DWFDefinedObjectInstance* pInstance = pObject->instance( L"1" );


        //
        // add some page properties
        //
		
		DWFProperty oProperty( L"file version", L"1", L"Source Code", L"", L"" );
        oPlot.addProperty( &oProperty, false );
		

		//
        // (1)
        //

        oPlot.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"creator", L"SimpleXPSPublisher.cpp", L"", L"", L"")), true );

        //
        // (2)
        //
        

        DWFProperty oProperty1( L"いくらですか？", L"さんぜん円", L"Source Code", L"", L"" );
        DWFProperty oProperty2( L"いくらですか？", L"さんぜん円", L"プロプルチズ", L"", L"" );
        DWFProperty oProperty3( L"FOO", L"さんぜん円", L"FOOCAT", L"", L"" );
        
        oPlot.addProperty( &oProperty1, false );
        oPlot.addProperty( &oProperty2, false );
        oPlot.addProperty( &oProperty3, false );

        oPlot.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Some guys", L"Dun & Broadstreet", L"", L"", L"")), true );
        oPlot.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Some units", L"3'", L"", L"", L"")), true );

        //
        // let's add the FPXAML extension
        //
        
		//
        // define the resource - this must be allocated on the heap
        //
        DWFResource* pFPXAMLExtensionResource = 
            DWFCORE_ALLOC_OBJECT( DWFResource( L"W2X Resource",                     // title
                                               DWFXML::kzRole_Graphics2dExtension,  // role
                                               DWFMIME::kzMIMEType_XML              // MIME type
                                                     ) );
		
		
        if (pFPXAMLExtensionResource == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        //
        // set the resource's stream from the FPXAML file
        //
        setResourceStreamFromFile( pFPXAMLExtensionResource, L"line.w2x" );

        //
        // finally, drop the resource into the page
        //
        ((DWFResourceContainer&) oPlot).addResource( pFPXAMLExtensionResource, false );

        //
        // let's add the font file
        //
        //
        // define the resource - this must be allocated on the heap
        //
        //DWFResource* pFontResource = 
        //    DWFCORE_ALLOC_OBJECT( DWFResource( L"Arial Font",                       // title
        //                                       DWFXML::kzRole_Font,                 // role
        //                                       DWFMIME::kzMIMEType_TTF              // MIME type
        //                                             ) );
        //if (pFontResource == NULL)
        //{
        //    _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        //}

        //pFontResource->setRequestedName( L"arial.ttf" );

        ////
        //// set the resource's stream from the ttf file
        ////
        //setResourceStreamFromFile( pFontResource, L"arial.ttf" );

        ////
        //// finally, drop the resource into the page
        ////
        //((DWFResourceContainer&) oPlot).addResource( pFontResource, false );

        
        //
        // now let's add the thumbnail
        //

        DWFImageResource* pThumbnail = 
            DWFCORE_ALLOC_OBJECT( DWFImageResource( L"ArborPress Thumbnail",
                                                     DWFXML::kzRole_Thumbnail,
                                                     DWFMIME::kzMIMEType_PNG) );

        if (pThumbnail == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        //
        // configure the resource
        //
        double anTransform2[4][4] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        double anExtents2[4] = { 0, 0, 170, 219 };

        pThumbnail->configureGraphic( (const double*)anTransform2,
                                      (const double*)anExtents2 );

        //
        // extras for the image
        // here we note the image has 24 bpp (color depth)
        //
        pThumbnail->configureImage( 24 );

        //
        // set the resource's stream from the thumbnail file
        //
        setResourceStreamFromFile( pThumbnail, L"line.png" );

        //
        // finally, drop the resource into the page
        //
		((DWFResourceContainer&) oPlot).addResource( pThumbnail, false );
#if 0
        //
        // let's add the remote resource dictionary file
        //
        //
        // define the resource - this must be allocated on the heap
        //
        DWFResource* pDictionaryResource = 
            DWFCORE_ALLOC_OBJECT( DWFResource( L"Remote Resource Dictionary",                   // title
                                               DWFXML::kzRole_Graphics2dDictionary,             // role
                                               DWFMIME::kzMIMEType_REMOTERESOURCEDICTIONARY     // MIME type
                                                     ) );
        if (pDictionaryResource == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        pDictionaryResource->setRequestedName( L"ArborPress2D.dict" );

        //
        // set the resource's stream from the FPXAML file
        //
        setResourceStreamFromFile( pDictionaryResource, L"ArborPress2D.dict" );
        //
        // finally, drop the resource into the page
        //
        pPage->addResource( pDictionaryResource, true );


        //
        // let's add the remote resource dictionary file
        //
        //
        // define the resource - this must be allocated on the heap
        //
        DWFImageResource* pImageRefResource = 
            DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Image Resource",       // title
                                                      DWFXML::kzRole_RasterReference,  // role
                                                      DWFMIME::kzMIMEType_PNG              // MIME type
                                                     ) );
        if (pImageRefResource == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        pImageRefResource->setRequestedName( L"ArborPress2D.png" );

        //
        // set the resource's stream from the FPXAML file
        //
        setResourceStreamFromFile( pImageRefResource, L"ArborPress2D.png" );
        //
        // finally, drop the resource into the page
        //
        pPage->addResource( pImageRefResource, true );
#endif

		//
		// close the plot and publish it
		//
        
		oPlot.close();
        oPlot.publish( *pPackagePublisher );

		//
		// Create the dwf
		//
		pPackagePublisher->publish();
        
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


