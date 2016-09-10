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
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_XAML_Gouraud_Polyline::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File&>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Gouraud_Polyline::serialize( *rFile.w2dContentFile() );
    }
	
    WD_CHECK( rFile.dump_delayed_drawable() ); //GPL's don't merge with others
	
    // Make sure we have a legal polyline
    WD_Assert(count() > 1);

    // Attributes that polylines care about:
    //        color, line weight, line cap, line join,
    //        line pattern, fill mode, visibility, URL
    //        layer

    WT_Integer32    parts_to_sync = //  WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                        WT_Rendition::Delineate_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                    //  WT_Rendition::View_Bit            |
                                    //  WT_Rendition::Background_Bit      |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                    //  WT_Rendition::Plot_Optimized_Bit  |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                        WT_Rendition::Dash_Pattern_Bit    |
                                    //  WT_Rendition::Line_Style_Bit      |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Viewport_Bit        |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit;


	if (file.desired_rendition().fill().fill() || file.rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_False;
        parts_to_sync |= WT_Rendition::Fill_Bit;
    }

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Gouraud_Polyline*>(this)->transform(file.heuristics().transform());
    }

    //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    // use a temporary point_set to avoid modifying the original
    // coordinates (this must be a local to the serialization)
    //
    WT_XAML_Point_Set_Data oLocal(WT_Point_Set_Data::count(), WT_Point_Set_Data::points());
    WT_Point2D *pVtx = oLocal.points();
    for(int k = 0; k < oLocal.count(); k++)
    {
        rFile.flipPoint(pVtx[ k ]);
    }

    WT_Point2D *pPoints = oLocal.points();
    WT_RGBA32 *pColors = colors();
    int nCount = oLocal.count();

    WD_CHECK( rFile.serializeRenditionSyncEndElement() );

    //Serialize a W2X entry indicating that we have x points in the gouraud polyline definition...
    rFile.w2xSerializer()->startElement( XamlXML::kpzGouraud_Polyline_Element );
    //Tie the W2X entry to the canvas name
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString()); 
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzCount_Attribute, count() );
    rFile.w2xSerializer()->endElement();

    //Wrap the Gouraud paths in a canvas so that we can deal with it as a special case on materialize
    XamlCanvas oCanvas;
    WD_CHECK( oCanvas.serialize( file ) ); //increments the name index
	rFile.nameIndex()--;  //decrement name index so that the rendition Sync tracks properly

    //pull the segments out, one by one
    int i;
    for (i=1; i<nCount; i++)
    {
        //create one Paths for each segment, do the magic trick with the gradients
        //for each vertex, create a Path
        XamlPath *pPath = DWFCORE_ALLOC_OBJECT( XamlPath );
        if (pPath == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        //populate the path with the point data
        WT_XAML_Point_Set_Data oSegment( 2, &pPoints[i-1], false );
        bool bClose = file.desired_rendition().delineate().delineate() && nCount > 2;
        XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(oSegment, bClose) );
        pPath->addDrawable(pXamlPolylineSegment); //XamlPathGeometry owns the segment
		
        // Spin through the XAML Path needs, and with special attention to precedence,
        // examine the rendition attributes to gather the necessary information:
		WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition &>(rFile.rendition());
        //WD_CHECK( static_cast<WT_XAML_Color&>(              rXamlRendition.color()        ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Fill&>(               rXamlRendition.fill()         ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Fill_Pattern&>(       rXamlRendition.fill_pattern() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        WD_CHECK( static_cast<WT_XAML_Visibility&>(         rXamlRendition.visibility()   ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Line_Weight&>(        rXamlRendition.line_weight()  ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Line_Pattern&>(       rXamlRendition.line_pattern() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Dash_Pattern&>(       rXamlRendition.dash_pattern() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Line_Style&>(         rXamlRendition.line_style()   ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        WD_CHECK( static_cast<WT_XAML_URL&>(                rXamlRendition.url()          ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_Viewport&>(           rXamlRendition.viewport()     ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_User_Fill_Pattern&>(  rXamlRendition.user_fill_pattern() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
        //WD_CHECK( static_cast<WT_XAML_User_Hatch_Pattern&>( rXamlRendition.user_hatch_pattern() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );

        //Gouraud polylines are always hairline in width, and don't respect line/dash patterns.  We need to
        //make lineweight settings for XAML that mimic that.  And so, this class provides the necessary Path
        //attributes to conform with the Consistent Nominal Stroke Width Xaml spec.

        WD_CHECK( providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );

        //we need a linear gradient that runs from one vertex to the other, for the Stroke
        //create a linear gradient brush
        XamlBrush::LinearGradient *pBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::LinearGradient );
        if (pBrush == NULL)
            return WT_Result::Out_Of_Memory_Error;

        pBrush->startPoint() = pPoints[i-1];
        pBrush->endPoint() = pPoints[i];
        pBrush->startColor() = pColors[i-1];
        pBrush->endColor() = pColors[i];

        XamlDrawableAttributes::StrokeProvider oStrokeProvider( pBrush ); //owns the brush
        WD_CHECK( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath)->consumeStroke( &oStrokeProvider ) );
        WD_CHECK( pPath->serialize( file ) );

        DWFCORE_FREE_OBJECT( pPath );
    }	

	WD_CHECK( rFile.dump_delayed_drawable() ); //GPL's don't merge with others

    //close the canvas wrapper
    WD_CHECK( rFile.serializeCanvasEndElement() );

	rFile.nameIndex()++;  //increment name index so that we progress

    return WT_Result::Success;
}
#else
WT_Result WT_XAML_Gouraud_Polyline::serialize(WT_File & file) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


WT_Result 
WT_XAML_Gouraud_Polyline::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* p, WT_XAML_File& ) const
{
    WD_CHECK( p->consumeStrokeDashArray( const_cast<WT_XAML_Gouraud_Polyline*>(this) ) );
    WD_CHECK( p->consumeStrokeDashOffset( const_cast<WT_XAML_Gouraud_Polyline*>(this) ) );
    WD_CHECK( p->consumeStrokeThickness( const_cast<WT_XAML_Gouraud_Polyline*>(this) ) );
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Gouraud_Polyline::provideStrokeDashArray( XamlDrawableAttributes::StrokeDashArray*& rpStrokeDashArray )
{
    if (rpStrokeDashArray == NULL)
    {
        rpStrokeDashArray = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeDashArray() );
        if (rpStrokeDashArray == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    XamlDrawableAttributes::StrokeDashArray::tDashGapPair ofDashGapPair(1.0f, 0.0f);
    XamlDrawableAttributes::StrokeDashArray::tDashGapVector voDashGapVector;

    voDashGapVector.push_back( ofDashGapPair );
    rpStrokeDashArray->dashGap() = voDashGapVector;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Gouraud_Polyline::provideStrokeDashOffset( XamlDrawableAttributes::StrokeDashOffset*& rpStrokeDashOffset )
{
    if (rpStrokeDashOffset == NULL)
    {
        rpStrokeDashOffset = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeDashOffset() );
        if (rpStrokeDashOffset == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    rpStrokeDashOffset->value() = -2.0;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Gouraud_Polyline::provideStrokeThickness( XamlDrawableAttributes::StrokeThickness*& rpStrokeThickness )
{
    if (rpStrokeThickness == NULL)
    {
        rpStrokeThickness = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeThickness() );
        if (rpStrokeThickness == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    rpStrokeThickness->weight() = 1;
    return WT_Result::Success;
}

WT_Result
WT_XAML_Gouraud_Polyline::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

    const char** ppValue = rMap.find( XamlXML::kpzCount_Attribute );
    if ( ppValue!=NULL && ppValue!=NULL )
    {
        m_count = (WT_Integer32) atoi( *ppValue );
		m_colors = DWFCORE_ALLOC_MEMORY(WT_RGBA32, m_count);
        if (m_colors == NULL)
            return WT_Result::Out_Of_Memory_Error;
        m_points = DWFCORE_ALLOC_MEMORY(WT_Logical_Point, m_count);
        if (m_points == NULL)
            return WT_Result::Out_Of_Memory_Error;

        m_colors_allocated = m_allocated = m_count;
        m_points_materialized = 0;        
    }

	return WT_Result::Success;
}

WT_Result
WT_XAML_Gouraud_Polyline::materializeSegment( const XamlPolylineSegment& rSegment, const WT_RGBA32& rStartColor, const WT_RGBA32& rEndColor, WT_XAML_File &rFile )
{
    //For the first segment, we take both points
    WD_Assert( m_points_materialized < m_count )
    
    if ( m_points_materialized >= m_count )
        return WT_Result::Internal_Error;

    if (m_points_materialized == 0)
    {
        m_points[m_points_materialized].m_x = (WT_Integer32) floor( 0.5 + rSegment.pointSet().points()[0].m_x );
        m_points[m_points_materialized].m_y = (WT_Integer32) floor( 0.5 + rSegment.pointSet().points()[0].m_y );
        m_colors[m_points_materialized] = rStartColor;
        m_points_materialized++;
    }

    //For subsequent segments, we only take the end point
    m_points[m_points_materialized].m_x = (WT_Integer32) floor( 0.5 + rSegment.pointSet().points()[1].m_x );
    m_points[m_points_materialized].m_y = (WT_Integer32) floor( 0.5 + rSegment.pointSet().points()[1].m_y );
    m_colors[m_points_materialized] = rEndColor;
    m_points_materialized++;

    if ( m_points_materialized == m_count )
    {
        rFile.set_materialized( this );
    }
	return WT_Result::Success;
}
