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
WT_Result WT_XAML_Polymarker::serialize(WT_File & file) const
{
    WT_Result res = WT_Result::Success;

    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Polymarker::serialize( *rFile.w2dContentFile() );
    }

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Polymarker*>(this)->transform(file.heuristics().transform());
    }

    WT_XAML_Point_Set_Data oLocal( WT_Point_Set_Data::count(), WT_Point_Set_Data::points() );

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

    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath(false) );  //unnamed rendition path
    if ( pPath==NULL )
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    //Dump previous drawables
    WD_CHECK( rFile.dump_delayed_drawable() );

    //Sync the fill and lineweight (used in calculations below)
    WT_Boolean bFill = rFile.rendition().fill().fill();
    rFile.desired_rendition().fill().fill() = WD_False;
    WD_CHECK( rFile.desired_rendition().sync(file, WT_Rendition::Fill_Bit | WT_Rendition::Line_Weight_Bit) );

    //Turn off merging
    WT_Boolean allow = file.heuristics().allow_drawable_merging();
    file.heuristics().set_allow_drawable_merging( WD_False );

    //Serialize RenditionSync (top element) if not yet done
    WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    //Canvas wrapper
    XamlCanvas oCanvas;
    WD_CHECK( oCanvas.serialize( file ) ); //increments the name index
	rFile.nameIndex()--;  //decrement name index so that the rendition Sync tracks properly

    //Create the 'dots'
    WD_CHECK( _createSegments( rFile, pPath, oLocal ) );
    WD_CHECK( pPath->serialize(file) ); // does not call serializeRenditionSyncEndElement

    //End canvas wrapper
    WD_CHECK( rFile.serializeCanvasEndElement() );

    //Restore merging
    file.heuristics().set_allow_drawable_merging( allow );

	//serialize w2x stuff
    //Serialize a W2X entry
    rFile.w2xSerializer()->startElement( XamlXML::kpzPolymarker_Element );
    //Tie the W2X entry to the current name (stored above)
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString() );
    rFile.w2xSerializer()->endElement();

	rFile.nameIndex()++;  //increment name index so that we progress

    //Restore the fill
    rFile.desired_rendition().fill().set( bFill );

    DWFCORE_FREE_OBJECT( pPath );

    return res;
}

WT_Result
WT_XAML_Polymarker::_createSegments( WT_XAML_File& rFile, XamlPath* pPath, WT_XAML_Point_Set_Data& rPointSet ) const
{
    if (pPath == NULL)
    {
        return WT_Result::Internal_Error;
    }

    float fWidth = rFile.convertToPaperSpace( (float)rFile.rendition().line_weight().weight_value() );
    if (fWidth <=0)
        fWidth = 1;

    int i;
    for (i=0; i< rPointSet.count(); i++)
    {
        //Create two points, a smidgen on either side of the existing point, to simulate a "dot".
        WT_Point2D oPoints[2];
        oPoints[0].m_x = rPointSet.points()[i].m_x - (fWidth/2);
        oPoints[0].m_y = rPointSet.points()[i].m_y;
        oPoints[1].m_x = rPointSet.points()[i].m_x + (fWidth/2);
        oPoints[1].m_y = rPointSet.points()[i].m_y;
        WT_XAML_Point_Set_Data oPointset(2, oPoints, true);
        XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(oPointset,true) );

        if ( pXamlPolylineSegment==NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        pPath->addDrawable( pXamlPolylineSegment );
    }

    return WT_Result::Success;
}

