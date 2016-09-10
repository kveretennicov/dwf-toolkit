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

//#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_XAML_Polygon::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Polygon::serialize( *rFile.w2dContentFile() );
    }

    if ( !file.rendition().fill().fill() || !file.desired_rendition().fill().fill() )
    {
        file.desired_rendition().fill() = WD_True;
    } 

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Polygon*>(this)->transform(file.heuristics().transform());
    }
    
    //Degenerate case - Whip can have 1 and 2 pt polygons
    //In that case, XAML needs to have close segment so adding one more
    //point to make startPoint = endPoint
    //XPS spec - 11.5.10	Rules for Degenerate Line and Curve Segments
    int iCount = WT_Point_Set_Data::count();
   
    WT_Logical_Point* pPoints = WT_Point_Set_Data::points();
    WT_Logical_Point* pDegeneratePoints = NULL;
    if(iCount <= 2)
    {  
        pDegeneratePoints = DWFCORE_ALLOC_MEMORY(WT_Logical_Point, iCount+1);
        if(pDegeneratePoints == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
        WD_COPY_MEMORY((void *)pPoints, iCount * sizeof(WT_Logical_Point), pDegeneratePoints);
        pDegeneratePoints[iCount].m_x = pDegeneratePoints[0].m_x;
        pDegeneratePoints[iCount].m_y = pDegeneratePoints[0].m_y;
        iCount += 1;
        file.desired_rendition().fill() = WD_False;
    }
    
    if(pDegeneratePoints)
        pPoints = pDegeneratePoints;

     //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    // use a temporary point_set to avoid modifying the original
    // coordinates (this must be a local to the serialization)
    //
    WT_XAML_Point_Set_Data local(iCount, pPoints);
    WT_Point2D *pVtx = local.points();
    for(int i = 0; i < local.count(); i++)
    {
        WD_CHECK( rFile.flipPoint(pVtx[ i ]) );
    }

    if(pDegeneratePoints)
        DWFCORE_FREE_MEMORY(pDegeneratePoints);

    XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(local,true) );
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath );
         
    //
    // - add the geometry object to the path
    // - serialize the path once (which will sync the rendition)
    //
    pPath->addDrawable(pXamlPolylineSegment); //XamlPathGeometry owns the segment

    WD_CHECK( pPath->serialize(file) ); //calls serializeRenditionSyncEndElement

    //
    // now, if there is a user_hatch_pattern set on the rendition and if this
    // hatch has multiple patterns, we need to composite them in XAML by
    // outputing several <Path> (same geometry and different patterns)
    //
    // we are sure, by design, to deal with a WT_XAML_User_Hatch_Pattern in
    // the rendition
    //
    WT_XAML_User_Hatch_Pattern *pPat =
        static_cast<WT_XAML_User_Hatch_Pattern *>(&file.rendition().user_hatch_pattern());

    if(pPat->pattern_count() > 1) 
    {
        
        //Wrap the user hatch pattern in a canvas so we can more easily ignore it on materialize
        XamlCanvas oWrapper;
        WD_CHECK(oWrapper.serialize(file));
        rFile.nameIndex()--;  //decrement name index so that the rendition Sync tracks properly
        
        XamlPath oPatternPath; //unnamed path
        oPatternPath = *pPath; 
       
        //
        // do dump first otherwise we'll miss a pattern
        //
        WD_CHECK( rFile.dump_delayed_drawable() );

        rFile.nameIndex()--;  //decrement name index so that the rendition Sync tracks properly        
        //
        // - check out what the merging status is and disable it if we
        // enter the loop (i.e we have multi-hatching)
        // - disable the merging
        //
        WT_Boolean allow = file.heuristics().allow_drawable_merging();
        file.heuristics().set_allow_drawable_merging(0);
        
        for(
            unsigned int i = 1;
            i < pPat->pattern_count();
            i++)
        {  
            pPat->set_pattern_number(i);
            WD_CHECK( oPatternPath.serialize(file) ); 
        }

        //
        // - reset the pattern index for the next WT_XAML_Polygon serialization
        // - reset the drawable merging
        //
        pPat->set_pattern_number(0);
        file.heuristics().set_allow_drawable_merging(allow);

        //close the canvas wrapper
        WD_CHECK( rFile.serializeCanvasEndElement() );        
    }

    DWFCORE_FREE_OBJECT( pPath );

    WD_CHECK(rFile.serializeRenditionSyncEndElement()); 
   
    //W2X serialization here
    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
    if(!pW2XSerializer)
    {
	    return WT_Result::Internal_Error;
    }

    pW2XSerializer->startElement(XamlXML::kpzPolygon_Element);

     pW2XSerializer->addAttribute(
        XamlXML::kpzRefName_Attribute,
        rFile.nameIndexString());

    if(pPat->pattern_count() > 1) 
        rFile.nameIndex()++;

    pW2XSerializer->endElement();
    
    return WT_Result::Success;
}

