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

// WriteContent.cpp : Defines the entry point for the console application.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/samples/DWFToolkit/WriteContent/WriteContent.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/STL.h"
#include <iostream>

#include "dwfcore/MIME.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/StreamFileDescriptor.h"

#include "dwf/package/EPlotSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/package/writer/DWF6PackageWriter.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;

//
// declarations
//
DWFGraphicResource* CreateGraphicsResource()
    throw( DWFException );

DWFImageResource* CreateThumbnailResource()
    throw( DWFException );

DWFEPlotSection* CreateSimpleEPlotSection( DWFContentManager* pContentManager )
    throw( DWFException );


///////////////////////////////////////////////////////////////////////////////

int main( int /*argc*/, char* /*argv[]*/ )
{
    try
    {
        //
        // Take ownership of the content manager
        //
        DWFContentManager* pContentMgr = DWFCORE_ALLOC_OBJECT( DWFContentManager() );

        DWFEPlotSection* pEPlotSection = CreateSimpleEPlotSection( pContentMgr );

        //
        // Write out the dwf with the created sections and associated content.
        //
        DWFFile oDWF( "content.dwf" );
        DWF6PackageWriter oWriter( oDWF );
        oWriter.attachContentManager( pContentMgr );
        oWriter.addSection( pEPlotSection );
        oWriter.write();
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

///////////////////////////////////////////////////////////////////////////////

DWFEPlotSection* CreateSimpleEPlotSection( DWFContentManager* pContentManager )
throw( DWFException )
{
    //
    //  This will be used to define where this DWF originated
    //
    DWFSource oSource( L"WriteContent.cpp", "DWFToolkit Samples", L"" );

    //
    //  We will be adding a graphic resource to this page (namely the W2D)
    //  this dictates that we define paper for this section
    //
    double anPaperClip[4] = { 0, 0, 11, 8.5 };
    DWFPaper oPaper( 11, 8.5, DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );

    // 
    //  Create the EPlot section with the paper and source information.
    //
    DWFEPlotSection* pPage = DWFCORE_ALLOC_OBJECT( DWFEPlotSection(L"Page Title", L"123", 1, oSource, 0x00ff00ff, &oPaper) );

    //
    //  Create a 2D graphics resource.
    //  Add it to the section. The section will take ownership.
    //
    DWFGraphicResource* p2Dgfx = CreateGraphicsResource();
    pPage->addResource( p2Dgfx, true );

    //
    //  Create the thumbnail/image resource.
    //  Add it to the page and be sure to mention that it "belongs" to the W2D 
    //  (and replace and delete)
    //
    DWFImageResource* pThumbnail = CreateThumbnailResource();
    pPage->addResource( pThumbnail, true, true, true, p2Dgfx );


    if (pContentManager)
    {
        //
        //  This will create a content library if it doesn't exist - this new content
        //  will also become the primary content since it is the first one created.
        //
        DWFContent* pContent = pContentManager->getContent();
        DWFString zSchema1 = pContent->getIDProvider()->next(true);
        DWFString zSchema2 = pContent->getIDProvider()->next(true);
        DWFString zSetID1 = pContent->getIDProvider()->next(true);

        //
        // Lets add a couple of property sets with properties to be shared.
        //
        DWFPropertySet* pPSet1 = pContent->addSharedPropertySet( L"PSet1" );
        pPSet1->addProperty( L"Geo", L"Ithaca" );
        pPSet1->addProperty( L"State", L"NY" );
        {
            DWFPropertySet* pPSubSet1 = pPSet1->addPropertySet( L"PSet subset1" );
            pPSubSet1->addProperty( L"SubGeo", L"North East Ithaca" );

            pPSubSet1->addProperty( L"Geo", L"Greater Ithaca" );
            pPSubSet1->setSchemaID( zSchema1 );
        }

        DWFPropertySet* pPSet2 = pContent->addSharedPropertySet( L"PSet2" );
        pPSet2->setClosed( zSetID1 );
        pPSet2->addProperty( L"Geo", L"SF" );
        pPSet2->addProperty( L"State", L"CA" );
        {
            DWFPropertySet* pSubSet2 = pPSet2->addPropertySet( zSchema1 );
            pSubSet2->addProperty( L"Geo", L"Southern SF" );
        }

        //
        // Add a couple of classes
        //
        DWFClass* pClass1 = pContent->addClass();

        DWFClass::tList oBaseClass;
        oBaseClass.push_back( pClass1 );
        DWFClass* pClass2 = pContent->addClass( oBaseClass );
        pClass2->addProperty( L"Class#", L"#2" );

        DWFClass* pClass3 = pContent->addClass();
        pClass3->addProperty( L"Class#", L"#3" );
        pClass3->addProperty( L"Color", L"Purple" );

        //
        // Add a couple of entities
        //
        oBaseClass.clear();
        oBaseClass.push_back( pClass2 );
        DWFEntity* pEntityParent = pContent->addEntity( oBaseClass );
        DWFEntity* pEntity1 = pContent->addEntity( pEntityParent );
        pContent->addClassToEntity( pEntity1, pClass3 );
        {
            pEntity1->addProperty( "EntityName", "Entity #1" );
            pEntity1->addProperty( "ObjectName", "ObjectEntity #1" );
        }
        
        //
        // Add a feature
        //
        DWFFeature* pFeature1 = pContent->addFeature();
        pFeature1->addProperty( L"EdgeType", L"Bevelled" );


        //
        // Add a couple of objects
        //
        DWFObject* pObject1 = pContent->addObject( pEntity1 );
        pObject1->setLabel( "Object #1" );
        {
            pContent->addSharedPropertySetToElement( pObject1, pPSet1 );
            pObject1->addProperty( L"ObjectName", L"Object1" );
            pObject1->addProperty( L"Location", L"Geo1" );
        }

        DWFObject* pObject2 = pContent->addObject( pEntity1, pObject1 );
        pContent->addSharedPropertySetToElement( pObject2, pPSet2 );


        //
        // Add a group with properties
        //
        DWFContentElement::tList oGroupItems;
        oGroupItems.push_back( pClass1 );
        oGroupItems.push_back( pClass2 );
        oGroupItems.push_back( pFeature1 );
        DWFGroup* pGroup1 = pContent->addGroup( oGroupItems );
        pGroup1->addProperty( L"Type", L"Assortment", L"None", L"string", L"wide" );
        pGroup1->addProperty( L"Object", L"A", L"None", L"string", L"wide" );
        {
            DWFPropertySet* pSetGroup1 = pGroup1->addPropertySet( L"GroupSet" );
            pSetGroup1->setClosed( zSetID1 );
            pSetGroup1->setSchemaID( pContent->getIDProvider()->next(true) );
            pSetGroup1->addProperty( L"GName1", L"GVal1" );
            pSetGroup1->addProperty( L"GName2", L"GVal2" );
            {
                DWFPropertySet* pSetGroup11 = pSetGroup1->addPropertySet( pContent->getIDProvider()->next(true) );
                pSetGroup11->addProperty( L"GName11", L"GVal11" );
                pSetGroup11->referencePropertyContainer( *pPSet1 );
            }
        }

        //
        //  Add a section content resource to tie the instance information to the graphics stream. 
        //  Assume we have access to graphics nodes 10 and 20 for the instances (below).
        //
        DWFSectionContentResource* pSCR = DWFCORE_ALLOC_OBJECT( DWFSectionContentResource(pContent) );
        pPage->addResource( pSCR, true, true, true, p2Dgfx );
        DWFString zResourceID = pSCR->objectID();
        //
        // Add a couple of instances based on the objects
        //
        DWFInstance* pInstance1 = pContent->addInstance( zResourceID, pObject1, 10 );
        pInstance1->setVisibility( false );
        pInstance1->setGeometricVariationIndex( 2 );
        /*DWFInstance* pInstance2 = */pContent->addInstance( zResourceID, pFeature1, 20 );
    }

    return pPage;
}

DWFGraphicResource* CreateGraphicsResource()
throw( DWFException )
{
    //
    // Create a graphics resource to bind to an existing w2d stream on disk.
    //
    DWFGraphicResource* p2Dgfx = 
        DWFCORE_ALLOC_OBJECT( DWFGraphicResource(L"Oceanarium",             // title
                                                 DWFXML::kzRole_Graphics2d, // role
                                                 DWFMIME::kzMIMEType_W2D,   // MIME type
                                                 L"Autodesk, Inc.",         // author
                                                 L"Oceanarium Sample",      // description
                                                 L"",                       // creation time
                                                 L"") );                    // modification time
    if (p2Dgfx == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
    }

    //
    // Configure the resource.
    //
    double anTransform[4][4] = { 0.00015625, 0, 0, 0, 0, 0.00015625, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    double anClip[4] = { 0, 0, 11, 8.5 };
    p2Dgfx->configureGraphic( (const double*)anTransform, NULL, (const double*)anClip );

    //
    // Bind a stream to the resource. We have a file on disk so we open the file with a 
    // streaming descriptor. Everything is created on the heap since the package writer 
    // will not use the stream immediately. It will need to own these resources.
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
    // Open the file and bind it to the stream
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

    return p2Dgfx;
}

DWFImageResource* CreateThumbnailResource()
throw( DWFException )
{
    DWFImageResource* pThumbnail = 
        DWFCORE_ALLOC_OBJECT( DWFImageResource( L"Oceanarium Thumbnail",
                                                 DWFXML::kzRole_Thumbnail,
                                                 DWFMIME::kzMIMEType_PNG) );
    if (pThumbnail == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate resource" );
    }

    //
    // Configure the resource.
    //
    double anTransform2[4][4] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    double anExtents2[4] = { 0, 0, 220, 170 };

    pThumbnail->configureGraphic( (const double*)anTransform2,
                                  (const double*)anExtents2 );

    //
    // Extras for the image. Here we note the image has 24 bpp (color depth).
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

    return pThumbnail;
}

