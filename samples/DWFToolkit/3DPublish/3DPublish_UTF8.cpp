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

#include "dwf/package/ContentManager.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/publisher/model/Model.h"
#include "dwf/publisher/impl/DWF6PackagePublisher.h"
#include "dwf/publisher/impl/DWFXPackagePublisher.h"

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

//
//
// Some of this sample code is taken from the HOOPS/Stream toolkit (v.10.00) samples
//
//

#define  USE_NEW_CONTENT_DEFINITION_FOR_3D_PUBLISH

void CreateSphere( float *center, 
				  float radius, 
				  int numsides, 
				  int *pcount_out, 
				  float **points_out, 
				  int *flistlen_out, 
				  int **faces_out );

void do_model( DWFModel& rModel,
			  bool bIncludeNormals );

void do_publishProxyGraph( DWFPackagePublisher* pPackagePublisher );

//
// define this macro to enable texture decals
//
#define   TEST_MULTIPLE_DECAL_TEXTURES

#ifdef	 TEST_MULTIPLE_DECAL_TEXTURES
// create images
static unsigned const width = 256;
static unsigned const height = 256;
static unsigned char pixel_data1[height][width][4];
static unsigned char pixel_data2[height][width][4];

void gen_top_img(unsigned const w,
				 unsigned const h,
				 unsigned char * const pixel_data)
{
	unsigned row, col;
	
	for(row = 0; row < h; ++row){
		for(col = 0; col < w; ++col){
			if(row / 8 % 2){
				pixel_data[4*row*w+4*col+0] = 0xff;    // r
				pixel_data[4*row*w+4*col+1] = 0xff;    // g
				pixel_data[4*row*w+4*col+2] = 0xff;    // b
				pixel_data[4*row*w+4*col+3] = 0x30;    // alpha, 50% transparence
			} else {
				pixel_data[4*row*w+4*col+0] = 0x00;    // r
				pixel_data[4*row*w+4*col+1] = 0x00;    // g
				pixel_data[4*row*w+4*col+2] = 0x00;    // b
				pixel_data[4*row*w+4*col+3] = 0x00;    // alpha, 100% transparence
			}
		}
	}
}

void gen_bot_img(unsigned const w,
				 unsigned const h,
				 unsigned char * const pixel_data)
{
	unsigned row, col;
	
	for(row = 0; row < h; ++row){
		for(col = 0; col < w; ++col){
			if(col / 8 % 2){
				pixel_data[4*row*w+4*col+0] = 0xff;    // r
				pixel_data[4*row*w+4*col+1] = 0xff;    // g
				pixel_data[4*row*w+4*col+2] = 0xff;    // b
				pixel_data[4*row*w+4*col+3] = 0x30;    // alpha, 50% transparence
			} else {
				pixel_data[4*row*w+4*col+0] = 0x00;    // r
				pixel_data[4*row*w+4*col+1] = 0x00;    // g
				pixel_data[4*row*w+4*col+2] = 0x00;    // b
				pixel_data[4*row*w+4*col+3] = 0x00;    // alpha, 100% transparence
			}
		}
	}
}
#endif

int main(int argc, char* argv[])
{
	
    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" [options] OUTPUTFILENAME" << endl;
        wcout << "Try " << argv[0] << "--help for more information" << endl;
        return ( 0 );
    }
	
    if (DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE( argv[1], "--help" ) == 0 ||
        DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE( argv[1], "-h" ) == 0)
    {
        wcout << "Usage:" << argv[0] << L" [options] file.dwf" << endl << endl;
        wcout << "  --package-type=dwfx\t specifies that a dwfx package should be created" << endl;
        return 0;
    }
	
    char* zFileName = NULL;
    bool bCreateDWF6Package = true;
	
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != L'-')
        {
            zFileName = argv[i];
        }
        else if (DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(argv[i] , "--package-type=dwfx") == 0)
        {
            bCreateDWF6Package = false;
        }
    }
	
    if (zFileName == NULL)
    {
        wcout << "Missing output filename" << endl;
        wcout << "Try " << argv[0] << "--help for more information" << endl;
        return 0;
    }
	
    try
    {
		
		
        //
        // start with a new model
        //
        DWFModel oModel( L"3D Publish Model Test", L"3DPublish.cpp" );
		
#ifdef  USE_NEW_CONTENT_DEFINITION_FOR_3D_PUBLISH
        //
        //  The content is required by the model. The publisher will take care of deleting
        //  the content manager and the content.
        //
        DWFContentManager* pContentManager = DWFCORE_ALLOC_OBJECT( DWFContentManager );
        DWFContent* pContent = pContentManager->getContent();
		
        oModel.open( pContent,
					DWFModel::eHandednessNone,
					DWFUnits::eCentimeters,
					NULL,                                  // transform
					true,                                  // default lights
					false,                                 // published edges
					true,                                  // silhouettes
					0,
					DWFPublisher::ePublishContentDefinition ); 
#else
        oModel.open( NULL,
					DWFModel::eHandednessNone,
					DWFUnits::eCentimeters,
					NULL,                                  // transform
					true,                                  // default lights
					false,                                 // published edges
					true,                                  // silhouettes
					0,
					DWFPublisher::ePublishObjectDefinition ); 
#endif
		
        W3DCamera oDefault;
        oDefault.setProjection( W3DCamera::eOrthographic );
        oDefault.setPosition( 0.0f, 0.0f, -15.0f );
        oDefault.setTarget( 1.0f, 1.4f, -0.1f );
        oDefault.setUpVector( 1.0f, 1.0f , 0.1f );
        oDefault.setField( 15.0f, 15.0f );
		
        oModel.createView( DWFModel::eInitialView, L"Initial", oDefault );
		
        W3DCamera oViewCubeHome;
        oViewCubeHome.setProjection( W3DCamera::eOrthographic );
        oViewCubeHome.setPosition( 0.0f, 0.0f, -15.0f );
        oViewCubeHome.setTarget( 0.0f, 0.0f, -0.0f );
        oViewCubeHome.setUpVector( 1.0f, 1.0f , 0.1f );
        oViewCubeHome.setField( 15.0f, 15.0f );
        oModel.createView( DWFModel::eViewCubeHome, L"Home", oViewCubeHome );
		
        //
        // create the graphics, etc.
        //
        do_model( oModel, true );
		
        //
        // add some properties that apply to the model as a whole
        //
        oModel.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Cutting Plane", L"Hide", L"View Modifier")), true );
        oModel.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Origin", L"DWFToolkit Sample Applications", L"General")), true );
        oModel.addProperty( DWFCORE_ALLOC_OBJECT(DWFProperty(L"Project", L"Bianchi", L"General")), true );
		
        //
        // close the model
        //
        oModel.close();
		
        //
        //  Create the DWFFile to which the package will be saved, and the publisher to use.
        //
        DWFFile oDWF( zFileName );
        DWFPackagePublisher* pPackagePublisher = NULL;
        if (bCreateDWF6Package)
        {
            pPackagePublisher = DWFCORE_ALLOC_OBJECT( DWF6PackagePublisher( oDWF ) );
        }
        else
        {
            pPackagePublisher = DWFCORE_ALLOC_OBJECT( DWFXPackagePublisher( oDWF ) );
            
            //
            //  publishing proxy graph for XPS viewer.
            //
            do_publishProxyGraph( pPackagePublisher );
        }
        
        pPackagePublisher->setViewsPresentationLabel( L"Sample Published Views" );
        //
        // publish the model "into" the publisher.
        //
        oModel.publish( *pPackagePublisher );
		
        //
        // create the DWF
        //
        pPackagePublisher->publish();
		
        DWFCORE_FREE_OBJECT( pPackagePublisher );
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


#define countof(x) (int)(sizeof(x)/sizeof(x[0]))


class HPoint 
	{ 
	public:
		float        x;	
		float        y;	
		float        z;	
		inline void Set(float X,float Y,float Z=0.0) { x=X,y=Y,z=Z; }
		
	}; 

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


void do_model( DWFModel& rModel,
			  bool bIncludeNormals )
{
    //
    // 
    //
    DWFProperty oProperty( L"", L"" );
	
    HPoint center; center.Set( 0.0, 0.0, 0.0 );
	
    float radius = 1.0f;
    float bbox[6];
    float *points, *normals;
    int p_count, flist_length, *flist, i;
	
    //
    // the first thing to do is create an include segment
    // that defines a collection of graphics that will be
    // reused throughout the model
    //
    DWFIncludeSegment oSphere_include = rModel.openIncludeSegment();
	
    //
    // segments are always first acquired from their aggregating component (i.e. Model or another Segment)
    // and then opened.  Include segments must be opened with a name. 
    //
    oSphere_include.open( "Sphere" );
    {
        //
        // we will define our sphere as a shell
        // notice that the handler is returned as a reference
        // be sure to obtain it this way; if you try and copy
        // it, you will get a compile time error since there
        // is no copy constructor defined for these objects
        //
        TK_Shell& rSphere = oSphere_include.getShellHandler();
        {
            //
            // it's always a good idea to set the bounding volume on your shells.
            //
            bbox[0] = center.x - radius;
            bbox[1] = center.y - radius;
            bbox[2] = center.z - radius;
            bbox[3] = center.x + radius;
            bbox[4] = center.y + radius;
            bbox[5] = center.z + radius;
            rSphere.SetBounding( bbox );
			
            //
            // Original HOOPS/Stream comment:
            // This is where you will go out to your gfx database and get the data defining the shell.
            // We're using a simple sphere generator to create a point cloud and some connectivity
            //
            CreateSphere( (float *)&center, radius, 30, &p_count, &points, &flist_length, &flist );
            
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
            if (bIncludeNormals)
            {
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
            }
			
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
		
        //
        // let's create a property on this segment
        //
        oProperty.setName( L"Material" );
        oProperty.setValue( L"Plastic" );
		
        //
        // add the property to the segment
        //
        oSphere_include.addProperty( oProperty );
		
        //
        // also add a set of properties 
        //
        DWFPropertySet* pProperties = DWFCORE_ALLOC_OBJECT( DWFPropertySet );
		
        oProperty.setName( L"Cost" );
        oProperty.setValue( L"$4.44" );
        oProperty.setCategory( L"Sphere Properties" );
        pProperties->addProperty( &oProperty, false );
		
        oProperty.setName( L"Vendor" );
        oProperty.setValue( L"Foo Ball" );
        pProperties->addProperty( &oProperty, false );
		
        oProperty.setName( L"Manufacturer" );
        oProperty.setValue( L"Ball Co." );
        pProperties->addProperty( &oProperty, false );
		
        //
        // add the property set to the segment
        //
        oSphere_include.addPropertyContainer( pProperties );
    }
	
    //
    // close the segment - this will serialize everything about the segment
    // into the graphics stream. we can now use this segment as a reference
    // part in our scene graph
    //
    oSphere_include.close();
	
	
    //
    // it is preferrable to "side-stream" texture images
    // in the DWF package rather than embedded them directly
    // in the graphics channel
    // so let's add our texture images up here
    //
    //
    // first, create the texture resource and add it to the model
    // here we load an image from disk...
    //
    DWFStreamFileDescriptor* pTextureFile = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor("hank.jpg", "rb") );
    pTextureFile->open();
	
    DWFFileInputStream* pTextureStream = DWFCORE_ALLOC_OBJECT( DWFFileInputStream );
    pTextureStream->attach( pTextureFile, true );
	
    DWFTexture* pTexture = DWFCORE_ALLOC_OBJECT( DWFTexture("hank",
															DWFMIME::kzMIMEType_JPG,
															24, 320, 240) );
	
    pTexture->attach( pTextureStream, true );
    rModel.addResource( pTexture );
    ///
    ///
	
    //
    // create a simple scene
    //
    DWFSegment oRootSegment = rModel.openSegment();
    
    //
    // segments don't have to be named - in this case
    // we will set up some attributes that will affect
    // the entire scene unless overridden in subsegments
    //
    oRootSegment.open();
    {
		
        TK_Color_RGB& rColor = oRootSegment.getColorRGBHandler();
        
        //
        // red lines
        //
        rColor.SetGeometry( TKO_Geo_Line );
        rColor.SetRGB( 1.0f, 0.0f, 0.0f );
        rColor.serialize();
		
        //
        // we can reuse the handler 
        // green text
        //
        rColor.SetGeometry( TKO_Geo_Text );
        rColor.SetRGB( 0.0f, 0.8f, 0.1f );
        rColor.serialize();
		
        //
        // we can reuse the handler 
        // blue faces
        //
        rColor.SetGeometry( TKO_Geo_Face );
        rColor.SetRGB( 0.0f, 0.0f, 0.75f );
        rColor.serialize();
		
        //
        // a simple set of properties
        //
        DWFPropertySet* pPartProperties = DWFCORE_ALLOC_OBJECT( DWFPropertySet );
		
        oProperty.setName( L"Cost" );
        oProperty.setValue( L"$23.55" );
        oProperty.setCategory( L"Part Properties" );
        pPartProperties->addProperty( &oProperty, false );
		
		
        //
        // now let's create a segment that will show up in the nav tree
        // note that this is built off the root segment not the model
        //
        DWFSegment oPart1 = oRootSegment.openSegment();
		
        //
        // open with a name, this will show in the nav tree
        //
        oPart1.open( L"First Part" );
        {
            //
            // add the sphere from our include library
            // since the segment that we are including the spehere in
            // is already named, it will "become" a part of this segment.
            // that is, selecting "First Part" in the tree will highlight the
            // sphere and vice-versa - there will be no tree node called "Sphere"
            //
            oPart1.include( oSphere_include );
			
            //
            // let's draw some lines
            // these lines will finish the element named "First Part"
            // all of the geometry built into this segment will be
            // treated as a single selected part
            //
            TK_Line& rLine = oPart1.getLineHandler();
			
            rLine.SetPoints( 0.0f,
							0.0f,
							0.0f,
							center.x + 2.0f*radius,
							center.y + 2.0f*radius,
							center.z + 2.0f*radius );
			
            rLine.serialize();
			
            rLine.SetPoints( center.x - 2.0f*radius,
							center.y - 2.0f*radius,
							center.z - 2.0f*radius,
							0.0f,
							0.0f,
							0.0f );
			
            rLine.serialize();
			
            oPart1.addPropertyContainer( pPartProperties );
            //
            // test textures
            //
            DWFSegment oTexturePart = oPart1.openSegment();
            oTexturePart.open( L"Hank Box" );
            {
                //
                // make a cube
                //
                float anPoints[24] = {  0.0f, 0.0f, 0.0f,
					0.2f, 0.0f, 0.0f,
					0.2f, 0.2f, 0.0f,
					0.0f, 0.2f, 0.0f,
					0.0f, 0.0f, 0.2f,
					0.2f, 0.0f, 0.2f,
					0.2f, 0.2f, 0.2f,
				0.0f, 0.2f, 0.2f };
				
                int anFaces[30] = { 4, 0, 1, 2, 3,
					4, 0, 4, 7, 3,
					4, 3, 2, 6, 7,
					4, 4, 5, 6, 7,
					4, 1, 5, 6, 2,
				4, 0, 1, 5, 4};
				
                TK_Shell& rCube = oTexturePart.getShellHandler();
                rCube.SetPoints( 8, anPoints );
                rCube.SetFaces( 30, anFaces );
                rCube.SetVertexParameters( anPoints );
                rCube.serialize();
				
#ifdef	 TEST_MULTIPLE_DECAL_TEXTURES
                // create images
    	        gen_top_img(width, height, (unsigned char* const)pixel_data1);
            	gen_bot_img(width, height, (unsigned char* const)pixel_data2);
				
                TK_Image& image1 = oTexturePart.getImageHandler();
                image1.SetName("top_image");
                image1.SetFormat(TKO_Image_RGBA);
                image1.SetSize(width,height);
                image1.SetBytes(sizeof(pixel_data1), (char const*)pixel_data1);
                image1.serialize();
				
                TK_Image& image2 = oTexturePart.getImageHandler();
                image2.SetName("bot_image");
                image2.SetFormat(TKO_Image_RGBA);
                image2.SetSize(width,height);
                image2.SetBytes(sizeof(pixel_data2), (char const*)pixel_data2);
                image2.serialize();
				
                // Set Visibility for images off, faces on.
                TK_Visibility& vis = oTexturePart.getVisibilityHandler();
                vis.SetGeometry(TKO_Geo_Image | TKO_Geo_Face);
                vis.SetValue(TKO_Geo_Face);
                vis.serialize();
#endif
				
                //
                // texture the cube
                //
                TK_Texture& rShellTexture = oTexturePart.getTextureHandler();
				
                //
                // this creates a texture from our earlier image
                // NOTE: the image is required to exist already
                //
#ifdef	 TEST_MULTIPLE_DECAL_TEXTURES
				int flags( TKO_Texture_Param_Source | TKO_Texture_Param_Offset | TKO_Texture_Decal );
#else
				int flags( 0 );
#endif
                rShellTexture.SetFlags( flags );
                rShellTexture.SetImage( "hank" );
                rShellTexture.SetName( "hank texture" );
                rShellTexture.serialize();
				
#ifdef	 TEST_MULTIPLE_DECAL_TEXTURES
                // add the second and third
                TK_Texture& rShellTexture1 = oTexturePart.getTextureHandler();
                rShellTexture1.SetFlags( TKO_Texture_Param_Source | TKO_Texture_Param_Offset | TKO_Texture_Decal );
                rShellTexture1.SetImage( "top_image" );
                rShellTexture1.SetName( "top_texture" );
                rShellTexture1.serialize();
				
                TK_Texture& rShellTexture2 = oTexturePart.getTextureHandler();
                rShellTexture2.SetFlags( TKO_Texture_Param_Source | TKO_Texture_Param_Offset | TKO_Texture_Decal );
                rShellTexture2.SetImage( "bot_image" );
                rShellTexture2.SetName( "bot_texture" );
                rShellTexture2.serialize();
#endif
				
                TK_Color& rTextureColor = oTexturePart.getColorHandler();
				
                //
                // set the attribute for this DWFSegment such that
                // faces are "colored" with our new (named) texture
                // NOTE: the texture is required to exist already
                //
                rTextureColor.SetGeometry( TKO_Geo_Face );
                rTextureColor.SetChannels( 1 << TKO_Channel_Diffuse );	
				
#ifdef	 TEST_MULTIPLE_DECAL_TEXTURES
                char diff_text_name[] = { "hank texture, top_texture, bot_texture" };
#else
				char diff_text_name[] = { "hank texture" };
#endif
                rShellTexture.SetFlags( flags );
				
                rTextureColor.SetDiffuseName( diff_text_name );
                rTextureColor.serialize();
				
                //
                // use rendering option's mask to disable camera scaling
                //
                TK_Rendering_Options& rRenderOptions = oTexturePart.getRenderingOptionsHandler();
                rRenderOptions.SetMask(0,TKO_Rendo_Mask_Transform);
                rRenderOptions.SetValue(0,TKO_Rendo_Mask_Transform);
                rRenderOptions.SetMaskTransform(TKO_Mask_Camera_Scale);
                rRenderOptions.serialize();
            }
            oTexturePart.close();
        }
        oPart1.close();
		
        //
        // create another part
        //
        DWFSegment oPart2 = oRootSegment.openSegment();
		
        oPart2.open( L"Second Part" );
        {
            //
            // let's first reposition everything in this segment
            //
            float anTransform[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
			2.33f, 0.67f, 0.0f, 1.0f };
			
            TK_Matrix& rMatrix = oPart2.getModellingMatrixHandler();
            rMatrix.SetMatrix( anTransform );
            rMatrix.serialize();
			
            //
            // open an unnamed segment
            //
            DWFSegment oSubpart = oPart2.openSegment();
            oSubpart.open();
            {
                //
                // add our sphere - this time, the sphere will appear
                // as it's own element in the nav tree.  this is due
                // to the fact that we are referencing the sphere
                // in an unnamed segment.  we could rename this element
                // if we wanted to. regardless, the behavior here is that
                // we are using a named part to build up the higher level
                // element, i.e. "Second Part"
                //
                oSubpart.include( oSphere_include );
				
                //
                // note that since we didn't name the segment into which this
                // part was included, we cannot modify the properties on JUST
                // the sphere, since the next named node up the tree is "Second Part"
                // any properties we add would end up there. 
                //
            }
            oSubpart.close();
			
            //
            // let's add a text box - use it's own segment or else
            // we will affect the included sphere - well it's not really
            // a text box but rather a polygon with some text in front of it
            //
            oSubpart = oSubpart.openSegment();
            oSubpart.open();
            {
                //
                // face
                //
                rColor.SetGeometry( TKO_Geo_Face );
                rColor.SetRGB( 0.11f, 0.88f, 0.44f );
                rColor.serialize();
				
                //
                // solid face
                //
                TK_Enumerated& rPatternEnum = oSubpart.getFacePatternHandler();
                rPatternEnum.SetIndex( TKO_Fill_Pattern_Solid );
                rPatternEnum.serialize();
				
                ////
                //// solid edges
                //// by the way, be sure to turn on edges in the viewer -
                //// "Shaded with Edges" since they are off by default
                ////
                //TK_Linear_Pattern& rLinePattern = oSubpart.getEdgePatternHandler();
                //rLinePattern.SetPattern( TKO_Line_Pattern_Solid );
                //rLinePattern.serialize();
				
                //
                // edge weight
                // by the way, be sure to turn on edges in the viewer -
                // "Shaded with Edges" since they are off by default
                //
                TK_Size& rPatternSize = oSubpart.getEdgeWeightHandler();
                rPatternSize.SetSize( 5.0f, TKO_Generic_Size_Points );
                rPatternSize.serialize();
				
                //
                // the polygon
                //
                TK_Polypoint& rPolygon = oSubpart.getPolygonHandler();
				
                float anBox[15] = { 2.0, 2.0, 2.0,
					2.0, 1.0, 2.0,
					1.0, 1.0, 2.0,
					1.0, 2.0, 2.0,
				2.0, 2.0, 2.0 };
				
                rPolygon.SetPoints( 5, anBox );
                rPolygon.serialize();
				
                //
                // override the green text with black
                //
                rColor.SetGeometry( TKO_Geo_Text );
                rColor.SetRGB( 0.0f, 0.0f, 0.0f );
                rColor.serialize();
				
                //
                // draw some text on the polygon
                //
                TK_Text& rText = oSubpart.getTextWithEncodingHandler();
                rText.SetEncoding( TKO_Enc_Unicode );
                rText.SetPosition( 1.5f, 1.5f, 1.5f );
                rText.SetString( (unsigned short const*)(L"3D DWF") );
                rText.serialize();
				
                //
                // by default, HOOPS draws text in 'annotation' mode only
                // we can turn this off and make our text part of the scene:
                //
                TK_Text_Font& rFont = oSubpart.getTextFontHandler();
                rFont.SetTransforms( TKO_Font_Transform_Full );
                
                rFont.SetMask( TKO_Font_Transforms );
                rFont.serialize();
            }
            oSubpart.close();
			
            //
            // let's make this kind of look like our first part and add some lines
            // this time we will make them a selectable unit
            //
            DWFSegment oLinesPart = oPart2.openSegment();
            
            oLinesPart.open( L"Lines" );
            {
                TK_Line& rLine = oLinesPart.getLineHandler();
				
                rLine.SetPoints( 0.0f,
								0.0f,
								0.0f,
								center.x + 2.0f*radius,
								center.y + 2.0f*radius,
								center.z + 2.0f*radius );
				
                rLine.serialize();
				
                rLine.SetPoints( center.x - 2.0f*radius,
								center.y - 2.0f*radius,
								center.z - 2.0f*radius,
								0.0f,
								0.0f,
								0.0f );
				
                rLine.serialize();
            }
            oLinesPart.close();
			
            //
            // add the property set to the part (by reference - part 1 owns it)
            //
            oPart2.referencePropertyContainer( *pPartProperties );
			
        }
        oPart2.close();
		
        //
        // create another part
        //
        DWFSegment oPart3 = oRootSegment.openSegment();
		
        oPart3.open( L"Third Part" );
        {
            //
            // red faces
            //
            rColor.SetGeometry( TKO_Geo_Face );
            rColor.SetRGB( 0.67f, 0.0f, 0.05f );
            rColor.serialize();
			
            //
            // let's first reposition everything in this segment
            //
            float anTransform[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
			-4.33f, -1.34f, -0.134f, 1.0f };
			
            TK_Matrix& rMatrix = oPart3.getModellingMatrixHandler();
            rMatrix.SetMatrix( anTransform );
            rMatrix.serialize();
			
            //
            // expliciting naming the segment
            //
            DWFSegment oSubpart1 = oPart3.openSegment();
            oSubpart1.open( L"Ball 1" );
            {
                //
                // add our sphere 
                //
                oSubpart1.include( oSphere_include );
				
                //
                // override the sphere properties
                //
                //oProperty.setName( L"Cost" );
                //oProperty.setValue( L"$3.00" );
                //oProperty.setCategory( L"Sphere Properties" );
                oProperty.setName( L"いくらですか？" );
                oProperty.setValue( L"さんぜん円" );
                oProperty.setCategory( L"プロプルチズ" );
                oSubpart1.addProperty( oProperty );
				
                oProperty.setName( L"Vendor" );
                oProperty.setValue( L"Dun & Broadstreet" );
                oSubpart1.addProperty( oProperty );
            }
            oSubpart1.close();
			
            //
            // unnamed segment
            //
            DWFSegment oSubpart2 = oPart3.openSegment();
            oSubpart2.open();
            {
                float anTransform[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 1.0f, 1.0f };
				
                TK_Matrix& rMatrix = oPart3.getModellingMatrixHandler();
                rMatrix.SetMatrix( anTransform );
                rMatrix.serialize();
				
                //
                // including with a different name
                //
                oSubpart2.include( oSphere_include, DWFString(L"Ball 2") );
            }
            oSubpart2.close();
			
            DWFSegment oSubpart3 = oPart3.openSegment();
            oSubpart3.open( L"Ball 3" );
            {
				
                float anTransform[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, -1.0f, -1.0f, 1.0f };
				
                TK_Matrix& rMatrix = oPart3.getModellingMatrixHandler();
                rMatrix.SetMatrix( anTransform );
                rMatrix.serialize();
				
                //
                // including with a different name
                //
                oSubpart3.include( oSphere_include );
            }
            oSubpart3.close();
			
            //
            // 
            //
            DWFSegment oCut = oPart3.openSegment();
            oCut.open( L"Cutting Plane" );
            {
                TK_Cutting_Plane& rCut = oCut.getCuttingPlaneHandler();
                rCut.SetPlane( -1.0f, -1.0f, 0.0f, 0.0f );
                rCut.serialize();
            }
            oCut.close();
			
        }
        oPart3.close();
		
        //
        // test embedded fonts 
        //
        DWFSegment oPart4 = oRootSegment.openSegment();
        oPart4.open();
        {
            //
            // we'll add some japanese text in MS PGothic
            // we aren't modifying the transform mask so this text will be billboarded
            //
            TK_Text_Font& rFont = oPart4.getTextFontHandler();
            rFont.SetNames( "MS PGothic" );
            rFont.SetMask( TKO_Font_Names );
            rFont.serialize();
			
            //
            // the text
            //
            TK_Text& rText = oPart4.getTextWithEncodingHandler();
            rText.SetEncoding( TKO_Enc_Unicode );
            rText.SetPosition( -2.0f, 4.0f, 0.0f );
            rText.SetString( (unsigned short const*)(L"日本語のテキスト") );
            rText.serialize();
			
            //
            // font handling is a platform specific issue
            // unfortunately right now
            //
#ifdef  _DWFCORE_WIN32_SYSTEM
			
            //
            // now embed the font in the DWF package
            // notice that this is win32 specific code
            //
            HFONT hFont = CreateFontW(100, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, L"MS PGothic");
			
            DWFEmbeddedFontImpl_Win32* pImpl2 = new DWFEmbeddedFontImpl_Win32( hFont );
            DWFEmbeddedFont* pFont = new DWFEmbeddedFont( pImpl2 );
			
            //
            // add just the characters that were used
            //
            pFont->addCharacters( L"日本語のテキスト" );
			
            rModel.embedFont( pFont );
#endif
			
        }
        oPart4.close();
		
    }
    oRootSegment.close();
	
    rModel.setBoundingSphere( 0.0f, 0.0f, 0.0f, 15.0f );
	
    //
    // we can also add a thumbnail image
    //
	
    //
    // open as a streaming binary file
    //
    DWFStreamFileDescriptor* pThumbnailFile = new DWFStreamFileDescriptor( "thumbnail.png", "rb" );
    pThumbnailFile->open();
	
    //
    // create a stream and attach the file descriptor
    //
    DWFFileInputStream* pThumbnailStream = new DWFFileInputStream;
    pThumbnailStream->attach( pThumbnailFile, false );
	
    //
    // create a thumbnail image and attach the file stream
    //
    DWFImage* pThumbnail = DWFCORE_ALLOC_OBJECT( DWFImage(DWFMIME::kzMIMEType_PNG, 
                                                          DWFImage::eThumbnail, 
                                                          24, 
                                                          218, 
                                                          213) );
	
    if (pThumbnail)
    {
        pThumbnail->attach( pThumbnailStream, true );
		
        //
        // add to the model
        //
        rModel.addResource( pThumbnail );
    }
}

void do_publishProxyGraph( DWFPackagePublisher* pPackagePublisher )
{
    //
    // Add the proxy graphics section
    //
	
    double  fPaperWidth(8.5);
    double  fPaperHeight(11.0);
    int nImageWidth = 640;
    int nImageHeight = 480;
    double proxyTransform[4][4] = { 1, 0, 0, 0, 
		0, 1, 0, 0, 
		0, 0, 1, 0, 
	0, 0, 0, 1 };
	
    double anPaperClip[4] = { 0, 0, 11, 8.5 };
	
    DWFToolkit::DWFPaper proxyPaper( fPaperWidth, fPaperHeight, DWFToolkit::DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );
	
    //
    // Create the page.
    //
    DWFPlot pProxyPlot( L"Proxy Page", L"3DDWFxPublish.cpp" );
	
    pProxyPlot.open( proxyPaper, (const double*)proxyTransform );
    //
    // Create the image resource.
    //
    DWFToolkit::DWFImageResource* pProxyImage = 
	DWFCORE_ALLOC_OBJECT(
						 DWFToolkit::DWFImageResource(
													  L"Proxy Graphics Image",                // title
													  DWFXML::kzRole_RasterReference,         // role
													  DWFCore::DWFMIME::kzMIMEType_PNG,       // MIME type
													  L"Autodesk, Inc.",                      // author
													  L"Proxy Graphics Image",                // description
													  L"",                                    // creation time
													  L""                                     // modification time
													  )
						 ); 
	
    if(pProxyImage == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate a DWFImageResource for the proxy content." );
    }
	
    //
    // configure the image
    //
	
    double anClip[4] = { 0, 0, fPaperWidth, fPaperHeight };
    double anOrigExtents[4] = { 0, 0, nImageWidth, nImageHeight };
	
    double imageTransform[4][4] = { 1, 0, 0, 0, 
		0, 1, 0, 0,
		0, 0, 1, 0, 
	0, 0, 0, 1 };
	
    pProxyImage->configureGraphic( (const double*)imageTransform,
								  NULL,(const double*)anClip, true, 1 );
	
    pProxyImage->configureImage( 0, false, false, 0, anOrigExtents ); 
	
	
    //
    // extras for the image
    // here we note the image has 24 bpp (color depth)
    //
    pProxyImage->configureImage( 24 );
	
    //
    // set the resource's stream from the thumbnail file
    //
    setResourceStreamFromFile( pProxyImage, L"ProxyImage.png" );
	
    //
    // Request a particular name. The FixedPage content will refer to the image by this name.
    //
    char zProxyImageName[] = "ProxyGraphics.png";
    pProxyImage->setRequestedName(zProxyImageName);
	
    //
    // The hyperlink target.
    //
    char zHyperlinkTarget[] = "http://www.autodesk.com/designreview-xpsviewer";
	
    //
    // Add the proxy image to the plot
    //
	
    ((DWFResourceContainer&) pProxyPlot).addResource( pProxyImage, false );
	
    //
    // Produce the FixedPage content.
    //
    // It's easier to do this directly, rather than use the Whip/XAML toolkit.
    //
    // We use char instead of wchar_t, because we have to provide UTF-8 for the resource input stream.
    //
	
    char zWidth[64], zHeight[64];
    DWFString::DoubleToString(zWidth, 64, fPaperWidth, 17);
    DWFString::DoubleToString(zHeight, 64, fPaperHeight, 17);
	
    //
    // fit the image to the paper, reducing by a given factor
    // x-scale is paper_width / image_width_in_pixels
    // y-scale is paper_height / image_height_in_pixels
    // actual scale is the minimum of x & y - for uniform scaling
    // x-translation : (paper_width  - image_width_in_pixels  * scale) / 2; this centers along the x-axis
    // y-translation : (paper_height - image_height_in_pixels * scale) / 2; this centers along the y-axis
    //
    double reductionFactor = 1.0;
    double xScale = fPaperWidth  / nImageWidth;
    double yScale = fPaperHeight / nImageHeight;
    double scale = (fabs(xScale) < fabs(yScale) ? xScale : yScale) * reductionFactor;
    double xOffset = (fPaperWidth  - (nImageWidth  * scale))/2;
    double yOffset = (fPaperHeight - (nImageHeight * scale))/2;
	
    char zImageXMin[64], zImageXMax[64], zImageYMin[64], zImageYMax[64];
    DWFString::DoubleToString(zImageXMin, 64, xOffset, 17);
    DWFString::DoubleToString(zImageXMax, 64, fPaperWidth-xOffset, 17);
    DWFString::DoubleToString(zImageYMin, 64, yOffset, 17);
    DWFString::DoubleToString(zImageYMax, 64, fPaperHeight-yOffset, 17);
	
    char zFormatString[] =
	"<Path Data=\"M0,0 L%s,0 L%s,%s L0,%s z\" FixedPage.NavigateUri=\"%s\">"
	"<Path.Fill>"
	"<ImageBrush ImageSource=\"%s\" Viewport=\"%s,%s, %s,%s\" ViewportUnits=\"Absolute\" Viewbox=\"0,0, %d,%d\" ViewboxUnits=\"Absolute\"/>"
	"</Path.Fill>"
	"</Path>";
	
    //
    // Strangely, to prevent non-uniform scaling, we need to specify a *square* viewbox.
    // It has to be as big as the largest pixel dimension of the image.
    //
    int nMaxDimension = (nImageWidth>nImageHeight) ? nImageWidth : nImageHeight;
	
    char zBuffer[16384];
    int nChars = sprintf(
						 zBuffer,
						 zFormatString,
						 zWidth,
						 zWidth,
						 zHeight,
						 zHeight,
						 zHyperlinkTarget,
						 zProxyImageName,    // /dwf/documents/99FCDD74-9D2C-46B3-900E-5A69098233FC/sections/2B7879BD-CC9A-4A0A-B993-AB98BB077BB9.png
						 zImageXMin,
						 zImageYMin,
						 zImageXMax,
						 zImageYMax,
						 nMaxDimension,
						 nMaxDimension
						 );
	
    //
    // Note, we do not need or want to carry along the null terminator, as
    // this buffer will be inserted into some surrounding XML by the DWFX
    // packager, and the null would make mal-formed XML.
    //
    char* pNewBuffer = DWFCORE_ALLOC_MEMORY(char, nChars);
    DWFCORE_COPY_MEMORY(pNewBuffer, zBuffer, nChars);
    DWFBufferInputStream* pFPIS = DWFCORE_ALLOC_OBJECT(DWFBufferInputStream(pNewBuffer, nChars, true));
	
    //
    // Attach the Stream to plot and close
    //
    pProxyPlot.attach(pFPIS, false);
    pProxyPlot.close();
	
    //
    // finally, drop the page into the writer
    //
    (( DWFXPackagePublisher *)pPackagePublisher)->addProxyGraphicsSection(&pProxyPlot, DWFToolkit::DWFXPackagePublisher::eIfOnlyPartialPageOutput);
}



