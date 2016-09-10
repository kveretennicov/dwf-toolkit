//  Copyright (c) 2006 by Autodesk, Inc.
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

#include "XAML/pch.h"

WT_Result
WT_XAML_Macro_Draw::serialize(WT_File & file) const 
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Macro_Draw::serialize( *rFile.w2dContentFile() );
    }

    // Macro_Defintion will not serialize out if the file revision is incorrect
    if (file.heuristics().target_version() < REVISION_WHEN_MACRO_IS_SUPPORTED)
        return WT_Result::Toolkit_Usage_Error;
    
    // dump anything delayed 
    WD_CHECK( file.dump_delayed_drawable() );

    WD_Assert(count() > 0);

    if (rFile.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Macro_Draw*>(this)->transform (file.heuristics().transform());
    }


    // sync Desired rendition so that any rendition already set will
    // be serialized. 
    // 0x7FFFFFFF is a nicer substitution since we need all the bits 
    // However the following is more human readable 
    WT_Integer32 parts_to_sync = WT_Rendition::BlockRef_Bit         |
                                 WT_Rendition::Code_Page_Bit        |
                                 WT_Rendition::Color_Bit            |
                                 WT_Rendition::Color_Map_Bit        |
                                 WT_Rendition::Dash_Pattern_Bit     |
                                 WT_Rendition::Fill_Bit             |
                                 WT_Rendition::Fill_Pattern_Bit     |
                                 WT_Rendition::Font_Bit             |
                                 WT_Rendition::Font_Extension_Bit   |
                                 WT_Rendition::Layer_Bit            |
                                 WT_Rendition::Line_Pattern_Bit     |
                                 WT_Rendition::Line_Style_Bit       |
                                 WT_Rendition::Line_Weight_Bit      |
                                 WT_Rendition::Macro_Scale_Bit      |
                                 WT_Rendition::Macro_Index_Bit      |
                                 WT_Rendition::Merge_Control_Bit    |
                                 WT_Rendition::Object_Node_Bit      |
                                 WT_Rendition::Pen_Pattern_Bit      |
                                 WT_Rendition::Projection_Bit       |
                                 WT_Rendition::URL_Bit              |
                                 WT_Rendition::Viewport_Bit         |
                                 WT_Rendition::Visibility_Bit       |
                                 WT_Rendition::Text_Background_Bit  |
                                 WT_Rendition::Text_HAlign_Bit      |
                                 WT_Rendition::Text_VAlign_Bit      |
                                 WT_Rendition::Delineate_Bit        |
                                 WT_Rendition::User_Fill_Pattern_Bit    |
                                 WT_Rendition::User_Hatch_Pattern_Bit   |
                                 WT_Rendition::Contrast_Color_Bit;

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));
    
    //Store the curent rendition
    WT_Rendition oSaveRendition = file.rendition();

	//serialize w2x stuff
	//End RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

	rFile.w2xSerializer()->startElement( XamlXML::kpzMacro_Draw_Element );

	rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString() );
    DWFString zPositions;

    WT_XAML_Macro_Definition *pMacro = NULL;
    WT_Integer32 nIndex = rFile.rendition().macro_index().macro();
    if ( ! rFile.findMacro( nIndex , pMacro ) )
        return WT_Result::Toolkit_Usage_Error;

    // Okay, this looks a little weird, but what it does is gets us a proper translation component
    // for the Y axis.  
    WT_Matrix2D oTransform;
    double fScale = fabs((double)rFile.rendition().macro_scale().scale() / (double)pMacro->scale_units());
    WT_Point2D oOrigin;
    rFile.flipPoint( oOrigin ); //gets the page height
    double fYOffset = oOrigin.m_y - (oOrigin.m_y*fScale); //unwind the Y-flipping

    WT_Integer32 i;
    for( i=0; i<count(); i++ )
    {
        WT_Logical_Box oBounds = pMacro->bounds();

        //Create a canvas
        XamlCanvas oCanvas;
        
        WT_Point2D oPosition( points()[i].m_x, points()[i].m_y );
        if (i==0)
        {
            zPositions = oPosition.toString( 10 );
        }
        else
        {
            zPositions.append(" ");
            zPositions.append(oPosition.toString( 10 ));
        }

        //Translate the canvas via the draw points
        oTransform(0,0) = fScale;
        oTransform(1,1) = fScale;

        WT_Integer32 nRotation = 0;
        if (rFile.heuristics().apply_transform())
        {
            nRotation = rFile.heuristics().transform().rotation();
        }

        switch(nRotation)
        {
        case 0:
            oTransform(2,0) = oPosition.m_x; 
            oTransform(2,1) = fYOffset - oPosition.m_y;
            break;
        case 90:
            oTransform(2,0) = fYOffset + (oPosition.m_x - oOrigin.m_y);
            oTransform(2,1) = fYOffset - oPosition.m_y;
            break;
        case 180:
            oTransform(2,0) = fYOffset + (oPosition.m_x - oOrigin.m_y);
            oTransform(2,1) = oOrigin.m_y - oPosition.m_y;
            break;
        case 270:
            oTransform(2,0) = oPosition.m_x;
            oTransform(2,1) = oOrigin.m_y - oPosition.m_y;
            break;
        }           

        XamlDrawableAttributes::RenderTransformProvider oProvider( oTransform );
        static_cast<XamlDrawableAttributes::CanvasAttributeConsumer*>(&oCanvas)->consumeRenderTransform( &oProvider );

        rFile.flipPoint(oPosition);
        rFile.update_current_point( oPosition );

        //Intercept and discard normal W2X output, so that when we
        //explode the object list, we don't track the objects as "new"
        //objects in the W2X... it's already defined by the macro definition,
        //so we don't want it defined twice.
        {
            DWFXMLSerializer *pW2XSerializer = rFile.w2xSerializer();
            DWFUUID oUUID;
            DWFXMLSerializer oDummy(oUUID), *pDummy = &oDummy;
            DWFBufferOutputStream oDummyStream( 32768 );
            oDummy.attach( oDummyStream );
            rFile.w2xSerializer() = pDummy;

            //Serialize the canvas
            WD_CHECK( oCanvas.serialize( rFile ) );
            //Serialize the macro graphics
            WD_CHECK( pMacro->serialize_stream( rFile ) );
            //Close the canvas
            WD_CHECK( rFile.serializeCanvasEndElement() );

            rFile.w2xSerializer() = pW2XSerializer;
            oDummy.detach();
        }
    }

    rFile.w2xSerializer()->addAttribute( XamlXML::kpzEnd_Attribute, rFile.nameIndex() );
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzPoints_Attribute, zPositions );
    rFile.w2xSerializer()->endElement();

    rFile.desired_rendition() = oSaveRendition;
    rFile.rendition() = oSaveRendition;

    return WT_Result::Success; 
}

WT_Result
WT_XAML_Macro_Draw::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile )
{
	if(!rMap.size())
		return WT_Result::Success;

    const char **ppValue = rMap.find( XamlXML::kpzPoints_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
    {
        vector<WT_Point2D> oPoints;
        char *pPtr = const_cast< char* >( *ppValue );
        WD_CHECK( XamlPathGeometry::XamlPathFigure::getPoints( &rFile, pPtr, oPoints, false ) );

        WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, oPoints.size() );
        if (pPoints==NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        for (size_t i=0; i<oPoints.size(); i++)
        {
            pPoints[i].m_x = (WT_Integer32) floor( 0.5 + oPoints[i].m_x );
            pPoints[i].m_y = (WT_Integer32) floor( 0.5 + oPoints[i].m_y );
        }
    
        set( (int)oPoints.size(), pPoints, WD_True );
        DWFCORE_FREE_MEMORY( pPoints );
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    ppValue = rMap.find( XamlXML::kpzEnd_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
    {
        //Skip the stuff that was serialized inside the macro stream
        rFile.nameIndex() = atoi( *ppValue );
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;

	return WT_Result::Success;
}
