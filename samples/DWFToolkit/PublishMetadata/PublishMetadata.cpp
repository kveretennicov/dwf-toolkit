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

// PublishMetadata.cpp : Defines the entry point for the console application.
//


#include <iostream>

#include "dwfcore/MIME.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/StreamFileDescriptor.h"

#ifdef _DWFCORE_WIN32_SYSTEM

    #if _USE_VLD
        #define _USE_VLD_FOR_MEMORY_LEAK_TEST
        #include <vld.h>
    #else
        #define _USE_CRTDBG_FOR_MEMORY_LEAK_TEST
        #include <crtdbg.h>
    #endif

#endif

#include "dwf/package/ContentManager.h"
#include "dwf/package/SectionContentResource.h"
#include "dwf/package/GraphicResource.h"

#include "dwf/publisher/model/Model.h"
#include "dwf/publisher/impl/DWF6PackagePublisher.h"
#include "dwf/publisher/impl/DWFXPackagePublisher.h"
#include "dwf/publisher/impl/PublishedContentElement.h"
#ifdef  _DWFCORE_WIN32_SYSTEM
#include "dwf/publisher/win32/EmbeddedFontImpl.h"
#endif

#include "dwf/presentation/ContentPresentationResource.h"
#include "dwf/presentation/ContentPresentationReferenceNode.h"
#include "dwf/presentation/ContentPresentationModelViewNode.h"
#include "dwf/presentation/PropertyReference.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;

////////////////////////////////////////////////////////////////////////////////////

//
// Some of this sample code is taken from the HOOPS/Stream toolkit (v.10.00) samples
//


////////////////////////////////////////////////////////////////////////////////////
DWFModel* createModel( DWFPublisher& rPublisher, DWFContent* pContent, bool bCompressAll );
DWFData*  createData(  DWFPublisher& oPublisher, DWFContent* pContent );

int main(int argc, char* argv[])
{
#if defined(_USE_CRTDBG_FOR_MEMORY_LEAK_TEST)
    //
    // Enable memory leak reporting in Debug mode under Win32.
    //
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );

    // For mem leak debugging... Please do not delete.
    //long foo = 0;
    //_CrtSetBreakAlloc(foo);
#endif

    if (argc > 1)
    {
        if (DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE( argv[1], "--help" ) == 0 ||
            DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE( argv[1], "-h" ) == 0)
        {
            wcout << "Usage:" << argv[0] << L" [options] [file.dwf]" << endl << endl;
            wcout << "  --package-type=dwfx\t specifies that a dwfx package should be created" << endl;
            return 0;
        }
    }

    DWFString zOutputFile( L"3DPublishOut.dwf" );
    bool bCreateDWF6Package = true;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != L'-')
        {
            zOutputFile.assign( argv[i] );
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(argv[i] , "--package-type=dwfx") == 0)
        {
            bCreateDWF6Package = false;
        }
    }

    try
    {
        //
        //  The file will be written out to 
        //
        DWFFile oDWF( zOutputFile );
        DWFPackagePublisher* pPublisher;

        if (bCreateDWF6Package)
        {
            pPublisher = DWFCORE_ALLOC_OBJECT( DWF6PackagePublisher( oDWF ) );
        }
        else
        {
            pPublisher = DWFCORE_ALLOC_OBJECT( DWFXPackagePublisher( oDWF ) );
        }

        pPublisher->setContentNavigationPresentationLabel( L"Model" );

        //
        //  To open the model we need to pass it a content library to store
        //  and modify the metadata. This content maybe used across multiple
        //  models.
        //
        /*
        DWFContentManager* pContentManager = DWFCORE_ALLOC_OBJECT( DWFContentManager );
        pPublisher->attachContentManager( pContentManager, true );

        DWFContent* pContent = pContentManager->getContent();
        */
        DWFContent* pContent = pPublisher->getContent();

        wcout << "\n\nStarting model 1\n" << endl;
        DWFModel* pModel = createModel( *pPublisher, pContent, false );

        wcout << "\n\nStarting model 2\n" << endl;
        DWFModel* pModel2 = createModel( *pPublisher, pContent, true );

        DWFData* pData   = createData(  *pPublisher, pContent );

        //
        // create the DWF
        //
        pPublisher->publish();

        DWFCORE_FREE_OBJECT( pPublisher );
        DWFCORE_FREE_OBJECT( pModel );
        DWFCORE_FREE_OBJECT( pModel2 );
        DWFCORE_FREE_OBJECT( pData );

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


class HPoint 
{ 
public:
    float        x;	
    float        y;	
    float        z;	
    inline void Set(float X,float Y,float Z=0.0) { x=X,y=Y,z=Z; }

}; 

void CreateSphere( float *center, 
                   float radius, 
                   int numsides, 
                   int *pcount_out, 
                   float **points_out, 
                   int *flistlen_out, 
                   int **faces_out );

DWFIncludeSegment CreateSphereInclude( DWFModel& rModel, 
                                       const DWFString& zIncName, 
                                       float fRadius, 
                                       float fR, float fG, float fB,
                                       const HPoint& center );

DWFIncludeSegment CreateUnitSphereInclude( DWFModel& rModel,
                                           const DWFString& zIncName,
                                           float fCol[],
                                           const HPoint& center );

void do_modelMetaData( DWFModel& rModel );

void saveContentToExternalFile( DWFModel& rModel );

void _playback_node( DWFContentPresentationModelViewNode* pViewNode );


class ContentCreationListener : public DWFPublishedContentElement::NotificationSink
{
public:

    virtual ~ContentCreationListener()
      throw()
    {
        ;
    }

    void notifyObjectCreated( DWFPublishedContentElement::tKey nKey,
                              DWFObject* pObject )
      throw()
    {
        wcout << L"Object created for key=" << nKey 
              << L"  with name=" << (const wchar_t*)(pObject->getLabel())
              << endl;

        _oKeyObjectMap[nKey] = pObject;
    }

    void notifyInstanceCreated( DWFPublishedContentElement::tKey nKey,
                                DWFInstance* pInstance )
      throw()
    {
        wcout << L"Instance created for key=" << nKey 
              << L"  with ID=" << (const wchar_t*)(pInstance->id())
              << endl << endl;

        _oKeyInstanceMap[nKey] = pInstance;
    }

    DWFObject* objectFromKey(DWFPublishedContentElement::tKey nKey)
    {
        return _oKeyObjectMap[nKey];
    }

    DWFInstance* instanceFromKey(DWFPublishedContentElement::tKey nKey)
    {
        return _oKeyInstanceMap[nKey];
    }

private:
    typedef map<DWFPublishedContentElement::tKey, DWFObject*> _tKeyObjectMap;
    _tKeyObjectMap _oKeyObjectMap;

    typedef map<DWFPublishedContentElement::tKey, DWFInstance*> _tKeyInstanceMap;
    _tKeyInstanceMap _oKeyInstanceMap;

};


DWFModel* createModel( DWFPublisher& rPublisher, DWFContent* pContent, bool bCompressAll )
{
    //
    // start with a new model
    //
    DWFModel* pModel = DWFCORE_ALLOC_OBJECT(DWFModel(
        L"3D Publish Model Test",       // title
        L"PublishMetaData.cpp",         // source
        L"",                            // source ID
        L"",                            // source HREF
        L"Model"                       // section label
        ));
    
    bool bUseContentDefinition = true;

    if (bUseContentDefinition)
    {
        pModel->open( pContent,
                     DWFModel::eHandednessNone,
                     DWFUnits::eCentimeters,
                     NULL,                      // transform
                     true,                      // default lights
                     false,                     // published edges
                     true );                    // silhouettes
    }
    else
    {
        pModel->open( NULL,
                     DWFModel::eHandednessNone,
                     DWFUnits::eCentimeters,
                     NULL,                      // transform
                     true,                      // default lights
                     false,                     // published edges
                     true,                      // silhouettes
                     0,
                     DWFPublisher::ePublishObjectDefinition );
    }

    W3DCamera oDefault;
    oDefault.setProjection( W3DCamera::eOrthographic );
    oDefault.setPosition( 0.0f, 0.0f, 10.0f );
    oDefault.setTarget( 4, -2, 0 );
    oDefault.setUpVector( 0, 1, 0 );
    oDefault.setField( 15.0f, 15.0f );

    pModel->createView( "default", oDefault );

    if (bCompressAll)
    {
        pModel->enableW3DCompression( true, true, true );
        pModel->enableNormalsCompression();
        pModel->enableVertexCompression();
    }

    //
    // create the graphics, etc.
    //
    float red[3]        = { 1, 0, 0 };
    float green[3]      = { 0, 1, 0 };
    float darkgreen[3]  = { 0, .5, 0 };
    float blue[3]       = { 0, 0, 1 };
    float darkblue[3]   = { 0, 0, .5 };
    float cyan[3]       = { 0, 1, 1 };
    float magenta[3]    = { 1, 0, 1 };
    float yellow[3]     = { 1, 1, 0 };
    float black[3]      = { 0, 0, 0 };
    float white[3]      = { 1, 1, 1 };
    float gray[3]       = { .5, .5, .5 };

    HPoint  center;

    center.Set( 0, 0, 0 );
    DWFIncludeSegment redInclude = CreateUnitSphereInclude( *pModel, "red", red, center );
    redInclude.addProperty( L"name", "Red Sphere" );
    redInclude.addProperty( "color", "red" );
    redInclude.addProperty( "location", "center" );
    redInclude.addProperty( "redvalue", "1" );

    center.Set( 2, 0, 0 );
    DWFIncludeSegment greenInclude = CreateUnitSphereInclude( *pModel, "green", green, center );
    greenInclude.addProperty( L"name", "Green Sphere" );
    greenInclude.addProperty( "color", "green" );
    greenInclude.addProperty( "location", "above" );
    greenInclude.addProperty( "greenvalue", "1" );

    center.Set( 2, -2, 0 );
    DWFIncludeSegment blueInclude = CreateUnitSphereInclude( *pModel, "blue", blue, center );
    blueInclude.addProperty( L"name", "Blue Sphere" );
    blueInclude.addProperty( "color", "blue" );
    blueInclude.addProperty( "location", "below" );
    blueInclude.addProperty( "bluevalue", "1" );

    center.Set( 4, 0, 0 );
    DWFIncludeSegment cyanInclude = CreateUnitSphereInclude( *pModel, "cyan", cyan, center );
    cyanInclude.addProperty( L"name", "Cyan Sphere" );
    cyanInclude.addProperty( "color", "cyan" );
    cyanInclude.addProperty( "location", "right" );
    cyanInclude.addProperty( "cyanvalue", "1" );

    center.Set( 4, -2, 0 );
    DWFIncludeSegment magentaInclude = CreateUnitSphereInclude( *pModel, "magenta", magenta, center );
    magentaInclude.addProperty( L"name", "Magenta Sphere" );
    magentaInclude.addProperty( "color", "magenta" );
    magentaInclude.addProperty( "location","left" );
    magentaInclude.addProperty( "magentavalue","1" );

    center.Set( 5, -4, 0 );
    DWFIncludeSegment yellowInclude = CreateUnitSphereInclude( *pModel, "yellow", yellow, center );
    yellowInclude.addProperty( L"name", "Yellow Sphere" );
    yellowInclude.addProperty( "color", "yellow" );
    yellowInclude.addProperty( "location","front" );
    yellowInclude.addProperty( "yellowvalue","1" );

    center.Set( 5, -6, 0 );
    DWFIncludeSegment blackInclude = CreateUnitSphereInclude( *pModel, "black", black, center );
    blackInclude.addProperty( L"name", "Black Sphere" );
    blackInclude.addProperty( "color", "black" );
    blackInclude.addProperty( "location","back" );
    blackInclude.addProperty( "blackvalue","1" );

    center.Set( 9, 0, 0 );
    DWFIncludeSegment whiteInclude = CreateUnitSphereInclude( *pModel, "white", white, center );
    whiteInclude.addProperty( L"name", "White Sphere" );
    whiteInclude.addProperty( "color", "white" );
    whiteInclude.addProperty( "location","white" );
    whiteInclude.addProperty( "whitevalue","1" );

    center.Set( 7, 0, 0 );
    DWFIncludeSegment grayInclude = CreateUnitSphereInclude( *pModel, "gray", gray, center );
    grayInclude.addProperty( L"name", "Gray Sphere" );
    grayInclude.addProperty( "color", "gray" );
    grayInclude.addProperty( "location","gray" );
    grayInclude.addProperty( "grayvalue","1" );

    center.Set( 7, -4, 0 );
    DWFIncludeSegment darkgreenInclude = CreateUnitSphereInclude( *pModel, "darkgreen", darkgreen, center );
    darkgreenInclude.addProperty( L"name", "Dark Green Sphere" );
    darkgreenInclude.addProperty( "color", "darkgreen" );
    darkgreenInclude.addProperty( "location","darkgreen" );
    darkgreenInclude.addProperty( "darkgreenvalue","1" );

    center.Set( 7, -6, 0 );
    DWFIncludeSegment darkblueInclude = CreateUnitSphereInclude( *pModel, "darkblue", darkblue, center );
    darkblueInclude.addProperty( L"name", "Dark blue Sphere" );
    darkblueInclude.addProperty( "color", "darkblue" );
    darkblueInclude.addProperty( "location","darkblue" );
    darkblueInclude.addProperty( "darkbluevalue","1" );

    //////////////////////////////////////////////////

    DWFSegment oRootSegment = pModel->openSegment();

    oRootSegment.open();

    DWFSegment oPart1 = oRootSegment.openSegment();
    oPart1.open( "Part1");
    {
        oPart1.include( redInclude, DWFString( L"This name will not show up" ) );
        oPart1.addProperty( L"name", L"Part 1" );
    }
    oPart1.close();

    DWFSegment oPart2 = oRootSegment.openSegment();
    oPart2.open( "Part2");
    {
        oPart2.include( greenInclude, DWFString( L"Naming includes on a named segment does no good" ) );
        oPart2.addProperty( L"name", L"Part 2" );
        oPart2.addProperty( "greenIncProperty", "Some value here" );
        oPart2.addProperty( "greenIncProperty", "in a category", "Cat6" );
    }
    oPart2.close();

    DWFSegment oPart3 = oRootSegment.openSegment();
    oPart3.open( "Part3");
    {
        oPart3.include( cyanInclude );
        oPart3.addProperty( L"name", L"Part 3" );
        DWFSegment oSubpartU = oPart3.openSegment();
        oSubpartU.open();
        {
            oSubpartU.include( yellowInclude );
            oSubpartU.addProperty( L"name", L"Sub Part U" );
        }
        oSubpartU.close();
        DWFSegment oSubpartN = oPart3.openSegment();
        oSubpartN.open( "This name will work" );
        {
            oSubpartN.include( blackInclude );
            oSubpartN.addProperty( L"name", L"Sub Part N" );
        }
        oSubpartN.close();
    }
    oPart3.close();

    DWFSegment oPart4 = oRootSegment.openSegment();
    oPart4.open( "Part 4" );
    {
        oPart4.include( grayInclude );
        oPart4.addProperty( L"name", L"Part 4" );

        DWFSegment oUnnamed = oPart4.openSegment();
        oUnnamed.open();
        {
            oUnnamed.include( darkgreenInclude, false );


            DWFSegment oUn2 = oUnnamed.openSegment();
            oUn2.open();
            {
                oUn2.include( darkblueInclude );
            }
            oUn2.close();
       }
        oUnnamed.close();
    }
    oPart4.close();
    oPart4.hideFromDefaultModel();

    DWFSegment oPart5 = oRootSegment.openSegment();
    oPart5.open( "No Graphics Associated" );
    {
        DWFSegment oNG1 = oPart5.openSegment();
        oNG1.open( "Grease" );
        {
            oNG1.addProperty( "Texture", "Gooey" );
            oNG1.addProperty( "Volume", "2", "", "integer", "mL" );
        }
        oNG1.close();

        DWFSegment oNG2 = oPart5.openSegment();
        oNG2.open( "Paint" );
        {
            oNG2.addProperty( "Quantity", "1", "NonGraphics", "", "gallon" );
            oNG2.addProperty( "Color", "Sage Green", "NonGraphics" );
        }
        oNG2.close();
    }
    oPart5.close();

    // test new primatives
    DWFSegment oPart6 = oRootSegment.openSegment();
    oPart6.open( "Part 6" );
    {
        TK_Color_RGB& rColor = oPart6.getColorRGBHandler();
        rColor.SetGeometry( TKO_Geo_Face );
        rColor.SetRGB( 1.0F, 1.0F, 1.0F );
        rColor.serialize();

        TK_Cylinder& rCylinder = oPart6.getCylinderHandler();
        rCylinder.SetCaps(3);
        rCylinder.SetAxis(0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 0.0f);
        rCylinder.SetRadius(0.5f);
        rCylinder.serialize();

        TK_Sphere& rSphere = oPart6.getSphereHandler();
        rSphere.SetCenter(0.0f, 10.0f, 0.0f);
        rSphere.SetRadius(0.5f);
        rSphere.serialize();

        TK_Point& rMarker = oPart6.getMarkerHandler();
        rMarker.SetPoint(0.0f, 12.0f, 0.0f);
        rMarker.serialize();
    }
    oPart6.close();

    //
    // Test ambient lights
    //
    DWFSegment lightSeg = oRootSegment.openSegment(); 
    lightSeg.open();
    {                 
        TK_Point& oLight = lightSeg.getDistantLightHandler();
        oLight.SetPoint(1.0F, -1.0F, 1.0F);
        oLight.SetOptions(TKO_Light_Camera_Relative);
        oLight.serialize();     

        // Publish the diffuse color and specular color
        TK_Color& oColor = lightSeg.getColorHandler();
        oColor.SetDiffuse(1.0F, 0.5F, 0.75F);
        oColor.SetSpecular(1.0F, 1.0F, 1.0F);
        oColor.SetGeometry(TKO_Geo_Extended | TKO_Geo_Light);
        oColor.serialize();

        // Publish the ambient color. Inconvieniently, we have to open the local lighting attributes to do this.
        lightSeg.openLocalLightAttributes();
        TK_Color& oAmbientColor = lightSeg.getColorHandler();
        oAmbientColor.SetDiffuse(0.6F, 0.0F, 0.6F);
        oAmbientColor.SetGeometry(TKO_Geo_Ambient);
        oAmbientColor.serialize();
        lightSeg.closeLocalLightAttributes();
    }

    lightSeg.close();             


    oRootSegment.close();

    pModel->setBoundingSphere( 0.0f, 0.0f, 0.0f, 15.0f );

    //
    // we can also add an icon image for the section "type"
    //

    //
    // open as a streaming binary file
    //
    DWFStreamFileDescriptor* pIconFile = new DWFStreamFileDescriptor( "icon.png", "rb" );
    pIconFile->open();

    //
    // create a stream and attach the file descriptor
    //
    DWFFileInputStream* pIconStream = new DWFFileInputStream;
    pIconStream->attach( pIconFile, true );

    //
    // create a icon image and attach the file stream
    //
    DWFImage* pIcon = DWFCORE_ALLOC_OBJECT( DWFImage(DWFMIME::kzMIMEType_PNG, 
                                                          DWFImage::eIcon, 
                                                          24, 
                                                          218, 
                                                          213) );

    if (pIcon)
    {
        pIcon->attach( pIconStream, true );

        //
        // add to the model
        //
        pModel->addResource( pIcon );
    }

    DWFUUID oUUID;
    DWFString zDV1ID = oUUID.next(true);

    size_t nURILen = wcslen(L"presentation") + 1 + wcslen(zDV1ID) + 1;
    wchar_t* pURI = new wchar_t[nURILen];
    swprintf( pURI, nURILen, L"presentation=%s", (const wchar_t*)zDV1ID );
    pModel->setInitialURI( DWFString(pURI) );
    delete[] pURI;

    //
    // close the model
    //
    pModel->close();

    //
    //  if the content definition is not being used then publish and return
    //
    if (!bUseContentDefinition)
    {
        pModel->publish( rPublisher );
        return pModel;
    }

    ContentCreationListener listener;
    DWFPublishedContentElement::Visitor* pVisitor = dynamic_cast<DWFPublishedContentElement::Visitor*>(rPublisher.getPublishedObjectVisitor());
    pVisitor->setNotificationSink( &listener );

    DWFContentPresentationResource* pPresentationResource = DWFCORE_ALLOC_OBJECT( DWFContentPresentationResource( DWFXML::kzRole_ContentPresentation ) );
    ((DWFResourceContainer&)*pModel).addResource(pPresentationResource, true);

    wcout << L"Model HREF = " << (const wchar_t*)(pModel->href()) << endl << endl;

    //
    // publish the model "into" the publisher
    //
    pModel->publish( rPublisher );


    DWFPublishedContentElement::tKey part1Key = oPart1.key();
    DWFPublishedContentElement::tKey part2Key = oPart2.key();
    DWFPublishedContentElement::tKey part3Key = oPart3.key();
    DWFPublishedContentElement::tKey part4Key = oPart4.key();

    DWFInstance* pPart1Instance = listener.instanceFromKey(part1Key); (void) pPart1Instance;
    DWFInstance* pPart2Instance = listener.instanceFromKey(part2Key); (void) pPart2Instance;
    DWFInstance* pPart3Instance = listener.instanceFromKey(part3Key); (void) pPart3Instance;
    DWFInstance* pPart4Instance = listener.instanceFromKey(part4Key); (void) pPart4Instance;


    DWFContentPresentation* pPresentation = DWFCORE_ALLOC_OBJECT( DWFContentPresentation(L"View Reps Sample", DWFContentPresentation::kzID_Views ) );
        DWFContentPresentationView* pDefaultView = DWFCORE_ALLOC_OBJECT( DWFContentPresentationView(L"Default View") );

            DWFPropertyReference * pPropRef = DWFCORE_ALLOC_OBJECT( DWFPropertyReference );
            pPropRef->setReference( L"Name", L"Cat", L"PSID");
            pDefaultView->addPropertyReference(pPropRef);

            DWFContentPresentationNode* pDesignViews = DWFCORE_ALLOC_OBJECT( DWFContentPresentationNode(L"Design Views") );
            {

                //
                // View Node: DV1
                //
                // This one resets the colors of all instances, then sets the color of instance1 to Red.
                //
                DWFContentPresentationModelViewNode* pDV1 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Reset all colors, then change color of first instance") );
                pDV1->setID( zDV1ID );

                pDV1->setResetFlags(DWFModelScene::eColor);
                if(pPart1Instance)
                {
                    pDV1->setCurrentInstance(*pPart1Instance);
                    pDV1->lockAttribute( DWFModelScene::eColor );
                    TK_Color& rDV1Color = pDV1->getColorHandler();
                    rDV1Color.SetDiffuse(0.0f, 1.00f, 0.00f);
                    rDV1Color.serialize();
                }
                pDV1->close();
                pDesignViews->addChild(pDV1);



                //
                // View Node: DV1.5
                //
                // This one resets the colors of all instances, then sets the color of instance1 to Red, with attribute locking.
                //
                DWFContentPresentationModelViewNode* pDV1_5 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Reset all colors, then change color of first instance - with attribute locking") );
                pDV1_5->setResetFlags(DWFModelScene::eColor);
                if(pPart1Instance)
                {
                    pDV1_5->setCurrentInstance(*pPart1Instance);
                    TK_Color& rDV1Color = pDV1_5->getColorHandler();
                    rDV1Color.SetDiffuse(1.0f, 0.00f, 0.00f);
                    rDV1Color.serialize();
                    pDV1_5->lockAttribute(DWFModelScene::eColor);
                }
                pDV1_5->close();
                pDesignViews->addChild(pDV1_5);



                //
                // View Node: DV2
                //
                // This one resets the colors of all instances, then sets the color of each instance.
                //
                DWFContentPresentationModelViewNode* pDV2 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Change colors of all instances") );
                pDV2->setResetFlags(DWFModelScene::eColor);

                if(pPart1Instance)
                {
                    pDV2->setCurrentInstance(*pPart1Instance);
                    TK_Color& rDV2Color = pDV2->getColorHandler();
                    rDV2Color.SetDiffuse(0.25f, 0.50f, 0.75f);
                    rDV2Color.serialize();
                }

                if(pPart2Instance)
                {
                    pDV2->setCurrentInstance(*pPart2Instance);
                    TK_Color& rDV2Color = pDV2->getColorHandler();
                    rDV2Color.SetSpecular(0.75f, 0.50f, 0.25f);
                    rDV2Color.serialize();
                }

                if(pPart3Instance)
                {
                    pDV2->setCurrentInstance(*pPart3Instance);
                    TK_Color& rDV2Color = pDV2->getColorHandler();
                    rDV2Color.SetMirror(0.50f, 0.50f, 0.50f);

                    rDV2Color.SetEnvironmentName("ENV");
                    rDV2Color.SetBumpName("Bump");

                    rDV2Color.serialize();
                }

                if(pPart4Instance)
                {
                    pDV2->setCurrentInstance(*pPart4Instance);
                    TK_Color& rDV2Color = pDV2->getColorHandler();
                    rDV2Color.SetEmission(0.75f, 0.75f, 0.75f);
                    rDV2Color.serialize();
                }

                pDV2->close();
                pDesignViews->addChild(pDV2);




                //
                // View Node: DV3
                //
                // This one resets the colors and camera to their initial state.
                //
                DWFContentPresentationModelViewNode* pDV3 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Reset camera and all instance colors") );
                pDV3->setCamera(oDefault, true);
                pDV3->setResetFlags(DWFModelScene::eColor);
                pDV3->close();
                pDesignViews->addChild(pDV3);

#ifdef  _DEBUG
                wcout << L"Playing back DESIGN VIEWS...." << endl;
                _playback_node( pDV1 );
                _playback_node( pDV1_5 );
                _playback_node( pDV2 );
                _playback_node( pDV3 );
#endif

            }
            pDefaultView->addNode(pDesignViews);


            DWFContentPresentationNode* pPositionalVariations = DWFCORE_ALLOC_OBJECT( DWFContentPresentationNode(L"Positional Variations") );
            {


                //
                // View Node: PV1
                //
                // This one just resets the positions of all instances.
                //
                DWFContentPresentationModelViewNode* pPV1 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Reset all positions") );
                pPV1->setResetFlags(DWFModelScene::eModellingMatrix);
                pPV1->close();
                pPositionalVariations->addChild(pPV1);
                

                //
                // View Node: PV2
                //
                // This one resets the positions of all instances, and then sets the matrix of instance1
                //
                DWFContentPresentationModelViewNode* pPV2 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Move first instance") );
                pPV2->setResetFlags(DWFModelScene::eModellingMatrix);
                if(pPart1Instance)
                {
                    pPV2->setCurrentInstance(*pPart1Instance);
                    TK_Matrix& rPV1Matrix = pPV2->getModellingMatrixHandler();
                    /*
                    float anTransform[16] = {   1.0f,  0.0f,  0.0f, 0.0f,
                                                0.0f,  1.0f,  0.0f, 0.0f,
                                                0.0f,  0.0f,  1.0f, 0.0f,
                                                2.33f, 0.67f, 0.0f, 1.0f };
                    */
                    // Make up a wacko matrix, as it uses lots more characters, and tests serialization of long strings.
                    float anTransform[16] = {   1.1111111111f, 0.0000000001f, 0.0000000001f, 0.0000000001f,
                                                0.0000000001f, 1.1111111111f, 0.0000000001f, 0.0000000001f,
                                                0.0000000001f, 0.0000000001f, 1.1111111111f, 0.0000000001f,
                                                2.3300000001f, 0.6700000001f, 0.0000000001f, 1.1111111111f };
                    rPV1Matrix.SetMatrix( anTransform );
                    rPV1Matrix.serialize();
                }
                pPV2->close();
                pPositionalVariations->addChild(pPV2);

#ifdef  _DEBUG
                wcout << L"Playing back POSITIONAL VARIATIONS...." << endl;
                _playback_node( pPV1 );
                _playback_node( pPV2 );
#endif

            }                    
            pDefaultView->addNode(pPositionalVariations);

            DWFContentPresentationNode* pMiscVariations = DWFCORE_ALLOC_OBJECT( DWFContentPresentationNode(L"Misc") );
            {
                //
                // View Node: MV1
                //
                // This one resets the visibilities of all instances
                //
                DWFContentPresentationModelViewNode* pMV1 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Reset all visibilities") );
                pMV1->setResetFlags(DWFModelScene::eInstanceVisibility);
                pMV1->close();
                pMiscVariations->addChild(pMV1);
                
                //
                // View Node: MV2
                //
                // This one turns off visibility of instance1
                //
                DWFContentPresentationModelViewNode* pMV2 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Hide first instance") );
                if(pPart1Instance)
                {
                    pMV2->setCurrentInstance(*pPart1Instance);
                    pMV2->setVisibility(false);
                }
                pMV2->close();
                pMiscVariations->addChild(pMV2);
                
                //
                // View Node: MV3
                //
                // This one makes instance2 transparent
                //
                DWFContentPresentationModelViewNode* pMV3 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Make second instance transparent") );
                if(pPart2Instance)
                {
                    pMV3->setCurrentInstance(*pPart2Instance);
                    pMV3->setTransparency(true);
                }
                pMV3->close();
                pMiscVariations->addChild(pMV3);
                
                //
                // View Node: MV4
                //
                // This one makes cutting planes
                //
                DWFContentPresentationModelViewNode* pMV4 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Make cutting planes") );
                TK_Cutting_Plane &rCuttingPlane = pMV4->getCuttingPlaneHandler();
                float planeData[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f,
                                        2.33f, 0.67f, 0.0f, 1.0f };
                rCuttingPlane.SetPlanes(4, planeData);

                //
                // Due to a possible bug in the xml parser I cant read back the namespace declaration, this screws up
                // the reader's attempt to write back what it read. So Im commenting this for now
                //
                //DWFModelSceneChangeHandler& rHandler = dynamic_cast<DWFModelSceneChangeHandler&>(rCuttingPlane);
                //pMV4->addNamespace( L"PublishMetaData", L"http://www.autodesk.com/PublishMetaData/sample.xsd" );
                //rHandler.addUserAttribute( L"numPlanes", L"4", L"PublishMetaData:" );
                //rHandler.addUserAttribute( L"hidden", L"false", L"PublishMetaData:" );
                //rHandler.addUserAttribute( L"flip", L"true", L"PublishMetaData:" );
                //rHandler.addUserAttribute( L"source", L"publisher", L"PublishMetaData:" );

                rCuttingPlane.serialize();
                pMV4->close();
                //for fun, mark pv4 as hidden
                pMV4->setHidden(true);
                pMiscVariations->addChild(pMV4);

                //
                //
                // View Node: MV5
                //
                // This one inserts others
                //
                DWFContentPresentationModelViewNode* pMV5 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationModelViewNode(L"Insertion of others") );
                pMV5->insertSceneChanges(*pMV1);
                pMV5->insertSceneChanges(*pMV2);
                pMV5->insertSceneChanges(*pMV3);
                pMV5->insertSceneChanges(*pMV4);
                pMV5->close();
                pMiscVariations->addChild(pMV5);

                // Refrence Nodes: 
                //
                //
                DWFContentPresentationReferenceNode* pRN1 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode(L"Ref node 1") );
                pRN1->setURI("www.cnn.com");
                DWFContentPresentationReferenceNode* pRN2 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode(L"Ref node 2") );
                pRN2->setURI("www.slashdot.com");
                pRN1->addChild(pRN2);
                DWFContentPresentationReferenceNode* pRN3 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode(L"Ref node 3") );
                pRN3->setURI("www.dealnews.com");
                //for fun, mark rn3 as hidden
                pRN3->setHidden(true);
                pRN2->addChild(pRN3);
                DWFContentPresentationReferenceNode* pRN4 = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode(L"Ref node 4") );
                pRN2->addChild(pRN4);
                pRN4->setURI("www.tomshardware.com");
                DWFObject* pPart1Object = listener.objectFromKey(part1Key);
                pRN4->setContentElement(*pPart1Object);
                pRN4->setPropertyReferenceURI(*pPropRef, L"Propref URI");
                pRN4->setResource(*pPresentationResource);

                pMiscVariations->addChild(pRN1);
                
#ifdef  _DEBUG
                wcout << L"Playing back OTHERS...." << endl;
                _playback_node( pMV1 );
                _playback_node( pMV2 );
                _playback_node( pMV3 );
                _playback_node( pMV4 );
                //_playback_node( pMV5 );
#endif

            }

            // just to test it, let's mark the misc variations node hidden
            pMiscVariations->setHidden(true);

            pDefaultView->addNode(pMiscVariations);

        pPresentation->addView(pDefaultView);
    pPresentationResource->addPresentation(pPresentation);

#ifdef _DEBUG
    saveContentToExternalFile( *pModel );
#endif

    pVisitor->removeNotificationSink( &listener );

    return pModel;
}

#define countof(x) (int)(sizeof(x)/sizeof(x[0]))


const float PI=3.1415926f;


void CreateSphere( float *center, 
                   float radius, 
                   int numsides, 
                   int *pcount_out, 
                   float **points_out, 
                   int *flistlen_out, 
                   int **faces_out )
{
    int i, j, htile, wtile, pts_cnt, flist_cnt;
    float theta, phi, dt, dp, x, y, z;
    HPoint *points;
    int *faces;

    htile = numsides/2;
    wtile = numsides;
    pts_cnt = htile*wtile;
    points = new HPoint[ pts_cnt ];
    // faces need wtile + 1 numbers to specify each sphere end cap
    // and 5*numsides numbers for each strip between the end caps.
    // There will be numsides/2-1 of these strips.
    flist_cnt = (wtile+1)*2 + 5*(htile-1)*wtile;
    faces = new int[ flist_cnt ];

    dt = 2.0f * PI / wtile;
    dp = 1.0f * PI / (htile+1);
    phi = dp;
    for( i = 0 ; i < htile ; i++ ) {
        theta = 0;
        for( j = 0 ; j < wtile ; j++ ) {
            x = radius * (float)cos(theta) * (float)sin(phi);
            y = radius * (float)cos(phi);
            z = radius * (float)sin(theta) * (float)sin(phi);
            points[ i*wtile + j ].Set( center[0] + x, center[1] + y, center[2] + z );
            theta += dt;
        }
        phi += dp;
    }

    // set the number of points in the end caps of the sphere in the connectivity list
    faces[5*(htile-1)*wtile] = wtile;
    faces[5*(htile-1)*wtile+wtile+1] = wtile;
    for (i = 0; i < (htile-1); i++)
    {
        for (j = 0; j < wtile; j++)
        {
            // generate the connectivity list for the side quads of the cylinder
            faces[(i*wtile + j)*5] = 4; // number of points in this polygon
            faces[(i*wtile + j)*5+1]=i*wtile + j;
            faces[(i*wtile + j)*5+2]=i*wtile + (j+1)%wtile; //next point along circle; wrap at numsides
            faces[(i*wtile + j)*5+3]=(i+1)*wtile + (j+1)%wtile;  // correcsponding point at top of cylinder
            faces[(i*wtile + j)*5+4]=(i+1)*wtile + j; //next point along circle; wrap at numsides
        }
    }
    for (j = 0; j < wtile; j++)
    {
        // generate the connectivity list for the ends of the cylinder
        faces[(htile-1)*wtile*5 + j + 1]= (wtile - j - 1);
        faces[(htile-1)*wtile*5 + j + 1 + wtile + 1]= wtile*(htile-1) + j;
    }
    *pcount_out = pts_cnt;
    *points_out = (float *)points;
    *flistlen_out = flist_cnt;
    *faces_out = (int *)faces;
}

DWFIncludeSegment CreateSphereInclude( DWFModel& rModel, 
                                       const DWFString& zIncName, 
                                       float fRadius, 
                                       float fR, float fG, float fB,
                                       const HPoint& center )
{
    float bbox[6];
    float *points, *normals;
    int p_count, flist_length, *flist, i;

    //
    // the first thing to do is create an include segment
    // that defines a collection of graphics that will be
    // reused throughout the model
    //
    DWFIncludeSegment oSphere = rModel.openIncludeSegment();

    //
    // segments are always first acquired from their aggregating component (i.e. Model or another Segment)
    // and then opened.  Include segments must be opened with a name. 
    //
    oSphere.open( zIncName );
    {
        //TK_Color_RGB& rColor = oSphere.getColorRGBHandler();
        //rColor.SetGeometry( TKO_Geo_Face );
        //rColor.SetRGB( fR, fG, fB );
        //rColor.serialize();

        TK_Color& rColor = oSphere.getColorHandler();
        rColor.SetDiffuse( fR, fG, fB );
        rColor.SetGeometry( TKO_Geo_Face );
        rColor.serialize();
        //
        // we will define our sphere as a shell
        // notice that the handler is returned as a reference
        // be sure to obtain it this way; if you try and copy
        // it, you will get a compile time error since there
        // is no copy constructor defined for these objects
        //
        TK_Shell& rSphere = oSphere.getShellHandler();
        {
            //
            // it's always a good idea to set the bounding volume on your shells.
            //
            bbox[0] = center.x - fRadius;
            bbox[1] = center.y - fRadius;
            bbox[2] = center.z - fRadius;
            bbox[3] = center.x + fRadius;
            bbox[4] = center.y + fRadius;
            bbox[5] = center.z + fRadius;
            rSphere.SetBounding( bbox );

            //
            // Original HOOPS/Stream comment:
            // This is where you will go out to your gfx database and get the data defining the shell.
            // We're using a simple sphere generator to create a point cloud and some connectivity
            //
            CreateSphere( (float *)&center, fRadius, 30, &p_count, &points, &flist_length, &flist );
            
            //
            // set the vertex list for the shell
            //
            rSphere.SetPoints( p_count, points );

            //
            // set the face list for the shell
            //
            rSphere.SetFaces( flist_length, flist );

                //
                // HOOPS 3DGS will calculate simple average normals from the faces
                // usually this adequate and saves space if we don't need to store
                // normals data in the file.  Edge normals should always be defined
                // with original data.  Also, normals are quantized by default to save
                // space while preserving visual fidelity.  A sphere is going to be
                // the worst case scenario for this compression mechanism but it is
                // worth adjusting this parameter as much as possible to acheive the
                // best visual results in the smallest file.
                //
            normals = new float[ 3 * p_count ];
            for ( i = 0 ; i < p_count * 3 ; i += 3) 
            {
                normals[ i + 0 ] = points[ i + 0 ] - center.x;
                normals[ i + 1 ] = points[ i + 1 ] - center.y;
                normals[ i + 2 ] = points[ i + 2 ] - center.z;
            }
         
            rSphere.SetVertexNormals( normals );

            //
            // NOTE here that the opcode handlers will always
            // copy the data that they are given
            //
            delete [] normals;

            //
            // clean up
            //
            //delete [] indices;
            delete [] points;
            delete [] flist;
        }
        
        //
        // once we are completely finished with a handler,
        // we must call serialize().  this will write all
        // of our graphics into the stream and reset the handler
        // for future use
        //
        rSphere.serialize();
    }

    //
    // close the segment - this will serialize everything about the segment
    // into the graphics stream. we can now use this segment as a reference
    // part in our scene graph
    //
    oSphere.close();

    return oSphere;
}

DWFIncludeSegment CreateUnitSphereInclude( DWFModel& rModel,
                                           const DWFString& zIncName,
                                           float fCol[3],
                                           const HPoint& center )
{
    return CreateSphereInclude( rModel, zIncName, 1.0, fCol[0], fCol[1], fCol[2], center );
}


void saveContentToExternalFile( DWFModel& rModel )
{
    DWFContent* pContent = rModel.getContent();
    if (pContent == NULL)
    {
        return;
    }

    /////////////////////////

    DWFFile oContentFile( "content.Global.Debug.xml" );

    DWFStreamFileDescriptor oCGFile( oContentFile, L"w+" );
    oCGFile.open();

    DWFFileOutputStream oOStream;
    oOStream.attach( &oCGFile, false );

    DWFXMLSerializer xmlSerializer( *(pContent->getIDProvider()) );
    xmlSerializer.attach( oOStream );

    pContent->serializeXML( xmlSerializer, DWFPackageWriter::eGlobalContent );

    xmlSerializer.detach();
    oOStream.detach();

    oCGFile.close();

    /////////////////////////

    //oContentFile = "content.Section.Debug.xml";

    //DWFStreamFileDescriptor oCSFile( oContentFile, L"w+" );
    //oCSFile.open();

    //oOStream.attach( &oCSFile, false );
    //xmlSerializer.attach( oOStream );

    //pContent->setSectionForSerialization( rModel.id() );

    //xmlSerializer.startElement( DWFXML::kzElement_ContentResource, DWFXML::kzNamespace_DWF );
    //
    //DWFString zTempString( L"DWF-ContentResource:" );
    //zTempString.append( _DWF_FORMAT_SECTION_CONTENT_VERSION_CURRENT_STRING );
    //xmlSerializer.addAttribute( L"dwf", zTempString, L"xmlns:" );
    //zTempString.append( L" " );
    //zTempString.append( _DWF_FORMAT_SECTION_CONTENT_SCHEMA_LOCATION_WIDE_STRING );
    //xmlSerializer.addAttribute( DWFXML::kzAttribute_SchemaLocation, zTempString, L"xmlns:" );
    //xmlSerializer.addAttribute( DWFXML::kzAttribute_Version, _DWF_FORMAT_SECTION_CONTENT_VERSION_CURRENT_STRING );

    //pContent->serializeXML( xmlSerializer, DWFPackageWriter::eSectionContent );
    //pContent->setSectionForSerialization( "" );

    //xmlSerializer.endElement();
    //
    //xmlSerializer.detach();
    //oOStream.detach();

    //oCSFile.close();

    /////////////////////////
}

void _playback_node( DWFContentPresentationModelViewNode* pViewNode )
{
    class local_scene : public DWFModelScene
    {
    public:

        local_scene() {;}
        virtual ~local_scene()
	  throw() {;}

        void setCurrentInstanceID( const DWFString& zID ) throw( DWFException )
        {
            wcout << endl << "Current instance now: " << (const wchar_t*)zID << endl;
            wcout << "============================================"  << endl;
        }

        void setCamera( W3DCamera& /*rCamera*/, bool /*bSmoothTransition*/) throw( DWFException )
        {
            wcout << L"Camera changed" << endl;
        }

        void setGeometricVariationIndex( unsigned int nIndex ) throw( DWFException )
        {
            wcout << L"New variation active: " << nIndex << endl;
        }

        void setVisibility( bool bVisibility ) throw( DWFException )
        {
            if (bVisibility)
            {
                wcout << L"Showing instance" << endl;
            }
            else
            {
                wcout << L"Hiding instance" << endl;
            }
        }

        void setTransparency( bool bTransparency ) throw( DWFException )
        {
            if (bTransparency)
            {
                wcout << L"Ghosting instance" << endl;
            }
            else
            {
                wcout << L"Unghosting instance" << endl;
            }
        }

        void lockAttribute( teAttributeType eType ) throw( DWFException )
        {
            wcout << L"Locking attribute: " << eType << endl;
        }

        void unlockAttribute( teAttributeType eType ) throw( DWFException )
        {
            wcout << L"Unlocking attribute: " << eType << endl;
        }

        void open() throw( DWFException )
        {
            wcout << endl << L"Scene opened"  << endl;
        }

        void close() throw( DWFException )
        {
            wcout << L"Scene closed" << endl;
        }

        void notify( BBaseOpcodeHandler* pHandler,
                     const void*         /*pTag = NULL*/ )
            throw( DWFException )
        {
            if (dynamic_cast<TK_Color*>(pHandler))
            {
                wcout << L"Color changed" << L"\t" << endl;
            }
            else if (dynamic_cast<TK_Cutting_Plane*>(pHandler))
            {
                wcout << L"Cutting plane added" << L"\t" << endl;
            }
            else if (dynamic_cast<TK_Matrix*>(pHandler))
            {
                wcout << L"Matrix changed" << L"\t" << endl;
            }
            else if (dynamic_cast<TK_Visibility*>(pHandler))
            {
                wcout << L"Visibility changed" << L"\t" << endl;
            }
        }
    };


    try
    {
        local_scene oScene;
        pViewNode->getModelSceneChanges( oScene );
    }
    catch (DWFException& ex)
    {
        wcout << ex.type() << L": " << ex.message() << endl;
        wcout << L"(function) " << ex.function() << endl;
        wcout << L"(file) " << ex.file() << endl;
        wcout << L"(line) " << ex.line() << endl;
    }
}

DWFData* createData( DWFPublisher& rPublisher, DWFContent* pContent)
{
    DWFData* pData = DWFCORE_ALLOC_OBJECT( DWFData( L"PublishMetaData.cpp", L"DWFToolkit Samples", L"" ));

    pData->addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"creator", L"PublishMetaData.cpp", L"", L"", L"")), true );

    //
    // open as a streaming binary file
    //
    DWFStreamFileDescriptor* pDefaultIconFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( "default.png", "rb" ) );
    pDefaultIconFile->open();

    //
    // create a stream and attach the file descriptor
    //
    DWFFileInputStream* pDefaultIconStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
    pDefaultIconStream->attach( pDefaultIconFile, true );

    DWFImageResource* pDefaultIconResource = DWFCORE_ALLOC_OBJECT( DWFImageResource("Default Icon", DWFXML::kzRole_Icon, DWFMIME::kzMIMEType_PNG) );
    pDefaultIconResource->setInputStream(pDefaultIconStream);
    ((DWFResourceContainer*)pData)->addResource( pDefaultIconResource, true );


    //
    // open as a streaming binary file
    //
    DWFStreamFileDescriptor* pExpandedIconFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor( "expanded.png", "rb" ) );
    pExpandedIconFile->open();

    //
    // create a stream and attach the file descriptor
    //
    DWFFileInputStream* pExpandedIconStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
    pExpandedIconStream->attach( pExpandedIconFile, true );

    DWFImageResource* pExpandedIconResource = DWFCORE_ALLOC_OBJECT( DWFImageResource("Expanded Icon", DWFXML::kzRole_Icon, DWFMIME::kzMIMEType_PNG) );
    pExpandedIconResource->setInputStream(pExpandedIconStream);
    ((DWFResourceContainer*)pData)->addResource( pExpandedIconResource, true );

    DWFContentPresentationResource* pPresentationResource = DWFCORE_ALLOC_OBJECT( DWFContentPresentationResource( DWFXML::kzRole_ContentPresentation ) );
    
        DWFContentPresentation* pPresentation = DWFCORE_ALLOC_OBJECT( DWFContentPresentation(L"BOM", DWFContentPresentation::kzID_Table) );
        
            DWFContentPresentationView* pPartsListView = DWFCORE_ALLOC_OBJECT( DWFContentPresentationView(L"PartsList", L"Parts List") );

                //
                // add the columns for this view
                //
                DWFPropertyReference* pColor = DWFCORE_ALLOC_OBJECT( DWFPropertyReference );
                pColor->setReference(L"color", L"", L"");
                pPartsListView->addPropertyReference(pColor);

                DWFPropertyReference* pLocation = DWFCORE_ALLOC_OBJECT( DWFPropertyReference );
                pLocation->setReference(L"location", L"", L"");
                pPartsListView->addPropertyReference(pLocation);

                //
                // iterate through the content and grab all entities and publish them into a BOM presentation
                //
                DWFEntity::tMap::Iterator* piEntity = pContent->getEntities();

                if (piEntity && piEntity->valid())
                {
                    for(; piEntity->valid(); piEntity->next())
                    {
                        DWFEntity* pEntity = piEntity->value();
                        DWFString zName = L"Unnamed Entity";
                        const DWFProperty* pName = pEntity->findProperty(L"name");
                        if (pName)
                        {
                            zName = pName->value();
                        }
                        DWFContentPresentationReferenceNode* pRow = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode( zName ) );

                        //
                        // iterate through the objects of this entity and add them as children of this row
                        //

                        //
                        // this isn't the most efficient way to do this
                        // it would be better to set up a map of vectors from each entity to its objects
                        // but Im not going to bother doing that
                        //

                        DWFObject::tMap::Iterator* piObject = pContent->getObjects();
                        for(; piObject && piObject->valid(); piObject->next())
                        {
                            DWFObject* pObject = piObject->value();
                            //
                            // if this object is realizing the current entity then add it as a child row
                            //
                            if (pObject->getEntity() == pEntity)
                            {
                                DWFString zName = L"Unnamed Part";
                                const DWFProperty* pName = pObject->findProperty(L"name");
                                if (pName)
                                {
                                    zName = pName->value();
                                }
                                DWFContentPresentationReferenceNode* pChildRow = DWFCORE_ALLOC_OBJECT( DWFContentPresentationReferenceNode( zName ) );
                                pRow->setContentElement( *pObject );
                                pRow->addChild(pChildRow);
                            }

                            pRow->setDefaultIconResource(*pDefaultIconResource);
                            pRow->setExpandedIconResource( *pExpandedIconResource);
                        }
                        DWFCORE_FREE_OBJECT(piObject);

                        pRow->setContentElement( *pEntity );
                        //
                        // give the cell corresponding to the Yellow Sphere row and the color column a URI
                        //
                        if (DWFCORE_COMPARE_WIDE_STRINGS( zName, L"Yellow Sphere" ) == 0)
                        {
                            pRow->setPropertyReferenceURI( *pColor, L"www.yellowsphere.com");
                        }


                        pPartsListView->addNode(pRow);
                    }
                }

                DWFCORE_FREE_OBJECT( piEntity );
        pPresentation->addView(pPartsListView);

    pPresentationResource->addPresentation(pPresentation);

    ((DWFResourceContainer*)pData)->addResource( pPresentationResource, true );

    pData->publish( rPublisher );

    return pData;
}
