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


///////////////////////////////////////////////////////////////////////////
//#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_XAML_Polyline::serialize(WT_File & file) const
{
    WT_Result res = WT_Result::Success;

    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Polyline::serialize( *rFile.w2dContentFile() );
    }

    if ( file.rendition().fill().fill() || file.desired_rendition().fill().fill() )
    {
        file.desired_rendition().fill() = WD_False;
    }

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Polyline*>(this)->transform(file.heuristics().transform());
    }

    WT_XAML_Point_Set_Data oLocal( WT_Point_Set_Data::count(), WT_Point_Set_Data::points() );
    int nCount = oLocal.count();

    //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    // use a temporary point_set to avoid modifying the original
    // coordinates (this must be a local to the serialization)
    //
    WT_Point2D *pVtx = oLocal.points();
    int i;
    for(i = 0; i < oLocal.count(); i++)
    {
        rFile.flipPoint(pVtx[ i ]);
    }

    //
    // polylines with only one point need to be artifically transformed
    // into a segment with the same endpoints, otherwise it won't render
    //
    if(oLocal.count() == 1)
    {
        WT_Point2D pDegenerate[ 2 ];
        pDegenerate[ 0 ] = pDegenerate[ 1 ] = *pVtx;
        oLocal.set(2, pDegenerate, true);
    }

    bool bClose = file.desired_rendition().delineate().delineate() && nCount > 2;
    XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(oLocal, bClose) );

    if ( pXamlPolylineSegment==NULL )
    {
        return WT_Result::Out_Of_Memory_Error;
    }
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath ); //calls serializeRenditionSyncEndElement
    if ( pPath==NULL )
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    pPath->addDrawable(pXamlPolylineSegment); // XamlPathGeometry owns the segment

    res = pPath->serialize(file);

    DWFCORE_FREE_OBJECT( pPath );

    return res;
}
//#else
////WT_Result WT_XamlPolyline::serialize(WT_File & file) const
//{
//    return WT_Result::Success;
//}
//#endif  // DESIRED_CODE()

