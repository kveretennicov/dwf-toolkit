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
        // create a simple page (eplot)
        //

        //
        // where did this dwf page originate?
        //
        DWFSource oSource( L"SimpleEPlotWriter.cpp", "DWFToolkit Samples", L"" );

        //
        // we will be adding a graphic resource to this page (namely the W2D)
        // this dictates that we define paper for this section
        //
        double anPaperClip[4] = { 0, 0, 11, 8.5 };
        DWFPaper oPaper( 11, 8.5, DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );

        DWFEPlotSection* pPage = DWFCORE_ALLOC_OBJECT( DWFEPlotSection(L"Page Title", L"123", 1, oSource, 0x00ff00ff, &oPaper) );

        //
        // add some page properties
        //

        //
        // (1)
        //
        pPage->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"creator", L"SimpleEPlotWriter.cpp", L"", L"", L"")), true );

        //
        // (2)
        //
//        DWFProperty oProperty( L"file version", L"1", L"Source Code", L"", L"" );
        //pPage->addProperty( &oProperty, false );

        //oProperty.setName( L"binary" );
        //oProperty.setValue( L"debug executable" );
        //pPage->addProperty( &oProperty, false );

        /*
        DWFProperty oProperty1( L"いくらですか？", L"さんぜん円", L"Source Code", L"", L"" );
        DWFProperty oProperty2( L"いくらですか？", L"さんぜん円", L"プロプルチズ", L"", L"" );
        DWFProperty oProperty3( L"FOO", L"さんぜん円", L"FOOCAT", L"", L"" );
        
        pPage->addProperty( &oProperty1, false );
        pPage->addProperty( &oProperty2, false );
        pPage->addProperty( &oProperty3, false );
        */

        pPage->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Some guys", L"Dun & Broadstreet", L"", L"", L"")), true );
        pPage->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Some units", L"3'", L"", L"", L"")), true );

//                oProperty.setName( L"Vendor" );
  //              oProperty.setValue( L"Dun & Broadstreet" );
    //            pPage->addProperty( oProperty );

        //
        // for this example, let's add a pre-existing w2d
        //

        //
        // define the resource - this must be allocated on the heap
        //
        DWFGraphicResource* p2Dgfx = 
            DWFCORE_ALLOC_OBJECT( DWFGraphicResource(L"Oceanarium",                       // title
                                                     DWFXML::kzRole_Graphics2d,           // role
                                                     DWFMIME::kzMIMEType_W2D,             // MIME type
                                                     L"Autodesk, Inc.",                   // author
                                                     L"Oceanarium Sample",                // description
                                                     L"",                                 // creation time
                                                     L"") );                              // modification time

        if (p2Dgfx == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
        }

        //
        // configure the resource
        //
        double anTransform[4][4] = { 0.00015625, 0, 0, 0, 0, 0.00015625, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        double anClip[4] = { 0, 0, 11, 8.5 };

        p2Dgfx->configureGraphic( (const double*)anTransform,
                                   NULL,
                                  (const double*)anClip );

        //
        // most importantly - bind a stream to the resource
        // in this case, we have a file on disk so we open the
        // file with a streaming descriptor.  we will also
        // create everything on the heap since the package writer
        // will not use the stream immediately, he will need to own
        // these resources.
        //
        DWFFile oW2DFilename( L"ocean_90.w2d" );
        DWFStreamFileDescriptor* pW2DFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oW2DFilename, L"rb") );

        if (pW2DFile == NULL)
        {
            DWFCORE_FREE_OBJECT( p2Dgfx );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file descriptor" );
        }

        DWFFileInputStream* pW2DFilestream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );

        if (pW2DFilestream == NULL)
        {
            DWFCORE_FREE_OBJECT( p2Dgfx );
            DWFCORE_FREE_OBJECT( pW2DFile );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file stream" );
        }

        //
        // open the file and bind it to the stream
        //
        pW2DFile->open();
        pW2DFilestream->attach( pW2DFile, true );

        //
        // hand the stream off to the resource
        // NOTE: since we don't already know the filesize (in the application space - of course we can look on disk...)
        // leave the second parameter (nBytes) default as zero, this will tell the package writer
        // to use the number of bytes it processed through the stream as the size attribute in the descriptor.
        //
        p2Dgfx->setInputStream( pW2DFilestream );

        //
        // finally, drop the resource into the page
        //
        pPage->addResource( p2Dgfx, true );

        //
        // now let's add the thumbnail
        //

        DWFImageResource* pThumbnail = 
            DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Oceanarium Thumbnail",
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
        double anExtents2[4] = { 0, 0, 220, 170 };

        pThumbnail->configureGraphic( (const double*)anTransform2,
                                      (const double*)anExtents2 );

        //
        // extras for the image
        // here we note the image has 24 bpp (color depth)
        //
        pThumbnail->configureImage( 24 );

        DWFFile oThumbnailFilename( L"ocean_thumbnail.png" );
        DWFStreamFileDescriptor* pThumbnailFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(oThumbnailFilename, L"rb") );

        if (pThumbnailFile == NULL)
        {
            DWFCORE_FREE_OBJECT( pThumbnail );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file descriptor" );
        }

        DWFFileInputStream* pThumbnailFilestream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );

        if (pThumbnailFilestream == NULL)
        {
            DWFCORE_FREE_OBJECT( pThumbnail );
            DWFCORE_FREE_OBJECT( pThumbnailFile );

            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate file stream" );
        }

        pThumbnailFile->open();
        pThumbnailFilestream->attach( pThumbnailFile, true );

        pThumbnail->setInputStream( pThumbnailFilestream );

        //
        // drop the resource into the page
        // and be sure to mention that it "belongs" to the W2D 
        // (and replace and delete)
        //
        pPage->addResource( pThumbnail, true, true, true, p2Dgfx );

        //
        // object definitions: in this small example,
        // let's assume we have a known object node in the w2d
        // we will create an object and instance for this node
        //
        DWFDefinedObject* pObject = DWFCORE_ALLOC_OBJECT( DWFDefinedObject );
        DWFDefinedObjectInstance* pInstance = pObject->instance( L"1" );

        //
        // add a property directly on the instance
        //
        pInstance->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"vendor", L"abc manu", L"vendors")), true );

        //
        // create the object definition resource
        //
        DWFObjectDefinitionResource* pObjDefRes = 
            DWFCORE_ALLOC_OBJECT( DWFObjectDefinitionResource(DWFXML::kzElement_PageObjectDefinition,DWFXML::kzRole_ObjectDefinition) );

        //
        // add the object
        //
        pObjDefRes->addObject( pObject );

        //
        // add the instance
        //
        pObjDefRes->addInstance( pInstance );

        //
        // add the resource (as a child of the w2d)
        //
        pPage->addResource( pObjDefRes, true, true, true, p2Dgfx );

        ///
        ///

        DWFFile oDWF( argv[1] );
        DWF6PackageWriter oWriter( oDWF );

        //
        // add some package-wide properties
        //
        DWFPropertyContainer& rManifestProperties = oWriter.getManifestProperties();
        rManifestProperties.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Package Property Name", L"Package Property Value", L"Package Property Category", L"", L"")), true );

        oWriter.addSection( pPage );
        oWriter.write( L"Autodesk", L"SimpleEPlotWriter.cpp", L"", L"Autodesk", _DWFTK_VERSION_STRING );

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


