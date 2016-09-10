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
#include <stack>

#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result   WT_XAML_Polytriangle::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Polytriangle::serialize( *rFile.w2dContentFile() );
    }

    if (!file.desired_rendition().fill().fill() || !file.rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_True;
    } 
    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Polytriangle*>(this)->transform(file.heuristics().transform());
    }

    WT_Logical_Point *pSrcPoints = WT_Point_Set_Data::points();
    WT_Logical_Point *pDestPoints = NULL;

   //If we're not just a simple triangle
    if (count()>4 || (count()==4 && pSrcPoints[0] != pSrcPoints[3]) )
    {
        //Tweak the points so that the shell of the polytriangle is the outline.
        //Basically, skipping every other point, pushing the skipped points onto a stack
        //and then popping them off after we iterate to the end of the point list.
        pDestPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, WT_Point_Set_Data::count() );
        if (pDestPoints == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        int i=1, j=0;
        std::stack< WT_Logical_Point* > oPointStack;
        for (; i<count(); i+=2 )
        {
            pDestPoints[j++] = pSrcPoints[i];
            oPointStack.push( &pSrcPoints[i-1] );
        }

        if (count()%2==1) //odd count, means we have one additional point to deal with
        {
            pDestPoints[j++] = pSrcPoints[i-1];
        }

        while( oPointStack.size() )
        {
            pDestPoints[j++] = *oPointStack.top();
            oPointStack.pop();
        }        
    }

    //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    // use a temporary point_set to avoid modifying the original
    // coordinates (this must be a local to the serialization)
    //
    WT_XAML_Point_Set_Data local(WT_Point_Set_Data::count(), pDestPoints!=NULL ? pDestPoints : pSrcPoints);
    WT_Point2D *pVtx = local.points();
    for(int i = 0; i < local.count(); i++)
    {
        rFile.flipPoint(pVtx[ i ]);
    }  

    XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment( local ) );
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath );
    if (pDestPoints!=NULL)
    {
        DWFCORE_FREE_MEMORY( pDestPoints );
    }
         
    //
    // - add the geometry object to the path
    // - serialize the path once (which will sync the rendition)
    //
    pPath->addDrawable(pXamlPolylineSegment); //XamlPathGeometry owns the segment

    //
    // Delay-serializaing polytriangles is problematic in that the points winding gets hosed.  
    // I.e. we reorder them above, but don't revert the order, merge, and reorder when
    // we get a new set of points.  For better performance (file size), we could revert the points 
    // winding before merge, and then re-wind the points.  However, we'd have to totally ensure that
    // the regions were properly intersecting (and define the correct merged polytriangle) and that 
    // merging actually makes sense.  The easiest solution (and probably the correct solution) is to 
    // simply suppress merging for this drawable. Fixes 930791, JK.
    //
    rFile.dump_delayed_drawable();
    WT_Boolean allowMerging = file.heuristics().allow_drawable_merging();
    file.heuristics().set_allow_drawable_merging( WD_False );

    WD_CHECK( pPath->serialize(file) );

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

    if(pPat->pattern_count() > 1) {

        //
        // - check out what the merging status is and disable it if we
        // enter the loop (i.e we have multi-hatching)
        // - disable the merging
        //
       
        for(
            unsigned int i = 1;
            i < pPat->pattern_count(); i++)
        {  
            pPat->set_pattern_number(i);
            WD_CHECK( pPath->serialize(file) );
        }

        //
        // - reset the pattern index for the next WT_XAML_Polytriangle serialization
        // - reset the drawable merging
        //
        pPat->set_pattern_number(0);
    }

    DWFCORE_FREE_OBJECT( pPath );

	WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    file.heuristics().set_allow_drawable_merging( allowMerging );

    return WT_Result::Success;
}
#else
WT_Result WT_XamlPolytriangle::serialize(WT_File & file) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

