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
WT_Result WT_XAML_Gouraud_Polytriangle::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Gouraud_Polytriangle::serialize( *rFile.w2dContentFile() );
    }
	
    WD_CHECK( rFile.dump_delayed_drawable() ); //GPT's don't merge with others 
	
    WD_Assert(m_count > 2);

    // FUTURE: we need to apply the write-time optimizations that normal
    // polytriangles apply.  In doing those optimizations, we want to
    // handle degenerate triangles, which are to be represented by Gouraud
    // Polylines (not yet in the spec, and only partially implemented in
    // the toolkit).
    //
    // FUTURE: While we still want to reject <3 point triangles coming in, if the
    // list of triangles are so acute (within some fuzz tolerance) that
    // they are or appear as a line, lets store them as such to save space
    // (we can optimize away ~ 50% of the vertices this way)

    // Make sure we have a legal triangle
    if (m_count < 3)
    {
        return WT_Result::Success; // Drawing a 2 pt. triangle is easy: do nothing!
    }

    WT_Integer32    parts_to_sync = //  WT_Rendition::Color_Bit           |
                                    //  WT_Rendition::Color_Map_Bit       |
                                    //  WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                    //  WT_Rendition::View_Bit            |
                                    //  WT_Rendition::Background_Bit      |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                    //  WT_Rendition::Plot_Optimized_Bit  |
                                        WT_Rendition::Visibility_Bit      |
                                    //  WT_Rendition::Line_Weight_Bit     |
                                    //  WT_Rendition::Pen_Pattern_Bit     |
                                    //  WT_Rendition::Line_Pattern_Bit    |
                                    //  WT_Rendition::Line_Caps_Bit       |
                                    //  WT_Rendition::Line_Join_Bit       |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
										WT_Rendition::Viewport_Bit		  |
                                        WT_Rendition::URL_Bit             |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::Delineate_Bit       |
                                        WT_Rendition::Contrast_Color_Bit  |
                                        WT_Rendition::User_Fill_Pattern_Bit |
                                        WT_Rendition::User_Hatch_Pattern_Bit;

    if (!file.desired_rendition().fill().fill() || !file.rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_True;
        parts_to_sync |= WT_Rendition::Fill_Bit;
    }

    WD_CHECK (file.desired_rendition().sync(file, parts_to_sync));

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Gouraud_Polytriangle*>(this)->transform(file.heuristics().transform());
    }

    //Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

    //Serialize a W2X entry indicating that we have x points in the polytriangle definition...
    rFile.w2xSerializer()->startElement( XamlXML::kpzGouraud_Polytriangle_Element );
    //Tie the W2X entry to the canvas name
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString()); 
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzCount_Attribute, count() );
    rFile.w2xSerializer()->endElement();

    //Wrap the Gouraud paths in a canvas so that we can deal with it as a special case on materialize
    XamlCanvas oCanvas;
    WD_CHECK( oCanvas.serialize( file ) ); //increments the name index
	rFile.nameIndex()--;  //decrement name index so that the rendition Sync tracks properly

    //pull the triangles out, one by one
    int i = 2;
    for (; i<count(); i++)
    {
        //create three Paths for each triangle, do the magic trick with the gradients
        //for each vertex, create a Path
        int j = 0;
        for(; j<3; j++)
        {
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

            XamlPath *pPath = DWFCORE_ALLOC_OBJECT( XamlPath );
            if (pPath == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }

            //populate the path with the point data
            WT_XAML_Point_Set_Data oTriangle( 3, &oLocal.points()[i-2], false );
            XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(oTriangle) );
            pPath->addDrawable(pXamlPolylineSegment); //XamlPathGeomoety owns the segment
			
            //we need a linear gradient that runs from the target vertex to a point on a line 
            //that contains the two other vertices

            //Consider the triangle ABC, and line segments AP and PC such that P lies on a
            //line containing B and C, and AP is perpendicular to PC.  We have A, B, and C,
            //but need to find point P.  Defining a gradient from A to P will give us a nice
            //smooth transition to B and C.
            //   P
            //         B  __
            //       /      _ C
            //     /    _ -
            //   /  _ -
            // A  - 

            //Setup the points we're using
            int iA = (j==0) ? i-2 : ((j==1) ? i-1 : i);
            int iB = (j==1) ? i-2 : ((j==2) ? i-1 : i);
            int iC = (j==2) ? i-2 : ((j==0) ? i-1 : i);

            const WT_Point2D &A = oLocal.points()[iA];
            const WT_Point2D &B = oLocal.points()[iB];
            const WT_Point2D &C = oLocal.points()[iC];

            // We need the slope of BC, whic is (Cy-By) / (Cx-Bx)
            // Check for vertical segment
            WT_Point2D P;

            if (C.m_x != B.m_x)
            {
                //the perpendicular to BC is AP, and its slope is 1/slopeBC
                if (C.m_y != B.m_y ) //check horizontal
                {
                    double fSlopeBC = (C.m_y-B.m_y) / (C.m_x-B.m_x);
                    double fSlopeAP = -1/fSlopeBC;
                    //solve the BC line equation to get the y-intercept of BC
                    double fBCb = B.m_y - ( fSlopeBC * B.m_x );
                    //solve the AP line equation to get the y-intercept of BC
                    double fAPb = A.m_y - ( fSlopeAP * A.m_x );

                    //now, setting the two equations ==, we solve for P.m_x
                    P.m_x = (fAPb - fBCb) / (fSlopeBC - fSlopeAP);
                    P.m_y = ((fSlopeAP * fBCb) - (fSlopeBC * fAPb)) / (fSlopeAP - fSlopeBC);
                }
                else //degenerate B.m_y == C.m_y;
                {
                    P.m_y = C.m_y;//(A.m_y > C.m_y) ? min(C.m_y, B.m_y) : max(C.m_y, B.m_y);
                    P.m_x = A.m_x;
                }
            }
            else //degenerate, B.m_x == C.m_x
            {
                P.m_x = C.m_x;//(A.m_x > C.m_x) ? min(C.m_x, B.m_x) : max(C.m_x, B.m_x);
                P.m_y = A.m_y;
            }

            // Spin through the XAML Path needs, and with special attention to precedence,
            // examine the rendition attributes to gather the necessary information:
            WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition &>(rFile.rendition());
            WD_CHECK( static_cast<WT_XAML_Visibility&>(         rXamlRendition.visibility()   ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
            WD_CHECK( static_cast<WT_XAML_URL&>(                rXamlRendition.url()          ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );

            //create a linear gradient along AP for the subpath, for the Fill
            XamlBrush::LinearGradient *pBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::LinearGradient );
            if (pBrush == NULL)
                return WT_Result::Out_Of_Memory_Error;

            pBrush->startPoint() = A;
            pBrush->endPoint() = P;
            pBrush->startColor() = colors()[iA];
            pBrush->endColor() = WT_RGBA32(colors()[iA].m_rgb.r, colors()[iA].m_rgb.g, colors()[iA].m_rgb.b, 0);

            XamlDrawableAttributes::FillProvider oFillProvider( pBrush ); //owns the brush
            WD_CHECK( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath)->consumeFill( &oFillProvider ) );
            XamlPath* pDelayedPath = rFile.delayed_path();
            if( pDelayedPath && pDelayedPath->fill() == pPath->fill() )
                 WD_CHECK( rFile.dump_delayed_drawable() ); //GPT's don't merge with others
            WD_CHECK( pPath->serialize( file ) );

            DWFCORE_FREE_OBJECT( pPath );
        }
    }

    WD_CHECK( rFile.dump_delayed_drawable() ); //GPT's don't merge with others

    //close the canvas wrapper
    WD_CHECK( rFile.serializeCanvasEndElement() );

	rFile.nameIndex()++;  //increment name index so that we progress

	return WT_Result::Success;
}
#else
WT_Result WT_XAML_Gouraud_Polytriangle::serialize(WT_File & file) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

WT_Result
WT_XAML_Gouraud_Polytriangle::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
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
WT_XAML_Gouraud_Polytriangle::materializePoint( const WT_Point2D& pt, const WT_RGBA32& color, WT_XAML_File &rFile )
{
    if ( m_points_materialized < 3 ||
        _eParseState == ThirdPoint )
    {
        m_points[ m_points_materialized ].m_x = (WT_Integer32) floor( 0.5 + pt.m_x );
        m_points[ m_points_materialized ].m_y = (WT_Integer32) floor( 0.5 + pt.m_y );
        m_colors[ m_points_materialized ] = color;
        m_points_materialized++;
    }

    //Some people love parades.  I love state machines.
    switch (_eParseState)
    {
    case FirstPoint:
        _eParseState = SecondPoint;
        break;
    case SecondPoint:
        _eParseState = ThirdPoint;
        break;
    case ThirdPoint:
        _eParseState = FirstPoint;
    }

    if ( m_points_materialized == m_count )
    {
        rFile.set_materialized( this );
    }

    return WT_Result::Success;
}
